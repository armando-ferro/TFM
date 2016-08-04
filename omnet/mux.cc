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
#include "Packet_m.h"
#include "Inter_layer_m.h"


class mux : public cSimpleModule {
private:
    /*variables*/
public:
    mux();
    virtual ~mux();
protected:
    virtual void handleMessage(cMessage *msg);
    virtual void initialize();

};

Define_Module(mux);


mux::mux() {


}

mux::~mux() {

}

void mux::initialize(){

}

void mux::handleMessage(cMessage *msg){
    /*Los paquetes que entran salen por out no es bidireccional*/
    /*empaqueter y mandar*/
    Packet *pk = check_and_cast<Packet *>(msg);
    inter_layer *il = new inter_layer();
    il->encapsulate(pk);
    send(il,"out");
}

