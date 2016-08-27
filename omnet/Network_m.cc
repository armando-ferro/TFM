//
// Generated file, do not edit! Created by nedtool 4.6 from Network.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "Network_m.h"

USING_NAMESPACE


// Another default rule (prevents compiler from choosing base class' doPacking())
template<typename T>
void doPacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doPacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}

template<typename T>
void doUnpacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doUnpacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}




// Template rule for outputting std::vector<T> types
template<typename T, typename A>
inline std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec)
{
    out.put('{');
    for(typename std::vector<T,A>::const_iterator it = vec.begin(); it != vec.end(); ++it)
    {
        if (it != vec.begin()) {
            out.put(','); out.put(' ');
        }
        out << *it;
    }
    out.put('}');
    
    char buf[32];
    sprintf(buf, " (size=%u)", (unsigned int)vec.size());
    out.write(buf, strlen(buf));
    return out;
}

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
inline std::ostream& operator<<(std::ostream& out,const T&) {return out;}

Register_Class(Network);

Network::Network(const char *name, int kind) : ::cPacket(name,kind)
{
    this->srcAddr_var = 0;
    this->dstAddr_var = 0;
    this->hopCount_var = 0;
    this->hopLimit_var = 0;
    this->protocol_var = 0;
}

Network::Network(const Network& other) : ::cPacket(other)
{
    copy(other);
}

Network::~Network()
{
}

Network& Network::operator=(const Network& other)
{
    if (this==&other) return *this;
    ::cPacket::operator=(other);
    copy(other);
    return *this;
}

void Network::copy(const Network& other)
{
    this->srcAddr_var = other.srcAddr_var;
    this->dstAddr_var = other.dstAddr_var;
    this->hopCount_var = other.hopCount_var;
    this->hopLimit_var = other.hopLimit_var;
    this->protocol_var = other.protocol_var;
}

void Network::parsimPack(cCommBuffer *b)
{
    ::cPacket::parsimPack(b);
    doPacking(b,this->srcAddr_var);
    doPacking(b,this->dstAddr_var);
    doPacking(b,this->hopCount_var);
    doPacking(b,this->hopLimit_var);
    doPacking(b,this->protocol_var);
}

void Network::parsimUnpack(cCommBuffer *b)
{
    ::cPacket::parsimUnpack(b);
    doUnpacking(b,this->srcAddr_var);
    doUnpacking(b,this->dstAddr_var);
    doUnpacking(b,this->hopCount_var);
    doUnpacking(b,this->hopLimit_var);
    doUnpacking(b,this->protocol_var);
}

unsigned short Network::getSrcAddr() const
{
    return srcAddr_var;
}

void Network::setSrcAddr(unsigned short srcAddr)
{
    this->srcAddr_var = srcAddr;
}

unsigned short Network::getDstAddr() const
{
    return dstAddr_var;
}

void Network::setDstAddr(unsigned short dstAddr)
{
    this->dstAddr_var = dstAddr;
}

unsigned int Network::getHopCount() const
{
    return hopCount_var;
}

void Network::setHopCount(unsigned int hopCount)
{
    this->hopCount_var = hopCount;
}

unsigned int Network::getHopLimit() const
{
    return hopLimit_var;
}

void Network::setHopLimit(unsigned int hopLimit)
{
    this->hopLimit_var = hopLimit;
}

unsigned short Network::getProtocol() const
{
    return protocol_var;
}

void Network::setProtocol(unsigned short protocol)
{
    this->protocol_var = protocol;
}

class NetworkDescriptor : public cClassDescriptor
{
  public:
    NetworkDescriptor();
    virtual ~NetworkDescriptor();

    virtual bool doesSupport(cObject *obj) const;
    virtual const char *getProperty(const char *propertyname) const;
    virtual int getFieldCount(void *object) const;
    virtual const char *getFieldName(void *object, int field) const;
    virtual int findField(void *object, const char *fieldName) const;
    virtual unsigned int getFieldTypeFlags(void *object, int field) const;
    virtual const char *getFieldTypeString(void *object, int field) const;
    virtual const char *getFieldProperty(void *object, int field, const char *propertyname) const;
    virtual int getArraySize(void *object, int field) const;

    virtual std::string getFieldAsString(void *object, int field, int i) const;
    virtual bool setFieldAsString(void *object, int field, int i, const char *value) const;

    virtual const char *getFieldStructName(void *object, int field) const;
    virtual void *getFieldStructPointer(void *object, int field, int i) const;
};

Register_ClassDescriptor(NetworkDescriptor);

NetworkDescriptor::NetworkDescriptor() : cClassDescriptor("Network", "cPacket")
{
}

NetworkDescriptor::~NetworkDescriptor()
{
}

bool NetworkDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<Network *>(obj)!=NULL;
}

const char *NetworkDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int NetworkDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 5+basedesc->getFieldCount(object) : 5;
}

unsigned int NetworkDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
    };
    return (field>=0 && field<5) ? fieldTypeFlags[field] : 0;
}

const char *NetworkDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "srcAddr",
        "dstAddr",
        "hopCount",
        "hopLimit",
        "protocol",
    };
    return (field>=0 && field<5) ? fieldNames[field] : NULL;
}

int NetworkDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='s' && strcmp(fieldName, "srcAddr")==0) return base+0;
    if (fieldName[0]=='d' && strcmp(fieldName, "dstAddr")==0) return base+1;
    if (fieldName[0]=='h' && strcmp(fieldName, "hopCount")==0) return base+2;
    if (fieldName[0]=='h' && strcmp(fieldName, "hopLimit")==0) return base+3;
    if (fieldName[0]=='p' && strcmp(fieldName, "protocol")==0) return base+4;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *NetworkDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "unsigned short",
        "unsigned short",
        "unsigned int",
        "unsigned int",
        "unsigned short",
    };
    return (field>=0 && field<5) ? fieldTypeStrings[field] : NULL;
}

const char *NetworkDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldProperty(object, field, propertyname);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        default: return NULL;
    }
}

int NetworkDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    Network *pp = (Network *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string NetworkDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    Network *pp = (Network *)object; (void)pp;
    switch (field) {
        case 0: return ulong2string(pp->getSrcAddr());
        case 1: return ulong2string(pp->getDstAddr());
        case 2: return ulong2string(pp->getHopCount());
        case 3: return ulong2string(pp->getHopLimit());
        case 4: return ulong2string(pp->getProtocol());
        default: return "";
    }
}

bool NetworkDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    Network *pp = (Network *)object; (void)pp;
    switch (field) {
        case 0: pp->setSrcAddr(string2ulong(value)); return true;
        case 1: pp->setDstAddr(string2ulong(value)); return true;
        case 2: pp->setHopCount(string2ulong(value)); return true;
        case 3: pp->setHopLimit(string2ulong(value)); return true;
        case 4: pp->setProtocol(string2ulong(value)); return true;
        default: return false;
    }
}

const char *NetworkDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        default: return NULL;
    };
}

void *NetworkDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    Network *pp = (Network *)object; (void)pp;
    switch (field) {
        default: return NULL;
    }
}


