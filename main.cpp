#include <QApplication>
#include <SHOW/mzplanclientwin.h>
#include <DATA/datauntil.h>
#include <SERVER/timerserver.h>
#include <NETWORK/networkuntil.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // DataUntil::getInstance();//载入必要数据
    //初始化网络
    if(DataUntil::getInstance().isSynchronize)
        NetWorkUntil::getInstance().synchronize(true);
    MZPlanClientWin::getInstance().show();//显示界面 这时才从数据库中读入数据
    TimerServer::getInstance().resetTimer();//设置定时器

    return a.exec();
}
