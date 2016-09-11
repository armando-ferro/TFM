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


class mux : public cSimpleModule {
private:
    /*variables*/
    int n;
    int state_machine;
    cXMLElement * xml;
    short * input_line ;    //transformación entre entrada y linea
    bool b_config;
    /*gestion entre el multiplexor y el demultiplexor*/
    cQueue *txQueue;
    cChannel * txChannel;
    cMessage *sent;
    int header_tam;
public:
    mux();
    virtual ~mux();
protected:
    virtual void handleMessage(cMessage *msg);
    virtual void initialize();
    virtual bool config(cXMLElement *xml);
    virtual void send_out(int line,cMessage *msg);

};

Define_Module(mux);


mux::mux() {
    b_config = true;
    state_machine = idle;
    txQueue = NULL;
    txChannel = NULL;
    sent = NULL;
    header_tam  = 0;
}

mux::~mux() {
    cancelAndDelete(sent);
    txQueue->~cQueue();
}

void mux::initialize(){
    n = gateSize("in");
    input_line = new short[n];


    /*inicializar a inexistente*/
    for(int i=0;i<n;i++){
        input_line[i]=-1;
    }

    if(par("config").containsValue()){
        xml = par("config").xmlValue();
        b_config = config(xml);
    }

    if(par("Header_Tam").containsValue()){
        header_tam = par("Header_Tam");
    }

    /*Cola de mensajes a enviar*/
    txQueue = new cQueue("txQueue");
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

void mux::handleMessage(cMessage *msg){
    /*Los paquetes que entran salen por out no es bidireccional*/
    if(not(b_config)){
        delete(msg);
        bubble("Configuración errónea");
        return;
    }
    if(msg == sent){
        /*ya se ha mandado*/
        if(txQueue->length()>0){
            /*Hay paquetes que transmitir*/
            Mux *qmx = (Mux *)txQueue->pop();
            send(qmx,"out");
            state_machine = sending;
            /*programar el auto mensaje para cambair de estado*/
            simtime_t txFinishTime = txChannel->getTransmissionFinishTime();
            scheduleAt(txFinishTime,sent);
        }else{
            state_machine = idle;
        }
    }else{
        int g_in,line;
        /*comprobar error*/
        cPacket *pk = check_and_cast<cPacket *>(msg);
        if(pk->hasBitError()){
            bubble("Paquete con error");
            delete(pk);
            return;
        }
        /*reconcoer por cual llega*/
        for(g_in=0;g_in<n;g_in++){
            if(msg->arrivedOn("in",g_in)){
                line = input_line[g_in];
                if(line<0){
                    /*no ha sido configurada*/
                    bubble("Entrada no configurada");
                    delete(msg);
                    return;
                }
                /*ya se tiene la línea y la entrada*/
                send_out(line,msg);
            }
        }
    }
}


bool mux::config(cXMLElement *xml){
    cXMLElement *tmp;
    short in,line;

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
            return false;
        }

        /*comprobar si ya ha sido asignada*/
        if(input_line[in]>0){
            EV << "La entrada "<<in<<" ya ha sido configurada:"<<input_line[in];
            return false;
        }

        line = atoi(tmp->getAttribute("line"));
        if(line < 0){
            EV << "La línea debe ser un número positivo";
            return false;
        }
        /*introduclir línea*/
        input_line[in] = line;
    }while((tmp=tmp->getNextSibling())!=NULL);

    return true;

}

void mux::send_out(int line,cMessage *msg){
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
        txQueue->insert(mx);
    }
    delete(il);
}

