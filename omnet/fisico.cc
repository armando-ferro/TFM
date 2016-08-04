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

/*Estados*/
const short idle = 0;
const short sending = 1;


class fisico: public cSimpleModule
{
private:
    int max_state;
    short state_machine;
    cQueue *txQueue;
    cChannel * txChannel;
    cMessage *sent;
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
    state_machine = idle;
    txQueue = NULL;
    txChannel = NULL;
    sent = NULL;
    max_state = -1;
}

fisico::~fisico() {
    cancelAndDelete(sent);

}

void fisico::initialize(){
    /*recoger parámetros*/
    max_state = par("QueueLength");

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
            cPacket *pk = rx->decapsulate();
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
                        sprintf(msgname,"cola llena-%d - %d",txQueue->length(),max_state);
                        bubble(msgname);
                        delete(pk);
                    }else{
                        /*todavía hay sitio*/
                        txQueue->insert(pk);
                    }
                }else{
                    /*no hay limite*/
                    txQueue->insert(pk);
                }

            }
            delete(rx);
        }else{
            /*externo, subir a la capa superior*/
            send(msg,"up_out");
        }
    }
}

void fisico::send_out(cPacket * pk){
    /*enviar por out*/
    send(pk,"out");
    state_machine = sending;
    /*programar el auto mensaje para cambair de estado*/
    simtime_t txFinishTime = txChannel->getTransmissionFinishTime();
    scheduleAt(txFinishTime,sent);

}
