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
#include "Packet_m.h"
#include "Inter_layer_m.h"


class injector : public cSimpleModule {
private:
    cMessage *nextPkt;
    int seq;
    int dest;
    /*Valores introducidos por el usuario que no se modificarán*/
    SimTime time_mean;
    std::string time_distribution;
    std::string pkt_distribution;
    int pkt_mean;
public:
    injector();
    virtual ~injector();
protected:
    virtual void handleMessage(cMessage *msg);
    virtual Packet *generateNewMessage();
    virtual void initialize();

};

Define_Module(injector);


injector::injector() {
    nextPkt = NULL;
    time_distribution = "cte";
    pkt_distribution = "cte";
    time_mean = 10;
    pkt_mean = 2;
    seq = 0;
    dest = 0;

}

injector::~injector() {
    cancelAndDelete(nextPkt);
}

void injector::initialize(){
    time_mean = par("Mean_Time");
    pkt_mean = par("Mean_Size");
    time_distribution = par("Time_Distribution").stdstringValue();
    pkt_distribution = par("Size_Distribution").stdstringValue();
    dest = par("Dst_Addr");
    seq = 0;

    nextPkt = new cMessage("new");
    if(time_distribution.compare("exp")==0){
        /*exponencial*/
        scheduleAt(simTime()+exponential(time_mean),nextPkt);
    }else{
        /*constante*/
        scheduleAt(simTime()+(time_mean),nextPkt);
    }

}

void injector::handleMessage(cMessage *msg){
    Packet *pkt = generateNewMessage();
    /*Generar el inter_layer y mandarlo*/
    inter_layer * il = new inter_layer("injector_il",0);
    il->setDestino(dest);
    il->encapsulate(pkt);
    send(il,"down_out");
    //scheduleAt(simTime()+exponential(time_mean),nextPkt);
    if(time_distribution.compare("exp")==0){
        /*exponencial*/
        scheduleAt(simTime()+exponential(time_mean),nextPkt);
    }else{
        /*constante*/
        scheduleAt(simTime()+(time_mean),nextPkt);
    }
}

Packet *injector::generateNewMessage(){
    /*Generar un paquete con distinto nombre cada vez*/
    char msgname[20];
    sprintf(msgname,"tic-%d",++seq);
    Packet *msg = new Packet(msgname,0);
    if(pkt_distribution.compare("exp")==0){
            /*exponencial*/
        int tam = exponential(pkt_mean);
            if(tam == 0){
                msg->setBitLength(1);
            }else{
                msg->setBitLength(tam);
            }
        }else{
            /*constante*/
            msg->setBitLength(pkt_mean);
        }

    msg->setType(a_msg_t);
    return msg;
}
