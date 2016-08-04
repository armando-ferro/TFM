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
    Packet *message;  // message that has to be re-sent on timeout
    cMessage *sent;
    int sent_seq;
    int origen;
    int header_tam;
    cChannel * txChannel;
    cQueue *txQueue;
    /*Señales*/
    simsignal_t s_queueState;
    simsignal_t s_rcvAck;
    simsignal_t s_rcvNack;
    int rcvAck;
    int rcvNack;
    /*Maquina de estados y estado*/
    short state_machine;
public:
    senderSW();
    virtual ~senderSW();

protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void sendCopyOf(Packet *msg);
    virtual void newPacket(inter_layer *il);
    virtual void send_pk(Packet *pk);
};

Define_Module(senderSW);

senderSW::senderSW() {
    /*constructor*/
    sent = NULL;
    message = NULL;
    txQueue = NULL;
    txChannel = NULL;
    sent_seq = 0;
    origen = 0;
    header_tam = 0;
    s_queueState = 0;
    s_rcvAck = 0;
    s_rcvNack = 0;
    rcvAck = 0;
    rcvNack = 0;
    state_machine = idle;
}

senderSW::~senderSW() {
    /*Destructor*/
    cancelAndDelete(sent);
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

    /*Parámetros*/
    origen = par("Addr");
    header_tam = par("Header_Tam");

    WATCH(state_machine);
    WATCH(sent_seq);

}

void senderSW::handleMessage(cMessage *msg){
    EV << "Handle Message";
    if(msg == sent){
        EV << " Sent";
        /*el mensaje ya ha sido enviado*/
        state_machine=w_ack;
    }
    else{
        /*comprobar si viene de fuera o de la capa superior*/
        if(msg->arrivedOn("up_in")){
            /*Paquete de la capa superior*/
            EV << " Capa superior";
            inter_layer *pk = check_and_cast<inter_layer *>(msg);
            newPacket(pk);
        }else{
            /*capa inferior*/
            EV << " Externo";
            Packet *up = check_and_cast<Packet *>(msg);
            /*ACK - NACK*/
            switch(up->getType()){
            case t_nack_t:
                EV << " NACK";
                sendCopyOf(message);
                emit(s_rcvNack,++rcvNack);
                break;
            case t_ack_t:
                EV << " ACK";
                /*extraer secuencia y comparar*/
                int ack_seq;
                ack_seq = up->getSeq();
                if(ack_seq == sent_seq){
                    /*ACK correcto*/
                    /*comprobar cola*/
                    if(txQueue->empty()){
                        /*no hay mensajes*/
                        state_machine = idle;
                    }else{
                        /*sacar de cola y mandar*/
                        Packet * sp = (Packet *)txQueue->pop();
                        send_pk(sp);
                    }
                }
                emit(s_rcvAck,++rcvAck);
                break;
            default:
                /*será como un NACK*/
                EV << " Default";
                sendCopyOf(message);
                break;
            }
        }
    }
}


void senderSW::sendCopyOf(Packet *msg)
{
    EV << " SendCopyOf";
    /*Duplicar el mensaje y mandar una copia*/
    inter_layer *copy = (inter_layer *) msg->dup();
    send(copy, "out");
    state_machine=sending;
    simtime_t txFinishTime = txChannel->getTransmissionFinishTime();
    scheduleAt(txFinishTime,sent);
}

void senderSW::newPacket(inter_layer *il){
    EV<< " NewPacket";
    /*Nuevo paquete de la capa superior*/
    /*Extraer datos y paquete*/
    int dest = il->getDestino();
    Packet * body = (Packet *)il->decapsulate();

    /*crear el paquete y encapsular*/
    Packet *sp = new Packet("senderSW",0);
    sp->setDestAddr(dest);
    sp->setSrcAddr(origen);
    sp->setBitLength(header_tam);
    sp->encapsulate(body);

    /*comprobar mandar o encolar*/
    if(state_machine == idle){
        /*no hay paquetes, mandar*/
        send_pk(sp);
    }else{
        /*mandando, encolar*/
        txQueue->insert(sp);
    }
}

void senderSW::send_pk(Packet *pk){
    EV << " Send pk";
    /*poner secuencia, mandar y crear copia*/
    pk->setSeq(++sent_seq);
    delete(message);
    message = pk->dup();
    send(pk,"out");
    state_machine = sending;
    simtime_t txFinishTime = txChannel->getTransmissionFinishTime();
    scheduleAt(txFinishTime,sent);
}

