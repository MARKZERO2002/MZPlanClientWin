# 介绍
一个基于Qt C++编写的Windows程序，用于帮助用户管理自己的日程、计划。
# 类
## SHOW
存放用户交互界面类
- 主界面 MZPlanClientWin (子界面都编写在其StackWidget界面里)
  - 计划界面 planPage
  - 日历界面 calendarPage
  - 设置界面 settingPage
    - 通用设置界面
    - 快捷键设置界面
  - 登陆界面 loginPage
  - 用户信息界面 userInfoPage
- 计划列表界面 PlanList
- 单条计划界面 Plan
- 提醒框界面 Remind
## SERVER
存放业务逻辑类
- UserServer
处理用户登陆、编写设置信息等等的类
- PlanServer
处理添加计划、修改计划等等的类
- TimerServer
处理定时、提醒等等的类
## NETWORK
存放与服务器通信的类
- NetWorkUntil
与服务端通信的类
## DATA
存放操作数据库、文件的类
- DataUntil
操作用户数据的类
# 数据存储
计划等使用sqlite数据库进行存储。(QSqlLite)

系统配置、用户配置用文件进行存储。(QSetting)

## systemConfig.ini
```c++
{
    defaultUser=QString
    position=QSize
    positionLock=false
}

```

