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
            per = 0;
            delay = 1s;
        }
    submodules:

        demux: demux1_3 {
            @display("p=484,157");
        }
        mux: Pmux3_1 {
            @display("p=317,157");
        }
        injector: injector {
            @display("p=94,50");
        }
        injector1: injector {
            @display("p=94,151");
        }
        fisico: fisico {
            @display("p=198,50");
        }
        fisico1: fisico {
            @display("p=198,151");
        }
        dump: dump {
            @display("p=723,62");
        }
        dump1: dump {
            @display("p=723,157");
        }
        fisico2: fisico {
            @display("p=611,62");
        }
        fisico3: fisico {
            @display("p=611,157");
        }
        injector2: injector {
            @display("p=94,272");
        }
        fisico4: fisico {
            @display("p=198,272");
        }
        dump2: dump {
            @display("p=723,272");
        }
        fisico5: fisico {
            @display("p=611,272");
        }
    connections:
        injector.down_out --> fisico.up_in;
        injector1.down_out --> fisico1.up_in;
        fisico2.up_out --> dump.down_in;
        fisico3.up_out --> dump1.down_in;

        fisico.out --> C --> mux.in1;
        fisico1.out --> C --> mux.in2;

        demux.out1 --> C --> fisico2.in;
        demux.out2 --> C --> fisico3.in;

        mux.out --> C --> demux.in;


        injector2.down_out --> fisico4.up_in;
        fisico5.up_out --> dump2.down_in;
        fisico4.out --> C --> mux.in3;
        demux.out3 --> C --> fisico5.in;
}
