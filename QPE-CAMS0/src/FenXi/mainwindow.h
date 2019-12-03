#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateTime>

#include"LogWriter.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QString readFile(QString fileName);
    QString ReplaceSpaceAndTab(QString str);
    QList<QString> readDir(QString dirName);

    void startProcess(QString sFileName);
    void writeFile(QString fileName,QString content,bool isAppend);
    void SetTimeFanWei(QString strInText);
    void GetWorkRadarInfo();
    void StartOneRadarWorkProess(QString strInText);
    bool CheckFileTimeIsWorkTime(QString strFileName);
    int GetBaseDataQCNum(void);

    QString m_strRunPath;
    QDateTime m_dtMinDateTime;
    QDateTime m_dtMaxDateTime;
    QDateTime m_dtMidDateTime;
    QStringList m_strWorkRadarList;     //保存着不同雷达的基本信息 
    QString m_strBaseDataQCrsltFenXiTxt;
    QString m_strQCQPEFenXiRsltTxt;
    QString m_strAllFileNameWrite;      //调用baseDataQC处理的所有数据文件名
    QString m_AppName;
    int m_iTmp;
    QDateTime m_dtOptimumDateTime;
    QString m_dtOptimumFileName;
    CLogWriter logWriter;
    QString strMsg;
    int m_iCntQuit;
private slots:
//    void TimerWork();
};

#endif // MAINWINDOW_H
