#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateTime>
#include <QTimer>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


private slots:
    void on_pushButtonWork_clicked();

private:
    Ui::MainWindow *ui;
    void GetWorkRadarInfo();
    QString readFile(QString fileName);
    QString ReplaceSpaceAndTab(QString str);
    void ReadMNK(QString fileName, int *M, int *N, int *K);
    void readDir(QString dirName);
    void GetNewDateTimeFanWei(QDateTime dtMinDateTime1,QDateTime dtMaxDateTime1,QDateTime dtMinDateTime2,QDateTime dtMaxDateTime2,QDateTime& dtMinDateTimeRslt,QDateTime& dtMaxDateTimeRslt);
    void GenerateTimeList(QDateTime dtMinDateTime,QDateTime dtMaxDateTime,int m,int n,int k);
    void startProcess(QString sFileName);
    void CheckAndCreateDir(QString strDirName);
    bool CheckFileIsExist(QString strPath,QString strFileName);
    void SetEndFlag(void);
    bool NoMosaicCtrl(void);
    bool CheckWorksDone(void);
    QString ReadTempPath();
    QString m_strRunPath;
    QStringList m_strWorkRadarList;
    QStringList m_strWorkTimeList;
    QDateTime m_dtMinDateTime;
    QDateTime m_dtMaxDateTime;
    QTimer *timerWork;
    int m_iWorkLoadCnt;             //当前载入的work的数目
    int m_iWorkDoneCnt;             //当前已经运行完成的work的数目
    int m_iTimerCnt;                //当前已等待的时间
    int m_iPreWorkStartPoint;       //最后一次启动work的时间点
    int m_iAllWorksDoneTimerCnt;    //所有任务完成后timer计数的次数
    int m_WaitInterval;                 //启用QuickMode时候，启动下一次work的等待间隔
    int m_RunWorkMax;
    bool m_bAllWorksLoaded;
    bool m_bAllWorksDone;
    bool m_bQuickMode;
    void getInputDateTime(QString *, QString *);
    void closeEvent(QCloseEvent *event);
    void CreateTimeSequenceIni();
private slots:
    void TimerWork();

    void on_pushButtonQM_clicked();
};

#endif // MAINWINDOW_H
