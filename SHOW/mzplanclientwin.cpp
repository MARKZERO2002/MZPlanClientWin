#include <QMessageBox>
#include <windows.h>
#include "mzplanclientwin.h"
#include "plan.h"
#include "plandetail.h"
#include "ui_mzplanclientwin.h"
#include <DATA/datauntil.h>
#include <DATA/shortCut.h>
#include <QStandardPaths>
#include <QLocalSocket>
#include <QSettings>
#include <SERVER/timerserver.h>
#include <NETWORK/networkuntil.h>

MZPlanClientWin::MZPlanClientWin(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MZPlanClientWin)
{
    ui->setupUi(this);
    this->checkRunning();
    this->initPlanList();
    this->initMainWindow();
    this->initKeyShortCut();
}

MZPlanClientWin::~MZPlanClientWin()
{
    this->systemtray->hide();//不加这句的话，系统退出后托盘仍然存在
    delete ui;
    this->unregisterHotKey();
}
//初始化计划列表
void MZPlanClientWin::initPlanList()
{
    //创建一个竖直布局
    this->planListLayout=new QVBoxLayout();
    this->planLists.resize(Plan::statusStr.size());
    //创建列表
    for(int i=0;i<Plan::statusStr.size();i++){
        PlanList *planList=new PlanList((PlanList::Status)i);
        this->planListLayout->addWidget(planList);
        this->planLists[i]=planList;
    }
    this->ui->planWidget->setLayout(this->planListLayout);
    //创建一个弹簧
    spacer=new QSpacerItem(10,20,QSizePolicy::Fixed,QSizePolicy::Expanding);
    planListLayout->addSpacerItem(spacer);
}

void MZPlanClientWin::initTray()
{
    this->systemtray=new QSystemTrayIcon();
    //初始化系统托盘
    this->systemtray->setIcon(QIcon(":/Resource/Icon/MZPlan_Win_client.ico"));//设置图标
    this->systemtray->setToolTip("MZ计划");//设置鼠标放上去的提示语
    //连接托盘信号：ActivationReason是个枚举，里面包含了单击托盘、双击等等信号
    connect(this->systemtray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(handleActivatedSysTrayIcon(QSystemTrayIcon::ActivationReason)));
    //建立托盘菜单项和菜单
    this->trayShow=new QAction("显示主界面",this);
    connect(this->trayShow,SIGNAL(triggered()),this,SLOT(handleTrayShow()));
    this->trayQuit=new QAction("退出",this);
    connect(this->trayQuit,SIGNAL(triggered()),this,SLOT(handleTrayQuit()));
    this->systemTrayMenu=new QMenu(this);
    this->systemTrayMenu->addAction(this->trayShow);
    this->systemTrayMenu->addSeparator();//分隔符
    this->systemTrayMenu->addAction(this->trayQuit);
    this->systemtray->setContextMenu(this->systemTrayMenu);//把菜单赋给托盘
    this->systemtray->show();
}

void MZPlanClientWin::initKeyShortCut()
{
    //从设置中加载快捷键
    QSettings sysSettings(DataUntil::getInstance().systemConfigPath,QSettings::IniFormat);
    if(sysSettings.contains(SHORTCUT))
        shortcutSeq = sysSettings.value(SHORTCUT, QKeySequence()).value<QKeySequence>();
    if (!shortcutSeq.isEmpty()) {
        this->ui->arouseKeySequenceEdit->setKeySequence(shortcutSeq);
        this->shortcut=new QShortcut(shortcutSeq, this);
        //设置全局快捷键
        if(!this->registerHotKey())
            QMessageBox::warning(this,"警告","快捷键设置失败1");
    }

}

bool MZPlanClientWin::checkShortcut()
{
#if defined(Q_OS_WIN)
    QString deskTopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    deskTopPath = deskTopPath+QDir::separator() + "MZPlanClientWin.lnk";
    QFile file(deskTopPath);
    if(file.exists())
        return true;
    return false;
#endif
}

bool MZPlanClientWin::checkSelfStart()
{
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    return settings.contains(QCoreApplication::applicationName());
}

bool MZPlanClientWin::registerHotKey()
{
    Qt::KeyboardModifiers allMods = Qt::ShiftModifier | Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier;
    int sc=shortcutSeq[0],am=allMods;
    Qt::Key key = Qt::Key((sc ^ am) & sc);
    Qt::KeyboardModifiers mods = Qt::KeyboardModifiers(sc & am);
    const quint32 nativeKey = nativeKeycode(key);
    const quint32 nativeMods = nativeModifiers(mods);
    MY_GLOBAL_HOTKEY_ID = nativeKey^nativeMods;
    return RegisterHotKey(reinterpret_cast<HWND>(this->winId()), MY_GLOBAL_HOTKEY_ID, nativeMods, nativeKey);
}

bool MZPlanClientWin::unregisterHotKey()
{
    return UnregisterHotKey(reinterpret_cast<HWND>(this->winId()),MY_GLOBAL_HOTKEY_ID);
}

/**
 * @brief MZPlanClientWin::checkRunning
 * 检查程序是否已经运行 使用localserver 监听实现
 */
void MZPlanClientWin::checkRunning()
{
    QLocalSocket *socket=new QLocalSocket(this);
    socket->connectToServer("MZPlanClientWin");
    if(socket->waitForConnected()){//连上了，说明已经有应用在运行了
        socket->close();
        delete socket;
        exit(0);
    }else{
        this->localServer=new QLocalServer(this);
        this->localServer->listen("MZPlanClientWin");
        connect(this->localServer,&QLocalServer::newConnection,this,&MZPlanClientWin::handleNewConnection);
    }
}

void MZPlanClientWin::initMainWindow()
{
    //去除标题栏
    this->setWindowFlags(Qt::FramelessWindowHint);
    //设置界面大小
    QSettings settings(DataUntil::getInstance().systemConfigPath,QSettings::IniFormat);
    QSize size=settings.value(SIZE).toSize();
    QPoint position=settings.value(POSITION).toPoint();
    this->setGeometry(position.x(),position.y(),size.width(),size.height());
    this->setMinimumWidth(600);
    this->setMinimumHeight(size.height());
    //设置日期为今天
    this->ui->currentDate->setDate(QDate::currentDate());
    //设置系统托盘
    this->initTray();
    //设置设置页
    this->ui->synchronizeBtn->setCheckState(DataUntil::getInstance().isSynchronize==true?Qt::Checked:Qt::Unchecked);
    //设置拖动
    QSettings sysS=QSettings(DataUntil::getInstance().systemConfigPath,QSettings::IniFormat);
    this->lock=sysS.value(POSITION_LOCK).toBool();
    if(this->lock){
        this->lockWindow();
    }else{
        this->unlockWindow();
    }
}

MZPlanClientWin &MZPlanClientWin::getInstance()
{
    static MZPlanClientWin instance;
    return instance;
}
//更新界面
void MZPlanClientWin::updateInterface()
{
    this->updatePlanList();
}
//更新计划列表
void MZPlanClientWin::updatePlanList()
{
    for(auto planList:this->planLists){
        planList->updatePlans();//每个列表去更新计划
    }
}

/**
 * @brief MZPlan_Win_client::newConnection
 * 打开了第二个程序时，第一个程序就收到该信号
 */
void MZPlanClientWin::handleNewConnection()
{
    //显示主界面 聚焦主界面
    this->show();
    this->activateWindow();
}

void MZPlanClientWin::handleActivatedSysTrayIcon(QSystemTrayIcon::ActivationReason act)
{
    switch(act){
    case QSystemTrayIcon::Trigger://单击
        this->handleTrayShow();
        break;
    case QSystemTrayIcon::DoubleClick://双击
        this->handleTrayShow();
        break;
    default:
        break;
    }
}

void MZPlanClientWin::handleTrayShow()
{
    this->show();
    //放在最上层
    this->activateWindow();
}

void MZPlanClientWin::handleTrayQuit()
{
    exit(0);
}

void MZPlanClientWin::setCurrenDate(QDate date)
{
    this->ui->currentDate->setDate(date);
    this->on_planPageBtn_clicked();
}

void MZPlanClientWin::on_planPageBtn_clicked()
{
    this->ui->stackedWidget->setCurrentWidget(this->ui->planPage);
}


void MZPlanClientWin::on_calendarPageBtn_clicked()
{
    if(this->calender==nullptr){
        this->calender=new PlanCalenderWidget();

        QVBoxLayout *VLCarlendarPage=new QVBoxLayout();
        VLCarlendarPage->addWidget(this->calender);
        this->ui->calendarPage->setLayout(VLCarlendarPage);
        this->calenderIllustrate = new QLabel(this);
        this->calenderIllustrate->setText("<span style='color:green;'>绿色</span>:有计划开始;<br><span style='color:#CDC673;'>"
                                          "黄色</span>:有计划到达提醒时间;<br><span style='color:red;'>红色</span>:有计划结束。");
        this->calenderIllustrate->setTextFormat(Qt::RichText);
        QFont font;
        font.setPointSize(15);
        this->calenderIllustrate->setFont(font);
        VLCarlendarPage->addWidget(this->calenderIllustrate);
    }else{
        this->calender->updateData();//重新获取数据
    }
    this->ui->stackedWidget->setCurrentWidget(this->ui->calendarPage);
}


void MZPlanClientWin::on_userBtn_clicked()
{
    if(DataUntil::getInstance().username!="local"){
        this->ui->stackedWidget->setCurrentWidget(this->ui->userInfoPage);
    }else{
        this->ui->stackedWidget->setCurrentWidget(this->ui->loginPage);
    }
}


void MZPlanClientWin::on_settingPageBtn_clicked()
{
    if(DataUntil::getInstance().username=="local"){//本地用户不显示同步按钮
        this->ui->synchronizeBtn->hide();
        this->ui->label_4->hide();
    }else{
        this->ui->synchronizeBtn->show();
        this->ui->label_4->show();
    }
    //检查桌面是否有快捷方式，有则按钮为true
    this->ui->desktopShortcutBtn->setChecked(this->checkShortcut());
    //检查开机自启
    this->ui->selfStartBtn->setChecked(this->checkSelfStart());
    //切换界面
    this->ui->stackedWidget->setCurrentWidget(this->ui->settingPage);
}


void MZPlanClientWin::on_normalSettingBtn_clicked()
{
    this->ui->settingStackedWidget->setCurrentWidget(this->ui->normalizePage);
}


void MZPlanClientWin::on_shortCutKeySettingBtn_clicked()
{
    this->ui->settingStackedWidget->setCurrentWidget(this->ui->shortCutKeyPage);
}

//添加计划
void MZPlanClientWin::on_addPlanBtn_clicked()
{
    //调用新建计划对话框
    PlanDetail *dialog=new PlanDetail(nullptr,this);
    //设置对话框标题
    dialog->setWindowTitle("新建计划");
    //获取对话框返回内容
    int ret=dialog->exec();
    if(QDialog::Accepted==ret){//点击确定
        //获取用户输入的计划
        Plan *plan=dialog->toPlan();
        //写入文件中
        DataUntil::getInstance().addPlan(plan);
    }
    delete dialog;
}


void MZPlanClientWin::on_currentDate_dateChanged(const QDate &date)
{
    this->choiceDate=date;
    for(auto planList:this->planLists)
        planList->clearPlans();//先清空，要不然切页就要删除掉plan了
    DataUntil::getInstance().reloadChoicePlan(date);
    this->updateInterface();
}


void MZPlanClientWin::on_backToddayBtn_clicked()
{
    this->ui->currentDate->setDate(QDate::currentDate());
}


void MZPlanClientWin::on_refreshBtn_clicked()
{
    for(auto planList:this->planLists)
        planList->clearPlans();//先清空，要不然切页就要删除掉plan了
    DataUntil::getInstance().reloadChoicePlan(this->choiceDate);
    DataUntil::getInstance().reloadCurrentPlan();
    //刷新定时器
    TimerServer::getInstance().resetTimer();
    this->updateInterface();
}


void MZPlanClientWin::on_synchronizeBtn_stateChanged(int arg1)
{
    if(arg1==Qt::Checked){
        DataUntil::getInstance().isSynchronize=true;
        //立即同步一次
        NetWorkUntil::getInstance().synchronize();
    }else{
        DataUntil::getInstance().isSynchronize=false;
    }
    //写入用户配置表中
    QSettings settings(DataUntil::getInstance().userConfigPath,QSettings::IniFormat);
    settings.setValue(MYSYNCHRONIZE,DataUntil::getInstance().isSynchronize);
}

//点击快捷方式按钮
void MZPlanClientWin::on_desktopShortcutBtn_clicked()
{
    //如果是打勾，则在桌面建立一个快捷方式
    QString deskTopPath,srcFile;
    if(this->ui->desktopShortcutBtn->isChecked()){
        //判断桌面快捷方式是否存在，存在就不管，不存在就添加
#if defined(Q_OS_WIN)
        deskTopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        deskTopPath = deskTopPath+QDir::separator() + "MZPlanClientWin.lnk";
        if(!this->checkShortcut()){
            srcFile = QCoreApplication::applicationDirPath()+QDir::separator()+"MZPlanClientWin.exe";//exe文件路径
            QFile::link(srcFile, deskTopPath);//创建桌面快捷方式
        }
#endif
    }else{//否则就删除快捷方式
#if defined(Q_OS_WIN)
        deskTopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        deskTopPath = deskTopPath+QDir::separator() + "MZPlanClientWin.lnk";
        QFile file(deskTopPath);
        if(file.exists())
            file.remove();
#endif
    }
}

//点击开机自启按钮
void MZPlanClientWin::on_selfStartBtn_clicked()
{
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    if(this->ui->selfStartBtn->isChecked()){
        //实现开机自启
        QString path = QCoreApplication::applicationFilePath();
        settings.setValue(QCoreApplication::applicationName(), path);
    }else{
        //取消开机自启
        settings.remove(QCoreApplication::applicationName());
    }
    //写入系统配置表中
    QSettings sysSettings(DataUntil::getInstance().systemConfigPath,QSettings::IniFormat);
    sysSettings.setValue(SELF_START,this->ui->selfStartBtn->isChecked());
}


void MZPlanClientWin::on_arouseKeySequenceEdit_editingFinished()
{
    //用户输入完快捷键后进入这里
    this->shortcutSeq = this->ui->arouseKeySequenceEdit->keySequence();
    //修改快捷键
    if(!this->shortcutSeq.isEmpty()){
        if(this->shortcut){
            this->shortcut->setKey(this->shortcutSeq);
            //先注销
            this->unregisterHotKey();
            this->registerHotKey();
        }else{
            shortcut = new QShortcut(this->shortcutSeq, this);
            if(!this->registerHotKey())
                QMessageBox::warning(this,"警告","快捷键设置失败2");
        }
    }
    //写入系统配置信息
    QSettings sysSettings(DataUntil::getInstance().systemConfigPath,QSettings::IniFormat);
    sysSettings.setValue(SHORTCUT,this->shortcutSeq);
}

void MZPlanClientWin::useKeyShortCut()
{
    if(this->isVisible()){
        this->hide();
    }else{
        this->show();
        this->activateWindow();
        this->raise();
    }
}
bool MZPlanClientWin::nativeEvent(const QByteArray &eventType, void *message, qintptr *result) {
    MSG *msg = reinterpret_cast<MSG*>(message);
    if (msg->message == WM_HOTKEY) {
        // 检查是否为您的快捷键
        int id = HIWORD(msg->lParam)^LOWORD(msg->lParam);
        if (id == MY_GLOBAL_HOTKEY_ID) {
            // 快捷键被按下，执行相应的操作
            this->useKeyShortCut();
            return true; // 事件已处理
        }
    }
    // 如果不是我们的快捷键或者我们不想处理该事件，则调用基类方法
    return QMainWindow::nativeEvent(eventType, message, result);
}

bool MZPlanClientWin::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->header) // 检查事件是否来自header QFrame
    {
        if (event->type() == QEvent::MouseButtonPress)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton)
            {
                // 记录鼠标按下的位置相对于窗口的位置
                lastMousePos = mouseEvent->pos();
                isDragging=true;
                return true; // 事件已处理
            }
        }
        else if (event->type() == QEvent::MouseMove && isDragging)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            // 计算鼠标移动了多少
            QPoint delta = mouseEvent->pos() - lastMousePos;
            // 移动窗口相应的距离
            QPoint finalPos=this->pos()+delta;
            //判断边界
            QRect availableSize=QGuiApplication::primaryScreen()->geometry();//可用屏幕，不包括任务栏
            int maxX=availableSize.width()-600;
            finalPos.setX(finalPos.x()>maxX?maxX:finalPos.x());
            finalPos.setX(finalPos.x()<0?0:finalPos.x());
            this->move(finalPos);
            return true;
        }
        else if (event->type() == QEvent::MouseButtonRelease && isDragging)
        {
            // 鼠标释放，结束拖动
            isDragging = false;
            //高度永远设为0
            this->move(this->pos().x(),0);
            //写入配置文件中
            QSettings sysS=QSettings(DataUntil::getInstance().systemConfigPath,QSettings::IniFormat);
            sysS.setValue(POSITION,this->pos());
            return true; // 事件已处理
        }
    }
    // 如果事件不是针对header或者没有被处理，则继续正常的事件处理
    return QMainWindow::eventFilter(obj, event);
}

void MZPlanClientWin::lockWindow()
{
    //移除过滤器 无法自由移动
    ui->header->removeEventFilter(this);
    //设置图标
    this->ui->lockBtn->setIcon(QIcon(":/Resource/Icon/lock.ico"));//设为固定的图标
    //写入配置信息
    QSettings sysS=QSettings(DataUntil::getInstance().systemConfigPath,QSettings::IniFormat);
    sysS.setValue(POSITION_LOCK,this->lock);
}

void MZPlanClientWin::unlockWindow()
{
    //设置过滤器 可以自由移动
    ui->header->installEventFilter(this);
    //设置图标
    this->ui->lockBtn->setIcon(QIcon(":/Resource/Icon/unlock.ico"));//设为解锁的图标
    //写入配置信息
    QSettings sysS=QSettings(DataUntil::getInstance().systemConfigPath,QSettings::IniFormat);
    sysS.setValue(POSITION_LOCK,this->lock);
}

void MZPlanClientWin::on_lockBtn_clicked()
{
    if(this->lock){
        this->lock=false;
        this->unlockWindow();
    }else{
        this->lock=true;
        this->lockWindow();
    }
}

