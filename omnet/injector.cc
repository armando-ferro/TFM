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
#include "Types.h"
#include "Aplication_m.h"
#include "Inter_layer_m.h"


class injector : public cSimpleModule {
private:
    cMessage *nextPkt;
    int seq;
    int dest;
    double delay;
    /*Valores introducidos por el usuario que no se modificarán*/
    int n_pkt;
    /*señales*/
    simsignal_t s_pktTam;
public:
    injector();
    virtual ~injector();
protected:
    virtual void handleMessage(cMessage *msg);
    virtual Aplication *generateNewMessage();
    virtual void initialize();

};

Define_Module(injector);


injector::injector() {
    nextPkt = NULL;
    seq = 0;
    dest = 0;
    s_pktTam = 0;
    n_pkt = -1;

}

injector::~injector() {
    cancelAndDelete(nextPkt);
}

void injector::initialize(){

    if(par("Dst_Addr").containsValue()){
        dest = par("Dst_Addr");
    }

    if(par("n_Pkt").containsValue()){
        n_pkt = par("n_Pkt");
    }

    seq = 0;

    s_pktTam = registerSignal("pktTam");

    nextPkt = new cMessage("new");
    delay = par("delayTime");
    scheduleAt(simTime()+delay, nextPkt);

}

void injector::handleMessage(cMessage *msg){
    if(seq<n_pkt||n_pkt<0){
        Aplication *pkt = generateNewMessage();
        /*Generar el inter_layer y mandarlo*/
        inter_layer * il = new inter_layer("injector_il",0);
        il->setDestino(dest);
        il->setProtocol(p_application);
        il->encapsulate(pkt);
        send(il,"down_out");
        delay = par("delayTime");
        scheduleAt(simTime()+delay, nextPkt);
    }
}

Aplication *injector::generateNewMessage(){
    /*Generar un paquete con distinto nombre cada vez*/
    char msgname[20];
    sprintf(msgname,"tic-%d",++seq);
    Aplication *msg = new Aplication(msgname,0);

    int tam = par("pktSize");
    if(tam<=0){
        tam = 1;
    }
    emit(s_pktTam,tam);
    msg->setBitLength(tam);

    double it = delay;

    msg->setTam(tam);
    msg->setInterTime(it);
    return msg;
}
