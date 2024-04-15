#ifndef NETWORKUNTIL_H
#define NETWORKUNTIL_H

#include <QTcpSocket>
#include <QObject>


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
    //销毁连接
    void logout();
private:
    QTcpSocket *tcpSocket=nullptr;
    bool status;
private:
    NetWorkUntil();
    ~NetWorkUntil();
    void initTcp();
public slots:
    void handleTcpSocketReadyRead();
};

#endif // NETWORKUNTIL_H
