#ifndef QTTEST_H
#define QTTEST_H


#include <QtGui/QMainWindow>
#include <QtGui/QPaintEvent>
#include "ui_qttest.h"

class qttest : public QMainWindow
{
    Q_OBJECT

public:
    qttest(QWidget *parent = 0, Qt::WFlags flags = 0);
    ~qttest();
    void paintEvent(QPaintEvent *paint);
private slots:
    void click1();
    

private:
    Ui::qttestClass ui;
};

#endif // QTTEST_H
