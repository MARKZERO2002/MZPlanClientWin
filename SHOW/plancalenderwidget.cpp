#include "plancalenderwidget.h"

#include "mzplanclientwin.h"

#include <DATA/datauntil.h>

#include <QPainter>

PlanCalenderWidget::PlanCalenderWidget(QWidget *parent)
    :QCalendarWidget(parent)
{
    connect(this,&PlanCalenderWidget::activated,&MZPlanClientWin::getInstance(),&MZPlanClientWin::getInstance().setCurrenDate);
    this->updateData();
}

PlanCalenderWidget::~PlanCalenderWidget()
{

}
//重新从数据库读取数据
void PlanCalenderWidget::updateData()
{
    DataUntil::getInstance().reloadOnce();
}

bool PlanCalenderWidget::isNotFinishPlan(Plan *plan) const
{
    //查找这个计划是否已完成
    for(auto donePlan:DataUntil::getInstance().onceDonePlans){
        if(plan->createdTime==donePlan->createdTime)
            return false;
    }
    return true;
}

void PlanCalenderWidget::paintCell(QPainter *painter, const QRect &rect, QDate date) const
{
    // 调用基类绘制日期格
    QCalendarWidget::paintCell(painter, rect, date);
    // 当前日期中含有几个一次性计划，要显示出来，而且根据是开始时间还是提醒时间还是结束时间显示不同颜色的数量
    //计算位置
    QRect beginRect =rect,remindRect=rect,endRect=rect;
    beginRect.adjust(-rect.width()/2, rect.height() / 2, 0, 0);
    remindRect.adjust(0,rect.height()/2,0,0);
    endRect.adjust(rect.width()/2,rect.height()/2,0,0);
    //获取数量
    //访问所有计划，如果是未完成的一次性计划，且该计划与Date对得上号就加入，如果该计划的开始、提醒、结束时间其中有两个以上在一天，则只取后者
    int beginCount=0,remindCount=0,endCount=0;
    for(const auto plan:DataUntil::getInstance().oncePlans){
        if(plan->rule=="once"&&this->isNotFinishPlan(plan)){
            if(plan->endTime.date()==date)
                endCount++;
            else if(plan->reminderTime.date()==date)
                remindCount++;
            else if(plan->startTime.date()==date)
                beginCount++;
        }
    }
    // 绘制文字
    QFont font;
    font.setPointSize(15);
    painter->setFont(font);
    if(beginCount!=0){
        painter->setPen(Qt::green);
        painter->drawText(beginRect, Qt::AlignCenter, QString::number(beginCount));
    }
    if(remindCount!=0){
        QPen pen;
        pen.setColor(QColor(205,198,115));
        painter->setPen(pen);
        painter->drawText(remindRect, Qt::AlignCenter, QString::number(remindCount));
    }
    if(endCount!=0){
        painter->setPen(Qt::red);
        painter->drawText(endRect, Qt::AlignCenter, QString::number(endCount));
    }
}
