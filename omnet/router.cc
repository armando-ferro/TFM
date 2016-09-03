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
#include "Network_m.h"
#include "Transport_m.h"
#include "Inter_layer_m.h"
#include "Types.h"
#include <cxmlelement.h>

const short max = 10;

struct output{
    short gate;
    double prob;
};

struct route{
    int start;
    int stop;
    int n_gates;
    output * gates;
};


class router : public cSimpleModule {
private:
    /*variables*/
    bool b_config;
    bool down_in[max];
    bool down_out[max];
    bool up_in[max];
    bool up_out[max];
    int down_inc,down_outc,up_inc,up_outc;
    int origen;
    int hopLimit;
    int headerTam;
    int errorPkt,rcvBit,rcvPkt,forwardedBit,forwardedPkt;
    route *routes;
    int n_routes;
    cXMLElement * xml;
    simsignal_t s_errorPkt;
    simsignal_t s_rcvBit;
    simsignal_t s_rcvPkt;
    simsignal_t s_forwardedBit;
    simsignal_t s_forwardedPkt;
public:
    router();
    virtual ~router();
protected:
    virtual void handleMessage(cMessage *msg);
    virtual void initialize();
    virtual int config(cXMLElement *xml);
    virtual void send_route(int dest,inter_layer *il);
    virtual void send_up(Network *nw);

};

Define_Module(router);


router::router() {
    down_inc = 0;
    down_outc = 0;
    up_inc = 0;
    up_outc = 0;

    errorPkt = 0;
    rcvBit = 0;
    rcvPkt = 0;
    forwardedBit = 0;
    forwardedPkt = 0;

    s_errorPkt = 0;
    s_rcvBit = 0;
    s_rcvPkt = 0;
    s_forwardedBit = 0;
    s_forwardedPkt = 0;

    headerTam = 0;
    origen = 0;
    hopLimit = 10;

    routes = NULL;
    n_routes = 0;
    b_config = true;
}

router::~router() {

    delete [] routes;
}

void router::initialize(){
    /*Comprobar que puertas están conecetadas*/
    b_config = true;
    int i=0;
    for(i=0;i<max;i++){
        if(gate("up_in",i)->isConnected()){
            up_in[i]=true;
            up_inc++;
        }else{
            up_in[i]=false;
        }

        if(gate("up_out",i)->isConnected()){
            up_out[i]=true;
            up_outc++;
        }else{
            up_out[i]=false;
        }

        if(gate("down_in",i)->isConnected()){
            down_in[i]=true;
            down_inc++;
        }else{
            down_in[i]=false;
        }

        if(gate("down_out",i)->isConnected()){
            down_out[i]=true;
            down_outc++;
        }else{
            down_out[i]=false;
        }
    }

    if(par("Addr").containsValue()){
        origen = par("Addr");
    }

    if(par("Hop_Limit").containsValue()){
        hopLimit = par("Hop_Limit");
    }

    if(par("Header_Tam").containsValue()){
            headerTam = par("Header_Tam");
        }

    if(par("Config_File").containsValue()){
        xml = par("Config_File").xmlValue();
        if((n_routes=config(xml))<0){
            b_config = false;
        }

    }

    s_rcvBit = registerSignal("rcvBit");
    s_rcvPkt = registerSignal("rcvPkt");
    s_errorPkt = registerSignal("errorPkt");
    s_forwardedBit = registerSignal("forwardedBit");
    s_forwardedPkt = registerSignal("forwardedPkt");

    WATCH(down_inc);
    WATCH(down_outc);
    WATCH(up_inc);
    WATCH(up_outc);
    WATCH(n_routes);
    WATCH(b_config);
    WATCH(s_rcvPkt);
}

void router::handleMessage(cMessage *msg){
    if(not(b_config)){
        return;
    }
    inter_layer *il = check_and_cast<inter_layer *>(msg);
    if(msg->arrivedOn("up_in")){
        /*llega de la capa superior*/
        /*obtener datos y paquetes*/
        int dest = il->getDestino();
        int protocol = il->getProtocol();

        switch(protocol){
        case p_application:
            /*directamente desde la aplicación*/
            EV << " Application";
            if(dest<min_trans||dest>max_trans){
                bubble("Destino no válido");
                delete(il);
                return;
            }else{
                dest = dest%100;
            }
            break;
        case p_transport:
            /*Capa de transporte*/
            EV << " Transport";
            if(dest<min_net||dest>max_net){
                bubble("Destino no válido");
                delete(il);
                return;
            }
            break;
        default:
            EV << " No válido";
            bubble("Protocolo no válido");
            delete(il);
            return;
        }
        cPacket *pk = il->decapsulate();
        /*crear cabecera*/
        char msgname[20];
        sprintf(msgname,"Network-%d-%d",origen,dest);
        Network *nw = new Network(msgname,0);
        nw->setSrcAddr(origen);
        nw->setDstAddr(dest);
        nw->setHopCount(0);
        nw->setHopLimit(hopLimit);
        nw->setProtocol(protocol);
        nw->setBitLength(headerTam);
        nw->encapsulate(pk);

        /*crear inter_layer*/
        inter_layer *dil = new inter_layer("Router_il",0);
        dil->setOrigen(origen);
        dil->setDestino(dest);
        dil->setProtocol(p_network);
        dil->encapsulate(nw);

        /*rutado*/
        send_route(dest,dil);

    }else{
        /*llega de la capa inferior*/
        /*Desencapsular*/
        Network *nw = (Network *)il->decapsulate();
        if(nw->hasBitError()){
            bubble("Paquete con error");
            /*no se pueden comprobar la dirección*/
            delete(nw);
            return;
        }
        int dest = nw->getDstAddr();
        if(dest==origen){
            /*soy el destinatario*/
            send_up(nw);
        }else{
            /*necesario rutar*/
            EV << " Rutar";
            /*comprobar hop limit*/
            if(nw->getHopCount()>=nw->getHopLimit()){
                /*eliminar el paquete*/
                bubble("Hop limit");
                delete(nw);
            }else{
                /*sumar un salto*/
                nw->setHopCount(nw->getHopCount()+1);
                /*generar el inter layer*/
                inter_layer *dil = new inter_layer("Router_il",0);
                dil->setOrigen(origen);
                dil->setDestino(dest);
                dil->setProtocol(p_network);
                dil->encapsulate(nw);
                send_route(dest,dil);
            }
        }
    }
    delete(il);

}

int router::config(cXMLElement *xml){
    cXMLElement *tmp;
    cXMLElement *gate_tmp;
    int n = 1;
    int i = 0;
    int ng = 1;
    int ig = 0;

    if(strcmp(xml->getTagName(),"routes")!=0){
        /*no es el tipo esperado*/
        return -1;
    }

    if(not(xml->hasChildren())){
        /*no contine rutas*/
        return -1;
    }


    /*comprobar número de rutas*/
    tmp = xml->getFirstChild();
    if(not(tmp->hasChildren())){
        EV << "Una ruta no tiene puertas";
        return -1;
    }

    while((tmp=tmp->getNextSibling())!=NULL){
        n++;
        if(not(tmp->hasChildren())){
            /*una ruta no tiene puertas*/
            EV << "Una ruta no tiene puertas";
            return -1;
        }
    }

    routes = new route[n];

    tmp = xml->getFirstChild();

    for(i=0;i<n;i++,tmp = tmp->getNextSibling()){

        gate_tmp = tmp->getFirstChild();
        ng = 1;
       /*comprobar número de puertas*/
       while((gate_tmp=gate_tmp->getNextSibling())!=NULL){
           ng++;
       }

       /*rellenar datos*/
       routes[i].start = atoi(tmp->getAttribute("start"));
       routes[i].stop = atoi(tmp->getAttribute("stop"));

       /*Comprobar direcciones*/
       if(routes[i].start<min_net||routes[i].start>max_net||routes[i].stop<min_net||routes[i].stop>max_net){
           EV << "Rango de direcciones erróneo";
           return -1;
       }
       if(routes[i].start>routes[i].stop){
           EV << "Start mayor que Stop";
           return -1;
       }

       routes[i].gates = new output[ng];
       routes[i].n_gates = ng;

       /*rellenar puertas*/
       double p;
       gate_tmp = tmp->getFirstChild();
       for(ig=0,p=0;ig<ng;ig++,gate_tmp = gate_tmp->getNextSibling()){
           routes[i].gates[ig].gate = atoi(gate_tmp->getAttribute("id"));
           if(routes[i].gates[ig].gate >= max){
               EV << "Solo hay "<< max <<" salidas";
               return -1;
           }
           if(not(down_out[routes[i].gates[ig].gate])){
               EV << "Puerta no conectada " << routes[i].gates[ig].gate;
               return -1;
           }

           if(gate_tmp->getAttribute("prob")!=NULL){
               routes[i].gates[ig].prob = atof(gate_tmp->getAttribute("prob"));
               if(routes[i].gates[ig].prob>1||routes[i].gates[ig].prob<=0){
                   EV << "Probabilidad errónea";
                   return -1;
               }
               p += routes[i].gates[ig].prob;
           }

       }
       /*la suma de las probabilidades debe ser 1 si hay mas de una puerta*/
       if(routes[i].n_gates>1&&p!=1){
           EV << "Error en la suma de probabilidades: " << p<< " n_gates:"<<routes[i].n_gates;
           return -1;
       }
    }

    return n;
}

void router::send_route(int dest,inter_layer *il){
    /*comprobar con que ruta encaja*/
    EV << " Rutando:"<<dest;
    /*se extrae el tamaño porque el il no tiene cabecera*/
    int tam = il->getBitLength();
    forwardedBit += tam;
    emit(s_forwardedBit,forwardedBit);
    emit(s_forwardedPkt,++forwardedPkt);
    double irand,crand;
    for(int i=0;i<n_routes;i++){
        if(dest>=routes[i].start&&dest<=routes[i].stop){
            /*esta ruta cumple*/
            /*comprobar salidas*/
            if(routes[i].n_gates == 1){
                /*solo una, por ella*/
                send(il,"down_out",routes[i].gates[0].gate);
                return;
            }else{
                /*cargar probabilidades*/
                irand = uniform(0, 1);
                crand = 0;
                for(int j=0;j<routes[i].n_gates;j++){
                    if(irand<(crand+=routes[i].gates[j].prob)){
                        /*cumple*/
                        send(il,"down_out",routes[i].gates[j].gate);
                        return;
                    }
                }
            }
        }
    }

}

void router::send_up(Network *nw){
    /*según el protocolo*/
    EV << "SendUP";
    inter_layer *il = new inter_layer("Network_up",0);
    switch(nw->getProtocol()){
        case p_application:{
            /*Primera puerta conectada*/
            EV << " Application";
            cPacket * app = (cPacket *) nw->decapsulate();
            /*generar inter layer*/
            il->setOrigen(nw->getSrcAddr());
            il->setDestino(origen);
            il->encapsulate(app);
            for(int i=0;i<max;i++){
                if(not(up_out[i])){
                    send(il,"up_out",i);
                }
            }
            int tam = app->getBitLength();
            rcvBit+=tam;
            emit(s_rcvBit,rcvBit);
            emit(s_rcvPkt,++rcvPkt);
            break;
        }
        case p_transport:{
            /*por el puerto indicado*/
            EV << " Transport";
            Transport * tp = (Transport *) nw->decapsulate();
            int gate = tp->getDstAddr();
            if(gate<min_trans||gate>max_trans){
                bubble("puerto erróneo");
                delete(tp);
                return;
            }else{
                gate = gate/100;
                /*generar inter layer*/
                il->setOrigen(nw->getSrcAddr());
                il->setDestino(origen);
                il->encapsulate(tp);
                send(il,"up_out",gate);
            }
            int tam = tp->getBitLength();
            rcvBit+=tam;
            emit(s_rcvBit,rcvBit);
            emit(s_rcvPkt,++rcvPkt);
            break;
        }
        default:
            bubble("Protocolo no válido");
            break;
    }
    delete(nw);
}



