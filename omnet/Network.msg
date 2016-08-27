//
// Represents a packet in the network. Network layer packet
//
packet Network
{
    unsigned short srcAddr;
    unsigned short dstAddr;
    unsigned int hopCount;
    unsigned int hopLimit;
    unsigned short protocol;
}