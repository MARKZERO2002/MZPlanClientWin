#include "plandetail.h"
#include "ui_plandetail.h"
#include "DATA/datauntil.h"

#include <QMessageBox>

/**
 * @brief PlanDetail::PlanDetail
 * @param parent
 * 新增计划时调用的构造函数
 */
PlanDetail::PlanDetail(Plan *plan,QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PlanDetail)
{
    ui->setupUi(this);
    this->initWiget();
    this->initValue(plan);
    //按钮组信号连接
    connect(this->btnGroup, SIGNAL(idToggled(int,bool)), this, SLOT(btnClicked(int,bool)));
    connect(this->btnMonthGroup,SIGNAL(idToggled(int,bool)),this,SLOT(btnMonthClicked(int,bool)));
}
void PlanDetail::initValue(Plan *plan)
{
    if(nullptr==plan){//新建计划
        QDateTime currentDateTime=QDateTime::currentDateTime();
        this->ui->BTdateTimeEdit->setDateTime(currentDateTime);
        this->ui->RTdateTimeEdit->setDateTime(currentDateTime.addSecs(30*60));//提醒时间晚于开始时间半个小时
        this->ui->ETdateTimeEdit->setDateTime(currentDateTime.addSecs(60*60));//晚一个小时
        this->setWindowTitle("新建计划");
        this->hideOther(ONCE);
    }else{//编辑原有计划
        this->tplan=plan;
        this->ui->BTdateTimeEdit->setDateTime(plan->startTime);
        this->ui->RTdateTimeEdit->setDateTime(plan->reminderTime);
        this->ui->ETdateTimeEdit->setDateTime(plan->endTime);
        this->ui->rule->setText(plan->rule);
        this->rule=plan->rule;
        this->ui->content->setText(plan->content);
        this->setWindowTitle("编辑计划");
        //设置自动勾选，即按计划的数据勾选按钮
        //勾选类型按钮
        QString pre,suf;
        if(this->rule=="once"){
            this->ui->onceRBtn->setChecked(true);
        }else{
            QList<QString> strs=this->rule.split(':');
            pre=strs[0];
            suf=strs[1];
            suf.removeAt(0);
            suf.removeAt(suf.size()-1);
            if(pre=="week")
                this->ui->weekRBtn->setChecked(true);
            else
                this->ui->monthRBtn->setChecked(true);
        }
        //勾选天数复选框，数据放入checkWeek checkMonth中 顺便更改周、月按钮的隐藏
        QList<QString> checkDays=suf.split(",");
        if(pre=="week"){
            for(auto &day:checkDays){
                int d=day.toInt();
                this->btnGroup->button(d)->setChecked(true);
                this->checkWeek.insert(d);
            }
            this->on_weekRBtn_clicked(true);
        }else if(pre=="month"){
            for(auto &day:checkDays){
                int d=day.toInt();
                this->btnMonthGroup->button(d)->setChecked(true);
                this->checkMonth.insert(d);
            }
            this->on_monthRBtn_clicked();
        }else{
            this->on_onceRBtn_clicked(true);
        }
    }
}
void PlanDetail::initWiget()
{
    //每周按钮组
    btnGroup=new QButtonGroup(this);
    btnGroup->addButton(this->ui->c1,1);
    btnGroup->addButton(this->ui->c2,2);
    btnGroup->addButton(this->ui->c3,3);
    btnGroup->addButton(this->ui->c4,4);
    btnGroup->addButton(this->ui->c5,5);
    btnGroup->addButton(this->ui->c6,6);
    btnGroup->addButton(this->ui->c7,7);
    this->btnGroup->setExclusive(false);//设置复选框不排他
    //每月按钮组
    this->btnMonthGroup=new QButtonGroup(this);
    this->btnMonthGroup->setExclusive(false);
    for(int i=0;i<31;i++){
        QCheckBox *checkBox=new QCheckBox(QString::number(i+1),this);
        this->monthCheckBoxList.append(checkBox);
        this->btnMonthGroup->addButton(checkBox,i+1);
        this->ui->monthLayout->addWidget(checkBox,i/7,i%7);
    }
}
void PlanDetail::hideOther(int checkedBtn)
{
    bool hide_week=false,hide_calendar=false;
    //检测哪个需要隐藏
    if(checkedBtn==0){
        hide_week=true;
        hide_calendar=true;
    }else if(checkedBtn==1){
        hide_calendar=true;
    }else{
        hide_week=true;
    }
    //隐藏
    if(hide_week&&!this->ui->c1->isHidden()){
        for(auto btn:this->btnGroup->buttons())
            btn->setVisible(false);
    }
    if(hide_calendar&&!this->btnMonthGroup->button(1)->isHidden()){
        for(auto btn:this->btnMonthGroup->buttons())
            btn->setVisible(false);
    }
    //需要两次才能变为正确大小，估计是resize后会改变sizehint
    this->resize(this->sizeHint());
    this->resize(this->sizeHint());
}
void PlanDetail::updateRule(Rule which)
{
    QString tempRule;
    bool suffix=false;
    switch(which){
    case ONCE:
        this->rule="once";
        break;
    case WEEK:
        //更新规则
        tempRule="week:[";
        for(auto i:this->checkWeek){
            if(!suffix){
                tempRule.append(QString::number(i));
                suffix=true;
            }
            else
                tempRule.append(","+QString::number(i));
        }
        tempRule.append(']');
        this->rule=tempRule;
        break;
    case MONTH:
        //更新规则
        tempRule="month:[";
        for(auto i:this->checkMonth){
            if(!suffix){
                tempRule.append(QString::number(i));
                suffix=true;
            }else
                tempRule.append(","+QString::number(i));
        }
        tempRule.append(']');
        this->rule=tempRule;
        break;
    }
    this->ui->rule->setText(this->rule);
}
PlanDetail::~PlanDetail()
{
    delete ui;
    delete this->btnGroup;
    for(auto btn:this->btnMonthGroup->buttons())
        delete btn;
    delete this->btnMonthGroup;
}

Plan *PlanDetail::toPlan()
{
    QString format;
    //如果当前是once被选中或当前计划已完成，则要使用QDateTime格式yyyy/MM/dd hh:mm:ss
    if(this->ui->onceRBtn->isChecked()||this->tplan->completedTime.isValid()){
        format=DTFORMAT;
    }else{//否则使用QTime格式hh:mm:ss
        format=TFORMAT;
    }
    //构建计划
    QString cT, co,  bT,  eT,  rT,  rl;
    QString fT=nullptr;
    if(nullptr==this->tplan)
        cT=QDateTime::currentDateTime().toString(DTFORMAT);
    else
        cT=this->tplan->createdTime.toString(DTFORMAT);
    co=this->ui->content->toPlainText();
    bT=this->ui->BTdateTimeEdit->dateTime().toString(format);
    rT=this->ui->RTdateTimeEdit->dateTime().toString(format);
    eT=this->ui->ETdateTimeEdit->dateTime().toString(format);
    rl=this->ui->rule->text();
    //根据用户输入的值，生成一个新的计划或编辑原有计划
    if(nullptr==this->tplan){//新建计划
        //建立一个新计划 不用在此处delete mydata中统一delete
        Plan* plan=new Plan(cT,bT,rT,eT,co,rl,QDate::currentDate(),fT);//不用delete，mydata会统一delete
        return plan;
    }
    //编辑计划，返回该计划
    if(!this->tplan->completedTime.isNull())
        fT=this->tplan->completedTime.toString(DTFORMAT);
    this->tplan=new Plan(cT,bT,rT,eT,co,rl,QDate::currentDate(),fT);
    return this->tplan;
}
PlanDetail::InvalidInput PlanDetail::checkValid()
{
    //检测内容不为空
    if(this->ui->content->toPlainText().isNull()||this->ui->content->toPlainText().isEmpty()){
        return this->contentIsNull;
    }
    //检测日期合法
    if(this->rule=="once"){//一次性的，开始时间要在结束时间之前，提醒时间要在开始时间之后，结束时间要在提醒时间之前
        QDateTime bT=this->ui->BTdateTimeEdit->dateTime(),rT=this->ui->RTdateTimeEdit->dateTime(),dT=this->ui->ETdateTimeEdit->dateTime();
        if(bT.secsTo(dT)<=0){
            return this->bTInvalid;
        }else if(rT.secsTo(bT)>=0||rT.secsTo(dT)<0){
            return this->rTInvalid;
        }
    }else{//非一次性的，提醒时间和结束时间可以在开始时间之前，说明是第二天的，

    }
    //检测规则合法,以once、week、month开头，后两者要有:[数字] 正则判断
    QString rule=this->ui->rule->text();
    QRegularExpression reg(R"(^(week|month)(:\[)(\d+(,\d+)*)\])");
    if(rule=="once"){//匹配once成功

    }
    else if(!reg.match(rule).hasMatch()){//匹配其它的失败
        return this->ruleInvalid;
    }
    return this->allValid;
}

void PlanDetail::on_accept_clicked()
{
    int checkResult=checkValid();
    switch(checkResult){
    case this->allValid://通过检验
        this->accept();
        break;
    case this->contentIsNull:
        QMessageBox::information(this,"提示","计划内容不能为空");
        break;
    case this->bTInvalid:
        QMessageBox::information(this,"提示","开始时间要在结束时间之前");
        break;
    case this->rTInvalid:
        QMessageBox::information(this,"提示","提醒时间要在开始时间和结束时间之间");
        break;
    case this->ruleInvalid:
        QMessageBox::information(this,"提示","生成规则格式错误");
        break;
    }
}


void PlanDetail::on_reject_clicked()
{
    this->close();
}


void PlanDetail::on_onceRBtn_clicked(bool checked)
{
    Q_UNUSED(checked);
    this->hideOther(ONCE);
    this->updateRule(ONCE);
    //更改日期的显示，显示日期+时间
    this->ui->BTdateTimeEdit->setDisplayFormat("yyyy/MM/dd hh:mm:ss");
    this->ui->RTdateTimeEdit->setDisplayFormat("yyyy/MM/dd hh:mm:ss");
    this->ui->ETdateTimeEdit->setDisplayFormat("yyyy/MM/dd hh:mm:ss");
}


void PlanDetail::on_weekRBtn_clicked(bool checked)
{
    Q_UNUSED(checked);
    this->hideOther(WEEK);
    //根据复选框创造规则
    this->updateRule(WEEK);
    this->ui->rule->setText(this->rule);
    //显示七天的复选框
    for(auto btn:this->btnGroup->buttons()){
        btn->show();
    }
    //更改日期的显示，只显示时间
    this->ui->BTdateTimeEdit->setDisplayFormat("hh:mm:ss");
    this->ui->RTdateTimeEdit->setDisplayFormat("hh:mm:ss");
    this->ui->ETdateTimeEdit->setDisplayFormat("hh:mm:ss");
}


void PlanDetail::on_monthRBtn_clicked()
{
    this->hideOther(MONTH);
    //根据复选框创造规则
    this->updateRule(MONTH);
    this->ui->rule->setText(this->rule);
    //显示31个复选框
    for(auto btn:this->btnMonthGroup->buttons()){
        btn->show();
    }
    //更改日期的显示，只显示时间
    this->ui->BTdateTimeEdit->setDisplayFormat("hh:mm:ss");
    this->ui->RTdateTimeEdit->setDisplayFormat("hh:mm:ss");
    this->ui->ETdateTimeEdit->setDisplayFormat("hh:mm:ss");
}
void PlanDetail::btnClicked(int btnId,bool status)
{
    if(status)
        this->checkWeek.insert(btnId);
    else
        this->checkWeek.erase(btnId);
    updateRule(WEEK);
}
void PlanDetail::btnMonthClicked(int btnId, bool status)
{
    if(status)
        this->checkMonth.insert(btnId);
    else
        this->checkMonth.erase(btnId);
    updateRule(MONTH);
}
