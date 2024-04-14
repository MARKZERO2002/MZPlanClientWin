#include "networkuntil.h"

NetWorkUntil &NetWorkUntil::getInstance()
{
    static NetWorkUntil instance;
    return instance;
}

void NetWorkUntil::synchronize()
{

}

NetWorkUntil::NetWorkUntil() {}

NetWorkUntil::~NetWorkUntil()
{

}
