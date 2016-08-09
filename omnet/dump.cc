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


class dump : public cSimpleModule {
private:
    int count;
    double tam;
    /*señal para extraer datos*/
    simsignal_t s_rcvBit;
    simsignal_t s_rcvPkt;
    simsignal_t s_bitThroughput;
    simsignal_t s_pktThroughput;
public:
    dump();
    virtual ~dump();
protected:
    virtual void handleMessage(cMessage *msg);
    virtual void initialize();

};

Define_Module(dump);


dump::dump() {
    count = 0;
    tam = 0;

    s_rcvBit = 0;
    s_rcvPkt = 0;
    s_bitThroughput = 0;
    s_pktThroughput = 0;
}

dump::~dump() {

}

void dump::initialize(){
    s_rcvBit = registerSignal("rcvBit");
    s_rcvPkt = registerSignal("rcvPkt");
    s_bitThroughput = registerSignal("bitThroughput");
    s_pktThroughput = registerSignal("pktThroughput");

   WATCH (count);
}

void dump::handleMessage(cMessage *msg){
    cPacket *pk = check_and_cast<cPacket *>(msg);
    tam += pk->getBitLength();
    SimTime time = simTime();
    delete(pk);
    emit(s_rcvBit,tam);
    emit(s_rcvPkt,++count);
    emit(s_bitThroughput,tam/time);
    emit(s_pktThroughput,count/time);
}


