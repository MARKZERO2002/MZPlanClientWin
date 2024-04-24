#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <QByteArray>
#include <QIODevice>
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
static QList<QString> MsgTypeMeans{"登录请求","注册请求","注销请求","同步请求"
                                   ,"登陆反馈","注册反馈","注销反馈","同步反馈","更新反馈"};
//客户端发来的消息类型
enum MsgType{
    //客户端发出
    LOGIN_REQUEST,
    REGIST_REQUEST,
    CANCEL_REQUEST,
    SYNOCHRONIZE_PLAN_REQUEST,//同步计划 指客户端要求检查哪边的数据更新，把新的发回去
    //服务端发出
    LOGIN_RESPONSE,
    REGIST_RESPONSE,
    CANCEL_RESPONSE,
    SYNOCHRONIZE_PLAN_RESPONSE,
    //要求更新客户端数据
    UPDATE,
    //心跳
    PALPITATE

};
struct PduHearder{
    MsgType msgType;//消息类型
    quint32 length;//消息长度 不包括头！
};

struct Pdu{
    PduHearder header;
    QJsonObject data;//数据
};



//构建发送数据
static QByteArray createSendData(const MsgType& msgType,const QJsonObject& jsObj){
    //计算消息长度
    QByteArray jsonData=QJsonDocument(jsObj).toJson();
    quint32 length=static_cast<quint32>(jsonData.size());
    //构建发送数据
    QByteArray data;
    QDataStream stream(&data,QIODevice::WriteOnly);
    stream<<static_cast<quint32>(msgType) << length;
    data+=jsonData;
    return data;
}

static PduHearder deserializeHeader(const QByteArray &data) {
    PduHearder header;
    QDataStream stream(data);
    stream >> header.msgType >> header.length;
    return header;
}


#endif // PROTOCOL_H
