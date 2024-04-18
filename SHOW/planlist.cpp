#include "planlist.h"
#include "ui_planlist.h"

#include <DATA/datauntil.h>

PlanList::PlanList(Status status,QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PlanList)
{
    ui->setupUi(this);
    this->initInterface(status);
}

PlanList::~PlanList()
{
    delete ui;
}

void PlanList::clearPlans()
{
    //清空原来的计划
    for(auto plan:this->plans){
        this->ui->planLayout->layout()->removeWidget(plan);
        plan->setParent(nullptr);
        plan->hide();
    }
    this->plans.clear();
}
//更新计划列表中的计划
void PlanList::updatePlans()
{
    this->ui->planLayout->layout()->removeItem(this->spacer);//去弹簧
    this->clearPlans();
    //从数据库中获取计划，选择对应计划放入该列表中
    this->plans=DataUntil::getInstance().getChoicePlans(this->planStatus);
    for(auto plan:this->plans){
        this->ui->planLayout->layout()->addWidget(plan);
        plan->show();
    }
    this->ui->planLayout->layout()->addItem(this->spacer);//加弹簧
    //设置计划个数
    this->ui->planCount->setText(QString::number(this->plans.size()));
}

void PlanList::initColor()
{
    //计划、标题框的颜色改变
    this->ui->showPlanArea->setStyleSheet("background-color: rgba(0, 0, 0,0)");
    switch(this->planStatus){
    case UNSTARAT:
        this->ui->Header->setStyleSheet("background-color: rgb(168, 168, 168);");
        this->ui->planLayout->setStyleSheet("background-color: rgba(168, 168, 168,100);");
        break;
    case DOING:
        this->ui->Header->setStyleSheet("background-color: rgb(255,250,205);");
        this->ui->planLayout->setStyleSheet("background-color: rgba(255,250,205,100);");
        break;
    case FINISH:
        this->ui->Header->setStyleSheet("background-color: rgb(127,255,212);");
        this->ui->planLayout->setStyleSheet("background-color: rgba(127,255,212,100);");
        break;
    case UNFINISH:
        this->ui->Header->setStyleSheet("background-color: rgb(255,106,106);");
        this->ui->planLayout->setStyleSheet("background-color: rgba(255,106,106,100);");
        break;
    }
}

void PlanList::mousePressEvent(QMouseEvent *event)
{
    if(event->button()==Qt::LeftButton&&this->ui->Header->underMouse()){
        if(this->ui->showPlanArea->isHidden())
            this->ShowPlanArea(true);
        else
            this->ShowPlanArea(false);
    }
}

void PlanList::ShowPlanArea(bool status)
{
    if(status){//打开
        this->ui->showPlanArea->show();
        //更改图标
        this->ui->statusImg->setPixmap(QPixmap(":/Resource/Icon/packup.ico"));
    }else{//关闭
        this->ui->showPlanArea->hide();
        //更改图标
        this->ui->statusImg->setPixmap(QPixmap(":/Resource/Icon/pulldown.ico"));
    }
    //重设大小
    // this->resize(this->sizeHint());
}
void PlanList::initInterface(Status status)
{
    //弹簧初始化
    spacer=new QSpacerItem(10,20,QSizePolicy::Fixed,QSizePolicy::Expanding);
    //设置标题
    this->planStatus=status;
    this->ui->title->setText(Plan::statusStr[status]);
    //隐藏界面
    if(status==1)
        this->ui->statusImg->setPixmap(QPixmap(":/Resource/Icon/packup.ico"));
    else
        this->ui->showPlanArea->hide();
    //更改界面颜色
    this->initColor();
}
