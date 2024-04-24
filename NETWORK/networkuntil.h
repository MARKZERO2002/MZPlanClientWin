#ifndef NETWORKUNTIL_H
#define NETWORKUNTIL_H

#include <QTcpSocket>
#include <QObject>
#include <QTimer>


class NetWorkUntil:public QObject
{
public:
    static NetWorkUntil& getInstance();
    //发送消息
    void regist(QString username,QString password);
    void login(QString username,QString password);
    void cancle();
    void synchronize(bool status=false);
    //处理收到的消息
    void handleRegist(QJsonObject data);
    void handleLogin(QJsonObject data);
    void handleCancle(QJsonObject data);
    void handleSynchronize(QJsonObject data);//status用于判断是否是程序初始化完成前发出的请求
    void handleUpdate(QJsonObject data);
    //销毁连接
    void logout();
private:
    QTcpSocket *tcpSocket=nullptr;
    bool status;//判断是不是打开软件时的同步
    QByteArray m_buffer;//接收缓冲区
    QTimer* timer=nullptr;//用于保活
private:
    NetWorkUntil();
    ~NetWorkUntil();
    bool initTcp();
public slots:
    void handleTcpSocketReadyRead();
    void sendPalpitatePacket();//发送心跳包
};

#endif // NETWORKUNTIL_H
