#ifndef TIMERSERVER_H
#define TIMERSERVER_H

#include <QObject>
#include <QQueue>
#include <QTimer>
#include <SHOW/plan.h>
#include "SHOW/remind.h"
class TimerServer : public QObject
{
    Q_OBJECT
public:
    enum TimeType{BEGIN_TIME,REMIND_TIME,DEAD_TIME,NEXT_DAY};
private:
    QTimer *timer;
    Plan *nowTimerPlan=nullptr;
    TimeType timeType=NEXT_DAY;
    QQueue<Remind*> remindDialogQueue;//弹窗队列
public:
    static TimerServer &getInstance();
    void resetTimer();
    void removeremindDialog(Remind* rDialog);//移除一个弹窗
private:
    TimerServer(QObject *parent = nullptr);
    ~TimerServer();
    void remind();
    bool isNotFinishPlan(Plan* plan);
public slots:
    void handleTimeout();//处理定时器到点
    void onTimeout();//消除误差
};

#endif // TIMERSERVER_H
