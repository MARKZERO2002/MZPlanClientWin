#ifndef PLAN_H
#define PLAN_H

#include <QDateTime>
#include <QWidget>

namespace Ui {
class Plan;
}

class Plan : public QWidget
{
    Q_OBJECT
public:
    //计划当前状态字段列表
    static QList<QString> statusStr;
    QDateTime createdTime;
    QDateTime startTime;
    QDateTime reminderTime;
    QDateTime endTime;
    QDateTime completedTime;
    QString content;
    QString rule;
public:
    explicit Plan(QString createdTime,QString startTime,QString reminderTime,QString endTime,QString content,QString rule,QDate date,QString completedTime=nullptr,QWidget *parent = nullptr);
    explicit Plan(Plan* plan,QDateTime completedTime,QWidget *parent = nullptr);
    Plan(Plan* plan);//复制函数
    ~Plan();
    bool isConfirmRule(const QDate &date) const;
private slots:
    void on_deleteBtn_clicked();

    void on_editBtn_clicked();

    void on_finishBtn_clicked();

private:
    void init(QString createdTime,QString startTime,QString reminderTime,QString endTime,QString content,QString rule,QDate date,QString completedTime);
    void updateDate(QDate date);
    void setDonePlanShow();
    void setPlanShow();
private:
    Ui::Plan *ui;


};

#endif // PLAN_H
