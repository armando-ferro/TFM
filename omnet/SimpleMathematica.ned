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


network MathematicaSW
{
    @display("bgb=610,164");
    types:
        channel C extends ned.DatarateChannel
        {
            datarate = 1bps;
            per = 0.1;
            delay = 1s;
            
        }
    submodules:
        injector: injector {
            @display("p=45,33");
            pktSize = exponential(10bit);
            delayTime = exponential(1s);
            Dst_Addr = 1;
            n_Pkt = 1000;
        }
        senderSW: senderSW {
            @display("p=45,107");
        }
        receiverACK: receiverACK {
            @display("p=539,107");
            Ack_Tam  = 1;
        }
        dump: dump {
            @display("p=539,33");
        }
    connections:
        injector.down_out --> senderSW.up_in;

        senderSW.out --> C --> receiverACK.in;
        receiverACK.out --> C --> senderSW.in;

        receiverACK.up_out --> dump.down_in;

}

network MathematicaGBN
{
    @display("bgb=610,164");
    types:
        channel C extends ned.DatarateChannel
        {
            datarate = 1bps;
            per = 0.1;
            delay = 1s;
        }
    submodules:
        injector: injector {
            @display("p=45,33");
            pktSize = exponential(10bit);
            delayTime = exponential(1s);
            Dst_Addr = 1;
            n_Pkt = 1000;
        }
        senderGBN: senderGBN {
            @display("p=45,107");
        }
        receiverACK: receiverACK {
            @display("p=539,107");
        }
        dump: dump {
            @display("p=539,33");
        }
    connections:
        injector.down_out --> senderGBN.up_in;

        senderGBN.out --> C --> receiverACK.in;
        receiverACK.out --> C --> senderGBN.in;

        receiverACK.up_out --> dump.down_in;

}
