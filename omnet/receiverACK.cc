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
#include <simtime.h>
#include "Types.h"
#include "Link_m.h"
#include "Inter_Layer_m.h"

class receiverACK : public cSimpleModule {
private:
    int seq;
    simsignal_t s_sndAck;
    simsignal_t s_sndNack;
    simsignal_t s_rcvBit;
    simsignal_t s_rcvPkt;
    int sndAck,sndNack,rcvBit,rcvPkt;
    int ack_tam;
public:
    receiverACK();
protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void sentNack(int s_seq);
    virtual void send_up(cPacket *pk);
};

Define_Module(receiverACK);


receiverACK::receiverACK() {
    seq = 0;
    sndAck = 0;
    sndNack = 0;
    rcvBit = 0;
    rcvPkt = 0;
    s_sndAck = 0;
    s_sndNack = 0;
    s_rcvBit = 0;
    s_rcvPkt = 0;
    ack_tam = 1;

}

/*receiverACK::~receiverACK() {
    // TODO Auto-generated destructor stub
}*/

void receiverACK::initialize(){
    s_sndAck = registerSignal("sndACK");
    s_sndNack = registerSignal("sndNACK");
    s_rcvBit = registerSignal("rcvBit");
    s_rcvPkt = registerSignal("rcvPkt");

    if(par("Ack_Tam").containsValue()){
        ack_tam = par("Ack_Tam");
    }
}

void receiverACK::handleMessage(cMessage *msg){
    Link *pk = check_and_cast<Link *>(msg);
    int r_seq = pk->getSeq();
    if (pk->hasBitError())
    {
        //paquete con error
        bubble("message error");
        if(r_seq==(seq+1)){
            //el que se esperaba
            sentNack(r_seq);
            emit(s_sndNack,++sndNack);
        }
        /*Si es menor ya se ha recivido correctametne y se obvia*/
        /*Si es mayor que el esperado se obvia*/
        delete(pk);
     }
     else
     {
         if(r_seq==(seq+1)){
            //paquete correcto
            Link * pkt = new Link("ACK",0);
            pkt->setBitLength(ack_tam);
            pkt->setType(e_ack_t);
            pkt->setSeq(++seq);
            send(pkt, "out");
            emit(s_sndAck,++sndAck);
            /*enviar el paquete a la capa supeior*/
            if(pk->hasEncapsulatedPacket()){
                cPacket * up = (cPacket *)pk->decapsulate();
                int tam = up->getBitLength();
                rcvBit += tam;
                emit(s_rcvBit,rcvBit);
                emit(s_rcvPkt,++rcvPkt);
                send_up(up);
            }
        }else if(r_seq<(seq+1)){
            /*En caso de recivir una secuencia menor se envia un ACK de la última (ACK acumulado)*/
            Link * pkt = new Link("ACK",0);
            pkt->setBitLength(ack_tam);
            pkt->setType(e_ack_t);
            pkt->setSeq(seq);
            send(pkt, "out");
            emit(s_sndAck,++sndAck);
        }
        /*En caso de que el paquete llega desordenado (un sequencia mayor), se obvia*/
         delete(pk);
     }
}

void receiverACK::sentNack(int s_seq){
    Link * pkt = new Link("NACK",1);
    pkt->setBitLength(ack_tam);
    pkt->setType(e_nack_t);
    pkt->setSeq(s_seq);
    send(pkt,"out");
}

void receiverACK::send_up(cPacket *pk){
    inter_layer *il = new inter_layer("fisicoUP",0);
    il->encapsulate(pk);

    send(il,"up_out");
}


