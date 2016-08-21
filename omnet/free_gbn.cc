#include <string.h>
#include <string>
#include <omnetpp.h>
#include <simtime.h>
#include "Types.h"
#include "Packet_m.h"
#include "Inter_layer_m.h"

/*Estados*/
const short idle = 0;
const short resending = 1;

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
    virtual void arrivedPacket(Packet * pk);
    virtual void newPacket(inter_layer * pk);
    virtual void send_Nack(int s_seq,int dest);
    virtual void send_Ack(int s_seq,int dest);
    virtual void send_pk(Packet * pk);
    virtual void send_up(Packet * pk);
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
    /*comprobar si es el propio*/
    if(msg == time){
        /*reenvio*/
        rptQueue();
    }
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
                for(int i = 0;i<ack_acum;i++){
                    inter_layer *il = (inter_layer *)ackQueue->pop();
                    delete(il);
                }
                ack_seq = seq;
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
    }
}


void free_gbn::rptQueue()
{
    EV << " Rpt";
    state_machine = resending;
    /*repetir todos los mensajes de la cola*/
    int rpt;
    rpt = 0;
    while(rpt < ackQueue->length()){
        inter_layer * il = (inter_layer *)ackQueue->get(rpt++);
        inter_layer * copy = (inter_layer *) il->dup();
        send(copy,"down_out");
    }
    /*comprobar que no hay más paquete que enviar o enviarlos*/
    Packet *pk;
    while(!txQueue->isEmpty()){
        pk = (Packet *)txQueue->pop();
        send_pk(pk);
    }
    state_machine = idle;
}

void free_gbn::newPacket(inter_layer *pk){
    /*Mensaje de la capa superior extraer origen y destino y el cuerpo*/
    EV << " New Packet";
    //int og = pk -> getOrigen();
    int ds = pk -> getDestino();
    cPacket *body = (cPacket *) pk->decapsulate();

    /*Crear el paquete y encapsular*/
    Packet * sp = new Packet("free_gbn",0);
    //sp->setSrcAddr(origen);
    //sp->setDestAddr(ds);
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

void free_gbn::arrivedPacket(Packet *pk){
    /*recivido nuevo mensaje externo*/
    EV << " Arrived Packet";
    int r_seq = pk->getSeq();
    //int dest = pk->getSrcAddr();
    if (pk->hasBitError())
    {
        EV << " Con error";
        //paquete con error
        bubble("message error");
        if(r_seq==(rcv_seq+1)){
            //el que se esperaba
            //send_Nack(r_seq,dest);
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
            //send_Ack(++rcv_seq,dest);
            send_up(pk);
            /*VIEJO*/
        }else {
            /*En caso de recivir una secuencia menor o mayor se envia un ACK de la última (ACK acumulado)*/
            //send_Ack(rcv_seq,dest);
        }
     }
}

void free_gbn::send_Nack(int s_seq,int dest){
    /*crear el Packet NACK el inter_layer y mandarlo*/
    EV << " Send Nack";

    /*creación del NACK*/
    char msgname[20];
    sprintf(msgname,"NACK-%d",s_seq);
    Packet * pkt = new Packet(msgname,1);
    pkt->setBitLength(ack_tam);
    pkt->setType(t_nack_t);
    pkt->setSeq(s_seq);
    //pkt->setSrcAddr(origen);
    //pkt->setDestAddr(dest);

    /*preparar el inter layer y mandarlo*/
    sprintf(msgname,"il_free_gbn_NACK-%d",s_seq);
    inter_layer * il  = new inter_layer(msgname,0);
    il->setOrigen(0);
    il->setDestino(0);
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
    Packet * pkt = new Packet(msgname,0);
    pkt->setBitLength(ack_tam);
    pkt->setType(t_ack_t);
    pkt->setSeq(s_seq);
    //pkt->setSrcAddr(origen);
    //pkt->setDestAddr(dest);

    /*preparar el inter layer y mandarlo*/
    sprintf(msgname,"il_free_gbn_ACK-%d",s_seq);
    inter_layer *il  = new inter_layer(msgname,0);
    il->setOrigen(0);
    il->setDestino(0);
    il->setBitLength(0);
    il->encapsulate(pkt);

    /*enviar il*/
    send(il,"down_out");
    emit(s_sndAck,++sndAck);
}
void free_gbn::send_pk(Packet * pk){
    EV << " Sen pk";
   /* poner secuencia rellenar inter_layer y mandar*/
    /*crear copia en message*/
    pk->setSeq(++sent_seq);
    char msgname[20];
    sprintf(msgname,"il_free_gbn-%d",sent_seq);
    inter_layer *il = new inter_layer(msgname,0);
    il->setOrigen(0);
    il->setDestino(0);
    il->setBitLength(0);
    il->encapsulate(pk);
    ackQueue->insert(il);
    inter_layer *snd = il->dup();
    send(snd,"down_out");
    state_machine = idle;
}

void free_gbn::send_up(Packet * pk){
    /*Desencapsular y mandar*/
    Packet *up = (Packet * )pk->decapsulate();
    send(up,"up_out");
}
