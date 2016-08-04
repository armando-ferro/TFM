#include <string.h>
#include <string>
#include <omnetpp.h>
#include <simtime.h>
#include "Types.h"
#include "Packet_m.h"
#include "Inter_layer_m.h"

/*Estados*/
const short idle = 0;
const short w_ack = 2;

/*Control de flujo free_sw*/

class free_sw : public cSimpleModule
{
  private:
    int sent_seq;
    int rcv_seq;
    int header_tam;
    int ack_tam;
    int origen;
    int destino;
    int rcvAck;
    int sndAck;
    int rcvNack;
    int sndNack;
    short state_machine;
    cQueue *txQueue;
    inter_layer * message;
    /*señales de extraccion de datos*/
    simsignal_t s_rcvAck;
    simsignal_t s_sndAck;
    simsignal_t s_rcvNack;
    simsignal_t s_sndNack;

  public:
    free_sw();
    virtual ~free_sw();

  protected:
    /*Funciones*/
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void sendCopyOf(inter_layer *msg);
    virtual void arrivedPacket(Packet * pk);
    virtual void newPacket(inter_layer * pk);
    virtual void send_Nack(int s_seq);
    virtual void send_Ack(int s_seq);
    virtual void send_pk(Packet * pk);
    virtual void send_up(Packet * pk);
};

// The module class needs to be registered with OMNeT++
Define_Module(free_sw);

/*Constructor*/
free_sw::free_sw()
{
    sent_seq = 0;
    rcv_seq = 0;
    header_tam = 0;
    ack_tam = 1;
    origen = 0;
    destino = 1;
    state_machine = idle;
    txQueue = NULL;
    message = NULL;
    /*valores de control*/
    rcvAck = 0;
    sndAck = 0;
    rcvNack = 0;
    sndNack = 0;
    /*señales*/
    s_rcvAck = 0;
    s_sndAck = 0;
    s_rcvNack = 0;
    s_sndNack = 0;

}

/*Destructor*/
free_sw::~free_sw()
{
    txQueue->~cQueue();
}

/*Función de inicialización*/
void free_sw::initialize()
{
    /*Cola de mensajes a enviar*/
    txQueue = new cQueue("txQueue");

    /*suscribir señales*/
    s_rcvAck = registerSignal("rcvACK");
    s_sndAck = registerSignal("sndACK");
    s_rcvNack = registerSignal("rcvNACK");
    s_sndNack = registerSignal("sndNACK");

    WATCH(sent_seq);
    WATCH(state_machine);
    WATCH(sndAck);
    WATCH(rcvAck);
    WATCH(sndNack);
    WATCH(rcvNack);
}

/*Recepción de mensajes*/
void free_sw::handleMessage(cMessage *msg)
{
    EV << "Handle Message";
    if(msg->arrivedOn("up_in")){
        /*paquete de la capa superior*/
        EV << "Capa Superior";
        inter_layer *pk = check_and_cast<inter_layer *>(msg);
        newPacket(pk);
        delete(pk);
    }
    else{
        EV << " Capa inferior";
        Packet *up = check_and_cast<Packet *>(msg);
        /*Paquete de la capa inferior*/
        switch(up->getType()){
        case t_nack_t:
            /*nack*/
            EV << " NACK";
            /*reenviar*/
            sendCopyOf(message);
            emit(s_rcvNack,++rcvNack);
            break;
        case t_ack_t:
            /*ack*/
            EV << " ACK";
            /*extraer secuencia y comprobar*/
            int ack_seq;
            ack_seq = up->getSeq();
            if(ack_seq == sent_seq){
                /*ACK correcto*/
                /*comprobar cola*/
                if(txQueue->empty()){
                    /*no hay mensajes, se espera otro*/
                    state_machine = idle;
                }
                else{
                    /*sacar de cola y mandar*/
                    Packet * sp  = (Packet *)txQueue->pop();
                    send_pk(sp);
                }
                emit(s_rcvAck,++rcvAck);
            }
            /*Si el ACK no es el esperado se obvia*/
            break;
        case t_msg_t:
            /*nuevo mensaje recivido*/
            EV << " Nuevo mensaje";
            arrivedPacket(up);
            break;
        default:
            EV << " Default";
            arrivedPacket(up);
            break;
        }
        delete(up);
    }
}


void free_sw::sendCopyOf(inter_layer *msg)
{
    EV << " Send Copy";
    // Duplicate message and send the copy.
    inter_layer *copy = (inter_layer *) msg->dup();
    send(copy, "down_out");
    state_machine = w_ack;
}

void free_sw::newPacket(inter_layer *pk){
    /*Mensaje de la capa superior extraer origen y destino y el cuerpo*/
    EV << " New Packet";
    //int og = pk -> getOrigen();
    //int ds = pk -> getDestino();
    cPacket *body = (cPacket *) pk->decapsulate();

    /*Crear el paquete y encapsular*/
    Packet * sp = new Packet("free_sw",0);
    sp->setSrcAddr(origen);
    sp->setDestAddr(destino);
    sp->setBitLength(header_tam);
    sp->encapsulate(body);

    /*comprobar mandar o encolar*/
    if(state_machine == idle){
        /*no hay paquetes, mandar*/
        send_pk(sp);
    }
    else{
        /*mandando, encolar*/
        txQueue->insert(sp);
    }
}

void free_sw::arrivedPacket(Packet *pk){
    /*recivido nuevo mensaje externo*/
    EV << " Arrived Packet";
    int r_seq = pk->getSeq();
    if (pk->hasBitError())
    {
        EV << " Con error";
        //paquete con error
        bubble("message error");
        if(r_seq==(rcv_seq+1)){
            //el que se esperaba
            send_Nack(r_seq);
        }
        /*Si es menor ya se ha recivido correctametne y se obvia*/
        /*Si es mayor que el esperado se obvia*/
     }
     else
     {
         EV << " Sin error";
         /*paquete sin errores comprobar secuencia*/
         if(r_seq==(rcv_seq+1)){
            //paquete correcto (secuencia esperada)
            send_Ack(++rcv_seq);
            send_up(pk);
            /*VIEJO*/
        }else if(r_seq<(rcv_seq+1)){
            /*En caso de recivir una secuencia menor se envia un ACK de la última (ACK acumulado)*/
            send_Ack(rcv_seq);
        }
        /*En caso de que el paquete llega desordenado (un sequencia mayor), se obvia*/
     }
}

void free_sw::send_Nack(int s_seq){
    /*crear el Packet NACK el inter_layer y mandarlo*/
    EV << " Send Nack";

    /*creación del NACK*/
    char msgname[20];
    sprintf(msgname,"NACK-%d",s_seq);
    Packet * pkt = new Packet(msgname,1);
    pkt->setBitLength(ack_tam);
    pkt->setType(t_nack_t);
    pkt->setSeq(s_seq);
    pkt->setSrcAddr(origen);
    pkt->setDestAddr(destino);

    /*preparar el inter layer y mandarlo*/
    sprintf(msgname,"il_free_sw_NACK-%d",s_seq);
    inter_layer * il  = new inter_layer(msgname,0);
    il->setOrigen(0);
    il->setDestino(0);
    il->setBitLength(0);
    il->encapsulate(pkt);

    /*enviar il*/
    send(il,"down_out");
    emit(s_sndNack,++sndNack);
}

void free_sw::send_Ack(int s_seq)
{
    EV << " Send Ack";
    /*crear el Packet NACK el inter_layer y mandarlo*/

    /*creación del ACK*/
    char msgname[20];
    sprintf(msgname,"ACK-%d",s_seq);
    Packet * pkt = new Packet(msgname,0);
    pkt->setBitLength(ack_tam);
    pkt->setType(t_ack_t);
    pkt->setSeq(s_seq);
    pkt->setSrcAddr(origen);
    pkt->setDestAddr(destino);

    /*preparar el inter layer y mandarlo*/
    sprintf(msgname,"il_free_sw_ACK-%d",s_seq);
    inter_layer *il  = new inter_layer(msgname,0);
    il->setOrigen(0);
    il->setDestino(0);
    il->setBitLength(0);
    il->encapsulate(pkt);

    /*enviar il*/
    send(il,"down_out");
    emit(s_sndAck,++sndAck);
}
void free_sw::send_pk(Packet * pk){
    EV << " Sen pk";
   /* poner secuencia rellenar inter_layer y mandar*/
    /*crear copia en message*/
    pk->setSeq(++sent_seq);
    char msgname[20];
    sprintf(msgname,"il_free_sw-%d",sent_seq);
    inter_layer *il = new inter_layer(msgname,0);
    il->setOrigen(0);
    il->setDestino(0);
    il->setBitLength(0);
    il->encapsulate(pk);
    delete(message);
    message = il->dup();
    send(il,"down_out");
    state_machine = w_ack;
}

void free_sw::send_up(Packet * pk){
    /*Desencapsular y mandar*/
    Packet *up = (Packet * )pk->decapsulate();
    send(up,"up_out");
}


