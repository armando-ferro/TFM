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
        @statistic[snd_Bit](title="sndBit"; source="sndBit"; record=vector);

        @signal[sndPkt](type="int");
        @statistic[snd_Pkt](title="sndPkt"; source="sndPkt"; record=vector);

        @signal[lostPkt](type="int");
        @statistic[lost_Pkt](title="lostPkt"; source="lostPkt"; record=vector);

        @signal[errorPkt](type="int");
        @statistic[error_Pkt](title="errorPkt"; source="errorPkt"; record=vector);

        @signal[QueueState](type="int");
        @statistic[Queue_State](title="QueueState"; source="QueueState"; record=vector);

        int Queue_Length = default(-1);
        int Header_Tam = default(0);
    gates:
        output out @loose;
        input in @loose;
        output up_out @loose;
        input up_in @loose;
}

simple simple_fisico
{
    parameters:
        @display("i=block/ifcard");

        @signal[rcvBit](type="int");
        @statistic[rcv_Bit](title="rcvBit"; source="rcvBit"; record=vector);

        @signal[rcvPkt](type="int");
        @statistic[rcv_Pkt](title="rcvPkt"; source="rcvPkt"; record=vector);

        @signal[sndBit](type="int");
        @statistic[snd_Bit](title="sndBit"; source="sndBit"; record=vector);

        @signal[sndPkt](type="int");
        @statistic[snd_Pkt](title="sndPkt"; source="sndPkt"; record=vector);
    gates:
        output out @loose;
        input in @loose;
        output up_out @loose;
        input up_in @loose;
}


simple senderSW
{
    parameters:

        int Header_Tam = default(0);
        int Queue_Length = default(-1);

        @signal[rcvACK](type="int");
        @statistic[rcv_ACK](title="rcvACK"; source="rcvACK"; record=vector);

        @signal[rcvNACK](type="int");
        @statistic[rcv_NACK](title="rcvNACK"; source="rcvNACK"; record=vector);

        @signal[QueueState](type="int");
        @statistic[Queue_State](title="QueueState"; source="QueueState"; record=vector);

        @signal[sndBit](type="int");
        @statistic[snd_Bit](title="sndBit"; source="sndBit"; record=vector);

        @signal[sndPkt](type="int");
        @statistic[snd_Pkt](title="sndPkt"; source="sndPkt"; record=vector);

        @signal[lostPkt](type="int");
        @statistic[lost_Pkt](title="lostPkt"; source="lostPkt"; record=vector);

        @display("i=block/queue");

    gates:
        input up_in;
        input in;
        output out;
}

simple senderGBN
{
    parameters:

        int Header_Tam = default(0);
        int Queue_Length = default(-1);
        int Window_Length = default(-1);

        @signal[rcvACK](type="int");
        @statistic[rcv_ACK](title="rcvACK"; source="rcvACK"; record=vector);

        @signal[rcvNACK](type="int");
        @statistic[rcv_NACK](title="rcvNACK"; source="rcvNACK"; record=vector);

        @signal[QueueState](type="int");
        @statistic[Queue_State](title="QueueState"; source="QueueState"; record=vector);

        @signal[WindowTam](type="int");
        @statistic[Window_Tam](title="WindowTam"; source="WindowTam"; record=vector);

        @signal[sndBit](type="int");
        @statistic[snd_Bit](title="sndBit"; source="sndBit"; record=vector);

        @signal[sndPkt](type="int");
        @statistic[snd_Pkt](title="sndPkt"; source="sndPkt"; record=vector);

        @signal[lostPkt](type="int");
        @statistic[lost_Pkt](title="lostPkt"; source="lostPkt"; record=vector);

        @display("i=block/queue");

    gates:
        input up_in;
        input in;
        output out;
}

simple receiverACK
{
    parameters:
        @signal[sndACK](type="int");
        @statistic[snd_ACK](title="sndACK"; source="sndACK"; record=vector);

        @signal[sndNACK](type="int");
        @statistic[snd_NACK](title="sndNACK"; source="sndNACK"; record=vector);

        @signal[rcvBit](type="int");
        @statistic[rcv_Bit](title="rcvBit"; source="rcvBit"; record=vector);

        @signal[rcvPkt](type="int");
        @statistic[rcv_Pkt](title="rcvPkt"; source="rcvPkt"; record=vector);

        int Ack_Tam = default(1);

        @display("i=block/arrival");
    gates:
        input in;
        output out;
        output up_out;

}

simple router
{
    parameters:
        @display("i=block/routing");
        int Addr = default(00);
        int Hop_Limit = default(10);
        int Header_Tam = default(0);
        xml Config_File = default(xmldoc("pr.xml"));

        @signal[errorPkt](type="int");
        @statistic[error_Pkt](title="errorPkt"; source="errorPkt"; record=vector);

        @signal[rcvBit](type="int");
        @statistic[rcv_Bit](title="rcvBit"; source="rcvBit"; record=vector);

        @signal[rcvPkt](type="int");
        @statistic[rcv_Pkt](title="rcvPkt"; source="rcvPkt"; record=vector);

        @signal[forwardedBit](type="int");
        @statistic[forwarded_Bit](title="forwardedBit"; source="forwardedBit"; record=vector);

        @signal[forwardedPkt](type="int");
        @statistic[forwarded_Pkt](title="forwardedPkt"; source="forwardedPkt"; record=vector);

    gates:
        output up_out[10] @loose;
        input up_in[10] @loose;
        output down_out[10] @loose;
        input down_in[10] @loose;

}

simple free_sw
{
    parameters:
        @display("i=block/queue");

        int Addr = default(100);
        int Header_Tam = default(0);
        int Ack_Tam = default(1);
        int Time_Out = default(300);
        int Queue_Tam = default(-1);

        @signal[rcvACK](type="int");
        @statistic[rcv_ACK](title="rcvACK"; source="rcvACK"; record=vector);

        @signal[sndACK](type="int");
        @statistic[snd_ACK](title="sndACK"; source="sndACK"; record=vector);

        @signal[queueTam](type="int");
        @statistic[queue_Tam](title="queueTam"; source="queueTam"; record=vector);

    gates:
        output down_out;
        input down_in;
        output up_out @loose;
        input up_in @loose;
}

simple free_gbn
{
    parameters:
        int Addr = default(100);
        int Header_Tam = default(0);
        int Ack_Tam = default(1);
        int Queue_Tam = default(-1);
        int Window_Tam = default(3);
        int Time_Out = default(300);

        @display("i=block/queue");

        @signal[rcvACK](type="int");
        @statistic[rcv_ACK](title="rcvACK"; source="rcvACK"; record=vector);

        @signal[sndACK](type="int");
        @statistic[snd_ACK](title="sndACK"; source="sndACK"; record=vector);

        @signal[queueTam](type="int");
        @statistic[queue_Tam](title="queueTam"; source="queueTam"; record=vector);

        @signal[sndWindow](type="int");
        @statistic[snd_Window](title="sndWindow"; source="sndWindow"; record=vector);

    gates:
        output down_out;
        input down_in;
        output up_out @loose;
        input up_in @loose;
}

simple injector
{
    parameters:
        int Dst_Addr = default(0);
        int n_Pkt = default(-1);
        volatile double delayTime @unit(s) = default(30s);   // delay before sending back message
        volatile double pktSize @unit(bit) = default(40bit);


        @signal[pktTam](type=int);
        @statistic[pkt_tam](title="pktTam"; source="pktTam"; record=vector);

        @display("i=block/source");
    gates:
        output down_out;
}

simple dump
{
    parameters:
        @display("i=block/sink");

        @signal[rcvBit](type="double");
        @statistic[rcv_bit](title="rcvBit"; source="rcvBit"; record=vector);

        @signal[rcvPkt](type="int");
        @statistic[rcv_Pkt](title="rcvPkt"; source="rcvPkt"; record=vector);

        @signal[bitThroughput](type="long");
        @statistic[bit_Throughput](title="bitThroughput"; source="bitThroughput"; record=vector);

        @signal[pktThroughput](type="long");
        @statistic[pkt_Throughput](title="pktThroughput"; source="pktThroughput"; record=vector);
    gates:
        input down_in;
}

simple mux
{
    parameters:
        @display("i=block/join");
        xml config = default(xmldoc("mux.xml"));
        int Header_Tam = default(0);
    gates:
        output out;
        input in[] @loose;

}

simple mux_priority
{
    parameters:
        @display("i=block/join");
        xml config = default(xmldoc("mux.xml"));
        int Header_Tam = default(0);
    gates:
        output out;
        input in[] @loose;

}

simple demux
{
    parameters:
        @display("i=block/join");
        xml config = default(xmldoc("demux.xml"));
        
        @signal[rcvBit](type="double");
        @statistic[rcv_bit](title="rcvBit"; source="rcvBit"; record=vector);

        @signal[rcvPkt](type="int");
        @statistic[rcv_Pkt](title="rcvPkt"; source="rcvPkt"; record=vector);
    gates:
        output out[];
        input in;

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
    connections:
        in1 --> m_in1.in;
        in2 --> m_in2.in;
        in3 --> m_in3.in;
        m_in1.up_out --> mux.in++;
        m_in2.up_out --> mux.in++;
        m_in3.up_out --> mux.in++;
        mux.out --> out;
}

module Pmux3_1
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
        mux: mux_priority {
            @display("p=120,120");
        }
    connections:
        in1 --> m_in1.in;
        in2 --> m_in2.in;
        in3 --> m_in3.in;
        m_in1.up_out --> mux.in++;
        m_in2.up_out --> mux.in++;
        m_in3.up_out --> mux.in++;
        mux.out --> out;
}

module demux1_3
{
    parameters:
        int Out_Queue_Length = default(-1);
        @display("bgb=240,240");
    gates:
        output out1;
        output out2;
        output out3;
        input in;
    submodules:
        m_out1: fisico {

            @display("p=50,60");
        }

        m_out2: fisico {
            @display("p=50,120");
        }

        m_out3: fisico {
            @display("p=50,170");
        }
        demux: demux {
            @display("p=120,120");
        }
    connections:
        in --> demux.in;
        demux.out++ --> m_out1.up_in;
        demux.out++ --> m_out2.up_in;
        demux.out++ --> m_out3.up_in;
        m_out1.out --> out1;
        m_out2.out --> out2;
        m_out3.out --> out3;
}


module InOut2SW
{
    parameters:
        int Addr = default(00);
        xml Config_File = default(xmldoc("pr.xml"));
        int L_Queue_Length = default(-1);
        int T_Queue_Length = default(-1);
        int T_Time_Out = default(100);
        int Dst_Addr = default(100);
        int n_Pkt = default(-1);
        int L_Header_Tam = default(0);
        int N_Header_Tam = default(0);
        int T_Header_Tam = default(0);
        int T_Ack_Tam = default(0);
        int N_Hop_Limit = default(10);

        volatile double delayTime @unit(s) = default(30s);   // delay before sending back message
        volatile double pktSize @unit(bit) = default(70bit);
    gates:
        input in1 @loose;
        input in2 @loose;
        output out1 @loose;
        output out2 @loose;
    submodules:
        injector: injector {
            Dst_Addr = Dst_Addr;
            n_Pkt = n_Pkt;
            delayTime = delayTime;
            pktSize = pktSize;
            @display("p=105,58");
        }
        dump: dump {
            @display("p=259,58");
        }
        transport: free_sw {
            Addr = 100+Addr;
            Header_Tam = T_Header_Tam;
            Ack_Tam = T_Ack_Tam;
            Time_Out = T_Time_Out;
            Queue_Tam = T_Queue_Length;
            @display("p=178,151");
        }
        router: router {
            Addr = Addr;
            Hop_Limit = N_Hop_Limit;
            Header_Tam = N_Header_Tam;
            Config_File = Config_File;
            @display("p=178,244");
        }
        if1: fisico {
            Queue_Length = L_Queue_Length;
            Header_Tam = L_Header_Tam;
            @display("p=105,348");
        }
        if2: fisico {
            Queue_Length = L_Queue_Length;
            Header_Tam = L_Header_Tam;
            @display("p=259,348");
        }
    connections:

        injector.down_out --> transport.up_in;
        transport.up_out --> dump.down_in;

        transport.down_out --> router.up_in[1];
        router.up_out[1] --> transport.down_in;

        router.down_out[1] --> if1.up_in;
        if1.up_out --> router.down_in[1];

        router.down_out[2] --> if2.up_in;
        if2.up_out --> router.down_in[2];

        if1.out --> out1;
        if2.out --> out2;

        in1 --> if1.in;
        in2 --> if2.in;

}


module InOut2GBN
{
    parameters:
        int Addr = default(00);
        xml Config_File = default(xmldoc("pr.xml"));
        int L_Queue_Length = default(-1);
        int T_Queue_Length = default(-1);
        int T_Time_Out = default(100);
        int Dst_Addr = default(100);
        int n_Pkt = default(-1);
        int L_Header_Tam = default(0);
        int N_Header_Tam = default(0);
        int T_Header_Tam = default(0);
        int T_Ack_Tam = default(0);
        int N_Hop_Limit = default(10);
        int T_Window_Length = default(3);

        volatile double delayTime @unit(s) = default(30s);   // delay before sending back message
        volatile double pktSize @unit(bit) = default(70bit);
    gates:
        input in1 @loose;
        input in2 @loose;
        output out1 @loose;
        output out2 @loose;
    submodules:
        injector: injector {
            Dst_Addr = Dst_Addr;
            n_Pkt = n_Pkt;
            delayTime = delayTime;
            pktSize = pktSize;
            @display("p=105,58");
        }
        dump: dump {
            @display("p=259,58");
        }
        transport: free_gbn {
            Addr = 100+Addr;
            Header_Tam = T_Header_Tam;
            Ack_Tam = T_Ack_Tam;
            Time_Out = T_Time_Out;
            Queue_Tam = T_Queue_Length;
            Window_Tam = T_Window_Length;
            @display("p=178,151");
        }
        router: router {
            Addr = Addr;
            Hop_Limit = N_Hop_Limit;
            Header_Tam = N_Header_Tam;
            Config_File = Config_File;
            @display("p=178,244");
        }
        if1: fisico {
            Queue_Length = L_Queue_Length;
            Header_Tam = L_Header_Tam;
            @display("p=105,348");
        }
        if2: fisico {
            Queue_Length = L_Queue_Length;
            Header_Tam = L_Header_Tam;
            @display("p=259,348");
        }
    connections:

        injector.down_out --> transport.up_in;
        transport.up_out --> dump.down_in;

        transport.down_out --> router.up_in[1];
        router.up_out[1] --> transport.down_in;

        router.down_out[1] --> if1.up_in;
        if1.up_out --> router.down_in[1];

        router.down_out[2] --> if2.up_in;
        if2.up_out --> router.down_in[2];

        if1.out --> out1;
        if2.out --> out2;

        in1 --> if1.in;
        in2 --> if2.in;

}

