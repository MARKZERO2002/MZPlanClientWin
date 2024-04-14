#include "plan.h"
#include "DATA/datauntil.h"
#include "mzplanclientwin.h"
#include "plandetail.h"
#include "ui_plan.h"

#include <QMessageBox>

QList<QString> Plan::statusStr={"未开始","进行中","未完成","已完成"};

Plan::Plan(QString createdTime, QString startTime, QString reminderTime, QString endTime, QString content, QString rule,QDate date,QString completedTime, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Plan)
{
    ui->setupUi(this);
    init(createdTime,startTime,reminderTime,endTime,content,rule,date,completedTime);
}

Plan::Plan(Plan *plan, QDateTime completedTime,QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Plan)
{
    ui->setupUi(this);
    init(plan->createdTime.toString(DTFORMAT),plan->startTime.toString(DTFORMAT),
         plan->reminderTime.toString(DTFORMAT),plan->endTime.toString(DTFORMAT),plan->content,plan->rule,QDate(),completedTime.toString(DTFORMAT));
}

Plan::Plan(Plan *plan)
    : QWidget(nullptr)
    , ui(new Ui::Plan)
{
    //除了parent其他值都复制过来
    this->createdTime=plan->createdTime;
    this->startTime=plan->startTime;
    this->reminderTime=plan->reminderTime;
    this->endTime=plan->endTime;
    this->completedTime=plan->completedTime;
    this->rule=plan->rule;
    this->content=plan->content;
}

Plan::~Plan()
{
    delete ui;
}

bool Plan::isConfirmRule(const QDate &date) const
{
    if(this->rule=="once")
        return true;
    //有规则的计划 判断是否符合规则
    QList<QString> list=this->rule.split(":");
    QString pre=list[0];
    //去除'[' ']'
    QString suf=list[1].removeAt(0);
    suf=suf.removeAt(suf.size()-1);
    list=suf.split(",");
    //匹配数字
    if(pre=="week"){//按周
        for(auto &number:list){
            if(number.toInt()==date.dayOfWeek())
                return true;
            //如果是跨天的，要看选择天数的前一天是否在规则内
            if(this->endTime.date()>this->startTime.date()&&number.toInt()==date.addDays(-1).dayOfWeek())
                return true;
        }
    }else{//按月
        for(auto &number:list){
            if(number.toInt()==date.day())
                return true;
            //如果是跨天的，要看选择天数的前一天是否在规则内
            if(this->endTime.date()>this->startTime.date()&&number.toInt()==date.addDays(-1).day())
                return true;
        }
    }
    return false;
}

void Plan::init(QString createdTime,QString startTime,QString reminderTime,QString endTime,
                QString content,QString rule,QDate date,QString completedTime)
{
    if(completedTime!=nullptr){
        this->completedTime=QDateTime::fromString(completedTime,DTFORMAT);
        this->ui->finishTimeLabel->setText(completedTime);
    }
    //存储日期
    this->createdTime=QDateTime::fromString(createdTime,DTFORMAT);
    this->content=content;
    this->rule=rule;
    //该计划是一次性的
    if(rule=="once"){
        this->startTime=QDateTime::fromString(startTime,DTFORMAT);
        this->endTime=QDateTime::fromString(endTime,DTFORMAT);
        this->reminderTime=QDateTime::fromString(reminderTime,DTFORMAT);
    }else{//非一次性的
        if(completedTime!=nullptr){//已完成的计划记录，所有字段都是DTFORMAT
            this->startTime=QDateTime::fromString(startTime,DTFORMAT);
            this->endTime=QDateTime::fromString(endTime,DTFORMAT);
            this->reminderTime=QDateTime::fromString(reminderTime,DTFORMAT);
        }else{
            this->startTime=QDateTime::fromString(startTime,TFORMAT);
            this->reminderTime=QDateTime::fromString(reminderTime,TFORMAT);
            this->endTime=QDateTime::fromString(endTime,TFORMAT);
            //有规则的 把日期设为用户当前选择的日期
            this->updateDate(date);
        }
    }
    if(completedTime!=nullptr)
        this->setDonePlanShow();
    else
        this->setPlanShow();
}

void Plan::updateDate(QDate date)
{
    //只有有规则且未完成的计划才会进入该函数
    //对于跨天的计划如果一条计划的开始日期是选择的日期，则继续运行
    //如果一条计划的开始日期是选择日期的昨天，也就是结束日期是今天，则today和tomorrow要减一天

    //把日期设为用户当前选择的日期
    //如果提醒日期的时间或结束日期的时间早于开始时间，说明提醒和结束是第二天 则设为第二天
    QDate today=date;
    QDate tomorrow=today.addDays(1);
    //判断today是不是开始日期
    this->startTime.setDate(today);
    if(this->reminderTime.time()<this->startTime.time())
        this->reminderTime.setDate(tomorrow);
    else
        this->reminderTime.setDate(today);
    if(this->endTime.time()<this->startTime.time())
        this->endTime.setDate(tomorrow);
    else
        this->endTime.setDate(today);
}

void Plan::setDonePlanShow()
{
    //已完成计划界面和普通的不一样
    this->ui->finishBtn->setVisible(false);//去除完成按钮
    //改为未完成按钮
    //显示完成时间
    this->ui->finishTimeLabel->setText(this->completedTime.toString(DTFORMAT));
    this->ui->label->show();
    this->ui->finishTimeLabel->show();
    //组件显示
    this->ui->beginTimeLabel->setText(this->startTime.toString(DTFORMAT));
    this->ui->remindTimeLabel->setText(this->reminderTime.toString(DTFORMAT));
    this->ui->endTimeLabel->setText(this->endTime.toString(DTFORMAT));
    this->ui->content->setText(this->content);
}
void Plan::setPlanShow(){
    this->ui->finishTimeLabel->setVisible(false);
    this->ui->label->setVisible(false);
    this->ui->finishBtn->show();
    //组件显示
    this->ui->beginTimeLabel->setText(this->startTime.toString(DTFORMAT));
    this->ui->remindTimeLabel->setText(this->reminderTime.toString(DTFORMAT));
    this->ui->endTimeLabel->setText(this->endTime.toString(DTFORMAT));
    this->ui->content->setText(this->content);
}
//删除计划
void Plan::on_deleteBtn_clicked()
{
    //用确认对话框询问用户确认删除
    QMessageBox::StandardButton box;
    box=QMessageBox::question(this,"提示","确定要删除吗?");
    if(box==QMessageBox::No){
        return;
    }else if(box==QMessageBox::Yes){
        DataUntil::getInstance().deletePlan(this);
    }
}


void Plan::on_editBtn_clicked()
{
    PlanDetail *dialog=new PlanDetail(this,&MZPlanClientWin::getInstance());
    //设置对话框标题
    dialog->setWindowTitle("编辑计划");
    //获取对话框返回内容
    int ret=dialog->exec();
    if(QDialog::Accepted==ret){//点击确定
        //获取用户输入的计划
        Plan *plan=dialog->toPlan();
        //写入文件中
        DataUntil::getInstance().editPlan(this,plan);
    }
    delete dialog;
}


void Plan::on_finishBtn_clicked()
{
    Plan *donePlan = new Plan(this,QDateTime::currentDateTime());
    DataUntil::getInstance().finishPlan(donePlan);
}

