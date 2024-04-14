#ifndef NETWORKUNTIL_H
#define NETWORKUNTIL_H

class NetWorkUntil
{
public:
    static NetWorkUntil& getInstance();
    void synchronize();
private:
    NetWorkUntil();
    ~NetWorkUntil();

};

#endif // NETWORKUNTIL_H
