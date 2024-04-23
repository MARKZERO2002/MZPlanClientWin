#include "networkuntil.h"

#include "protocol.h"

#include <DATA/datauntil.h>

#include <QMessageBox>
#include <QSettings>

#include <SHOW/mzplanclientwin.h>

#include <SERVER/timerserver.h>

NetWorkUntil &NetWorkUntil::getInstance()
{
    static NetWorkUntil instance;
    return instance;
}

void NetWorkUntil::regist(QString username,QString password)
{
    this->initTcp();
    if(!this->tcpSocket)
        return;
    QJsonObject data;
    data.insert(USERNAME,username);
    data.insert(PASSWORD,password);
    this->tcpSocket->write(createSendData(REGIST_REQUEST,data));
    if(!this->tcpSocket->waitForReadyRead(5000)){
        //连接失败 说明网络不好 销毁并且设置同步信号为false
        this->tcpSocket->disconnectFromHost();
    }
}

void NetWorkUntil::login(QString username,QString password)
{
    this->initTcp();
    if(!this->tcpSocket)
        return;
    QJsonObject data;
    data.insert(USERNAME,username);
    data.insert(PASSWORD,password);
    this->tcpSocket->write(createSendData(LOGIN_REQUEST,data));
    if(!this->tcpSocket->waitForReadyRead(5000)){
        //连接失败 说明网络不好 销毁并且设置同步信号为false
        this->tcpSocket->disconnectFromHost();
    }
}

void NetWorkUntil::cancle()
{
    this->initTcp();
    if(!this->tcpSocket)
        return;
    QJsonObject data;
    data.insert(USERNAME,DataUntil::getInstance().username);
    this->tcpSocket->write(createSendData(CANCEL_REQUEST,data));
    if(!this->tcpSocket->waitForReadyRead(5000)){
        //连接失败 说明网络不好 销毁并且设置同步信号为false
        this->tcpSocket->disconnectFromHost();
    }
}

void NetWorkUntil::synchronize(bool status)
{
    this->status=status;
    this->initTcp();
    if(!this->tcpSocket)
        return;
    QJsonObject data;
    QString dbdata=DataUntil::getInstance().getDbData().toBase64();
    data.insert(DB_DATA,dbdata);
    data.insert(USERNAME,DataUntil::getInstance().username);
    data.insert(MEDIFYTIME,DataUntil::getInstance().medifyTime);
    this->tcpSocket->write(createSendData(SYNOCHRONIZE_PLAN_REQUEST,data));
    if(!this->tcpSocket->waitForReadyRead(5000)){
        //连接失败 说明网络不好 销毁并且设置同步信号为false
        this->tcpSocket->disconnectFromHost();
    }
}

void NetWorkUntil::handleRegist(QJsonObject data)
{
    //收到了服务器传来的消息
    bool flag=data.value(CHECK).toBool();
    if(flag){
        //注册成功
        QMessageBox::information(&MZPlanClientWin::getInstance(),"注册","注册成功");
    }else{
        //注册失败
        QMessageBox::critical(&MZPlanClientWin::getInstance(),"注册","注册失败:"+data.value(MSG_STRING).toString());
    }
}

void NetWorkUntil::handleLogin(QJsonObject data)
{
    //收到了服务器传来的消息
    bool flag=data.value(CHECK).toBool();
    if(flag){
        //登陆成功
        //切换用户
        DataUntil::getInstance().changeUser(data.value(USERNAME).toString());
        DataUntil::getInstance().updateUser();
        //切换页面
        MZPlanClientWin::getInstance().on_userBtn_clicked();
    }else{
        //登陆失败
        QMessageBox::critical(&MZPlanClientWin::getInstance(),"登陆","登陆失败:"+data.value(MSG_STRING).toString());
    }
}

void NetWorkUntil::handleCancle(QJsonObject data)
{
    //收到了服务器传来的消息
    bool flag=data.value(CHECK).toBool();
    if(flag){
        //注销成功
        //删除当前用户的所有文件 在更换用户前进行删除
        DataUntil::getInstance().removeUserDir();
        //切换到本地用户
        DataUntil::getInstance().changeUser("local");
        DataUntil::getInstance().updateUser();
        //切换页面
        MZPlanClientWin::getInstance().on_userBtn_clicked();
        //登出
        this->logout();
        QMessageBox::information(&MZPlanClientWin::getInstance(),"注销","注销成功");
    }else{
        QMessageBox::critical(&MZPlanClientWin::getInstance(),"注销","注销失败:"+data.value(MSG_STRING).toString());
    }
}

void NetWorkUntil::handleSynchronize(QJsonObject data)
{
    if(data.contains(MEDIFYTIME)){
        //覆盖本地文件 覆盖之前要记得断开数据库
        QByteArray dbdata=QByteArray::fromBase64(data.value(DB_DATA).toString().toUtf8());
        DataUntil::getInstance().writeDbData(dbdata);
        //更新时间
        DataUntil::getInstance().updateMedifyTime(data.value(MEDIFYTIME).toString());
        //如果是打开软件时的同步，则主界面类还没初始化，不能执行下面语句
        if(!this->status){
            //更新计划数据
            QDate date=MZPlanClientWin::getInstance().choiceDate;
            DataUntil::getInstance().reloadChoicePlan(date);
            DataUntil::getInstance().reloadCurrentPlan();
            //刷新界面
            MZPlanClientWin::getInstance().updateInterface();
        }
    }
    qDebug()<<data.value(MSG_STRING).toString();
}

void NetWorkUntil::handleUpdate(QJsonObject data)
{
    //打开同步请求的才做修改
    if(DataUntil::getInstance().isSynchronize){
        QString newMedifyTime=data.value(MEDIFYTIME).toString();
        //如果本地的信息旧则更新
        if(DataUntil::getInstance().medifyTime<newMedifyTime){
            QByteArray dbdata=QByteArray::fromBase64(data.value(DB_DATA).toString().toUtf8());
            DataUntil::getInstance().writeDbData(dbdata);
            DataUntil::getInstance().updateMedifyTime(newMedifyTime);
            //更新计划数据
            QDate date=MZPlanClientWin::getInstance().choiceDate;
            MZPlanClientWin::getInstance().clearPlanList();
            DataUntil::getInstance().reloadChoicePlan(date);
            DataUntil::getInstance().reloadCurrentPlan();
            //刷新界面
            MZPlanClientWin::getInstance().updateInterface();
        }
    }
}

void NetWorkUntil::logout()
{
    if(this->tcpSocket&&this->tcpSocket->state()==QAbstractSocket::ConnectedState){
        this->tcpSocket->disconnectFromHost();
    }
}

NetWorkUntil::NetWorkUntil()
    :QObject(nullptr)
{
}

NetWorkUntil::~NetWorkUntil()
{

}

void NetWorkUntil::initTcp()
{
    if(!this->tcpSocket){
        this->tcpSocket=new QTcpSocket();
        //从文件中读取配置
        QSettings settings(DataUntil::getInstance().systemConfigPath,QSettings::IniFormat);
        QString address=settings.value("address").toString();
        int port=settings.value("port").toInt();
        qDebug()<<"连接到："<<address<<" "<<port;
        this->tcpSocket->connectToHost(QHostAddress(address),port);
        //连接信号
        connect(this->tcpSocket,&QTcpSocket::disconnected,[this](){
            //失去连接就删除该对象
            this->tcpSocket->deleteLater();
            this->tcpSocket=nullptr;
            DataUntil::getInstance().setSynchronize(false);
            MZPlanClientWin::getInstance().setSynchronize(false);
        });
        connect(this->tcpSocket,&QTcpSocket::readyRead,this,&NetWorkUntil::handleTcpSocketReadyRead);//读数据
        QTimer::singleShot(2000,this,[this](){
            if(this->tcpSocket->state()!=QAbstractSocket::ConnectedState){
                QMessageBox::warning(&MZPlanClientWin::getInstance(),"网络连接","连接服务器失败，请检查网络或联系管理员");
                MZPlanClientWin::getInstance().setSynchronize(false);
                DataUntil::getInstance().setSynchronize(false);
                this->tcpSocket->deleteLater();
                this->tcpSocket=nullptr;
            }
        });
    }
}

void NetWorkUntil::handleTcpSocketReadyRead()
{
    while(this->tcpSocket&&this->tcpSocket->bytesAvailable()){
        //存入缓冲区
        this->m_buffer.append(this->tcpSocket->readAll());
        //查看是否收到完整头
        if(this->m_buffer.size()<static_cast<qsizetype>(sizeof(quint32)*2)){//因为协议的头是由两个32int组成的
            qDebug()<<"没接收到完整头";
            return;
        }
        //查看是否接受到完整数据
        PduHearder header=deserializeHeader(this->m_buffer.left(sizeof(quint32)*2));//用两个int32组成头部

        if(this->m_buffer.mid(sizeof(quint32)*2).size()<header.length){//如果当前缓冲区的数据去掉头，长度不与length一致
            qDebug()<<"没接收到完整消息";
            return;
        }
        QByteArray jsData=this->m_buffer.mid(sizeof(quint32)*2,header.length);
        this->m_buffer.remove(0,sizeof(quint32)*2+header.length);//移除缓冲区数据
        qDebug()<<"接收到完整数据，开始处理";
        //组建pdu
        Pdu pdu;
        pdu.header=header;
        pdu.data=QJsonDocument::fromJson(jsData).object();
        switch(pdu.header.msgType){
        case REGIST_RESPONSE:
            this->handleRegist(pdu.data);
            break;
        case LOGIN_RESPONSE:
            this->handleLogin(pdu.data);
            break;
        case CANCEL_RESPONSE:
            this->handleCancle(pdu.data);
            break;
        case SYNOCHRONIZE_PLAN_RESPONSE:
            this->handleSynchronize(pdu.data);
            break;
        case UPDATE:
            this->handleUpdate(pdu.data);
            break;
        default:
            qDebug()<<"未知消息";
            break;
        }
    }
}
