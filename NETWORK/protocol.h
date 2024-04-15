#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
//发送和接收的字段
#define MSGTYPE "msgtype"
#define DATA "data"
#define CHECK "check" //检查用户的请求是否正确，比如登陆成功为true，登陆失败为false
#define MSG_STRING "msg_string" //附带的字符消息，比如“密码错误”
#define USERNAME "username"
#define PASSWORD "password"
#define PLAN "plan"
#define DONEPLAN "donePlan"
#define MEDIFYTIME "medifyTime"
#define DB_DATA "dbData"
//客户端发来的消息类型
enum MsgType{
    //接收
    LOGIN_REQUEST,
    REGIST_REQUEST,
    CANCEL_REQUEST,
    SYNOCHRONIZE_PLAN_REQUEST,//同步计划 指客户端要求检查哪边的数据更新，把新的发回去
    //发回去
    LOGIN_RESPONSE,
    REGIST_RESPONSE,
    CANCEL_RESPONSE,
    SYNOCHRONIZE_PLAN_RESPONSE
};

struct PDU{
    PDU(){

    }
    PDU(QByteArray byteData){
        //从byteData中读出消息类型
        QJsonParseError e;
        QJsonDocument jsDoc=QJsonDocument::fromJson(byteData,&e);
        if(e.error!=QJsonParseError::NoError){
            qDebug()<<"(protocol.h)解析传输数据时失败:"<<e.errorString();
            return;
        }
        //根据消息类型从byteData中读出数据，存入jsonDoc中
        QJsonObject jsObj=jsDoc.object();
        this->msgType=(MsgType)jsObj.value(MSGTYPE).toInt();
        this->data=jsObj.value(DATA).toObject();
    }
    QByteArray toByteArray(){
        //把数据变为QByteArray
        QJsonObject obj;
        obj.insert(MSGTYPE,this->msgType);
        obj.insert(DATA,this->data);
        QJsonDocument doc=QJsonDocument(obj);
        return doc.toJson();
    }
    MsgType msgType;//消息类型
    QJsonObject data;//数据
};

#endif // PROTOCOL_H
