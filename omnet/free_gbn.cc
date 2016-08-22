#include <string.h>
#include <string>
#include <omnetpp.h>
#include <simtime.h>
#include "Types.h"
#include "Transport_m.h"
#include "Inter_layer_m.h"

/*Estados*/
const short idle = 0;
const short full_w = 1;

/*Control de flujo free_gbn*/

class free_gbn : public cSimpleModule
{
  private:
    /*gestion de secuencia*/
    int sent_seq,ack_seq;
    int rcv_seq;
    cMessage * time;
    /*parámetros de paquete*/
    int header_tam;
    int ack_tam;
    int origen;
    int time_out;
    int window_tam;
    int queue_tam;
    /*variables de estadísticas*/
    int rcvAck;
    int sndAck;
    int rcvNack;
    int sndNack;
    /*maquina de estados*/
    short state_machine;
    /*colas*/
    cQueue *txQueue;
    cQueue *ackQueue;
    /*señales de extraccion de datos*/
    simsignal_t s_rcvAck;
    simsignal_t s_sndAck;
    simsignal_t s_rcvNack;
    simsignal_t s_sndNack;
    simsignal_t s_queueTam;
    simsignal_t s_sndWindow;

  public:
    free_gbn();
    virtual ~free_gbn();

  protected:
    /*Funciones*/
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void rptQueue();
    virtual void arrivedPacket(Transport * pk);
    virtual void newPacket(inter_layer * pk);
    virtual void send_Nack(int s_seq,int dest);
    virtual void send_Ack(int s_seq,int dest);
    virtual void send_pk(Transport * pk);
    virtual void send_up(Transport * pk);
};

// The module class needs to be registered with OMNeT++
Define_Module(free_gbn);

/*Constructor*/
free_gbn::free_gbn()
{
    sent_seq = 0;
    rcv_seq = 0;
    ack_seq = 0;
    time = NULL;
    header_tam = 0;
    ack_tam = 1;
    origen = 0;
    state_machine = idle;
    queue_tam = -1;
    window_tam = -1;
    /*colas*/
    txQueue = NULL;
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
    s_queueTam = 0;
    s_sndWindow = 0;

}

/*Destructor*/
free_gbn::~free_gbn()
{
    txQueue->~cQueue();
    ackQueue->~cQueue();
    cancelAndDelete(time);
}

/*Función de inicialización*/
void free_gbn::initialize()
{
    /*Parámetros*/
        if(par("Addr").containsValue()){
            origen  = par("Addr");
            if(origen < 100){
               bubble("dirección origen no válida");
               finish();
               endSimulation();
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
        if(par("Window_Tam").containsValue()){
            window_tam = par("Window_Tam");
        }
    /*Cola de mensajes a enviar*/
    txQueue = new cQueue("txQueue");
    ackQueue = new cQueue("ackQueue");

    time = new cMessage("time_out",0);

    /*suscribir señales*/
    s_rcvAck = registerSignal("rcvACK");
    s_sndAck = registerSignal("sndACK");
    s_rcvNack = registerSignal("rcvNACK");
    s_sndNack = registerSignal("sndNACK");
    s_queueTam = registerSignal("queueTam");
    s_sndWindow = registerSignal("sndWindow");

    WATCH(sent_seq);
    WATCH(ack_seq);
    WATCH(rcv_seq);
    WATCH(state_machine);
    WATCH(sndAck);
    WATCH(rcvAck);
    WATCH(sndNack);
    WATCH(rcvNack);
}

/*Recepción de mensajes*/
void free_gbn::handleMessage(cMessage *msg)
{
    EV << "Handle Message";
    /*comprobar si es el propio*/
    if(msg == time){
        /*reenvio*/
        EV << " Time out";
        bubble("Time out");
        rptQueue();
        return;
    }
    inter_layer *pk = check_and_cast<inter_layer *>(msg);
    if(msg->arrivedOn("up_in")){
        /*paquete de la capa superior*/
        EV << " Capa Superior";
        newPacket(pk);
        delete(pk);
    }
    else{
        EV << " Capa inferior";
        Transport *up = (Transport *) pk->decapsulate();
        if(up->getDstAddr()!=origen){
            bubble("Destinatario erróneo");
            return;
        }
        /*Paquete de la capa inferior*/
        int seq;
        switch(up->getType()){
        case t_nack_t:
            /*nack*/
            EV << " NACK";
            /*comprobar la secuencia*/
            seq = up->getSeq();
            if(seq == (ack_seq+1)){
                /*no se ha perdido ningún ACK*/
                rptQueue();
            }else if(seq > (ack_seq+1)){
                /*se han perdido ACKs por el camino se consider un ACK acumulado (erronea la seq recivida)*/
                for(int i = 0;i>((seq-1)-ack_seq);i++){
                    inter_layer *il = (inter_layer *)ackQueue->pop();
                    delete(il);
                }
                ack_seq=seq-1;
                rptQueue();
            }
            emit(s_rcvNack,++rcvNack);
            break;
        case t_ack_t:
            /*ack*/
            EV << " ACK";
            /*extraer secuencia y comprobar*/
            seq = up->getSeq();
            /*comprobar si es un ack acumulado*/
            int ack_acum;
            ack_acum = seq - ack_seq;
            if(ack_acum >= 1){
                emit(s_rcvAck,++rcvAck);
                for(int i = 0;i<ack_acum;i++){
                    inter_layer *il = (inter_layer *)ackQueue->pop();
                    delete(il);
                }
                ack_seq = seq;
                /*temporizador*/
                cancelEvent(time);
                if(ackQueue->length()>0){
                    scheduleAt(simTime()+time_out,time);
                }
                /*Comprobar estado*/
                if(state_machine == full_w){
                    /*comprobar cola*/
                    if(txQueue->isEmpty()){
                        state_machine = idle;
                    }else{
                        Transport *tx ;
                        while(ackQueue->length()<window_tam && txQueue->length()>0){
                            tx = (Transport *)txQueue->pop();
                            send_pk(tx);
                        }
                        if(ackQueue->length()<window_tam){
                            state_machine = idle;
                        }else{
                            state_machine = full_w;
                        }
                    }
                }
            }
            /*Si el ACK es menor se obvia, ya se ha gestinado*/
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
        delete(pk);
    }
}


void free_gbn::rptQueue()
{
    EV << " Rpt";
    /*repetir todos los mensajes de la cola*/
    int rpt;
    rpt = 0;
    while(rpt < ackQueue->length()){
        inter_layer * il = (inter_layer *)ackQueue->get(rpt++);
        inter_layer * copy = (inter_layer *) il->dup();
        send(copy,"down_out");
    }
    /*Reponer temporizador*/
    cancelEvent(time);
    scheduleAt(simTime()+time_out,time);
    /*comprobar que no se ha llegado al límite y si no se ha llegado si no hay más paquetes*/
    Transport *tx ;
    while(ackQueue->length()<window_tam && txQueue->length()>0){
        tx = (Transport *)txQueue->pop();
        send_pk(tx);
    }
    if(ackQueue->length()<window_tam){
        state_machine = idle;
    }else{
        state_machine = full_w;
    }
}

void free_gbn::newPacket(inter_layer *pk){
    /*Mensaje de la capa superior extraer origen y destino y el cuerpo*/
    EV << " New Packet";
    //int og = pk -> getOrigen();
    int ds = pk -> getDestino();
    cPacket *body = (cPacket *) pk->decapsulate();

    /*Crear el paquete y encapsular*/
    Transport * sp = new Transport("free_gbn",0);
    sp->setSrcAddr(origen);
    sp->setDstAddr(ds);
    sp->setBitLength(header_tam);
    sp->encapsulate(body);

    /*comprobar mandar o encolar*/
    if(state_machine == idle){
        /*no hay paquetes, mandar*/
        send_pk(sp);
    }
    else{
        /*mandando, encolar*/
        if(queue_tam<0){
            /*no hay límites*/
            txQueue->insert(sp);
        }else{
            if(txQueue->length()>=queue_tam){
                bubble("Cola llena");
                delete(sp);
            }
            else{
                txQueue->insert(sp);
            }
        }
    }
}

void free_gbn::arrivedPacket(Transport *pk){
    /*recivido nuevo mensaje externo*/
    EV << " Arrived Packet";
    int r_seq = pk->getSeq();
    int dest = pk->getSrcAddr();
    if (pk->hasBitError())
    {
        EV << " Con error";
        //paquete con error
        bubble("message error");
        if(r_seq==(rcv_seq+1)){
            //el que se esperaba
            send_Nack(r_seq,dest);
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
            send_Ack(++rcv_seq,dest);
            send_up(pk);
        }else {
            /*En caso de recivir una secuencia menor o mayor se envia un ACK de la última (ACK acumulado)*/
            send_Ack(rcv_seq,dest);
        }
     }
}

void free_gbn::send_Nack(int s_seq,int dest){
    /*crear el Packet NACK el inter_layer y mandarlo*/
    EV << " Send Nack";

    /*creación del NACK*/
    char msgname[20];
    sprintf(msgname,"NACK-%d",s_seq);
    Transport * pkt = new Transport(msgname,1);
    pkt->setBitLength(ack_tam);
    pkt->setType(t_nack_t);
    pkt->setSeq(s_seq);
    pkt->setSrcAddr(origen);
    pkt->setDstAddr(dest);

    /*preparar el inter layer y mandarlo*/
    sprintf(msgname,"il_free_gbn_NACK-%d",s_seq);
    inter_layer * il  = new inter_layer(msgname,0);
    il->setOrigen(origen);
    il->setDestino(dest%10);
    il->setBitLength(0);
    il->encapsulate(pkt);

    /*enviar il*/
    send(il,"down_out");
    emit(s_sndNack,++sndNack);
}

void free_gbn::send_Ack(int s_seq, int dest)
{
    EV << " Send Ack";
    /*crear el Packet NACK el inter_layer y mandarlo*/

    /*creación del ACK*/
    char msgname[20];
    sprintf(msgname,"ACK-%d",s_seq);
    Transport * pkt = new Transport(msgname,0);
    pkt->setBitLength(ack_tam);
    pkt->setType(t_ack_t);
    pkt->setSeq(s_seq);
    pkt->setSrcAddr(origen);
    pkt->setDstAddr(dest);

    /*preparar el inter layer y mandarlo*/
    sprintf(msgname,"il_free_gbn_ACK-%d",s_seq);
    inter_layer *il  = new inter_layer(msgname,0);
    il->setOrigen(origen);
    il->setDestino(dest%10);
    il->setBitLength(0);
    il->encapsulate(pkt);

    /*enviar il*/
    send(il,"down_out");
    emit(s_sndAck,++sndAck);
}
void free_gbn::send_pk(Transport * pk){
    EV << " Sen pk";
   /* poner secuencia rellenar inter_layer y mandar*/
    /*crear copia en message*/
    pk->setSeq(++sent_seq);
    char msgname[20];
    sprintf(msgname,"il_free_gbn-%d",sent_seq);
    inter_layer *il = new inter_layer(msgname,0);
    il->setOrigen(origen);
    il->setDestino((pk->getDstAddr())%10);
    il->encapsulate(pk);
    /*Preparar el temporizador si es necesario*/
    if(ackQueue->length()==0){
        /*primer paquete, no hay evento, preparar*/
        scheduleAt(simTime()+time_out,time);
    }
    ackQueue->insert(il);
    inter_layer *snd = il->dup();
    send(snd,"down_out");

    if(ackQueue->length()<window_tam){
        state_machine = idle;
    }else{
        state_machine = full_w;
    }
}

void free_gbn::send_up(Transport * pk){
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
