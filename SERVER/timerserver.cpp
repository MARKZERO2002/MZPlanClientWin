#include "timerserver.h"

#include <DATA/datauntil.h>

#include <SHOW/mzplanclientwin.h>
#include <QGuiApplication>
TimerServer &TimerServer::getInstance()
{
    static TimerServer instance;
    return instance;
}

void TimerServer::resetTimer()
{
    //根据Data中的今日计划列表 设置定时器
    QDateTime currentDateTime=QDateTime::currentDateTime();//加点误差，因为执行该函数时可能在合理时间之前
    QDateTime nextDay=QDateTime(currentDateTime.date().addDays(1),QTime(0,0,0));
    int minD=currentDateTime.secsTo(nextDay);//到第二天0点的时间
    //遍历所有计划找到里当前时间的开始时间、提醒时间、结束时间最近的那个计划，存入nowTimerPlan中
    this->nowTimerPlan=nullptr;
    this->timeType=NEXT_DAY;
    for(const auto& plan:DataUntil::getInstance().currentPlans){
        if(!isNotFinishPlan(plan))//已完成计划
            continue;
        //这个计划不是一次性的且不满足规则
        if(plan->rule!="once"&&!plan->isConfirmRule(currentDateTime.date()))
            continue;
        //取最近的那个目标时间
        //如果是有规则的 把计划的日期改为今天
        QDateTime beginTime=plan->startTime,remindTime=plan->reminderTime,endTime=plan->endTime;
        if(plan->rule!="once"){
            beginTime.setDate(currentDateTime.date());
            remindTime.setDate(currentDateTime.date());
            endTime.setDate(currentDateTime.date());
        }
        QList<QDateTime> dts={beginTime,remindTime,endTime};
        for(int i=0;i<dts.size();i++){
            int D=currentDateTime.secsTo(dts[i]);//当前时间到目标时间的秒数，目标时间在后则为正数，不足1s按0s
            if(D>0&&minD>D){
                this->timeType=TimeType(i);//0是开始时间 1是提醒时间 2是结束时间
                minD=D;
                this->nowTimerPlan=plan;
            }
        }
    }
    this->timer->start(minD*1000);//ms=s*1000
    QTime time=QTime(0,0,0).addSecs(minD);
    if(nullptr!=this->nowTimerPlan)
        qDebug()<<"为"<<this->nowTimerPlan->content<<"设定定时器，倒计时:"<<time.toString("hh:mm:ss");
    else
        qDebug()<<"为第二天零点设置定时器，倒计时："<<time.toString("hh:mm:ss");
}

void TimerServer::removeremindDialog(Remind *rDialog)
{
    remindDialogQueue.removeOne(rDialog);
    delete rDialog;
}

TimerServer::TimerServer(QObject *parent)
    :QObject(parent)
{
    this->timer=new QTimer(this);
    this->timer->stop();
    this->timer->setTimerType(Qt::PreciseTimer);//设置定时器精度
    DataUntil::getInstance().reloadCurrentPlan();
    //定时器到点后要重绘界面、给用户发出提醒
    connect(this->timer,SIGNAL(timeout()),this,SLOT(handleTimeout()));
}

TimerServer::~TimerServer()
{
    for(auto r:this->remindDialogQueue){
        delete r;
        r=nullptr;
    }
    this->remindDialogQueue.clear();
}

void TimerServer::remind()
{
    if(this->nowTimerPlan==nullptr)
        return;
    //检查当前的弹窗到没到三个
    if(this->remindDialogQueue.size()==3){
        //删掉一个最先的
        auto dialog=this->remindDialogQueue.dequeue();//dequeue会移除队列中的元素
        dialog->close();
        delete dialog;
    }
    //判断要发出什么类型的弹窗
    Remind *remindDialog;
    switch(this->timeType){
    case BEGIN_TIME:
        //发出一个新开始弹窗
        remindDialog=new Remind("计划开始","你有一个计划开始啦："+this->nowTimerPlan->content,nullptr);
        this->remindDialogQueue.enqueue(remindDialog);
        remindDialog->show();
        break;
    case REMIND_TIME:
        //发出一个新提醒弹窗
        remindDialog=new Remind("计划到达提醒时间","你所定下的计划到达提醒时间啦："+this->nowTimerPlan->content,nullptr);
        this->remindDialogQueue.enqueue(remindDialog);
        remindDialog->show();
        break;
    case DEAD_TIME:
        //发出一个计划未完成窗
        remindDialog=new Remind("计划未完成","噢！你有一个计划没有完成，快来检查是否已经完成："+this->nowTimerPlan->content,nullptr);
        this->remindDialogQueue.enqueue(remindDialog);
        remindDialog->show();
        break;
    default:
        break;
    }
    //把弹窗移动到相应位置
    //计算每个弹窗的位置 每个弹窗大小为300*150 第一个弹窗应该是在可用窗口宽-300高-150的位置,第二个是高-300
    int x,y;
    QScreen *screen=QGuiApplication::primaryScreen();
    QRect availableSize=screen->availableGeometry();//可用屏幕，不包括任务栏
    QRect fullSize=screen->geometry();
    x=availableSize.width()-300;
    y=availableSize.height()-(fullSize.height()-availableSize.height());
    for(int i=0;i<this->remindDialogQueue.size();i++){
        y-=150;
        this->remindDialogQueue.at(i)->move(x,y);
    }
}

bool TimerServer::isNotFinishPlan(Plan *plan)
{
    //在已完成表中找不到则为不存在
    for(auto donePlan:DataUntil::getInstance().currentDonePlans){
        if(donePlan->createdTime==plan->createdTime)
            return false;
    }
    return true;
}

//定时器到点后进入这个函数
void TimerServer::handleTimeout()
{
    //定时器总在目标时间前500ms进入该函数,等待1000ms,为了精准度
    // qDebug()<<QDateTime::currentDateTime();
    QTimer::singleShot(1000, this, &TimerServer::onTimeout);
}

void TimerServer::onTimeout()
{
    // qDebug()<<QDateTime::currentDateTime();
    //发出提醒消息
    if(this->timeType!=NEXT_DAY)
        this->remind();
    //如果当前选择时间是今日，则更新主界面
    if(MZPlanClientWin::getInstance().choiceDate==QDate::currentDate())
        MZPlanClientWin::getInstance().updateInterface();
    //重新计算定时器
    this->timer->stop();
    this->resetTimer();
}
