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


network Pruebas
{
    @display("bgb=826,265");
    types:
        channel C extends ned.DatarateChannel
        {
            datarate = 1bps;
            per = 0.1;
            delay = 1s;
        }
    submodules:
        injector: injector {
            @display("p=67,33");
            pktSize = exponential(70bit);
            delayTime = 30s;
            n_Pkt = 1000;
        }
        dump: dump {
            @display("p=510,33");
        }
        dump1: dump {
            @display("p=165,33");
        }
        injector1: injector {
            @display("p=610,33");
            n_Pkt = 1000;
        }
        receiverACK: receiverACK {
            @display("p=510,136");
        }
        receiverACK1: receiverACK {
            @display("p=165,194;b=7,5");
        }
        senderGBN: senderGBN {
            @display("p=67,136");
        }
        senderSW: senderSW {
            @display("p=610,194");
        }
    connections:
        
        injector.down_out-->senderGBN.up_in;
        senderGBN.out-->C-->receiverACK.in;
        receiverACK.out-->C-->senderGBN.in;
        receiverACK.up_out-->dump.down_in;
        
        injector1.down_out-->senderSW.up_in;
        senderSW.out-->C-->receiverACK1.in;
        receiverACK1.out-->C-->senderSW.in;
        receiverACK1.up_out-->dump1.down_in;
        
}
