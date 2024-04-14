#ifndef PLANCALENDERWIDGET_H
#define PLANCALENDERWIDGET_H

#include "plan.h"

#include <QCalendarWidget>
#include <QObject>

class PlanCalenderWidget : public QCalendarWidget
{
    Q_OBJECT
public:
    explicit PlanCalenderWidget(QWidget *parent=nullptr);
    ~PlanCalenderWidget();
    void updateData();//重新从数据库读取数据
private:
    bool isNotFinishPlan(Plan* plan) const;
protected:
    void paintCell(QPainter *painter, const QRect &rect, QDate date) const override;
};

#endif // PLANCALENDERWIDGET_H
