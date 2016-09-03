#include <string.h>
#include <string>
#include <omnetpp.h>
#include <simtime.h>
#include "Types.h"
#include "Transport_m.h"
#include "Inter_layer_m.h"

/*Estados*/
const short idle = 0;
const short w_ack = 2;

/*Control de flujo free_sw*/

class free_sw : public cSimpleModule
{
  private:
    /*gestión*/
    bool b_up,b_down;   //comprueba los enlaces up y down
    int sent_seq;
    int rcv_seq;
    cQueue *txQueue;
    inter_layer * message;
    cMessage *time;
    /*parámetros*/
    int time_out;
    int header_tam;
    int ack_tam;
    int origen;
    int queue_tam;
    bool config;    //comprueba la dirección
    /*Para señales*/
    int rcvAck;
    int sndAck;
    short state_machine;
    /*señales de extraccion de datos*/
    simsignal_t s_rcvAck;
    simsignal_t s_sndAck;
    simsignal_t s_queueTam;

  public:
    free_sw();
    virtual ~free_sw();

  protected:
    /*Funciones*/
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void sendCopyOf(inter_layer *msg);
    virtual void arrivedPacket(Transport * pk);
    virtual void newPacket(inter_layer * pk);
    virtual void send_Ack(int s_seq,int sender);
    virtual void send_pk(Transport * pk);
    virtual void send_up(Transport * pk);
};

// The module class needs to be registered with OMNeT++
Define_Module(free_sw);

/*Constructor*/
free_sw::free_sw()
{
    sent_seq = 0;
    rcv_seq = 0;
    state_machine = idle;
    txQueue = NULL;
    message = NULL;
    time = NULL;
    config = true;
    b_up = true;
    b_down = true;
    /*parámetros*/
    time_out = 300;
    header_tam = 0;
    ack_tam = 1;
    origen = 0;
    queue_tam = -1;
    /*valores de control*/
    rcvAck = 0;
    sndAck = 0;
    /*señales*/
    s_rcvAck = 0;
    s_sndAck = 0;
    s_queueTam = 0;

}

/*Destructor*/
free_sw::~free_sw()
{
    txQueue->~cQueue();
    cancelAndDelete(time);
}

/*Función de inicialización*/
void free_sw::initialize()
{
    /*Parámetros*/
    if(par("Addr").containsValue()){
        origen  = par("Addr");
        if(origen < min_trans||origen>max_trans){
            bubble("dirección origen no válida");
            config = false;
        }
    }

    if(par("Header_Tam").containsValue()){
        header_tam = par("Header_Tam");
    }

    if(par("Ack_Tam").containsValue()){
        ack_tam = par("Ack_Tam");
    }

    if(par("Time_Out").containsValue()){
        time_out = par("Time_Out");
    }

    if(par("Queue_Tam").containsValue()){
        queue_tam = par("Queue_Tam");
    }

    /*Cola de mensajes a enviar*/
    txQueue = new cQueue("txQueue");

    /*time out de reenvio*/
    time = new cMessage("time_out");

    /*suscribir señales*/
    s_rcvAck = registerSignal("rcvACK");
    s_sndAck = registerSignal("sndACK");
    s_queueTam = registerSignal("queueTam");

    if(not(gate("down_out")->isConnected())){
        b_down = false;
    }

    if(not(gate("up_out")->isConnected())){
        b_up = false;
    }

    WATCH(sent_seq);
    WATCH(state_machine);
    WATCH(sndAck);
    WATCH(rcvAck);
    WATCH(header_tam);
    WATCH(origen);
    WATCH(ack_tam);
    WATCH(time_out);

}

/*Recepción de mensajes*/
void free_sw::handleMessage(cMessage *msg)
{
    if(not(config)){
        bubble("Dirección origen errónea");
        delete(msg);
        return;
    }

    if(not(b_down)){
        bubble("Puerta de saldia no conectada");
        delete(msg);
        return;
    }

    /*comprobar si es time_out*/
    if(msg == time){
        /*remandar*/
        bubble("time_out");
        sendCopyOf(message);
    }
    else{
        if(msg->arrivedOn("up_in")){
            /*paquete de la capa superior*/
            inter_layer *pk = check_and_cast<inter_layer *>(msg);
            newPacket(pk);
            delete(pk);
            emit(s_queueTam,txQueue->length());
        }
        else{
            /*paquete de la capa inferior*/
            inter_layer *d_il = check_and_cast<inter_layer *>(msg);
            Transport * tp = (Transport *) d_il->decapsulate();
            if(tp->hasBitError()){
                bubble("Error en el paquete");
                /*imposible detectar tipo o secuencia, se elimina (se retransmitirá por time out)*/
                delete(tp);
                delete(d_il);
                return;
            }
            if(tp->getDstAddr()==origen){
                /*destinatario correcto*/
                switch(tp->getType()){
                case t_ack_t:
                    /*ack*/
                    /*extraer secuencia y comprobar*/
                    int ack_seq;
                    ack_seq = tp->getSeq();
                    if(ack_seq == sent_seq){
                        /*ACK correcto*/
                        cancelEvent(time);
                        /*comprobar cola*/
                        if(txQueue->empty()){
                            /*no hay mensajes, se espera otro*/
                            state_machine = idle;
                        }
                        else{
                            /*sacar de cola y mandar*/
                            Transport * sp  = (Transport *)txQueue->pop();
                            emit(s_queueTam,txQueue->length());
                            send_pk(sp);
                        }
                        emit(s_rcvAck,++rcvAck);
                    }
                    /*Si el ACK no es el esperado se obvia*/
                    break;
                case t_msg_t:
                    /*nuevo mensaje recivido*/
                    arrivedPacket(tp);
                    break;
                default:
                    arrivedPacket(tp);
                    break;
                }
            }else{
                /*destinatario erronea*/
                bubble("Mensaje para otro");
            }
            delete(tp);
            delete(d_il);
        }
    }
}


void free_sw::sendCopyOf(inter_layer *msg)
{
    // Duplicate message and send the copy.
    inter_layer *copy = (inter_layer *) msg->dup();
    send(copy, "down_out");
    state_machine = w_ack;
    cancelEvent(time);
    scheduleAt(simTime()+time_out,time);
}

void free_sw::newPacket(inter_layer *pk){
    /*Mensaje de la capa superior extraer origen y destino y el cuerpo*/
    //int og = pk -> getOrigen();
    int ds = pk -> getDestino();
    if(ds<min_trans||ds>max_trans){
        bubble("Dirección destino no válida");
        return;
    }
    cPacket *body;
    if(pk->hasEncapsulatedPacket()){
        body = (cPacket *) pk->decapsulate();
    }else{
        bubble("No hay paquete que mandar");
        return;
    }


    /*Crear el paquete y encapsular*/
    Transport * tp = new Transport("free_sw",0);
    tp->setSrcAddr(origen);
    tp->setDstAddr(ds);
    tp->setType(t_msg_t);
    tp->setBitLength(header_tam);
    tp->encapsulate(body);

    /*comprobar mandar o encolar*/
    if(state_machine == idle){
        /*no hay paquetes, mandar*/
        send_pk(tp);
    }
    else{
        /*mandando, encolar*/
        if(queue_tam<0){
            /*no hay limite*/
            txQueue->insert(tp);
        }else{
            /*limite, comprobar*/
            if(txQueue->length()>=queue_tam){
                /*no hay hueco*/
                bubble("Cola llena");
                delete(tp);
            }else{
                txQueue->insert(tp);
            }
        }
        emit(s_queueTam,txQueue->length());
    }
}

void free_sw::arrivedPacket(Transport *pk){
    /*recivido nuevo mensaje externo*/
    int r_seq = pk->getSeq();
    int sender = pk->getSrcAddr();
     /*paquete sin errores comprobar secuencia*/
     if(r_seq==(rcv_seq+1)){
        //paquete correcto (secuencia esperada)
        send_Ack(++rcv_seq,sender);
        send_up(pk);
    }else if(r_seq<(rcv_seq+1)){
        /*En caso de recivir una secuencia menor se envia un ACK de la última (ACK acumulado)*/
        send_Ack(rcv_seq,sender);
    }
    /*En caso de que el paquete llega desordenado (un sequencia mayor), se obvia*/
}

void free_sw::send_Ack(int s_seq,int sender)
{
    /*crear el Packet ACK el inter_layer y mandarlo*/

    /*creación del ACK*/
    char msgname[20];
    sprintf(msgname,"ACK-%d",s_seq);
    Transport * pkt = new Transport(msgname,0);
    pkt->setBitLength(ack_tam);
    pkt->setType(t_ack_t);
    pkt->setSeq(s_seq);
    pkt->setSrcAddr(origen);
    pkt->setDstAddr(sender);

    /*preparar el inter layer y mandarlo*/
    sprintf(msgname,"il_free_sw_ACK-%d",s_seq);
    inter_layer *il  = new inter_layer(msgname,0);
    il->setOrigen(origen);
    il->setDestino(sender%10);
    il->setProtocol(p_transport);
    il->setBitLength(0);
    il->encapsulate(pkt);

    /*enviar il*/
    send(il,"down_out");
    emit(s_sndAck,++sndAck);
}
void free_sw::send_pk(Transport * pk){
   /* poner secuencia rellenar inter_layer y mandar*/
    /*crear copia en message*/
    pk->setSeq(++sent_seq);
    char msgname[20];
    sprintf(msgname,"il_free_sw-%d",sent_seq);
    inter_layer *il = new inter_layer(msgname,0);
    il->setOrigen(origen);
    il->setDestino((pk->getDstAddr())%10);
    il->setProtocol(p_transport);
    il->encapsulate(pk);
    delete(message);
    message = il->dup();
    send(il,"down_out");
    state_machine = w_ack;
    /*set time out*/
    cancelEvent(time);
    scheduleAt(simTime()+time_out,time);
}

void free_sw::send_up(Transport * pk){
    if(not(b_up)){
        bubble("Puerta superior no conectada");
        delete(pk);
        return;
    }
    /*Desencapsular y subir*/
    if(pk->hasEncapsulatedPacket()){
        cPacket *up = (cPacket * )pk->decapsulate();
        int orig = pk->getSrcAddr();
        inter_layer * il = new inter_layer("TransprotILup",0);
        il->setOrigen(orig);
        il->setDestino(origen);
        il->encapsulate(up);
        send(il,"up_out");
    }
}


