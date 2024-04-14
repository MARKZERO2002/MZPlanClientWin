#include <QApplication>
#include <SHOW/mzplanclientwin.h>
#include <DATA/datauntil.h>
#include <SERVER/timerserver.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DataUntil::getInstance();//载入必要数据
    MZPlanClientWin::getInstance().show();//显示界面
    TimerServer::getInstance().resetTimer();//设置定时器

    return a.exec();
}
