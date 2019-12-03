#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "inputdialog.h"
#include <QTableWidget>
#include <QSystemTrayIcon>
#include <QProcess>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void inputTimeDate(QString fileName);
    void setTimeDate(QString fileName);
    void writeHeightFile(QString fileName, QString content, bool isAppend);
    void saveHeightConfig(QString fileName);
    void saveProductGenerateConfig(QString fileName);
    void writeFile(QString fileName, QString content, bool isAppend);
    //void loadTableContent(QString strRadarFileName);


    void checkFile();
    void deleteExpiredLogFile();


    void readRadarInfor();//读取雷达临时文件,获取雷达站名
    void showOrHide();

    void changeTitle();

    //void closeEvent(QCloseEvent *event);
    void shiShiChangeTitle();
signals:
    void mainWindowSendData(QString);
private slots:
    void on_action_triggered();
    void on_action_3_triggered();//历史模式分析
    void on_action_7_triggered();
    void on_pushButton_apply_clicked();
    void on_pushButton_cancel_clicked();
    QHash<QString,QString> parseHeigh(QString strHeigh);
    QString saveHeigh(QHash<QString,QString> heightList);
    void saveConfig(QString fileName);
    void ShowInfo(QHash<QString,QString> list, QString fileName);
    bool parseBool(QString tmpString);
    QString boolParse(bool tmpBool);
    void saveTableContent(QString fileName);
    QString ColumnParse(int coulumnID);
    void showFile();
    void showTmpFile();
    void showInputDlg();
    void loadTableContent(QString strRadarFileName);
    void receiveData(QString data);
    void showInputEditDlg();
    void BindConnection();
    QHash<QString,QString> loadConfig(QString fileName);
    QString getStardString(QString str);
    QString getReplaceString(QString str);
    QString ReplaceSpaceAndTab(QString str);
    QString readFile(QString fileName);
    void writeConfig(QString fileName,QHash<QString,QString> configList);
    void on_tableWidget_itemDoubleClicked(QTableWidgetItem *item);

    void on_pushButton_delete_clicked();

    void on_pushButton_Select_clicked();

    void startMyTimer();
    void timertick();
    bool hasFile(QString sFileName);
    void startProcess(QString sFileName);
    QList<QString> readDir(QString dirName);
    QList<QString> GetFitConditionFile(QList<QString> srcList);

    void startReadOperationInfoTimer();
    void ReadOperationInfoTimerTick();
    void lastTimerTick();
    void startRaderProcess();

   // void on_actionFenXi_triggered();

    void on_action_2_triggered();   //调用实时分析模式
    void timerWork();
    void addBtnTimerWork();

    void chooseRadarFromMap();
    void chooseRadarFromList();
    void addRadarFromMap();
    void addRadarFromList();
    void receiveAddRadarFromListData(QString data);

    void on_action_8_triggered();//退出

    void on_Hide_triggered();//模式可见性,隐藏

    void on_Show_triggered();//模式可见性,显示

    void boolProcessDone();//槽函数,用于变化title

    void deleteFileTimerWork();//槽函数,计数器,用于删除空文件

    void shiShiDeleteFileTimerWork();//实时,槽函数,计数器,用于删除空文件

    void boolShiShiProcessDone();//槽函数,用于变化title
    void addDeleteTimerWork();//槽函数,计数器,用于删除添加雷达时的空文件
    void chooseDeleteTimerWork();//槽函数,计数器,用于删除重建雷达时的空文件
    void iconIsActived(QSystemTrayIcon::ActivationReason reason);
    void on_actionStop_triggered();

    void on_toolButton_clicked();

    void QOK_clicked();
private:

    QCheckBox *MapBox;
    QCheckBox *ListBox;
    QPushButton *QOK;

    QString strData;//用于保存选中的RadarList.ini里的信息
    Ui::MainWindow *ui;

    QTimer *timer;
    QTimer *addTimer;
    QTimer *titleTimer;
    QTimer *shiShiTitleTimer;
    QTimer *deleteFileTimer;
    QTimer *shiShiDeleteFileTimer;
    QTimer *addDeleteFileTimer;
    QTimer *chooseDeleteFileTimer;

    QAction *action_show;
    QAction *action_quit;
    QAction *action_minimize;
    QAction *action_ShiShiFenXi;
    QAction *action_LiShiFenXi;
    QAction *action_Stop;

    int count;
    int addDeleteCount;
    int chooseDeleteCount;
    int iconShishiChangeCount = 0;
    int iconLishiChangeCount = 0;

    QSystemTrayIcon *systemTrayIcon;

    QDialog *chooseDialog;
    QDialog *dialog;//用于选择雷达时的窗口
    QDialog *addRadarDialog;

    QProcess *newProcess;
    QProcess *myProcess;

    QProcess *chooseProcess;
    QProcess *addProcess;
    QProcess *liShiProcess;
    QProcess *shiShiProcess;

    QStringList strRadarName;            //用于保存之前sTemporaryRadarFileName里的雷达站名
    void myTrayIcon();

   // void changeEvent(QEvent *event);
    void closeEvent(QCloseEvent *event);
    void saveDataPath(QString fileName);
    void saveRadarRegion();
    void showRadarRegion();
};

#endif // MAINWINDOW_H
