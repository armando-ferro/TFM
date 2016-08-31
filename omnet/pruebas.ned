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
    @display("bgb=810,360");
    types:
        channel C extends ned.DatarateChannel
        {
            datarate = 1bps;
            per = 0.1;
            delay = 1s;
        }
    submodules:

        injector: injector {
            @display("p=145,113");
        }
        dump: dump {
            @display("p=352,113");
        }
        senderSW: senderGBN {
            @display("p=145,220");
            Queue_Length = 5;
        }
        receiverACK: receiverACK {
            @display("p=352,220");
        }
    connections:
        injector.down_out --> senderSW.up_in;
        senderSW.out --> C --> receiverACK.in;
        receiverACK.up_out --> dump.down_in;
        receiverACK.out --> C --> senderSW.in;
}
