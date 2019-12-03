#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
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

private:
    Ui::MainWindow *ui;

    QString readFile(QString fileName);
    bool CheckAllFile();
    void startProcess(QString sFileName);
    void InvokeRealMosaic();
    bool writeFile(QString fileName);
    void SetEndFlag(void);
    QStringList m_PreparWorkFileName;

    QTimer *timerWork;
    QString m_runPath;
    QString GenMrefDerivProd_Endflag ;
    QString RealMosaic_Endflag ;
    QString GenTrecProd_Endflag ;
    QString GenQpeProd_Endflag;
    QString m_strMidDateTime;
    QString m_strMrefPath;
    QString m_AppName;
    int m_iTaskNumber;      //argv[3]
    int m_iCntTimer;
    int m_iWorkPeriod;
    int m_iNotFindCnt;
    int m_iMatchCnt;
    int m_iReadyCnt;
    bool m_bReady;
    bool m_taskOver;
    bool m_runGenMrefDeivProd;  //是否需要运行MrefDeivProd模块
    bool m_runGenTrecProd;      //是否需要运行TrecProd模块
    bool m_runGenQPEProd;       //是否需要运行QPEProd模块
    bool m_MrefDerivProdReady;
    bool m_TrecProdReady;
    bool m_QpeProdReady;


private slots:
    void TimerWork();
};

#endif // MAINWINDOW_H
