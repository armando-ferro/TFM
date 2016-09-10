//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include "Mux_m.h"
#include "Inter_layer_m.h"
#include <cxmlelement.h>

const short sending = 1;
const short idle = 0;

struct input{
    short line;
    int priority;
};


class mux_priority : public cSimpleModule {
private:
    /*variables*/
    int n,prioritys;
    int state_machine;
    cXMLElement * xml;
    input *input_line ;    //transformación entre entrada y linea
    bool b_config;
    /*gestion entre el multiplexor y el demultiplexor*/
    cArray *txQueue;
    cChannel * txChannel;
    cMessage *sent;
    short *line_queue;
    int header_tam;
public:
    mux_priority();
    virtual ~mux_priority();
protected:
    virtual void handleMessage(cMessage *msg);
    virtual void initialize();
    virtual int config(cXMLElement *xml);
    virtual void send_out(int line,short priority,cMessage *msg);

};

Define_Module(mux_priority);


mux_priority::mux_priority() {
    b_config = true;
    state_machine = idle;
    prioritys = -1;
    n = 0;
    txQueue = NULL;
    txChannel = NULL;
    sent = NULL;
    line_queue = NULL;
}

mux_priority::~mux_priority() {
    cancelAndDelete(sent);
    txQueue->~cArray();
}

void mux_priority::initialize(){
    n = gateSize("in");
    input_line = new input[n];

    /*parametros*/
    if(par("Header_Tam").containsValue()){
        header_tam = par("Header_Tam");
    }

    /*inicializar a inexistente*/
    for(int i=0;i<n;i++){
        input_line[i].line=-1;
    }

    if(par("config").containsValue()){
        xml = par("config").xmlValue();
        prioritys = config(xml);
        if(prioritys == -1){
            b_config = false;
        }else{
            b_config = true;
        }
    }

    /*mensaje que indica cuando se ha terminado de enviar un packete*/
    sent = new cMessage("sent");
    /*Canal de salida*/
    if(gate("out")->isConnected()){
        txChannel = gate("out")->getTransmissionChannel();
    }else{
        b_config = false;
    }

    WATCH(n);
    WATCH(b_config);
}

void mux_priority::handleMessage(cMessage *msg){
    /*Los paquetes que entran salen por out no es bidireccional*/
    if(not(b_config)){
        delete(msg);
        bubble("Configuración errónea");
        return;
    }

    if(msg == sent){
        /*ya se ha mandado comprobar todas las colas*/
        for(int i=0;i<prioritys;i++){
            cQueue *tx;
            tx = (cQueue *)txQueue->get(i);
            if(tx->length()>0){
                /*Hay paquetes que transmitir*/
               Mux *qmx = (Mux *)tx->pop();
               send(qmx,"out");
               state_machine = sending;
               /*programar el auto mensaje para cambair de estado*/
               simtime_t txFinishTime = txChannel->getTransmissionFinishTime();
               scheduleAt(txFinishTime,sent);
               return;
            }
        }
        /*Si no hay reterun*/
        state_machine = idle;
    }else{
        int g_in,line;
        /*comprobar si error*/
        cPacket *pk = check_and_cast<cPacket *>(msg);
        if(pk->hasBitError()){
            bubble("Paquete con error");
            delete(pk);
            return;
        }
        /*reconcoer por cual llega*/
        for(g_in=0;g_in<n;g_in++){
            if(msg->arrivedOn("in",g_in)){
                line = input_line[g_in].line;
                if(line<0){
                    /*no ha sido configurada*/
                    bubble("Entrada no configurada");
                    delete(msg);
                    return;
                }
                /*ya se tiene la línea y la entrada*/
                send_out(line,input_line[g_in].priority,msg);
            }
        }
    }
}


int mux_priority::config(cXMLElement *xml){
    cXMLElement *tmp;
    short in,line;
    int pmax = 0;
    int n_priority = 0;
    int priority,i;

    if(strcmp(xml->getTagName(),"inputs")!=0){
          /*no es el tipo esperado*/
          EV << "XML inesperado";
          return false;
    }

    if(not(xml->hasChildren())){
        /*no contine rutas*/
        EV << "No hay lineas";
        return false;
    }

    tmp = xml->getFirstChild();
    do{
        in=atoi(tmp->getAttribute("gate"));
        /*comprobar si la puerta existe y si la línea es corrrecta*/
        if(in<0||in>= n){
            EV << "Solo hay "<< n<< "entradas";
            return -1;
        }

        /*comprobar si ya ha sido asignada*/
        if(input_line[in].line>0){
            EV << "La entrada ya ha sido configurada";
            return -1;
        }

        line = atoi(tmp->getAttribute("line"));
        if(line < 0){
            EV << "La línea debe ser un número positivo";
            return -1;
        }
        /*introduclir línea*/
        input_line[in].line = line;

        /*comprobar si hay prioridad*/
        if(tmp->getAttribute("priority")!=NULL){
            priority = atoi(tmp->getAttribute("priority"));
            if(priority<0){
                EV << "La prioridad debe ser mayor que 0";
                return -1;
            }
        }else{
            /*si no hay prioridad se asigna el menor valor*/
            priority = 0;
        }
        input_line[in].priority = priority;
        /*seleccionar el máximo*/
        if(input_line[in].priority>pmax){
            pmax = input_line[in].priority;
        }
    }while((tmp=tmp->getNextSibling())!=NULL);

    /*inicialziar la transformación entre prioridad y cola*/
    line_queue = new short[pmax+1];
    for(i=0;i<=pmax;i++){
        line_queue[i]=-1;
    }

    /*detectar cuantas prioridades distintas hay*/
    for(i=0;i<n;i++){
        if(input_line[i].line>=0){
            /*La entrada ha sido configurada*/
            priority = input_line[i].priority;
            if(line_queue[priority]==-1){
                /*No ha sido asignado*/
                n_priority++;
                line_queue[priority]=-2;
            }
        }
    }

    /*asignar las colas a las prioridades y crear las colas*/
    txQueue = new cArray("QueueArray");
    cQueue *tx;
    char msgname[20];
    int j=0;

    for(i=(pmax);i>=0;i--){
        if(line_queue[i]==-2){
            /*la prioridad existe*/
            sprintf(msgname,"txQueue-%d",i);
            tx = new cQueue(msgname);
            j=txQueue->add(tx);
            line_queue[i]=j;
        }
    }
    return n_priority;

}

void mux_priority::send_out(int line,short priority,cMessage *msg){
    /*descampsular*/
    inter_layer *il = check_and_cast<inter_layer *>(msg);
    cPacket *pk = il->decapsulate();
    /*empaqueter y mandar*/
    char msgname[20];
    sprintf(msgname,"Mux-%d",line);
    Mux *mx = new Mux(msgname,0);
    mx->setLine(line);
    mx->setBitLength(header_tam);
    mx->encapsulate(pk);
    /*comprobar estado*/
    if(state_machine == idle){
        send(mx,"out");
        state_machine = sending;
        /*programar el auto mensaje para cambair de estado*/
        simtime_t txFinishTime = txChannel->getTransmissionFinishTime();
        scheduleAt(txFinishTime,sent);
    }else{
        /*seleccionar la cola con prioridad*/
        short queue = line_queue[priority];
        cQueue * tx = (cQueue *) txQueue->get(queue);
        tx->insert(mx);

    }
    delete(il);
}
