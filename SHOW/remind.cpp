#include "mzplanclientwin.h"
#include "remind.h"
#include "ui_remind.h"

#include <SERVER/timerserver.h>

Remind::Remind(QString title,QString content,QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Remind)
{
    ui->setupUi(this);
    //设置显示界面
    setWindowModality(Qt::NonModal);//非模态
    setWindowFlags(Qt::FramelessWindowHint|Qt::Tool|Qt::WindowStaysOnTopHint);//设置无边框和窗口总在最上层
    //设置显示内容
    this->ui->title->setText(title);
    this->ui->content->setPlainText(content);
    this->ui->time->setText(QDateTime::currentDateTime().toString());
}

Remind::~Remind()
{
    delete ui;
}

void Remind::on_closeBtn_clicked()
{
    this->close();
    //发出消息让plantimer删除自己，释放自己的空间
    TimerServer::getInstance().removeremindDialog(this);
}

void Remind::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    //切换到主界面
    MZPlanClientWin::getInstance().show();
    MZPlanClientWin::getInstance().activateWindow();
    //删除自己
    this->close();
    TimerServer::getInstance().removeremindDialog(this);
}

