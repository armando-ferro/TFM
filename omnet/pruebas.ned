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
    @display("bgb=621,372");
    types:
        channel C extends ned.DatarateChannel
        {
            datarate = 1bps;
            per = 0;
            delay = 1s;
        }
    submodules:

        host0: InOut2SW {
            @display("p=172,77");
            Addr = 00;
            Dst_Addr = 103;
            n_Pkt = 10;
            delayTime = 30s;
            pktSize = 20bit;
        }
        host1: InOut2SW {
            @display("p=511,77");
            Addr = 01;
            Dst_Addr = 102;
            n_Pkt = 10;
            delayTime = 30s;
            pktSize = 20bit;
        }
        host2: InOut2SW {
            @display("p=172,247");
            Addr = 02;
            Dst_Addr = 101;
            n_Pkt = 10;
            delayTime = 30s;
            pktSize = 20bit;
        }
        host3: InOut2SW {
            @display("p=511,247");
            Addr = 03;
            Dst_Addr = 100;
            n_Pkt = 10;
            delayTime = 30s;
            pktSize = 20bit;
        }
    connections:
        host0.out1 --> C --> host1.in1;
        host1.out1 --> C --> host0.in1;

        host1.out2 --> C --> host3.in2;
        host3.out2 --> C --> host1.in2;

        host3.out1 --> C --> host2.in1;
        host2.out1 --> C --> host3.in1;

        host0.out2 --> C --> host2.in2;
        host2.out2 --> C --> host0.in2;
}
