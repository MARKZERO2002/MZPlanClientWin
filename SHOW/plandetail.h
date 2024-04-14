#ifndef PLANDETAIL_H
#define PLANDETAIL_H

#include "plan.h"

#include <QButtonGroup>
#include <QDialog>
#include <set>


class QCheckBox;
namespace Ui {
class PlanDetail;
}

class PlanDetail : public QDialog
{
    Q_OBJECT

public:
    explicit PlanDetail(Plan *plan=nullptr,QWidget *parent = nullptr);
    ~PlanDetail();
    Plan *toPlan();

private slots:
    void on_accept_clicked();
    void on_reject_clicked();
    void on_onceRBtn_clicked(bool checked);
    void on_weekRBtn_clicked(bool checked);
    void btnClicked(int btnId,bool status);
    void on_monthRBtn_clicked();
    void btnMonthClicked(int btnId,bool status);

private:
    Ui::PlanDetail *ui;
    Plan *tplan=nullptr;//存储用户点击编辑计划时的该计划指针
    QString rule="once";
    QButtonGroup *btnGroup;
    std::set<int> checkWeek,checkMonth;
    QButtonGroup *btnMonthGroup;
    QList<QCheckBox*> monthCheckBoxList;
    enum InvalidInput{allValid,contentIsNull,bTInvalid,rTInvalid,ruleInvalid};
    enum Rule{ONCE,WEEK,MONTH};
private:
    //初始化对话框内组件的默认值
    void initValue(Plan *plan=nullptr);
    //初始化对话框内组件隐藏与显示
    void initWiget();
    //隐藏组件
    void hideOther(int checkedBtn);
    void updateRule(Rule which);
    //检测用户输入信息的合法性
    InvalidInput checkValid();
};

#endif // PLANDETAIL_H
