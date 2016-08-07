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


simple fisico
{
    parameters:
        @display("i=block/ifcard");
        
        @signal[rcvBit](type="int");
        @statistic[rcv_Bit](title="rcvBit"; source="rcvBit"; record=vector);
        
        @signal[rcvPkt](type="int");
        @statistic[rcv_Pkt](title="rcvPkt"; source="rcvPkt"; record=vector);
        
        @signal[sndBit](type="int");
        @statistic[rcv_Bit](title="rcvBit"; source="rcvBit"; record=vector);
        
        @signal[sndPkt](type="int");
        @statistic[rcv_Pkt](title="rcvPkt"; source="rcvPkt"; record=vector);
        
        @signal[lostPkt](type="int");
        @statistic[lost_Pkt](title="lostPkt"; source="lostPkt"; record=vector);
        
        @signal[errorPkt](type="int");
        @statistic[error_Pkt](title="errorPkt"; source="errorPkt"; record=vector);

        int Queue_Length = default(-1);
        int Header_Tam = default(0);
    gates:
        output out @loose;
        input in @loose;
        output up_out @loose;
        input up_in @loose;
}

simple free_sw
{
    parameters:
        @display("i=block/queue");
        
        int Addr = default(0);
        int Header_Tam = default(0);
        int Ack_Tam = default(1);
        int Time_Out = default(300);
        int Queue_Tam = default(-1);

        @signal[rcvACK](type="int");
        @statistic[rcv_ACK](title="rcvACK"; source="rcvACK"; record=vector);

        @signal[sndACK](type="int");
        @statistic[snd_ACK](title="sndACK"; source="sndACK"; record=vector);

        @signal[rcvNACK](type="int");
        @statistic[rcv_NACK](title="rcvNACK"; source="rcvNACK"; record=vector);

        @signal[sndNACK](type="int");
        @statistic[snd_NACK](title="sndNACK"; source="sndNACK"; record=vector);
    gates:
        output down_out;
        input down_in;
        output up_out;
        input up_in;
}

simple free_gbn
{
    parameters:
        int ack_tam = default(1)
        
        @display("i=block/queue");

        @signal[rcvACK](type="int");
        @statistic[rcv_ACK](title="rcvACK"; source="rcvACK"; record=vector);

        @signal[sndACK](type="int");
        @statistic[snd_ACK](title="sndACK"; source="sndACK"; record=vector);

        @signal[rcvNACK](type="int");
        @statistic[rcv_NACK](title="rcvNACK"; source="rcvNACK"; record=vector);

        @signal[sndNACK](type="int");
        @statistic[snd_NACK](title="sndNACK"; source="sndNACK"; record=vector);
    gates:
        output down_out;
        input down_in;
        output up_out;
        input up_in;
}

simple injector
{
    parameters:
        double Mean_Time = default(70);
        int Mean_Size = default(30);
        string Time_Distribution = default("cte");
        string Size_Distribution = default("cte");
        int Dst_Addr = default(0);

        @display("i=block/source");
    gates:
        output down_out;
        input down_in;
}

simple dump
{
    parameters:
        @display("i=block/sink");

        @signal[rcvBit](type="double");
        @statistic[rcv_bit](title="rcvBit"; source="rcvBit"; record=vector);        
        
        @signal[rcvPkt](type="int");
        @statistic[rcv_Pkt](title="rcvPkt"; source="rcvPkt"; record=vector);
    gates:
        output down_out;
        input down_in;
}

simple senderSW
{
    parameters:
        int Addr = default(0);
        int Header_Tam = default(0);
        
        @signal[rcvACK](type="int");
        @statistic[rcv_ACK](title="rcvACK"; source="rcvACK"; record=vector);
        
        @signal[rcvNACK](type="int");
        @statistic[rcv_NACK](title="rcvNACK"; source="rcvNACK"; record=vector);
        
        @signal[QueueState](type="int");
        @statistic[Queue_State](title="QueueState"; source="QueueState"; record=vector);
        
        @display("i=block/queue");
        
    gates:
        input up_in;
        output up_out;
        input in;
        output out;
}

simple mux
{
    parameters:
        @display("i=block/join");
    gates:
        output out;
        input in[];

}

module mux3_1
{
    parameters:
        int Out_Queue_Length = default(-1);
        @display("bgb=240,240");
    gates:
        output out;
        input in1;
        input in2;
        input in3;
    submodules:
        m_in1: fisico {

            @display("p=50,60");
        }

        m_in2: fisico {
            @display("p=50,120");
        }

        m_in3: fisico {
            @display("p=50,170");
        }
        mux: mux {
            @display("p=120,120");
        }
        m_out: fisico {
            @display("p=190,120");
        }
    connections:
        in1 --> m_in1.in;
        in2 --> m_in2.in;
        in3 --> m_in3.in;
        m_in1.up_out --> mux.in++;
        m_in2.up_out --> mux.in++;
        m_in3.up_out --> mux.in++;
        mux.out --> m_out.up_in;
        m_out.out --> out;
}

