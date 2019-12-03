#include "mainwindow.h"
#include <QApplication>
#include "configitem.h"
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);



    QTextCodec::setCodecForLocale(QTextCodec::codecForName(QTextCodec::codecForLocale()->name()));

    MainWindow w;
    //去掉最大化
    //QEvent::ignore();
    w.setWindowFlags(w.windowFlags()&~Qt::WindowMaximizeButtonHint);//“&~”取反
    w.show();

    return a.exec();
}
