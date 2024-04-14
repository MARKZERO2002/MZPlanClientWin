#ifndef PLANLIST_H
#define PLANLIST_H

#include "plan.h"

#include <QMouseEvent>
#include <QSpacerItem>
#include <QWidget>

namespace Ui {
class PlanList;
}

class PlanList : public QWidget
{
    Q_OBJECT
public:
    enum Status{UNSTARAT,DOING,UNFINISH,FINISH};
public:
    explicit PlanList(Status status,QWidget *parent = nullptr);
    ~PlanList();
    void addPlan(Plan *plan);
    void editPlan(Plan *oldPlan,Plan *newPlan);
    void deletePlan(Plan *plan);
    void clearPlans();
    void updatePlans();
private:
    Ui::PlanList *ui;
    QSpacerItem* spacer;
    Status planStatus;
    QList<Plan*> plans;

private:
    void initInterface(Status status);//初始化界面
    void initColor();
    //点击事件
    void mousePressEvent(QMouseEvent *event);
    void ShowPlanArea(bool status);
};

#endif // PLANLIST_H
