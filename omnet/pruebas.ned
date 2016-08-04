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
            @display("p=45,33");
            Time_Distribution = "exp";
            Size_Distribution = "exp";
            Mean_Time = 30;
            Mean_Size = 70;
        }
        senderSW: senderSW {
            @display("p=45,190");
        }
        fisico1: fisico {
            @display("p=539,190");
        }
        gbn1: free_gbn {
            @display("p=539,117");
            ack_tam = 1;
        }
        dump: dump {
            @display("p=539,50");
        }
    connections:
        injector.down_out --> senderSW.up_in;
        senderSW.up_out --> injector.down_in;
        
        senderSW.out --> C --> fisico1.in;
        fisico1.out --> C --> senderSW.in;
        
        gbn1.down_out --> fisico1.up_in;
        fisico1.up_out --> gbn1.down_in;

        gbn1.up_out --> dump.down_in;
        dump.down_out --> gbn1.up_in;


}
