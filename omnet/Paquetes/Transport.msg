//
// Represents a packet in the network. Aplication layer packet
//
packet Transport
{
    unsigned int seq;
   	unsigned short type;
    unsigned short srcAddr;
    unsigned short dstAddr;
}