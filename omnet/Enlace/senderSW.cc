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
#include "Link_m.h"
#include "Inter_layer_m.h"

const short idle = 0;
const short sending = 1;
const short w_ack = 2;

class senderSW : public cSimpleModule{
private:
    Link *message;  // message that has to be re-sent on error
    cMessage *sent;
    bool b_config;
    int rpt,sndBit,sndPkt,lostPkt;
    unsigned int header_tam;
    int max_state;
    unsigned int ack_seq,sent_seq;
    cChannel * txChannel;
    cQueue *txQueue;
    /*Señales*/
    simsignal_t s_queueState;
    simsignal_t s_rcvAck;
    simsignal_t s_rcvNack;
    simsignal_t s_sndBit;
    simsignal_t s_sndPkt;
    simsignal_t s_lostPkt;
    /*control*/
    unsigned int rcvAck;
    unsigned int rcvNack;
    /*Maquina de estados y estado*/
    short state_machine;
public:
    senderSW();
    virtual ~senderSW();

protected:
    virtual void sendCopyOf(Link *msg);
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual Link *getPacket(cPacket *msg);
};

Define_Module(senderSW);

senderSW::senderSW() {
    /*constructor*/
    header_tam = 0;
    max_state = -1;
    sent = NULL;
    message = NULL;
    txQueue = NULL;
    txChannel = NULL;
    rpt = 0;
    ack_seq = 0;
    sent_seq = 0;
    sndBit = 0;
    sndPkt = 0;
    lostPkt = 0;
    rcvAck = 0;
    rcvNack = 0;
    s_rcvAck = 0;
    s_rcvNack = 0;
    s_lostPkt = 0;
    s_queueState = 0;
    s_sndBit = 0;
    s_sndPkt = 0;
    state_machine = idle;
    b_config = true;
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
    s_lostPkt = registerSignal("lostPkt");
    s_sndBit = registerSignal("sndBit");
    s_sndPkt = registerSignal("sndPkt");


    if(gate("out")->isConnected()){
        txChannel = gate("out")->getTransmissionChannel();
    }else{
        b_config = false;
    }

    sent = new cMessage("sent");
    txQueue = new cQueue("txQueue");

    if(par("Queue_Length").containsValue()){
        max_state = par("Queue_Length");
    }

    if(par("Header_Tam").containsValue()){
        header_tam = par("Header_Tam");
    }

    WATCH(state_machine);
    WATCH(sent_seq);
    WATCH(ack_seq);
    WATCH(rpt);
    WATCH(rcvAck);
    WATCH(rcvNack);
}

void senderSW::handleMessage(cMessage *msg){
    if(not(b_config)){
        /*configuración errónea*/
        bubble("Canal no conectado");
        delete(msg);
        return;
    }
    if(msg == sent){
        /*el mensaje ya ha sido enviado*/
        state_machine=w_ack;
    }
    else{
        if(msg->arrivedOn("up_in")){
            /*llega un paquete nuevo*/
            /*extrare el original*/
            inter_layer *il = check_and_cast<inter_layer *>(msg);
            cPacket *up = (cPacket *)il->decapsulate();
            if(state_machine==idle){
                /*Enviar el paquete*/
                delete(message);
                message = getPacket(up);
                int tam = up->getBitLength();
                sndBit += tam;
                emit(s_sndBit,sndBit);
                emit(s_sndPkt,++sndPkt);
                sendCopyOf(message);
            }
            else{
                /*almacenarlo en la cola*/
                if(max_state < 0){
                    /*no hay límite*/
                    txQueue->insert(up);
                    emit(s_queueState,txQueue->length());
                }
                else{
                    if(max_state>=txQueue->length()){
                        /*se ha llegado al límite*/
                        emit(s_lostPkt,++lostPkt);
                        char msgname[20];
                        sprintf(msgname,"cola llena-%d",max_state);
                        bubble(msgname);
                        delete(up);
                    }else{
                        txQueue->insert(up);
                        emit(s_queueState,txQueue->length());
                    }
                }
            }
            delete(il);
        }else{
            /*capa inferior*/
            /*comprobar estado*/
            if(state_machine != w_ack){
                delete(msg);
                return;
            }
            /*Comprobar si es un ACK o un NACK*/
            Link *pk = check_and_cast<Link *>(msg);
            if (pk->getType()==e_nack_t)
            {
                /*NACK*/
                /*Se añade una repetición*/
                if(pk->getSeq() == sent_seq){
                    rpt++;
                    sendCopyOf(message);
                    emit(s_rcvNack,++rcvNack);
                }

            }
            else if(pk->getType()==e_ack_t)
            {

                /*ACK*/
                unsigned int r_seq = pk->getSeq();
                if(r_seq == sent_seq){
                    rpt=0;
                    ack_seq=r_seq;
                    if(txQueue->empty()){
                        /*No hay mensajen en cola se espera otro*/
                        state_machine = idle;
                    }else{
                        /*se debe extraer un mensaje de la cola y enviar*/
                        delete(message);
                        cPacket *txpk = (cPacket *)txQueue->pop();
                        int tam  = txpk->getBitLength();
                        sndBit += tam;
                        emit(s_sndBit,sndBit);
                        emit(s_sndPkt,++sndPkt);
                        message = getPacket(txpk);
                        emit(s_queueState,txQueue->length());
                        sendCopyOf(message);
                    }
                    emit(s_rcvAck,++rcvAck);
                }

            }
            delete(pk);
        }
    }
}

Link *senderSW::getPacket(cPacket *msg){
    char msgname[20];
    sprintf(msgname,"LinkSW-%d",++sent_seq);
    Link *lk = new Link(msgname,0);
    lk->setType(e_msg_t);
    lk->setSeq(sent_seq);
    lk->setBitLength(header_tam);
    lk->encapsulate(msg);
    rpt=0;
    return lk;
}

void senderSW::sendCopyOf(Link *msg)
{
    /*Duplicar el mensaje y mandar una copia*/
    Link *copy = (Link *) msg->dup();
    send(copy, "out");
    rpt++;
    state_machine=sending;
    simtime_t txFinishTime = txChannel->getTransmissionFinishTime();
    scheduleAt(txFinishTime,sent);
}

