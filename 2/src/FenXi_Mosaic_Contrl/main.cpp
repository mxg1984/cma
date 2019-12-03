#include "mainwindow.h"
#include <QApplication>
#include <QSettings>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    QSettings settings("./developer.ini", QSettings::IniFormat);
    bool bShowForm=settings.value("FormOption/bShowForm","false").toBool();
    if(bShowForm) w.show();

    return a.exec();
}




