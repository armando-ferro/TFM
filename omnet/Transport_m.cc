//
// Generated file, do not edit! Created by nedtool 4.6 from Transport.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "Transport_m.h"

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

Register_Class(Transport);

Transport::Transport(const char *name, int kind) : ::cPacket(name,kind)
{
    this->seq_var = 0;
    this->type_var = 0;
    this->srcAddr_var = 0;
    this->dstAddr_var = 0;
}

Transport::Transport(const Transport& other) : ::cPacket(other)
{
    copy(other);
}

Transport::~Transport()
{
}

Transport& Transport::operator=(const Transport& other)
{
    if (this==&other) return *this;
    ::cPacket::operator=(other);
    copy(other);
    return *this;
}

void Transport::copy(const Transport& other)
{
    this->seq_var = other.seq_var;
    this->type_var = other.type_var;
    this->srcAddr_var = other.srcAddr_var;
    this->dstAddr_var = other.dstAddr_var;
}

void Transport::parsimPack(cCommBuffer *b)
{
    ::cPacket::parsimPack(b);
    doPacking(b,this->seq_var);
    doPacking(b,this->type_var);
    doPacking(b,this->srcAddr_var);
    doPacking(b,this->dstAddr_var);
}

void Transport::parsimUnpack(cCommBuffer *b)
{
    ::cPacket::parsimUnpack(b);
    doUnpacking(b,this->seq_var);
    doUnpacking(b,this->type_var);
    doUnpacking(b,this->srcAddr_var);
    doUnpacking(b,this->dstAddr_var);
}

unsigned int Transport::getSeq() const
{
    return seq_var;
}

void Transport::setSeq(unsigned int seq)
{
    this->seq_var = seq;
}

unsigned short Transport::getType() const
{
    return type_var;
}

void Transport::setType(unsigned short type)
{
    this->type_var = type;
}

int Transport::getSrcAddr() const
{
    return srcAddr_var;
}

void Transport::setSrcAddr(int srcAddr)
{
    this->srcAddr_var = srcAddr;
}

int Transport::getDstAddr() const
{
    return dstAddr_var;
}

void Transport::setDstAddr(int dstAddr)
{
    this->dstAddr_var = dstAddr;
}

class TransportDescriptor : public cClassDescriptor
{
  public:
    TransportDescriptor();
    virtual ~TransportDescriptor();

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

Register_ClassDescriptor(TransportDescriptor);

TransportDescriptor::TransportDescriptor() : cClassDescriptor("Transport", "cPacket")
{
}

TransportDescriptor::~TransportDescriptor()
{
}

bool TransportDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<Transport *>(obj)!=NULL;
}

const char *TransportDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int TransportDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 4+basedesc->getFieldCount(object) : 4;
}

unsigned int TransportDescriptor::getFieldTypeFlags(void *object, int field) const
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
    };
    return (field>=0 && field<4) ? fieldTypeFlags[field] : 0;
}

const char *TransportDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "seq",
        "type",
        "srcAddr",
        "dstAddr",
    };
    return (field>=0 && field<4) ? fieldNames[field] : NULL;
}

int TransportDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='s' && strcmp(fieldName, "seq")==0) return base+0;
    if (fieldName[0]=='t' && strcmp(fieldName, "type")==0) return base+1;
    if (fieldName[0]=='s' && strcmp(fieldName, "srcAddr")==0) return base+2;
    if (fieldName[0]=='d' && strcmp(fieldName, "dstAddr")==0) return base+3;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *TransportDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "unsigned int",
        "unsigned short",
        "int",
        "int",
    };
    return (field>=0 && field<4) ? fieldTypeStrings[field] : NULL;
}

const char *TransportDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
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

int TransportDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    Transport *pp = (Transport *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string TransportDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    Transport *pp = (Transport *)object; (void)pp;
    switch (field) {
        case 0: return ulong2string(pp->getSeq());
        case 1: return ulong2string(pp->getType());
        case 2: return long2string(pp->getSrcAddr());
        case 3: return long2string(pp->getDstAddr());
        default: return "";
    }
}

bool TransportDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    Transport *pp = (Transport *)object; (void)pp;
    switch (field) {
        case 0: pp->setSeq(string2ulong(value)); return true;
        case 1: pp->setType(string2ulong(value)); return true;
        case 2: pp->setSrcAddr(string2long(value)); return true;
        case 3: pp->setDstAddr(string2long(value)); return true;
        default: return false;
    }
}

const char *TransportDescriptor::getFieldStructName(void *object, int field) const
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

void *TransportDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    Transport *pp = (Transport *)object; (void)pp;
    switch (field) {
        default: return NULL;
    }
}


