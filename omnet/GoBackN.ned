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

network GoBackN
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
        gbn: free_gbn {
            @display("p=45,107");
            Ack_Tam = 1;
        }
        fisico: fisico {
            @display("p=45,190");
        }
        fisico1: fisico {
            @display("p=539,190");
        }
        gbn1: free_gbn {
            @display("p=539,107");
            Ack_Tam = 1;
        }
        dump: dump {
            @display("p=539,33");
        }
    connections:
        injector.down_out --> gbn.up_in;

        gbn.down_out --> fisico.up_in;
        fisico.up_out --> gbn.down_in;

        gbn1.down_out --> fisico1.up_in;
        fisico1.up_out --> gbn1.down_in;

        fisico.out --> C --> fisico1.in;
        fisico1.out --> C --> fisico.in;

        gbn1.up_out --> dump.down_in;

}
