#ifndef MZPLANCLIENTWIN_H
#define MZPLANCLIENTWIN_H



#include "plancalenderwidget.h"
#include "planlist.h"

#include <QLabel>
#include <QLocalServer>
#include <QMainWindow>
#include <QMenu>
#include <QShortcut>
#include <QSpacerItem>
#include <QSystemTrayIcon>

QT_BEGIN_NAMESPACE

class QVBoxLayout;
namespace Ui {
class MZPlanClientWin;
}
QT_END_NAMESPACE

class MZPlanClientWin : public QMainWindow
{
    Q_OBJECT
private:
    MZPlanClientWin(QWidget *parent = nullptr);
    ~MZPlanClientWin();
    //初始化
    void checkRunning();//检查程序是否已经运行
    void initMainWindow();
    void initPlanList();
    void initTray();
    void initKeyShortCut();
    //辅助函数
    bool checkShortcut();//检查快捷方式
    bool checkSelfStart();//检查开机自启
    bool checkValid();//检查用户输入用户名、密码合法
    //全局快捷键
    bool  registerHotKey();//注册windows快捷键
    bool  unregisterHotKey();//注销windows快捷键
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result);//事件过滤
    //拖动事件
    bool eventFilter(QObject *obj,QEvent *event);
    void lockWindow();
    void unlockWindow();
public:
    static MZPlanClientWin &getInstance();
    void updateInterface();//刷新界面
    void updatePlanList();//刷新计划列表
    void clearPlanList();//清空计划
private:
    //显示
    Ui::MZPlanClientWin *ui;
    QList<PlanList *> planLists;
    PlanCalenderWidget *calender=nullptr;
    QLabel *calenderIllustrate;
    //用于判断程序是否已经启动
    QLocalServer *localServer;
    //系统托盘
    QMenu* systemTrayMenu;//系统托盘右键菜单
    QSystemTrayIcon* systemtray;//系统托盘
    QAction* trayQuit;//菜单栏退出选项
    QAction* trayShow;//菜单栏显示主界面选项
    //布局
    QVBoxLayout *planListLayout;
    QSpacerItem *spacer;
    //快捷键
    QShortcut *shortcut=nullptr;
    QKeySequence shortcutSeq;
    QList<int> MY_GLOBAL_HOTKEY_ID;
    //拖动
    bool lock;
    QPoint lastMousePos;
    bool isDragging=false;
    QPoint initialWindowPos;
public:
    //数据
    QDate choiceDate;
public slots:
    void handleNewConnection();//打开了第二个程序就会发送这个信号给第一个程序
    //托盘信号
    void handleActivatedSysTrayIcon(QSystemTrayIcon::ActivationReason act);
    void handleTrayShow();
    void handleTrayQuit();
    void setCurrenDate(QDate date);
public slots:
    //页面切换
    void on_planPageBtn_clicked();
    void on_calendarPageBtn_clicked();
    void on_userBtn_clicked();
    void on_settingPageBtn_clicked();
    //设置页面的页面切换
    void on_normalSettingBtn_clicked();
    void on_shortCutKeySettingBtn_clicked();
    //主界面按钮
    void on_addPlanBtn_clicked();
    void on_currentDate_dateChanged(const QDate &date);
    void on_backToddayBtn_clicked();
    void on_refreshBtn_clicked();
    void on_synchronizeBtn_stateChanged(int arg1);
    void on_desktopShortcutBtn_clicked();
    void on_selfStartBtn_clicked();
    void on_arouseKeySequenceEdit_editingFinished();
    void useKeyShortCut();//使用了快捷键
    void on_lockBtn_clicked();
    void on_registBtn_clicked();
    void on_loginBtn_clicked();
private slots:
    void on_logoutBtn_clicked();
    void on_cancelBtn_clicked();
};
#endif // MZPLANCLIENTWIN_H
