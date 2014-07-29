#include "qttest.h"
#include <windows.h>
#include <QtGui/QPainter>
#include <QtGui/QImage>

qttest::qttest(QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags)
{
    ui.setupUi(this);
    //connect(ui.btn1,SIGNAL(clicked()),this,SLOT(click1()));
}

qttest::~qttest()
{

}

void qttest::click1()
{
    
    char ch[100] = {0};
    
    //sprintf(ch,"the windows id:%d", handle());
   MessageBoxA(NULL, ch,"hello",MB_OK);
}

void qttest::paintEvent(QPaintEvent *paint)
{
    QPainter painter(this);

    painter.drawRect(QRect(0,0,50,50));
    QImage img;
    img.load("f:\\back.bmp");
    
    painter.drawImage(0,0,img);
    
    painter.
}