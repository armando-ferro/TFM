//
// Represents a packet in the network. Network layer packet
//
packet Network
{
    int srcAddr;
    int dstAddr;
    int hopCount;
    int hopLimit;
    unsigned short protocol;
}