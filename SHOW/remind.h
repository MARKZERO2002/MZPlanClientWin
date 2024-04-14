#ifndef REMIND_H
#define REMIND_H

#include <QDialog>

namespace Ui {
class Remind;
}

class Remind : public QDialog
{
    Q_OBJECT

public:
    explicit Remind(QString title,QString content,QWidget *parent = nullptr);
    ~Remind();

private slots:
    void on_closeBtn_clicked();
protected:
    void mouseDoubleClickEvent(QMouseEvent *event);
private:
    Ui::Remind *ui;
};

#endif // REMIND_H
