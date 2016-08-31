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
#include "Types.h"

/*Estados*/
const short idle = 0;
const short sending = 1;


class simple_fisico: public cSimpleModule
{
private:
    /*gestiones internas*/
    bool b_config;
    short state_machine;
    cQueue *txQueue;
    cChannel * txChannel;
    cMessage *sent;
    /*extracción de estadísticas*/
    int sndBit;
    int sndPkt;
    int rcvBit;
    int rcvPkt;
    simsignal_t s_sndBit;
    simsignal_t s_sndPkt;
    simsignal_t s_rcvPkt;
    simsignal_t s_rcvBit;
public:
    simple_fisico();
    virtual ~simple_fisico();

protected:
  /*Funciones*/
  virtual void initialize();
  virtual void handleMessage(cMessage *msg);
  virtual void send_out(cPacket *pk);
  virtual void send_up(cPacket *pk);
};

// The module class needs to be registered with OMNeT++

Define_Module(simple_fisico);


simple_fisico::simple_fisico() {
    /*gestion de funcionamiento*/
    state_machine = idle;
    txQueue = NULL;
    txChannel = NULL;
    sent = NULL;
    /*extracción de estadísticas*/
    sndBit = 0;
    sndPkt = 0;
    rcvBit = 0;
    rcvPkt = 0;
    s_sndBit = 0;
    s_sndPkt = 0;
    s_rcvBit = 0;
    s_rcvPkt = 0;

    b_config = true;
}

simple_fisico::~simple_fisico() {
    cancelAndDelete(sent);
    txQueue->~cQueue();

}

void simple_fisico::initialize(){
    /*recoger parámetros*/

    /*Enganchar señales*/
    s_sndBit = registerSignal("sndBit");
    s_sndPkt = registerSignal("sndPkt");
    s_rcvBit = registerSignal("rcvBit");
    s_rcvPkt = registerSignal("rcvPkt");

    /*Cola de mensajes a enviar*/
    txQueue = new cQueue("txQueue");
    /*mensaje que indica cuando se ha terminado de enviar un packete*/
    sent = new cMessage("sent");
    /*Canal de salida*/
    if(gate("out")->isConnected()){
        txChannel = gate("out")->getTransmissionChannel();
    }else{
        b_config = false;
    }


    WATCH(state_machine);
    WATCH(b_config);
}

void simple_fisico::handleMessage(cMessage *msg){
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
            if(not(b_config)){
                bubble("Imposible mandar mensajes, no conectado");
                delete(msg);
                return;
            }
            inter_layer *rx = check_and_cast<inter_layer *>(msg);
            cPacket *pk;
            if(rx->hasEncapsulatedPacket()){
                pk = rx->decapsulate();
            }else{
                delete(rx);
                return;
            }
            if(state_machine == idle){
                /*enviar mensaje*/
                send_out(pk);
            }else{
                /*enviando otro paquete, meterlo en la cola*/
                txQueue->insert(pk);
            }
            delete(rx);
        }else{
            /*externo, subir*/
            cPacket * rxp = check_and_cast<cPacket *>(msg);
            send_up(rxp);
        }
    }
}

void simple_fisico::send_out(cPacket * pk){
    /*empaquetar sacar estadísticas*/
    int tam;
    tam = pk->getBitLength();
    sndBit += tam;
    /*enviar por out*/
    send(pk,"out");
    state_machine = sending;
    /*programar el auto mensaje para cambair de estado*/
    simtime_t txFinishTime = txChannel->getTransmissionFinishTime();
    scheduleAt(txFinishTime,sent);
    emit(s_sndBit,sndBit);
    emit(s_sndPkt,++sndPkt);
}

void simple_fisico::send_up(cPacket *pk){
    /*encapsular con la información*/
    inter_layer *il = new inter_layer("simple_fisicoUP",0);
    int tam;
    tam = pk->getBitLength();
    rcvBit+=tam;
    il->encapsulate(pk);
    send(il,"up_out");
    emit(s_rcvBit,rcvBit);
    emit(s_sndPkt,++rcvPkt);

}
