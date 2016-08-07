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

#include <string.h>
#include <omnetpp.h>
#include <simtime.h>
#include "Inter_Layer_m.h"
#include "Packet_m.h"

/*Estados*/
const short idle = 0;
const short sending = 1;


class fisico: public cSimpleModule
{
private:
    /*gestiones internas*/
    int max_state;
    int header_tam;
    short state_machine;
    cQueue *txQueue;
    cChannel * txChannel;
    cMessage *sent;
    /*extracci�n de estad�sticas*/
    int sndBit;
    int sndPkt;
    int rcvBit;
    int rcvPkt;
    int lostPkt;
    int errorPkt;
    simsignal_t s_sndBit;
    simsignal_t s_sndPkt;
    simsignal_t s_rcvPkt;
    simsignal_t s_rcvBit;
    simsignal_t s_lostPkt;
    simsignal_t s_errorPkt;
public:
    fisico();
    virtual ~fisico();

protected:
  /*Funciones*/
  virtual void initialize();
  virtual void handleMessage(cMessage *msg);
  virtual void send_out(cPacket *pk);
};

// The module class needs to be registered with OMNeT++

Define_Module(fisico);


fisico::fisico() {
    /*gestion de funcionamiento*/
    header_tam = 0;
    state_machine = idle;
    txQueue = NULL;
    txChannel = NULL;
    sent = NULL;
    max_state = -1;
    /*extracci�n de estad�sticas*/
    sndBit = 0;
    sndPkt = 0;
    rcvBit = 0;
    rcvPkt = 0;
    lostPkt = 0;
    errorPkt = 0;
    s_sndBit = 0;
    s_sndPkt = 0;
    s_rcvBit = 0;
    s_rcvPkt = 0;
    s_lostPkt = 0;
    s_errorPkt = 0;

    WATCH(max_state);
}

fisico::~fisico() {
    cancelAndDelete(sent);
    txQueue->~cQueue();

}

void fisico::initialize(){
    /*recoger par�metros*/
    if(par("Queue_Length").containsValue()){
        max_state = par("Queue_Length");
    }

    if(par("Header_Tam").containsValue()){
        header_tam = par("Header_Tam");
    }

    /*Enganchar se�ales*/
    s_sndBit = registerSignal("sndBit");
    s_sndPkt = registerSignal("sndPkt");
    s_rcvBit = registerSignal("rcvBit");
    s_rcvPkt = registerSignal("rcvPkt");
    s_lostPkt = registerSignal("lostPkt");
    s_errorPkt = registerSignal("errorPkt");

    /*Cola de mensajes a enviar*/
    txQueue = new cQueue("txQueue");
    /*mensaje que indica cuando se ha terminado de enviar un packete*/
    sent = new cMessage("sent");
    /*Canal de salida*/
    txChannel = gate("out")->getTransmissionChannel();

    WATCH(state_machine);
    WATCH(max_state);
}

void fisico::handleMessage(cMessage *msg){
    if(msg == sent){
        /*propio comprobar cola o idle*/
        if(txQueue->empty()){
            /*No hay mensajen en cola se espera otro*/
            state_machine = idle;
        }else{
            /*se debe extraer un mensaje de la cola y enviar*/
            cPacket *message = (cPacket *)txQueue->pop();
            send_out(message);
        }
    }
    else{
        /*comprobar si es de capa superior o exterior*/
        if(msg->arrivedOn("up_in")){
            /*capa superior, extraer comprobar estado*/
            inter_layer *rx = check_and_cast<inter_layer *>(msg);
            cPacket *pk;
            if(rx->hasEncapsulatedPacket()){
                pk = rx->decapsulate();
            }else{
                return;
            }
            if(state_machine == idle){
                /*enviar mensaje*/
                send_out(pk);
            }else{
                /*enviando otro paquete, meterlo en la cola*/
                /*comprobar cola*/
                if(max_state >= 0){
                    /*hay limite*/
                    if(txQueue->length() >= max_state){
                        /*se ha superado el limite*/
                        char msgname[20];
                        sprintf(msgname,"cola llena-%d",max_state);
                        bubble(msgname);
                        delete(pk);
                    }else{
                        /*todav�a hay sitio*/
                        txQueue->insert(pk);
                    }
                }else{
                    /*no hay limite*/
                    txQueue->insert(pk);
                }

            }
            delete(rx);
        }else{
            /*externo, comprobar eror, desencapsular y subir a la capa superior*/
            Packet * rxp = check_and_cast<Packet *>(msg);
            if(rxp->hasBitError()){
                bubble("Paquete con error");
                emit(s_errorPkt,++errorPkt);
                delete(rxp);
            }else{
                if(rxp->hasEncapsulatedPacket()){
                    Packet *data = (Packet *) rxp->decapsulate();
                    int tam = data->getBitLength();
                    rcvBit += tam;
                    emit(s_rcvBit,rcvBit);
                    emit(s_rcvPkt,++rcvPkt);
                    send(data,"up_out");
                }else{
                    bubble("Paquete inesperado");
                }
                delete(rxp);
            }
        }
    }
}

void fisico::send_out(cPacket * pk){
    /*empaquetar sacar estad�sticas*/
    int tam;
    tam = pk->getBitLength();
    sndBit += tam;
    char msgname[20];
    sprintf(msgname,"fisico-%d",++sndPkt);
    Packet  *rx = new Packet(msgname,0);
    rx->setBitLength(header_tam);
    rx->encapsulate(pk);
    /*enviar por out*/
    send(rx,"out");
    state_machine = sending;
    /*programar el auto mensaje para cambair de estado*/
    simtime_t txFinishTime = txChannel->getTransmissionFinishTime();
    scheduleAt(txFinishTime,sent);
    emit(s_sndBit,sndBit);
    emit(s_sndPkt,sndPkt);
}