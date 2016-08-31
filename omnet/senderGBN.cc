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
#include <ctopology.h>
#include <cstringtokenizer.h>
#include "Types.h"
#include "Link_m.h"
#include "inter_layer_m.h"

const short idle = 0;
const short sending_i = 1;
const short sending_r = 2;

class senderGBN : public cSimpleModule{
private:
    int sent_seq,ack_seq,rpt_seq;
    int rpt_total;  //total packege to repeat after nack
    int rcvAck,rcvNack,sndBit,sndPkt,lostPkt;
    unsigned int header_tam;
    int max_state;
    /*Señales*/
    simsignal_t s_queueState;
    simsignal_t s_rcvAck;
    simsignal_t s_rcvNack;
    simsignal_t s_windowTam;
    simsignal_t s_sndBit;
    simsignal_t s_sndPkt;
    simsignal_t s_lostPkt;
    /*Mensaje para enviarse a si mismo al terminar de enviar un paquete*/
    cMessage *sent;
    /*Canal*/
    cChannel * txChannel;
    /*Colas*/
    cQueue *txQueue;
    cQueue *ackQueue;
    /*Maquina de estados y estado*/
    short state_machine;
public:
    senderGBN();
    virtual ~senderGBN();

protected:
    virtual void sendCopyOf(Link *msg);
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual Link *getPacket(cPacket *msg);
    virtual void startRetrasmision();
};

Define_Module(senderGBN);

senderGBN::senderGBN() {
    /*constructor*/
    ack_seq = 0;
    sent_seq= 0;
    rpt_seq = 0;
    rpt_total = 0;
    lostPkt = 0;
    sent = NULL;
    txChannel = NULL;
    txQueue = NULL;
    ackQueue = NULL;
    state_machine = idle;

    max_state = -1;
    header_tam = 0;

    sndBit = 0;
    sndPkt = 0;
    rcvAck = 0;
    rcvNack = 0;
    s_rcvAck = 0;
    s_rcvNack = 0;
    s_queueState = 0;
    s_windowTam = 0;
    s_lostPkt = 0;
}

senderGBN::~senderGBN() {
    /*Destructor*/
    cancelAndDelete(sent);
    txQueue->~cQueue();
    ackQueue->~cQueue();

}

void senderGBN::initialize(){
    s_queueState = registerSignal("QueueState");
    s_rcvAck = registerSignal("rcvACK");
    s_rcvNack = registerSignal("rcvNACK");
    s_windowTam = registerSignal("WindowTam");
    s_lostPkt = registerSignal("lostPkt");
    s_sndBit = registerSignal("sndBit");
    s_sndPkt = registerSignal("sndPkt");

    if(par("Queue_Length").containsValue()){
        max_state = par("Queue_Length");
    }

    if(par("Header_Tam").containsValue()){
        header_tam = par("Header_Tam");
    }

    /*Inicializar variables*/
    txChannel = gate("out")->getTransmissionChannel();
    sent = new cMessage("sent");
    txQueue = new cQueue("txQueue");
    ackQueue = new cQueue("ackQueue");

    WATCH(state_machine);
    WATCH(ack_seq);
    WATCH(sent_seq);
    WATCH(rpt_seq);
    WATCH(rpt_total);

}

void senderGBN::handleMessage(cMessage *msg){
    if(msg == sent){
        EV << " Propio. ";
        /*el mensaje ya ha sido enviado*/
        /*Comprobar en que estado de envio estamos*/
        switch(state_machine){
        case sending_i:
            EV << " En sending_i";
            /*Comprobar la cola*/
            if(txQueue->isEmpty()){
                /*vuelve a estado idle*/
                state_machine = idle;
            }else{
                /*hay mensajes que enviar*/
                /*no hay limite*/
                cPacket *pk = (cPacket *)txQueue->pop();
                emit(s_queueState,txQueue->length());
                Link * lk = getPacket(pk);
                sendCopyOf(lk);
                state_machine = sending_i;
            }
            break;
        case sending_r:
            EV << " En sending_r";
            /*comprobar si hay que envia otro o no*/
            if(rpt_seq == rpt_total){
                /*se ha enviado toda la repetición*/
                /*Comprobar cola de trasmisión*/
                if(txQueue->isEmpty()){
                    /*vuelve a estado idle*/
                    state_machine = idle;
                }else{
                    /*hay mensajes que enviar*/
                    cPacket *pk = (cPacket *)txQueue->pop();
                    emit(s_queueState,txQueue->length());
                    Link * lk = getPacket(pk);
                    sendCopyOf(lk);
                    state_machine = sending_i;
                }
            }
            else{
                /*Se debe seguir transmitiendo la repetición*/
                Link *pk = (Link *)ackQueue->get(rpt_seq++);
                sendCopyOf(pk);
                state_machine = sending_r;
            }
            break;
        }
    }
    else{
        /*El mensaje no es el sent*/

        if(msg->arrivedOn("up_in")){
            /*Es un mensaje nuevo*/
            /*desencapsular*/
            inter_layer *il = check_and_cast<inter_layer *>(msg);
            cPacket *up = (cPacket *)il->decapsulate();
            EV << " Message nuevo. ";
            /*llega un paquete nuevo*/
            if(state_machine == idle){
                /*si se esta esperando se envia el mensaje*/
                Link *lk = getPacket(up);
                sendCopyOf(lk);
                state_machine = sending_i;
            }else{
                /*comprobar cola*/
                if(max_state<0){
                    /*no hay límite*/
                    txQueue->insert(up);
                    emit(s_queueState,txQueue->length());
                }else{
                    /*comprobar límtie*/
                    if(txQueue->length()<max_state){
                        /*entra*/
                        txQueue->insert(up);
                        emit(s_queueState,txQueue->length());
                    }else{
                        /*no hay sitio*/
                        delete(up);
                        emit(s_lostPkt,++lostPkt);
                    }
                }

            }
            delete(il);
        }else{
            /*Comprobar si es un ACK o un NACK*/
            Link *pk = check_and_cast<Link *>(msg);
            int rcv_seq = pk->getSeq();
            if (pk->getType()==e_nack_t)
            {
                EV << " NACK. ";
                /*NACK*/
                /*Comprobar la secuencia*/
                if(rcv_seq == (ack_seq+1)){
                    /*se solicita el último enviado*/
                    startRetrasmision();
                    emit(s_rcvNack,++rcvNack);
                } else if(rcv_seq > (ack_seq+1)){
                    /*se han perdido ack*/
                    /*se supone que ack acumulado, se debe repetir a partir del erroneo*/
                    for(int i = 0;i<((rcv_seq-1)-ack_seq);i++){
                        Link *pkt = (Link *) ackQueue->pop();
                        delete(pkt);
                    }
                    ack_seq = rcv_seq-1;
                    emit(s_windowTam,(sent_seq-ack_seq));
                    startRetrasmision();
                    emit(s_rcvNack,++rcvNack);
                }
                /*si ya se ha recivido un ack superior no se puede recivir uno inferior*/
            }
            else if(pk->getType()==e_ack_t)
            {
                EV << " ACK. ";
                /*ACK*/
                /*comprobar si es ack acumlado*/
                int ack_acum = rcv_seq - ack_seq;
                if(ack_acum >= 1){
                    for(int i = 0;i<ack_acum;i++){
                        Link *pkt = (Link *)ackQueue->pop();
                        delete(pkt);
                    }
                    ack_seq = rcv_seq;
                    emit(s_windowTam,(sent_seq-ack_seq));
                    /*Comprobar si se esta retrasmitiendo*/
                    if(state_machine == sending_r){
                        /*actualizar variables de retrasmisión*/
                        rpt_total -= ack_acum;
                        rpt_seq -= ack_acum;
                    }
                    emit(s_rcvAck,++rcvAck);
                }
                /*en caso contrario es un ack que ya ha sido gestionado*/
            }
            delete(pk);
        }
    }
}

void senderGBN::startRetrasmision(){
    /*Iniciar los parámetros de la retrasmisión*/
    rpt_total = sent_seq - ack_seq;
    rpt_seq = 0;
    /*Comprobar estado*/
    if(state_machine == idle){
        /*enviar primer paquete repetido*/
        Link *pk = (Link *)ackQueue->get(rpt_seq++);
        sendCopyOf(pk);
    }
    /*este o no retrasmistiendo el nuevo estado es el de reenvio*/
    state_machine = sending_r;
}

Link *senderGBN::getPacket(cPacket *msg){
    EV << " generando secuencia";

    /*Generar nuevo paquete, encapsular y guardar*/
    char msgname[20];
    sprintf(msgname,"LinkGBN-%d",++sent_seq);
    Link *lk = new Link(msgname,0);
    lk->setType(e_msg_t);
    lk->setSeq(sent_seq);
    lk->encapsulate(msg);
    int tam = msg->getBitLength();
    sndBit += tam;
    emit(s_sndBit,sndBit);
    emit(s_sndPkt,++sndPkt);
    ackQueue->insert(lk);
    emit(s_windowTam,(sent_seq-ack_seq));
    return lk;
}

void senderGBN::sendCopyOf(Link *msg)
{
    EV << " Enviando mensaje";
    /*Duplicar el mensaje y mandar una copia*/
    Link *copy = (Link *) msg->dup();
    send(copy, "out");
    simtime_t txFinishTime = txChannel->getTransmissionFinishTime();
    scheduleAt(txFinishTime,sent);
}

