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

const short idle = 0;
const short sending = 1;
const short w_ack = 2;

class senderSW : public cSimpleModule{
private:
    Packet *message;  // message that has to be re-sent on error
    cMessage *sent;
    int rpt;
    int ack_seq,sent_seq;
    cChannel * txChannel;
    cQueue *txQueue;
    /*Señales*/
    simsignal_t s_queueState;
    simsignal_t s_rcvAck;
    simsignal_t s_rcvNack;
    /*control*/
    int rcvAck;
    int rcvNack;
    /*Maquina de estados y estado*/
    short state_machine;
public:
    senderSW();
    virtual ~senderSW();

protected:
    virtual void sendCopyOf(Packet *msg);
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual Packet *getPacket(Packet *msg);
};

Define_Module(senderSW);

senderSW::senderSW() {
    /*constructor*/
    sent = NULL;
    message = NULL;
    txQueue = NULL;
    txChannel = NULL;
    rpt = 0;
    ack_seq = 0;
    sent_seq = 0;
    rcvAck = 0;
    rcvNack = 0;
    s_rcvAck = 0;
    s_rcvNack = 0;
    s_queueState = 0;
    state_machine = idle;
}

senderSW::~senderSW() {
    /*Destructor*/
    cancelAndDelete(sent);
    txQueue->~cQueue();
    delete message;
}

void senderSW::initialize(){
    /*Inicializar variables*/
    s_queueState = registerSignal("QueueState");
    s_rcvAck = registerSignal("rcvACK");
    s_rcvNack = registerSignal("rcvNACK");

    txChannel = gate("out")->getTransmissionChannel();

    sent = new cMessage("sent");
    txQueue = new cQueue("txQueue");

    WATCH(state_machine);
    WATCH(sent_seq);
    WATCH(ack_seq);
    WATCH(rpt);
    WATCH(rcvAck);
    WATCH(rcvNack);
}

void senderSW::handleMessage(cMessage *msg){
    EV << "llega mensaje";
    if(msg == sent){
        EV << " sent";
        /*el mensaje ya ha sido enviado*/
        state_machine=w_ack;
    }
    else{
        if(msg->arrivedOn("pkt")){
            EV << " nuevo";
            /*llega un paquete nuevo*/
            /*extrare el original*/
            inter_layer *il = check_and_cast<inter_layer *>(msg);
            Packet *up = (Packet *)il->decapsulate();
            emit(s_queueState,txQueue->length());
            switch(state_machine){
            case idle:
                /*Enviar el paquete*/
                delete(message);
                message = getPacket(up);
                sendCopyOf(message);
                break;
            case sending:
                /*almacenarlo en la cola*/
                txQueue->insert(up);
                break;
            case w_ack:
                /*almacenarlo en la cola*/
                txQueue->insert(up);
                break;
            }
            delete(il);
        }else{
            /*Comprobar si es un ACK o un NACK*/
            Packet *pk = check_and_cast<Packet *>(msg);
            if (pk->getType()==nack_t)
            {
                EV << " NACK";
                /*NACK*/
                /*Se añade una repetición*/
                if(pk->getSeq() == sent_seq){
                    rpt++;
                    sendCopyOf(message);
                    emit(s_rcvNack,++rcvNack);
                }

            }
            else if(pk->getType()==ack_t)
            {
                EV << " ACK";
                /*ACK*/
                int r_seq = pk->getSeq();
                if(r_seq == sent_seq){
                    rpt=0;
                    ack_seq=r_seq;
                    if(txQueue->empty()){
                        /*No hay mensajen en cola se espera otro*/
                        state_machine = idle;
                    }else{
                        /*se debe extraer un mensaje de la cola y enviar*/
                        delete(message);
                        message = getPacket((Packet *)txQueue->pop());
                        sendCopyOf(message);
                    }
                    emit(s_rcvAck,++rcvAck);
                }

            }
            delete(pk);
        }
    }
}

Packet *senderSW::getPacket(Packet *msg){
    msg->setSeq(++sent_seq);
    rpt=0;
    return msg;
}

void senderSW::sendCopyOf(Packet *msg)
{
    /*Duplicar el mensaje y mandar una copia*/
    Packet *copy = (Packet *) msg->dup();
    send(copy, "out");
    rpt++;
    state_machine=sending;
    simtime_t txFinishTime = txChannel->getTransmissionFinishTime();
    scheduleAt(txFinishTime,sent);
}

