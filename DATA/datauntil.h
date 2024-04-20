#ifndef DATAUNTIL_H
#define DATAUNTIL_H
#include <SHOW/plan.h>
#include <SHOW/planlist.h>
#include <QCoreApplication>
#include <QDir>
#include <QSqlDatabase>
//配置文件的宏定义
#define DEFAULT_USER "defaultUser"
#define POSITION "position"
#define POSITION_LOCK "positionLock"
#define SIZE "size"
#define MYSYNCHRONIZE "synchronize"
#define MEDIFY_TIME "medifyTime"
#define SELF_START "selfStart"
#define SHORTCUT "shortCut"
//日期格式
#define DTFORMAT "yyyy/MM/dd hh:mm:ss"
#define TFORMAT "hh:mm:ss"
#define DFORMAT "yyyy/MM/dd"
class DataUntil
{
public:
    QList<Plan*> choicePlans;
    QList<Plan*> choiceDonePlans;
    QList<Plan*> currentPlans;
    QList<Plan*> currentDonePlans;
    QList<Plan*> oncePlans;
    QList<Plan*> onceDonePlans;
    //各种路径
    QString dataPath=QCoreApplication::applicationDirPath()+QDir::separator()+"data";
    QString systemConfigPath=dataPath+QDir::separator()+"systemConfig.ini";
    QString userDirPath;
    QString userConfigPath;
    QString databasePath;
    //配置信息
    QString username;//用户名
    bool isSynchronize=false;
    QString medifyTime;
    //数据库
    QSqlDatabase DB;
public:
    static DataUntil &getInstance();
    //用户操作
    void updateUser();
    bool changeUser(const QString &username);//更改当前用户
    void removeUserDir();
    //数据操作
    QList<Plan*> getChoicePlans(PlanList::Status status);//获取某个状态的所有计划
    void addPlan(Plan *plan);//添加计划
    void deletePlan(Plan *plan);//删除计划
    void editPlan(Plan *oldPlan,Plan *newPlan);//编辑计划
    void finishPlan(Plan *donePlan);//完成计划
    void reloadChoicePlan(QDate date);
    void reloadCurrentPlan();
    void reloadOnce();
    void setSynchronize(bool flag);
    //数据库
    QByteArray getDbData();
    void writeDbData(QByteArray dbData);
    void updateMedifyTime(QString newMedifyTime);
private:
    DataUntil();
    ~DataUntil();
    //初始化数据
    void initSystemConfig();//初始化系统配置文件
    void initUserConfig();//初始化用户配置文件
    void initDateBase();//初始化数据库
    //用户操作
    bool createUserDir();//创建用户文件夹及数据 本机登陆新用户时调用
    //数据操作
    bool isCompeleted(const Plan* plan);
    void listDeletePlan(Plan* plan);
    void listAppendPlan(Plan* plan);
    bool isCorrelative(const Plan *plan,const QDate &date);
    //数据库操作
    void dbinsert(Plan* plan);
    void dbdelete(Plan* plan);
    QList<Plan*> loadPlan(const QDate &date);//读取某日的计划
    QList<Plan*> loadDonePlan(const QDate &date);//读取某日的已完成计划
    QList<Plan*> loadOncePlan();//读取所有的一次性计划
    QList<Plan*> loadOnceDonePlan();//读取所有的一次性已完成计划
};

#endif // DATAUNTIL_H
