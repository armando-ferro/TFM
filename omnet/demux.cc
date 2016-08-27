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
#include "Mux_m.h"
#include "Inter_layer_m.h"

class demux : public cSimpleModule {
private:
    /*variables*/
    int n;
    cXMLElement * xml;
    short *line_input;    //transformación entre entrada y linea
    bool b_config;
public:
    demux();
    virtual ~demux();
protected:
    virtual void handleMessage(cMessage *msg);
    virtual void initialize();
    virtual bool config(cXMLElement * xml);

};

Define_Module(demux);


demux::demux() {
    n = -1;
    xml = NULL;
    line_input = NULL;
    b_config = true;
}

demux::~demux() {
}

void demux::initialize(){
    n = gateSize("out");
    if(par("config").containsValue()){
        xml = par("config").xmlValue();
        b_config = config(xml);
    }

    WATCH(n);
    WATCH(b_config);
}

void demux::handleMessage(cMessage *msg){
    int gate,line;
    if(not(b_config)){
        bubble("Configuración errónea");
        return;
    }
    /*solo hay una entrada*/
    Mux *mx = check_and_cast<Mux *>(msg);
    line = mx->getLine();
    gate = line_input[line];
    if(gate<0){
        bubble("Línea no asignada");
        return;
    }

    /*desencapsular, preparar inter_layer y mandar*/
    cPacket *pk = (cPacket *)mx->decapsulate();
    inter_layer *il = new inter_layer("Demux_IL",0);
    il->encapsulate(pk);
    send(il,"out",gate);
    delete(mx);

}

bool demux::config(cXMLElement * xml){
    cXMLElement *tmp;
    int i,line,tmp_line;
    short gate;

    line = 0;
    /*detectar cuantas líneas hay*/
    tmp = xml->getFirstChild();
    do{
        tmp_line = atoi(tmp->getAttribute("line"));
        if(tmp_line<0){
            EV << "La línea tiene que ser un valor positivo";
            return false;
        }
        if(tmp_line > line){
            line = tmp_line;
        }
    }while((tmp=tmp->getNextSibling())!=NULL);

    /*inicializar la línea*/
    line_input = new short[line];
    for(i=0;i<=line;i++){
        line_input[i]=-1;
    }

    /*cargar datos*/
    tmp = xml->getFirstChild();
    do{
        tmp_line = atoi(tmp->getAttribute("line"));
        if(line_input[tmp_line]>0){
            EV << "La línea ya ha sido asignada:" <<tmp_line;
            return false;
        }
        gate=atoi(tmp->getAttribute("gate"));
        if(gate<0||gate>n){
            EV << "La puerta fuera de rango (0-"<<n<<")";
            return false;
        }
        line_input[tmp_line]=gate;
    }while((tmp=tmp->getNextSibling())!=NULL);

    return true;
}

