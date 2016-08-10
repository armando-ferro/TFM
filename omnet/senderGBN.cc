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
#include "Packet_m.h"
#include "inter_layer_m.h"

const short idle = 0;
const short sending_i = 1;
const short sending_r = 2;

class senderGBN : public cSimpleModule{
private:
    int sent_seq,ack_seq,rpt_seq;
    int rpt_total;  //total packege to repeat after nack
    int rcvAck,rcvNack;
    /*Señales*/
    simsignal_t s_queueState;
    simsignal_t s_rcvAck;
    simsignal_t s_rcvNack;
    simsignal_t s_windowTam;
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
    virtual void sendCopyOf(Packet *msg);
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual Packet *getPacket(Packet *msg);
    virtual void startRetrasmision();
};

Define_Module(senderGBN);

senderGBN::senderGBN() {
    /*constructor*/
    ack_seq = 0;
    sent_seq= 0;
    rpt_seq = 0;
    rpt_total = 0;
    sent = NULL;
    txChannel = NULL;
    txQueue = NULL;
    ackQueue = NULL;
    state_machine = idle;

    rcvAck = 0;
    rcvNack = 0;
    s_rcvAck = 0;
    s_rcvNack = 0;
    s_queueState = 0;
    s_windowTam = 0;
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
    EV << "MENSAJE: ";
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
                Packet *pk = (Packet *)txQueue->pop();
                pk = getPacket(pk);
                sendCopyOf(pk);
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
                    Packet *pk = (Packet *)txQueue->pop();
                    pk = getPacket(pk);
                    sendCopyOf(pk);
                    state_machine = sending_i;
                }
            }
            else{
                /*Se debe seguir transmitiendo la repetición*/
                Packet *pk = (Packet *)ackQueue->get(rpt_seq++);
                sendCopyOf(pk);
                state_machine = sending_r;
            }
            break;
        }
    }
    else{
        /*El mensaje no es el sent*/

        if(msg->arrivedOn("pkt")){
            /*Es un mensaje nuevo*/
            /*desencapsular*/
            inter_layer *il = check_and_cast<inter_layer *>(msg);
            Packet *up = (Packet *)il->decapsulate();
            emit(s_queueState,txQueue->length());
            emit(s_windowTam,(sent_seq-ack_seq));
            EV << " Message nuevo. ";
            /*llega un paquete nuevo*/
            if(state_machine == idle){
                /*si se esta esperando se envia el mensaje*/
                up = getPacket(up);
                sendCopyOf(up);
                state_machine = sending_i;
            }else{
                /*se almacena el mensaje en la cola*/
                txQueue->insert(up);
            }
            delete(il);
        }else{
            /*Comprobar si es un ACK o un NACK*/
            Packet *pk = check_and_cast<Packet *>(msg);
            int rcv_seq = pk->getSeq();
            if (pk->getType()==nack_t)
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
                        Packet *pkt = (Packet *) ackQueue->pop();
                        delete(pkt);
                    }
                    ack_seq = rcv_seq-1;
                    startRetrasmision();
                    emit(s_rcvNack,++rcvNack);
                }
                /*si ya se ha recivido un ack superior no se puede recivir uno inferior*/
            }
            else if(pk->getType()==ack_t)
            {
                EV << " ACK. ";
                /*ACK*/
                /*comprobar si es ack acumlado*/
                int ack_acum = rcv_seq - ack_seq;
                if(ack_acum >= 1){
                    for(int i = 0;i<ack_acum;i++){
                        Packet *pkt = (Packet *)ackQueue->pop();
                        delete(pkt);
                    }
                    ack_seq = rcv_seq;
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
        Packet *pk = (Packet *)ackQueue->get(rpt_seq++);
        sendCopyOf(pk);
    }
    /*este o no retrasmistiendo el nuevo estado es el de reenvio*/
    state_machine = sending_r;
}

Packet *senderGBN::getPacket(Packet *msg){
    EV << " generando secuencia";
    /*Función al enviar un nuevo paquete, adaptarlo y mandarlo al código*/
    msg->setSeq(++sent_seq);
    ackQueue->insert(msg);
    return msg;
}

void senderGBN::sendCopyOf(Packet *msg)
{
    EV << " Enviando mensaje";
    /*Duplicar el mensaje y mandar una copia*/
    Packet *copy = (Packet *) msg->dup();
    send(copy, "out");
    simtime_t txFinishTime = txChannel->getTransmissionFinishTime();
    scheduleAt(txFinishTime,sent);
}

