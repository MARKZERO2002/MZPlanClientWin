#include "datauntil.h"

#include <QDate>
#include <QSettings>
#include <QSqlQuery>
#include <QMessageBox>
#include <QSqlRecord>
#include <QGuiApplication>
#include <QScreen>
#include <QSqlError>
#include <QLayout>

#include <SHOW/mzplanclientwin.h>

#include <SERVER/timerserver.h>

#include <NETWORK/networkuntil.h>

DataUntil &DataUntil::getInstance()
{
    static DataUntil instance;
    return instance;
}

void DataUntil::updateUser(){
    //刷新计划数据 在changeUser里刷新会造成单例模式循环
    MZPlanClientWin::getInstance().clearPlanList();
    this->reloadChoicePlan(MZPlanClientWin::getInstance().choiceDate);
    this->reloadCurrentPlan();
    MZPlanClientWin::getInstance().updateInterface();
    //刷新定时器 在changeUser里刷新会造成单例模式循环
    TimerServer::getInstance().resetTimer();
}
bool DataUntil::changeUser(const QString &username)
{
    //更改用户名及相关地址
    this->username=username;
    this->userDirPath=this->dataPath+QDir::separator()+username;
    this->userConfigPath=this->userDirPath+QDir::separator()+"user.ini";
    this->databasePath=this->userDirPath+QDir::separator()+"database.db3";
    //创建用户文件夹
    bool ans=this->createUserDir();
    //读取用户信息
    this->initUserConfig();
    //更改数据库
    this->initDateBase();
    //更改默认用户
    QSettings settings(this->systemConfigPath,QSettings::IniFormat);
    settings.setValue(DEFAULT_USER,username);
    return ans;
}

void DataUntil::removeUserDir()
{
    //断开DB不然删除会失败
    if(this->DB.isOpen()){
        this->DB.close();
        this->DB=QSqlDatabase();
        this->DB.removeDatabase("QSQLITE");
    }
    QDir(this->userDirPath).removeRecursively();//递归删除
}
//判断该计划是否已完成
bool DataUntil::isCompeleted(const Plan *plan)
{
    //遍历已完成数组
    for(auto donePlan:this->choiceDonePlans){
        //如果一个已完成计划的创建时间与开始时间都与plan一致，说明该plan已完成
        if(donePlan->createdTime==plan->createdTime&&donePlan->startTime==plan->startTime)
            return true;
    }
    return false;
}

void DataUntil::listDeletePlan(Plan *plan)
{
    //判断该计划是否与当前选择日期有关，有则从choicePlans choiceDonePlans删除
    if(isCorrelative(plan,MZPlanClientWin::getInstance().choiceDate)){
        if(plan->completedTime.isNull())
            this->choicePlans.removeOne(plan);
        else
            this->choiceDonePlans.removeOne(plan);
    }
    //判断该计划是否与今日有关，有则从currentPlans currentDonePlans删除 因为与plan是不同的计划，所以要用值删除
    if(isCorrelative(plan,QDate::currentDate())){
        if(plan->completedTime.isNull()){
            for(auto tplan:this->currentPlans){
                if(tplan->createdTime==plan->createdTime){
                    this->currentPlans.removeOne(tplan);
                    break;
                }
            }
        }
        else{
            for(auto tplan:this->currentDonePlans){
                if(tplan->createdTime==plan->createdTime){
                    this->currentDonePlans.removeOne(tplan);
                    break;
                }
            }
        }
    }
}

void DataUntil::listAppendPlan(Plan *plan)
{
    //判断该计划是否与当前选择日期有关，有则加入choicePlans choiceDonePlans
    if(isCorrelative(plan,MZPlanClientWin::getInstance().choiceDate)){
        if(plan->completedTime.isNull())
            this->choicePlans.append(plan);
        else
            this->choiceDonePlans.append(plan);
    }
    //判断该计划是否与今日有关，有则复制一个加入currentPlans currentDonePlans 加入一个新new的计划
    Plan *tplan=new Plan(plan);
    if(isCorrelative(plan,QDate::currentDate())){
        if(plan->completedTime.isNull())
            this->currentPlans.append(tplan);
        else
            this->currentDonePlans.append(tplan);
    }
}

void DataUntil::dbinsert(Plan *plan)
{
    QSqlQuery query(this->DB);
    QString sql;
    if(plan->completedTime.isNull()){
        //写入plan表
        sql="INSERT INTO plan (createdTime,startTime,reminderTime,endTime,content,rule) "
              "VALUES (?,?,?,?,?,?);";
        query.prepare(sql);
        if(plan->rule=="once"){
            query.bindValue(0,plan->createdTime.toString(DTFORMAT));
            query.bindValue(1,plan->startTime.toString(DTFORMAT));
            query.bindValue(2,plan->reminderTime.toString(DTFORMAT));
            query.bindValue(3,plan->endTime.toString(DTFORMAT));
            query.bindValue(4,plan->content);
            query.bindValue(5,plan->rule);
        }else{//有规则的计划 格式用TFOMAT
            query.bindValue(0,plan->createdTime.toString(DTFORMAT));
            query.bindValue(1,plan->startTime.toString(TFORMAT));
            query.bindValue(2,plan->reminderTime.toString(TFORMAT));
            query.bindValue(3,plan->endTime.toString(TFORMAT));
            query.bindValue(4,plan->content);
            query.bindValue(5,plan->rule);
        }
    }else{
        //写入donePlan表
        sql="INSERT INTO donePlan(createdTime,startTime,reminderTime,endTime,completedTime,rule,content) VALUES (?,?,?,?,?,?,?);";
        query.prepare(sql);
        query.bindValue(0,plan->createdTime.toString(DTFORMAT));
        query.bindValue(1,plan->startTime.toString(DTFORMAT));
        query.bindValue(2,plan->reminderTime.toString(DTFORMAT));
        query.bindValue(3,plan->endTime.toString(DTFORMAT));
        query.bindValue(4,plan->completedTime.toString(DTFORMAT));
        query.bindValue(5,plan->rule);
        query.bindValue(6,plan->content);
    }
    if(!query.exec()){
        QMessageBox::critical(nullptr,"错误","写入数据库错误，原因："+query.lastError().text());
        exit(0);
    }
}

void DataUntil::dbdelete(Plan *plan)
{
    QSqlQuery query(this->DB);
    QString sql;
    if(plan->completedTime.isNull()){//plan表
        sql="DELETE FROM plan WHERE createdTime=?;";//删除要用createdTime，因为新加的计划在计划列表中可能没有id
        query.prepare(sql);
        query.bindValue(0,plan->createdTime.toString(DTFORMAT));
    }else{//donePlan表
        sql="DELETE FROM donePlan WHERE completedTime=? AND createdTime=?;";//完成计划的完成时间是唯一的
        query.prepare(sql);
        query.bindValue(0,plan->completedTime.toString(DTFORMAT));
        query.bindValue(1,plan->createdTime.toString(DTFORMAT));
    }
    if(!query.exec()){
        QMessageBox::critical(nullptr,"错误","删除数据库计划错误，原因："+query.lastError().text());
        exit(0);
    }
}
//获得某状态列表的计划，按照选择日期,因为选择日期只影响显示
QList<Plan *> DataUntil::getChoicePlans(PlanList::Status status)
{
    if(status==PlanList::FINISH){
        return this->choiceDonePlans;
    }
    QList<Plan *> ans;
    QDateTime currentTime=QDateTime::currentDateTime();
    for(auto plan:this->choicePlans){
        if(!isCompeleted(plan)){//不是已完成的计划
            switch(status){
            case PlanList::UNSTARAT:
                if(plan->startTime>currentTime)
                    ans.push_back(plan);
                break;
            case PlanList::DOING:
                if(plan->startTime<=currentTime&&plan->endTime>currentTime)
                    ans.push_back(plan);
                break;
            case PlanList::UNFINISH:
                if(plan->endTime<=currentTime)
                    ans.push_back(plan);
                break;
            default:
                break;
            }
        }
    }
    return ans;
}
//判断一条计划和一个日期是否相关
bool DataUntil::isCorrelative(const Plan *plan,const QDate &date){
    if(plan->rule!="once"){
        return plan->isConfirmRule(date);
    }
    if(plan->startTime.date()<=date&&plan->endTime.date()>=date)
        return true;
    return false;
}
//增加一条计划
void DataUntil::addPlan(Plan *plan)
{
    //把该计划写入数据库中
    this->dbinsert(plan);
    this->listAppendPlan(plan);
    //更新时间
    this->updateMedifyTime(QDateTime::currentDateTime().toString(DTFORMAT));
    if(this->isSynchronize)
        NetWorkUntil::getInstance().synchronize();
    //刷新主界面
    MZPlanClientWin::getInstance().updateInterface();
    //刷新定时器
    TimerServer::getInstance().resetTimer();
}

void DataUntil::deletePlan(Plan *plan)
{
    //把该计划从数据库中删除
    this->dbdelete(plan);
    this->listDeletePlan(plan);
    //更新时间
    this->updateMedifyTime(QDateTime::currentDateTime().toString(DTFORMAT));
    if(this->isSynchronize)
        NetWorkUntil::getInstance().synchronize();
    //刷新主界面
    MZPlanClientWin::getInstance().updateInterface();
    //刷新定时器
    TimerServer::getInstance().resetTimer();
    //释放计划
    delete plan;
    plan=nullptr;
}

void DataUntil::editPlan(Plan *oldPlan, Plan *newPlan)
{
    //在数据库中删除旧计划，增添新计划
    this->dbdelete(oldPlan);
    this->dbinsert(newPlan);
    //移除和增加计划
    this->listDeletePlan(oldPlan);
    this->listAppendPlan(newPlan);
    //更新时间
    this->updateMedifyTime(QDateTime::currentDateTime().toString(DTFORMAT));
    if(this->isSynchronize)
        NetWorkUntil::getInstance().synchronize();
    //刷新主界面
    MZPlanClientWin::getInstance().updateInterface();
    //刷新定时器
    TimerServer::getInstance().resetTimer();
    //释放旧指针内存
    delete oldPlan;
    oldPlan=nullptr;
}

void DataUntil::finishPlan(Plan *donePlan)
{
    //在数据库中新增完成计划
    this->dbinsert(donePlan);
    //增加完成计划
    this->listAppendPlan(donePlan);
    //更新时间
    this->updateMedifyTime(QDateTime::currentDateTime().toString(DTFORMAT));
    if(this->isSynchronize)
        NetWorkUntil::getInstance().synchronize();
    //刷新主界面
    MZPlanClientWin::getInstance().updateInterface();
    //刷新定时器
    TimerServer::getInstance().resetTimer();
}

DataUntil::DataUntil() {
    //初始化系统配置
    this->initSystemConfig();
    //初始化用户配置
    this->initUserConfig();
}

DataUntil::~DataUntil()
{
    //currentlist里的计划都是无parent的，所以要销毁
    for(auto plan:this->currentPlans)
        delete plan;
    for(auto donePlan:this->currentDonePlans)
        delete donePlan;
    //oncelist里的计划都是无parent的，所以要销毁
    for(auto plan:this->oncePlans)
        delete plan;
    for(auto donePlan:this->onceDonePlans)
        delete donePlan;
    this->DB.close();
}

//初始化系统配置
void DataUntil::initSystemConfig()
{
    //判断系统配置文件是否存在，不存在则创建与赋默认值
    QFile file(this->systemConfigPath);
    if(!file.exists()){
        //计算界面大小
        QScreen *screen=QGuiApplication::primaryScreen();
        QRect availableSize=screen->availableGeometry();//可用屏幕，不包括任务栏
        QSettings settings(this->systemConfigPath,QSettings::IniFormat);//创建ini格式文件
        settings.setValue(DEFAULT_USER,"local");
        settings.setValue(POSITION,QPoint(availableSize.width()-600,0));//程序默认位置
        settings.setValue(SIZE,QSize(600,availableSize.height()));//程序默认大小
        settings.setValue(POSITION_LOCK,false);//程序位置是否锁定
        settings.setValue(SELF_START,false);//开机自启
        settings.setValue("address","8.138.130.176");
        settings.setValue("port",1314);
    }
    //读取系统配置文件 设置用户
    QSettings settings(this->systemConfigPath,QSettings::IniFormat);
    this->changeUser(settings.value(DEFAULT_USER).toString());
}
//初始化用户配置文件
void DataUntil::initUserConfig()
{
    if(this->username=="local")
        return;
    //读取用户配置文件
    QSettings settings(this->userConfigPath,QSettings::IniFormat);
    this->isSynchronize=settings.value(MYSYNCHRONIZE).toBool();
    this->medifyTime=settings.value(MEDIFY_TIME).toString();
}
//初始化数据库
void DataUntil::initDateBase()
{
    //打开SQLite数据库
    if(QSqlDatabase::contains("QSQLITE")){
        this->DB=QSqlDatabase::database("QSQLITE");
        this->DB.setDatabaseName(this->databasePath);
    }
    else{
        this->DB=QSqlDatabase::addDatabase("QSQLITE","QSQLITE");
        this->DB.setDatabaseName(this->databasePath);
    }
    if(!this->DB.open()){
        QMessageBox::warning(nullptr,"错误","打开数据库失败："+this->databasePath);
        exit(0);
    }
    //如果数据库中没表，则要创建表
    QSqlQuery query(this->DB);
    QString createTableSql;
    query.exec(QString("PRAGMA table_info(%1);").arg("plan"));
    if(!query.next()){
        //没有plan表
        createTableSql="CREATE TABLE plan (createdTime TIMESTAMP NOT NULL,startTime TIMESTAMP NOT NULL,reminderTime TIMESTAMP  NOT NULL,endTime TIMESTAMP NOT NULL,content TEXT NOT NULL,rule TEXT NOT NULL);";
        query.exec(createTableSql);
    }
    query.exec(QString("PRAGMA table_info(%1);").arg("donePlan"));
    if(!query.next()){
        //没有donePlan表
        createTableSql="CREATE TABLE donePlan (createdTime TIMESTAMP NOT NULL,startTime TIMESTAMP NOT NULL,reminderTime TIMESTAMP  NOT NULL,endTime TIMESTAMP NOT NULL,content TEXT NOT NULL,rule TEXT NOT NULL,completedTime TIMESTAMP NOT NULL);";
        query.exec(createTableSql);
    }
}

void DataUntil::reloadChoicePlan(QDate date)
{
    for(auto plan:this->choicePlans)
        delete plan;
    for(auto plan:this->choiceDonePlans)
        delete plan;
    //获取计划
    this->choicePlans=this->loadPlan(date);
    this->choiceDonePlans=this->loadDonePlan(date);
}

void DataUntil::reloadCurrentPlan()
{
    QDate date=QDate::currentDate();
    for(auto plan:this->currentPlans)
        delete plan;
    for(auto plan:this->currentDonePlans)
        delete plan;
    //获取计划
    this->currentPlans=this->loadPlan(date);
    this->currentDonePlans=this->loadDonePlan(date);
}

void DataUntil::reloadOnce()
{
    for(auto plan:this->oncePlans)
        delete plan;
    for(auto donePlan:this->onceDonePlans)
        delete donePlan;
    this->oncePlans=this->loadOncePlan();
    this->onceDonePlans=this->loadOnceDonePlan();
}

void DataUntil::setSynchronize(bool flag)
{
    if(this->username=="local")
        return;
    this->isSynchronize=flag;
    QSettings settings(this->userConfigPath,QSettings::IniFormat);
    settings.setValue(MYSYNCHRONIZE,flag);
}

QByteArray DataUntil::getDbData()
{
    QFile file(this->databasePath);
    file.open(QFile::ReadOnly);
    QByteArray ans=file.readAll();
    file.close();
    return ans;
}

void DataUntil::writeDbData(QByteArray dbData)
{
    //写入之前要关闭数据库
    if(this->DB.isOpen()){
        this->DB.close();
    }
    QFile file(this->databasePath);
    file.open(QFile::WriteOnly);
    file.resize(0);
    file.write(dbData);
    file.close();
    //写完后打开数据库
    this->initDateBase();
}

void DataUntil::updateMedifyTime(QString newMedifyTime)
{
    this->medifyTime=newMedifyTime;
    QSettings s(this->userConfigPath,QSettings::IniFormat);
    s.setValue(MEDIFY_TIME,newMedifyTime);
}

//读取某日的计划
QList<Plan *> DataUntil::loadPlan(const QDate &date)
{
    //读取所有 (date在计划开始时间和结束时间之内) (rule！=once而且create_time.date<=date)
    QSqlQuery query(this->DB);
    QString sql="SELECT * FROM plan WHERE (rule=='once' AND substr(startTime,1,10)<=? AND substr(endTime,1,10)>=?) OR (rule!='once' AND substr(createdTime,1,10)<=?);";
    query.prepare(sql);
    query.bindValue(0,date.toString(DFORMAT));
    query.bindValue(1,date.toString(DFORMAT));
    query.bindValue(2,date.toString(DFORMAT));
    if(!query.exec()){
        QMessageBox::warning(nullptr,"错误","读取plan数据库失败："+this->databasePath+",原因："+query.lastError().text());
        exit(0);
    }
    QSqlRecord r;
    QList<Plan *> ans;//返回值
    while(query.next()){
        r=query.record();
        //如果计划满足规则，则放入返回值。
        Plan *plan=new Plan(r.value("createdTime").toString(),r.value("startTime").toString(),
                              r.value("reminderTime").toString(),r.value("endTime").toString(),
                              r.value("content").toString(),r.value("rule").toString(),date);
        if(plan->isConfirmRule(date))
            ans.append(plan);
        else{
            delete plan;
            plan=nullptr;
        }
    }
    return ans;
}
//读取某日的已完成计划
QList<Plan *> DataUntil::loadDonePlan(const QDate &date)
{
    //读取所有 date在开始时间和结束时间之内的一次性计划 和 在startTime当天的有规则计划
    QSqlQuery query(this->DB);
    QString sql="SELECT * FROM donePlan WHERE (rule=='once' AND substr(startTime,1,10)<=? AND substr(endTime,1,10)>=?) OR (rule!='once' AND substr(startTime,1,10)==?)";
    query.prepare(sql);
    query.bindValue(0,date.toString(DFORMAT));
    query.bindValue(1,date.toString(DFORMAT));
    query.bindValue(2,date.toString(DFORMAT));
    if(!query.exec()){
        QMessageBox::warning(nullptr,"错误","读取donePlan数据库失败："+this->databasePath+",原因："+query.lastError().text());
        exit(0);
    }
    QSqlRecord r;
    QList<Plan *> ans;//返回值
    while(query.next()){
        r=query.record();
        Plan *plan=new Plan(r.value("createdTime").toString(),r.value("startTime").toString(),
                              r.value("reminderTime").toString(),r.value("endTime").toString(),r.value("content").toString(),
                              r.value("rule").toString(),date,r.value("completedTime").toString());
        ans.append(plan);
    }
    return ans;
}

QList<Plan *> DataUntil::loadOncePlan()
{
    QSqlQuery query(this->DB);
    QString sql="SELECT * FROM plan WHERE rule=='once';";
    if(!query.exec(sql)){
        QMessageBox::warning(nullptr,"错误","读取数据库的oncePlan时失败："+this->databasePath+",原因："+query.lastError().text());
        exit(0);
    }
    QList<Plan *> ans;//返回值
    QSqlRecord r;
    while(query.next()){
        r=query.record();
        Plan *plan=new Plan(r.value("createdTime").toString(),r.value("startTime").toString(),
                              r.value("reminderTime").toString(),r.value("endTime").toString(),
                              r.value("content").toString(),r.value("rule").toString(),QDate());
        ans.append(plan);
    }
    return ans;
}

QList<Plan *> DataUntil::loadOnceDonePlan()
{
    QSqlQuery query(this->DB);
    QString sql="SELECT * FROM donePlan WHERE rule=='once';";
    if(!query.exec(sql)){
        QMessageBox::warning(nullptr,"错误","读取数据库的onceDonePlan时失败："+this->databasePath+",原因："+query.lastError().text());
        exit(0);
    }
    QList<Plan *> ans;//返回值
    QSqlRecord r;
    while(query.next()){
        r=query.record();
        Plan *plan=new Plan(r.value("createdTime").toString(),r.value("startTime").toString(),
                              r.value("reminderTime").toString(),r.value("endTime").toString(),r.value("content").toString(),
                              r.value("rule").toString(),QDate(),r.value("completedTime").toString());
        ans.append(plan);
    }
    return ans;
}
//创建用户文件夹
bool DataUntil::createUserDir()
{
    bool newUser=false;
    //创建文件夹
    QDir dir(this->userDirPath);
    if(!dir.exists()){
        dir.mkdir(this->userDirPath);
        qDebug()<<"创建"+this->username<<"文件夹";
        if(this->username!="local")
            newUser=true;
    }
    //创建用户配置文件 local则不创建
    if(this->username!="local"){
        QFile file(this->userConfigPath);
        if(!file.exists()){
            QSettings settings(this->userConfigPath,QSettings::IniFormat);
            settings.setValue(MEDIFY_TIME,"2000/01/01 00:00:00");
            settings.setValue(MYSYNCHRONIZE,false);
        }
    }
    return newUser;
}
