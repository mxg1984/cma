#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QStringList>
#include <QTreeWidgetItem>
#include <QSettings>
#include <QDebug>
#include <QFile>
#include <QStandardItemModel>
#include <QFileDialog>
#include <tabledialog.h>
#include <QProcess>
#include <QtCore>
#include "Common.h"
#include <QThread>
#include <QMutex>
#include <QTextStream>
#include <LogWriter.h>
#include <QSystemTrayIcon>
#include <QCloseEvent>

const static QString sTimeParameterConfigFileName="timeParameterConfig.ini";
const static QString sDataHandlerConfigFileName="dataHandlerConfig.ini";
const static QString sProductGenerateConfigFileName="productGenerateConfig.ini";
const static QString sHeightAndResolutionConfigFileName="heightAndResolutionConfig.ini";
const static QString sDataPathConfigFileName="dataPathConfig.ini";
const static QString sRadarFileName="Radar.ini";
const static QString sRadarListFileName="RadarList.ini";
const static QString sDateTimeFileName = "inputTimeDate.ini";
const static QString sTemporaryRadarFileName="TemporaryRadarList.ini";
const static QString sMapFlagFileName="MapFlag.txt";
const static QString sAddMapRadarFlagFileName="AddMapRadarFlag.txt";
const static QString sFenXiCtrlCompleteName="FenXiCtrl_Complete.txt";
const static QString sShiShiFenXiCtrlCompleteName="ShiShiFenXiCtrl_Complete.txt";
const static QString sHideOrShowName="developer.ini";
const static QString sReplaceOperate="!@#$%";

//文件所在目录
const static QString sDataFilePath="testFile/";
//testFile所在的目录
const static QString sDataTestFilePath="testFile/";

//raderFile所在的目录
const static QString sRaderFilePath="raderFile/";

//开启的进程名称
const static QString sStartProcessName="ThreadMutex";


const int LastTimeSpan=3;
//主timer的启动间隔M
int g_mainTimerMSpan=1000*60*8;

//主timer需要查找的区间N
int g_mainTimerNSpan=1000*60*3;

//根据上面m,n计算出主timer的实际启动时间
static int mainTimer;

//实际开启主timer的延迟时间
static int mainTimerDelay=1000*60*1;

//文件要查询的时间
static QDateTime currentMyDateTime;

//二维公共数组
static QMap<int,QList<QString> > taskOperateInfo;
//列表三个
static int taskOperateInfoCount=3;
//这个timer3000秒启动一次
static int ReadOperationInfoTimerSpan=3000;
//当前是第几个
static int taskOperateCurrentIndex=0;


//
static quint64 fSttttttttartTime=20140518023041;
static quint64 fSttttttttartSpanTime=80;


//lastTimer
static int raderTickSpan=60*LastTimeSpan;

MainWindow::MainWindow(QWidget *parent) :QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setWindowTitle(tr("大区域天气雷达数据QC及组网QPE系统"));

    //绑定按钮事件
    BindConnection();
    //判断相应文件夹是否存在,以及相应文件是否存在
    checkFile();
    //分析模式"显示"或者"隐藏"初始化
    showOrHide();

    strData ="";

    //加载配置信息
    ShowInfo(loadConfig(sTimeParameterConfigFileName),sTimeParameterConfigFileName);
    ShowInfo(loadConfig(sDataHandlerConfigFileName),sDataHandlerConfigFileName);
    ShowInfo(loadConfig(sProductGenerateConfigFileName),sProductGenerateConfigFileName);
    ShowInfo(loadConfig(sHeightAndResolutionConfigFileName),sHeightAndResolutionConfigFileName);
    ShowInfo(loadConfig(sDataPathConfigFileName),sDataPathConfigFileName);
    setTimeDate(sDateTimeFileName);
    showRadarRegion();//显示区域名称

    //加载表格数据
    loadTableContent(sRadarFileName);
    ui->tableWidget->setShowGrid(true);


    //这个表格先隐藏起来
    ui->listWidget->setVisible(false);
    ui->listWidget_2->setVisible(false);
    ui->listWidget_3->setVisible(false);
    ui->listWidget_4->setVisible(false);


    //开启读取操作信息的timer
    startReadOperationInfoTimer();

    deleteExpiredLogFile();//删除过期日志

   myTrayIcon();

   chooseProcess = new QProcess(this);
   addProcess = new QProcess(this);
   shiShiProcess = new QProcess(this);
   liShiProcess = new QProcess(this);
}


/*****************************************************实时分析Ctrl变化Title**************************************************/
void MainWindow::shiShiChangeTitle()
{
    iconShishiChangeCount=0;
    shiShiTitleTimer = new QTimer(this);
    connect(shiShiTitleTimer,SIGNAL(timeout()),this,SLOT(boolShiShiProcessDone()));
    shiShiTitleTimer->start(500);
    systemTrayIcon->setToolTip("实时模式进行中");
    setWindowTitle(tr("实时模式进行中..."));
}

/**************************************************槽函数,用于实时分析Ctrl变化title,搜索标志文件*********************************************/
void MainWindow::boolShiShiProcessDone()
{
    iconShishiChangeCount++;
    if(iconShishiChangeCount%2==0)
    {
        systemTrayIcon->setIcon(QIcon("greenTrayIcon.png"));
    }
    else
    {
        systemTrayIcon->setIcon(QIcon("trayIcon.png"));
    }


    QString m_strRunPath = QCoreApplication::applicationDirPath();
    QFile isFile(m_strRunPath+"/Temp/"+sShiShiFenXiCtrlCompleteName);//这里是不断扫描整个程序运行完成的标志文件
    if(isFile.exists())
    {
        iconShishiChangeCount=0;
        systemTrayIcon->setIcon(QIcon("trayIcon.png"));
        ui->action_2->setEnabled(true);//在进行实时分析的时候,会让“实时分析”菜单不可操作,整个过程结束后,使“实时分析”菜单再次可以操作
        ui->action_3->setEnabled(true);
        action_ShiShiFenXi->setEnabled(true);
        action_LiShiFenXi->setEnabled(true);
        shiShiTitleTimer->stop();

        systemTrayIcon->setToolTip("大区域天气雷达数据QC及组网QPE系统");
        setWindowTitle(tr("大区域天气雷达数据QC及组网QPE系统"));
        systemTrayIcon->showMessage(QString("QC_QPE"), QString("实时模式处理完成"));

        //下面是用于删除这个空文件,延时2s
        count = 1;
        shiShiDeleteFileTimer = new QTimer(this);
        connect(shiShiDeleteFileTimer,SIGNAL(timeout()),this,SLOT(shiShiDeleteFileTimerWork()));
        shiShiDeleteFileTimer->start(1000);
    }
}

/***********************************************实时,槽函数,计数器,用于删除空文件**************************************************/
void MainWindow::shiShiDeleteFileTimerWork()
{
    QString m_strRunPath = QCoreApplication::applicationDirPath();
    if(count<=0)
    {
        shiShiDeleteFileTimer->stop();
        //删除空文件
        QDir myDir;
        myDir.remove(m_strRunPath+"/Temp/"+sShiShiFenXiCtrlCompleteName);
    }
    else
    {
        count--;
    }
}

/*********************************************************变化Title***********************************************************/
void MainWindow::changeTitle()
{
    iconLishiChangeCount=0;
    titleTimer = new QTimer(this);
    connect(titleTimer,SIGNAL(timeout()),this,SLOT(boolProcessDone()));
    titleTimer->start(500);
    systemTrayIcon->setToolTip("分析模式进行中");
    setWindowTitle(tr("分析模式进行中..."));
}

/*************************************************槽函数,用于变化title,搜索标志文件***********************************************/
void MainWindow::boolProcessDone()
{
    iconLishiChangeCount++;
    if(iconLishiChangeCount%2==0)
    {
        systemTrayIcon->setIcon(QIcon("yellowTrayIcon.png"));
    }
    else
    {
        systemTrayIcon->setIcon(QIcon("trayIcon.png"));
    }

    QString m_strRunPath = QCoreApplication::applicationDirPath();
    QFile isFile(m_strRunPath+"/Temp/"+sFenXiCtrlCompleteName);//这里是不断扫描整个程序运行完成的标志文件
    if(isFile.exists())
    {
        iconShishiChangeCount=0;
        systemTrayIcon->setIcon(QIcon("trayIcon.png"));
        ui->action_3->setEnabled(true);//在进行历史分析的时候,会让“历史分析”菜单不可操作,整个过程结束后,使“历史分析”菜单再次可以操作
        ui->action_2->setEnabled(true);
        action_ShiShiFenXi->setEnabled(true);
        action_LiShiFenXi->setEnabled(true);
        titleTimer->stop();
        systemTrayIcon->setToolTip("大区域天气雷达数据QC及组网QPE系统");
        setWindowTitle(tr("大区域天气雷达数据QC及组网QPE系统"));
        systemTrayIcon->showMessage(QString("QC_QPE"), QString("分析模式处理完成"));

        //下面是用于删除这个空文件,延时2s
        count = 1;
        deleteFileTimer = new QTimer(this);
        connect(deleteFileTimer,SIGNAL(timeout()),this,SLOT(deleteFileTimerWork()));
        deleteFileTimer->start(1000);
    }  
}

/***********************************************槽函数,计数器,用于删除空文件****************************************************/
void MainWindow::deleteFileTimerWork()
{
    QString m_strRunPath = QCoreApplication::applicationDirPath();
    if(count<=0)
    {
        deleteFileTimer->stop();
        //删除空文件
        QDir myDir;
        myDir.remove(m_strRunPath+"/Temp/"+sFenXiCtrlCompleteName);
    }
    else
    {
        count--;
    }
}

/*********************************************************删除过期日志***********************************************************/
void MainWindow::deleteExpiredLogFile()
{
    CLogWriter WriteLogFile;
    int n =ui->spinBox->text().toInt();
    WriteLogFile.DeleteExpiredLogFile(n);
}

/**********************************************判断文件夹是否存在,以及相应文件是否存在*************************************************/
void MainWindow::checkFile()
{


    QString m_strRunPath = QCoreApplication::applicationDirPath();

    QDir *temp = new QDir;
    QFile TempFile(m_strRunPath+"/Temp");
    bool isExist = TempFile.exists();
    if(!isExist)
    {
        temp->mkdir(m_strRunPath+"/Temp");
    }

    QFile file(m_strRunPath+"/NOWCASTDATA");
    bool isFileExist = file.exists();
    if(!isFileExist)
    {
        temp->mkdir(m_strRunPath+"/NOWCASTDATA");
    }
    else
    {
        QFile isFile(m_strRunPath+"/NOWCASTDATA/PrevMosaic.tmp");
        if(isFile.exists())
        {
            QDir myDir;
            myDir.remove(m_strRunPath+"/NOWCASTDATA/PrevMosaic.tmp");
        }
    }

    QDir myDir;
    myDir.remove(m_strRunPath+"/Temp/"+sMapFlagFileName);
    myDir.remove(m_strRunPath+"/Temp/"+sTemporaryRadarFileName);
    QString radarContent;
    radarContent = readFile(sRadarFileName);//这里就是复制
    writeFile(m_strRunPath+"/Temp/"+sTemporaryRadarFileName,radarContent,false);//复制到临时ini中


    QFile fileRadarRegion(m_strRunPath+"/radarRegion.ini");
    if(!fileRadarRegion.exists())
    {
        QSettings settings("./radarRegion.ini", QSettings::IniFormat);
        settings.setValue("Region/strRadarRegion","");
    }

    QFile fileMapOrList(m_strRunPath+"/mapOrList.ini");
    if(!fileMapOrList.exists())
    {
        QSettings settings("./mapOrList.ini", QSettings::IniFormat);
        settings.setValue("FormOption/bShowForm",true);
    }
}

/*********************************************************定时器***********************************************************/
//定时器
void MainWindow::startReadOperationInfoTimer()
{
    QTimer *timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(ReadOperationInfoTimerTick()));
    timer->start(ReadOperationInfoTimerSpan);   // 3s为间隔
}

/*********************************************************获取目录下所有文件***********************************************************/
void MainWindow::ReadOperationInfoTimerTick()
{
    //获取目录下所有文件
    //如果文件是在1下的，显示在1里
    //在2下的显示在2里
    //...
    //如果文件不在这里面，但是有删除标志，删掉它

    QList<QString> listFilePathList=readDir("taskFileInfo/");//“taskFileInfo”这是什么意思，哪的目录

    QList<QString> listFilePathOnlyName;

    for(int i=0;i<listFilePathList.count();i++)
    {
        QFileInfo file_info = listFilePathList.at(i);
        QString file_onlyName = file_info.fileName();
        listFilePathOnlyName.append(file_onlyName);
    }


    for(int i=0;i<listFilePathOnlyName.count();i++)
    {
        QString currentFilePath=listFilePathOnlyName[i];

        if(taskOperateInfo[0].contains(currentFilePath))
        {
            ui->listWidget_2->addItem(new QListWidgetItem(currentFilePath+"执行结束"));
        }
        else if(taskOperateInfo[1].contains(currentFilePath))
        {
            ui->listWidget_3->addItem(new QListWidgetItem(currentFilePath+"执行结束"));
        }
        else if(taskOperateInfo[2].contains(currentFilePath))
        {
            ui->listWidget_4->addItem(new QListWidgetItem(currentFilePath+"执行结束"));
        }
        else
        {
            /*QFileInfo file_info = listFilePathList.at(i);
            QString file_onlyName = file_info.fileName();
            QFile::remove(file_onlyName);*/
        }

        QFile f;
        f.remove(listFilePathList.at(i));
    }

}

/******************************************************按钮事件*******************************************************/
void MainWindow::BindConnection()
{
    //绑定数据路径浏览  产品
    connect(ui->pushButton_brow2,SIGNAL(clicked()),this,SLOT(showFile()));
    //绑定数据路径浏览  临时数据
    connect(ui->pushButton_brow2_3,SIGNAL(clicked()),this,SLOT(showTmpFile()));
    //绑定添加雷达站添加按钮事件
    connect(ui->pushButton_ADD,SIGNAL(clicked()),this,SLOT(showInputDlg()));
    //绑定添加雷达站编辑事件
    connect(ui->pushButton_modify,SIGNAL(clicked()),this,SLOT(showInputEditDlg()));
}

/*********************************************************slot数据路径,产品***********************************************************/
void MainWindow::showFile()
{
    QString s = QFileDialog::getExistingDirectory(this,"浏览",QDir::currentPath());
    //ui->txtOutputProduct->setText(s);

    if(s == NULL)
    {
        ui->txtOutputProduct->setText(ui->txtOutputProduct->text());
    }
    else
    {
        ui->txtOutputProduct->setText(s);
    }
}

/**************************************************slot数据路径,临时数据******************************************************/
void MainWindow::showTmpFile()
{
    QString s = QFileDialog::getExistingDirectory(this,"浏览",QDir::currentPath());
    //ui->txtOutputTmp->setText(s);

    if(s == NULL)
    {
        ui->txtOutputTmp->setText(ui->txtOutputTmp->text());
    }
    else
    {
        ui->txtOutputTmp->setText(s);
    }
}

/******************************************************修改按钮响应事件*******************************************************/
void MainWindow::showInputEditDlg()
{
    inputDialog* inputDlg = new inputDialog(this);
    connect(this,SIGNAL(mainWindowSendData(QString)),inputDlg,SLOT(receiveMainWindowData(QString)));//接受emit信号

    int nCurrenRow = ui->tableWidget->currentRow();
    qDebug()<<nCurrenRow;
    QString sendItem="";
    for(int j=0;j<ui->tableWidget->columnCount();j++)
    {
        if(ui->tableWidget->item(nCurrenRow,j)==NULL||(ui->tableWidget->item(nCurrenRow,j)&&ui->tableWidget->item(nCurrenRow,j)->text()==tr("")))
        {
            sendItem+=" "+sReplaceOperate;
        }
        else
        {
            sendItem+=ui->tableWidget->item(nCurrenRow,j)->text()+sReplaceOperate;
        }
    }

    emit mainWindowSendData(sendItem);   //emit发射信号
    connect(inputDlg,SIGNAL(sendData(QString)),this,SLOT(receiveData(QString)));
    inputDlg->show();
}

/******************************************************添加按钮响应事件*******************************************************/
void MainWindow::showInputDlg()
{
//    addRadarDialog = new QDialog(this);
//    addRadarDialog->setWindowTitle("选择雷达方式");
//    addRadarDialog->setGeometry(300,300,100,100);

//    QPushButton *mapButton =new QPushButton;
//    mapButton->setText(tr("地图"));
//    mapButton->setBaseSize(10,10);
//    QPushButton *listButton =new QPushButton;
//    listButton->setText(tr("列表"));
//    listButton->setBaseSize(10,10);

//    QHBoxLayout *queryLayout = new QHBoxLayout();
//    queryLayout->addWidget(mapButton);
//    queryLayout->addWidget(listButton);

//    QGridLayout *mainLayout=new QGridLayout(addRadarDialog);
//    mainLayout->addLayout(queryLayout,0,0);
//    addRadarDialog->show();

//    connect(mapButton,SIGNAL(clicked()),this,SLOT(addRadarFromMap()));
//    connect(listButton,SIGNAL(clicked()),this,SLOT(addRadarFromList()));



    QSettings settings("./mapOrList.ini", QSettings::IniFormat);
    QString value = settings.value("FormOption/bShowForm",false).toString();

    if(value=="true")
    {
        addRadarFromMap();
    }
    else if(value=="false")
    {
        addRadarFromList();
    }
}

/******************************************************地图添加雷达*******************************************************/
void MainWindow::addRadarFromMap()
{
    //addRadarDialog->close();
    QString m_strRunPath = QCoreApplication::applicationDirPath();
    addProcess->start(m_strRunPath+"/addMapRadar");
    addTimer = new QTimer(this);
    connect(addTimer, SIGNAL(timeout()), this, SLOT(addBtnTimerWork()));
    addTimer->start(500);
}

/******************************************************列表添加雷达*******************************************************/
void MainWindow::addRadarFromList()
{
    //addRadarDialog->close();
    tableDialog* tableDlg = new tableDialog(this);
    connect(tableDlg,SIGNAL(sendData(QString)),this,SLOT(receiveAddRadarFromListData(QString)));
    tableDlg->show();
}

/******************************************************列表添加雷达槽函数*******************************************************/
void MainWindow::receiveAddRadarFromListData(QString data)
{
    //data保存的就是选择的雷达
    readRadarInfor();//每次都要获取TemporaryRadarList.ini中的雷达信息
    QString m_strRunPath = QCoreApplication::applicationDirPath();
    QString preContent = readFile(m_strRunPath+"/Temp/"+sTemporaryRadarFileName);//之前的雷达信息

    //将要修改的文件先保存在临时文件中
    strData = data;
    QStringList strNewRadarName;//新增加的雷达名
    QStringList strNewDataLine;
    QStringList newRadarList;//用于清除与之前雷达重复的一个雷达列表,这个列表不会与之前的雷达重复
    if(!data.isNull())
    {
        strNewDataLine = data.split("\r\n");//新增雷达信息的每一行

        for(int i=0;i<strNewDataLine.size();i++)
        {
           if(strNewDataLine.at(i)!=NULL)
           {
               QStringList strRadarListItem = strNewDataLine.at(i).simplified().split(" "); //每一行按空格分割
               QString zhanMing = strRadarListItem.at(1);
               qDebug()<<"zhanMingzhanMing"<<zhanMing;
               strNewRadarName.append(zhanMing);
               newRadarList.append(strNewDataLine.at(i));
           }
        }
    }

    //这里是之前的雷达与新选的雷达比较,看看是否有重复的,若有则去掉
    for(int m=0;m<strRadarName.size();m++)
    {
        for(int n=0;n<strNewRadarName.size();n++)
        {
            if(strRadarName.at(m).trimmed()==strNewRadarName.at(n).trimmed())
            {
                qDebug()<<"重复"<<strNewRadarName;
                //这里要进行重复代码的操作
                newRadarList.removeOne(strNewDataLine.at(n));
            }
        }
    }
    QString strFinalRadar;//最终的新增雷达信息
    for(int i=0;i<newRadarList.size();i++)
    {
        strFinalRadar = strFinalRadar + newRadarList.at(i)+"\r\n";
    }
    //writeFile(m_strRunPath+"/Temp/"+sTemporaryRadarFileName,data,false);
    //writeFile(m_strRunPath+"/Temp/"+sTemporaryRadarFileName,preContent+data,false);
    writeFile(m_strRunPath+"/Temp/"+sTemporaryRadarFileName,preContent+strFinalRadar,false);

    //加载配置信息
    ShowInfo(loadConfig(sTimeParameterConfigFileName),sTimeParameterConfigFileName);
    ShowInfo(loadConfig(sDataHandlerConfigFileName),sDataHandlerConfigFileName);
    ShowInfo(loadConfig(sProductGenerateConfigFileName),sProductGenerateConfigFileName);
    ShowInfo(loadConfig(sHeightAndResolutionConfigFileName),sHeightAndResolutionConfigFileName);
    ShowInfo(loadConfig(sDataPathConfigFileName),sDataPathConfigFileName);
    //加载表格数据
    loadTableContent(m_strRunPath+"/Temp/"+sTemporaryRadarFileName); //这里加载的是sRadarFileName.ini里的内容
}

/*****************************************************读取雷达临时文件,获取雷达站名************************************************/
void MainWindow::readRadarInfor()
{
    //这里讲strRadarName设为全局变量有问题,重复点击的时候,之前的数据不会清空,必须添加clear()
    strRadarName.clear();
    qDebug()<<"clear"<<strRadarName;
    QString m_strRunPath = QCoreApplication::applicationDirPath();
    QString strRadarInfor = readFile(m_strRunPath+"/Temp/"+sTemporaryRadarFileName);
    QStringList strRadarList = strRadarInfor.split("\r\n");

    for(int i=0;i<strRadarList.size();i++)
    {
       if(strRadarList.at(i)!=NULL)
       {
           QStringList strRadarListItem = strRadarList.at(i).simplified().split(" "); //每一行按空格分割
           QString zhanMing = strRadarListItem.at(1);
           qDebug()<<"zhanMing"<<zhanMing;
           strRadarName.append(zhanMing);//保存所有雷达站的站名
       }
    }
    qDebug()<<strRadarName;
}

/******************************************************地图添加雷达,用一空文件作为信号*********************************************/
void MainWindow::addBtnTimerWork()
{
    QString m_strRunPath = QCoreApplication::applicationDirPath();
    QFile isFile(m_strRunPath+"/Temp/"+sAddMapRadarFlagFileName);
    if(isFile.exists())
    {
        addTimer->stop();
        loadTableContent(m_strRunPath+"/Temp/"+sTemporaryRadarFileName); //这里加载的是sTemporaryRadarFileName.ini里的内容

        //下面是用于删除这个空文件,延时0.5s
        addDeleteCount = 1;
        addDeleteFileTimer = new QTimer(this);
        connect(addDeleteFileTimer,SIGNAL(timeout()),this,SLOT(addDeleteTimerWork()));
        addDeleteFileTimer->start(500);
    }

    //感觉这里做个几秒钟的延迟,然后再删除比较好吧
//    QDir myDir;
//    myDir.remove(m_strRunPath+"/Temp/"+sAddMapRadarFlagFileName);
}

/******************************************槽函数,计数器,用于删除添加雷达时的空文件**********************************************/
void MainWindow::addDeleteTimerWork()
{
    QString m_strRunPath = QCoreApplication::applicationDirPath();
    if(addDeleteCount<=0)
    {
        addDeleteFileTimer->stop();
        //删除空文件
        QDir myDir;
        myDir.remove(m_strRunPath+"/Temp/"+sAddMapRadarFlagFileName);
    }
    else
    {
        addDeleteCount--;
    }
}

/******************************************************双击Item*******************************************************/
void MainWindow::on_tableWidget_itemDoubleClicked(QTableWidgetItem *item)
{
    showInputEditDlg();
}

/******************************************************删除button*******************************************************/
void MainWindow::on_pushButton_delete_clicked()
{
    QTableWidgetItem * item = ui->tableWidget->currentItem();
    if(item==Q_NULLPTR)
    {
        QMessageBox::information(this,tr("大区域天气雷达数据QC及组网QPE系统"),tr("请选择雷达"));
        return;
    }
    ui->tableWidget->removeRow(item->row());

    //saveTableContent();
    QString m_strRunPath = QCoreApplication::applicationDirPath();
    saveTableContent(m_strRunPath+"/Temp/"+sTemporaryRadarFileName);
}

/******************************************************应用button*******************************************************/
void MainWindow::on_pushButton_apply_clicked()
{
    //时次间隔<=匹配时间的一半
    int nShiCiJianGe=ui->txtShiCiJianGe->text().toInt();

    int nPiPeiTime=ui->txtPiPeiTime->text().toInt();
    if(nShiCiJianGe>nPiPeiTime/2)
    {
          QMessageBox::critical(this,tr("提示"),tr("匹配时间必须<=时次间隔的一半"));
    }
    else if((ui->doubleSpinBox->value()<=0)||(ui->doubleSpinBox_2->value()<=0))
    {
        QMessageBox::critical(this,tr("水平分辨率输入错误"),tr("水平分辨率输入错误，径向度数或纬向度数必须>0"));
        if(ui->doubleSpinBox->value()<=0)
        {
           ui->doubleSpinBox->setValue(0.01);
        }

        if(ui->doubleSpinBox_2->value()<=0)
        {
           ui->doubleSpinBox_2->setValue(0.01);
        }
    }
    else if(ui->startDateTimeEdit->dateTime()>=ui->endDateTimeEdit->dateTime())
    {
        QMessageBox::critical(this,tr("时间区间输入错误"),tr("开始时间必须小于截止时间"));
    }
    else
    {      
        inputTimeDate(sDateTimeFileName); //保存起止日期和截止日期
        saveConfig(sTimeParameterConfigFileName);
        saveConfig(sDataHandlerConfigFileName);
        saveConfig(sProductGenerateConfigFileName);
        //saveConfig(sHeightAndResolutionConfigFileName);//写入ini文件
        saveHeightConfig(sHeightAndResolutionConfigFileName);
        saveProductGenerateConfig(sProductGenerateConfigFileName);

        //saveConfig(sDataPathConfigFileName);
        // saveConfig(sDataPathConfigFileName);//保存数据路径
         saveDataPath(sDataPathConfigFileName);
        //saveTableContent();
        saveTableContent(sRadarFileName);
        saveRadarRegion();//保存雷达区域
        QMessageBox::information(this,tr("大区域天气雷达数据QC及组网QPE系统"),tr("保存成功"));
    }
}

/*************************************************保存数据路径***********************************************************/
void MainWindow::saveDataPath(QString fileName)
{
     /*数据路径*/
    QString strConfig;
    QString tmpKeyName="";
    QString tmpKeyValue="";
    tmpKeyName="m_ProductPath";//输出数据 产品
    tmpKeyValue=ui->txtOutputProduct->text();
    strConfig = tmpKeyName+"="+tmpKeyValue;

    tmpKeyName="m_TemDataPath";//输入数据 临时数据
    tmpKeyValue=ui->txtOutputTmp->text();
    strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    writeHeightFile(fileName,strConfig,false);
}

/*********************************************************作废***********************************************************/
//运行
void MainWindow::on_action_triggered()
{
    /*
    QTimer *lastTimer=new QTimer(this);
    connect(lastTimer,SIGNAL(timeout()),this,SLOT(lastTimerTick()));//这是一个倒计时
    lastTimer->start(1000);

    //计算mainTimer的运行时间
    //mainTimer=mainTimerMSpan+mainTimerNSpan+mainTimerDelay;

    //延迟mainTimer秒之后开启定时器
    //QTimer::singleShot(mainTimer, this, SLOT(startMyTimer()) );
    //开启定时器
    //startMyTimer();

    startRaderProcess();

    */
}

/*******************************************************调用实时分析模式*********************************************************/
void MainWindow::on_action_2_triggered()
{
    QDir *temp = new QDir;
    QStringList dataPathList;
    QStringList contentList;
    QString strDataPath=readFile(sDataPathConfigFileName);
    //先按照换行分割
    contentList = strDataPath.split("\r\n");
    for(int i=0;i<contentList.size();i++)
    {
       if(contentList.at(i).trimmed()!="")
       {
           QString strPath = contentList.at(i).split("=").at(1).trimmed();
           qDebug()<<strPath<<"PATH";
           dataPathList.append(strPath);
           QFile pathFile(strPath);
           if(pathFile.exists())
           {
               qDebug()<<"存在";
           }
           else
           {
               temp->mkdir(strPath);
           }
       }
    }
    qDebug()<<dataPathList<<"dataPathList";

    systemTrayIcon->setIcon(QIcon("greenTrayIcon.png"));
    ui->action_2->setDisabled(true);
    ui->action_3->setDisabled(true);
    action_ShiShiFenXi->setDisabled(true);
    action_LiShiFenXi->setDisabled(true);
    shiShiChangeTitle();//改变QC_QPE标题,作为判断是否完成的标志

    QString m_strRunPath = QCoreApplication::applicationDirPath();
    //在这里新加一个对Temp文件夹的操作,清除掉所有的空文件
    QString strFileName;//文件名
    QFileInfo file_info;//文件夹下的文件信息
    QList<QString> listFilePathList=readDir(m_strRunPath+"/Temp");//读取路径下的Temp文件夹里的文件
    for(int j=0;j<listFilePathList.count();j++)
    {
        file_info = listFilePathList.at(j);
        strFileName=file_info.fileName();
        if(strFileName!=sTemporaryRadarFileName)//如果文件名不等于TemporaryRadarList.ini
        {
            qDebug()<<"实时模式:"<<strFileName;
            QDir myDir;
            myDir.remove(m_strRunPath+"/Temp/"+strFileName);
        }
    }

    //startProcess(m_strRunPath+"/QC_QPE_ShiShiFenXiCtrl");//QC_QPE_ShiShiFenXiCtrl
    //shiShiProcess = new QProcess(this);
    shiShiProcess->start(m_strRunPath+"/QC_QPE_ShiShiFenXiCtrl");

}

/******************************************************历史模式分析*******************************************************/
void MainWindow::on_action_3_triggered()
{
    QDir *temp = new QDir;
    QStringList dataPathList;
    QStringList contentList;
    QString strDataPath=readFile(sDataPathConfigFileName);
    //先按照换行分割
    contentList = strDataPath.split("\r\n");
    for(int i=0;i<contentList.size();i++)
    {
       if(contentList.at(i)!=NULL)
       {
           QString strPath = contentList.at(i).split("=").at(1).trimmed();
           qDebug()<<strPath<<"PATH";
           dataPathList.append(strPath);
           QFile pathFile(strPath);
           if(pathFile.exists())
           {
               qDebug()<<"存在";
           }
           else
           {
               temp->mkdir(strPath);
           }
       }
    }

    systemTrayIcon->setIcon(QIcon("yellowTrayIcon.png"));
    ui->action_3->setDisabled(true);//让“历史分析”不能二次操作
    ui->action_2->setDisabled(true);
    action_ShiShiFenXi->setDisabled(true);
    action_LiShiFenXi->setDisabled(true);
    changeTitle();//改变QC_QPE标题,作为判断是否完成的标志

    QString m_strRunPath = QCoreApplication::applicationDirPath();
    //在这里新加一个对Temp文件夹的操作,清除掉所有的空文件
    QString strFileName;//文件名
    QFileInfo file_info;//文件夹下的文件信息
    QList<QString> listFilePathList=readDir(m_strRunPath+"/Temp");//读取路径下的Temp文件夹里的文件
    for(int j=0;j<listFilePathList.count();j++)
    {
        file_info = listFilePathList.at(j);
        strFileName=file_info.fileName();
        if(strFileName!=sTemporaryRadarFileName)//如果文件名不等于TemporaryRadarList.ini
        {
            qDebug()<<strFileName;
            QDir myDir;
            myDir.remove(m_strRunPath+"/Temp/"+strFileName);
        }
    }

    //startProcess(m_strRunPath+"/QC_QPE_FenXinCtrl");
    //liShiProcess = new QProcess(this);
    liShiProcess->start(m_strRunPath+"/QC_QPE_FenXinCtrl");
}

/*********************************************************作废***********************************************************/
//停止
void MainWindow::on_action_7_triggered()
{
    QMessageBox::information(this,tr("大区域天气雷达数据QC及组网QPE系统"),tr("停止"));
}

/******************************************************取消按钮*******************************************************/
void MainWindow::on_pushButton_cancel_clicked()
{
    //"Temp"文件夹中的sTemporaryRadarFileName与radar.ini文件保持一致
    QString m_strRunPath = QCoreApplication::applicationDirPath();
    QDir myDir;
    myDir.remove(m_strRunPath+"/Temp/"+sMapFlagFileName);
    myDir.remove(m_strRunPath+"/Temp/"+sTemporaryRadarFileName);
    QString radarContent;
    radarContent = readFile(sRadarFileName);
    writeFile(m_strRunPath+"/Temp/"+sTemporaryRadarFileName,radarContent,false);

    //加载配置信息
    //ShowInfo(loadConfig(sTimeParameterConfigFileName),sTimeParameterConfigFileName);

    //加载表格数据
    loadTableContent(sRadarFileName);
    //这里应该加载所有ini中的内容
    //加载配置信息
    ShowInfo(loadConfig(sTimeParameterConfigFileName),sTimeParameterConfigFileName);
    ShowInfo(loadConfig(sDataHandlerConfigFileName),sDataHandlerConfigFileName);
    ShowInfo(loadConfig(sProductGenerateConfigFileName),sProductGenerateConfigFileName);
    ShowInfo(loadConfig(sHeightAndResolutionConfigFileName),sHeightAndResolutionConfigFileName);
    ShowInfo(loadConfig(sDataPathConfigFileName),sDataPathConfigFileName);
    setTimeDate(sDateTimeFileName);

}

/******************************************************重建button*******************************************************/
void MainWindow::on_pushButton_Select_clicked()
{
//    dialog = new QDialog(this);
//    dialog->setWindowTitle("选择雷达方式");
//    dialog->setGeometry(300,300,100,100);
//    //dialog->setGeometry();

//    QPushButton *mapButton =new QPushButton;
//    mapButton->setText(tr("地图"));
//    mapButton->setBaseSize(10,10);
//    QPushButton *listButton =new QPushButton;
//    listButton->setText(tr("列表"));
//    listButton->setBaseSize(10,10);

//    QHBoxLayout *queryLayout = new QHBoxLayout();
//    queryLayout->addWidget(mapButton);
//    queryLayout->addWidget(listButton);

//    QGridLayout *mainLayout=new QGridLayout(dialog);
//    //mainLayout->addLayout(centorLayout,0,0);
//    mainLayout->addLayout(queryLayout,0,0);

//    dialog->show();

//    connect(mapButton,SIGNAL(clicked()),this,SLOT(chooseRadarFromMap()));
//    connect(listButton,SIGNAL(clicked()),this,SLOT(chooseRadarFromList()));


    QSettings settings("./mapOrList.ini", QSettings::IniFormat);
    QString value = settings.value("FormOption/bShowForm",false).toString();

    if(value=="true")
    {
        chooseRadarFromMap();
        //connect(mapButton,SIGNAL(clicked()),this,SLOT(chooseRadarFromMap()));
    }
    else if(value=="false")
    {
        chooseRadarFromList();
        //connect(listButton,SIGNAL(clicked()),this,SLOT(chooseRadarFromList()));
    }

}

//"重建"选择模式
void MainWindow::on_toolButton_clicked()
{
    QSettings settings("./mapOrList.ini", QSettings::IniFormat);
    QString value = settings.value("FormOption/bShowForm",true).toString();

    qDebug()<<value;
    chooseDialog = new QDialog(this);
    chooseDialog->setWindowTitle("选择雷达方式");
    chooseDialog->setGeometry(300,300,50,50);

    MapBox = new QCheckBox("Map", this);
    ListBox= new QCheckBox("List", this);
    QOK = new QPushButton("OK",this);

    QHBoxLayout *queryLayout = new QHBoxLayout();
    queryLayout->addWidget(MapBox);
    queryLayout->addWidget(ListBox);
    queryLayout->addWidget(QOK);

    QGridLayout *mainLayout=new QGridLayout(chooseDialog);
    mainLayout->addLayout(queryLayout,0,0);

    chooseDialog->show();

    //这里是初始化
    if(value=="true")
    {
        MapBox->setChecked(true);
        ListBox->setChecked(false);
    }
    else if(value=="false")
    {
        ListBox->setChecked(true);
        MapBox->setChecked(false);
    }

    connect(QOK,SIGNAL(clicked()),this,SLOT(QOK_clicked()));

}

//QOK
void MainWindow::QOK_clicked()
{
    if(boolParse(MapBox->isChecked())=="true"&&boolParse(ListBox->isChecked())=="false")//10
    {
        QSettings settings("./mapOrList.ini", QSettings::IniFormat);
        settings.setValue("FormOption/bShowForm","true");
    }
    else if(boolParse(MapBox->isChecked())=="false"&&boolParse(ListBox->isChecked())=="true")//01
    {
        QSettings settings("./mapOrList.ini", QSettings::IniFormat);
        settings.setValue("FormOption/bShowForm","false");
    }
    else if(boolParse(MapBox->isChecked())=="true"&&boolParse(ListBox->isChecked())=="true")//11
    {
        QMessageBox::critical(this,tr("提示"),tr("只能选择一种方式"));
        //QMessageBox::information(this,tr("Information消息框"),tr("123"));
    }
    chooseDialog->close();
}

/******************************************************重建button,通过列表选择雷达*****************************************************/
void MainWindow::chooseRadarFromList()
{
   // dialog->close();
    tableDialog* tableDlg = new tableDialog(this);
    connect(tableDlg,SIGNAL(sendData(QString)),this,SLOT(receiveData(QString)));
    tableDlg->show();
}

/******************************************************重建button,通过地图选择雷达*****************************************************/
void MainWindow::chooseRadarFromMap()
{
   // dialog->close();
    QString m_strRunPath = QCoreApplication::applicationDirPath();
    //startProcess(m_strRunPath+"/MapWidget");
    //chooseProcess = new QProcess(this);
    chooseProcess->start(m_strRunPath+"/MapWidget");

    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(timerWork()));
    timer->start(500);
}

/***************************************重建按钮下,做一个定时器,不断扫描文件Flag.txt是否存在*****************************************/
void MainWindow::timerWork()
{
    QString m_strRunPath = QCoreApplication::applicationDirPath();
    QFile isFile(m_strRunPath+"/Temp/"+sMapFlagFileName);
    if(isFile.exists())
    {
        timer->stop();
        loadTableContent(m_strRunPath+"/Temp/"+sTemporaryRadarFileName); //这里加载的是sRadarFileName.ini里的内容

        //下面是用于删除这个空文件,延时0.5s
        chooseDeleteCount = 1;
        chooseDeleteFileTimer = new QTimer(this);
        connect(chooseDeleteFileTimer,SIGNAL(timeout()),this,SLOT(chooseDeleteTimerWork()));
        chooseDeleteFileTimer->start(500);
    }

//    QDir myDir;
//    myDir.remove(m_strRunPath+"/Temp/"+sMapFlagFileName);
}

/******************************************槽函数,计数器,用于删除重建雷达时的空文件**********************************************/
void MainWindow::chooseDeleteTimerWork()
{
    QString m_strRunPath = QCoreApplication::applicationDirPath();
    if(chooseDeleteCount<=0)
    {
        chooseDeleteFileTimer->stop();
        //删除空文件
        QDir myDir;
        myDir.remove(m_strRunPath+"/Temp/"+sMapFlagFileName);
    }
    else
    {
        chooseDeleteCount--;
    }
}

//槽函数
void MainWindow::receiveData(QString data)
{
    //将要修改的文件先保存在临时文件中
    strData = data;
    QString m_strRunPath = QCoreApplication::applicationDirPath();
    writeFile(m_strRunPath+"/Temp/"+sTemporaryRadarFileName,data,false);
    //writeFile(sRadarFileName,data,false);//这里不应该直接修改Radar.ini文件,应该在点击应用之后,修改

    //加载配置信息
    ShowInfo(loadConfig(sTimeParameterConfigFileName),sTimeParameterConfigFileName);
    ShowInfo(loadConfig(sDataHandlerConfigFileName),sDataHandlerConfigFileName);
    ShowInfo(loadConfig(sProductGenerateConfigFileName),sProductGenerateConfigFileName);
    ShowInfo(loadConfig(sHeightAndResolutionConfigFileName),sHeightAndResolutionConfigFileName);
    ShowInfo(loadConfig(sDataPathConfigFileName),sDataPathConfigFileName);
    //加载表格数据
    loadTableContent(m_strRunPath+"/Temp/"+sTemporaryRadarFileName); //这里加载的是sRadarFileName.ini里的内容
}

//读取加载文件内容，并将文件中的内容放入resultList
QHash<QString,QString> MainWindow::loadConfig(QString fileName)
{
    //处理，得到一个键值对
    QHash<QString,QString> resultList;
    QString tmpKeyName="";
    QString tmpKeyValue="";

    QString strConfig=readFile(fileName);
    QStringList strList = strConfig.split("\r\n");

    for(int i=0;i<strList.length();i++)
    {
        QString tmpStr=strList[i];
        //不是#号开头，并且包含一个=号的，算作是有效字符串
        if(!tmpStr.startsWith("#")&&tmpStr.contains("="))
        {
            //按等号分割后,从分割后的字符串两端删除空白字符
            QStringList contentStrList = tmpStr.split("=");
            tmpKeyName=getStardString(contentStrList[0]);//“=”前的内容
            if(contentStrList.length()>1)
            {
                tmpKeyValue=getStardString(contentStrList[1]);
            }
            resultList.insert(tmpKeyName,tmpKeyValue);
            tmpKeyName="";
            tmpKeyValue="";
        }
    }
    return resultList;
}

/****************************************************保存处理***********************************************************/
//保存处理
void MainWindow::saveConfig(QString fileName)
{
    QString strConfig=readFile(fileName);   //先读取文件

    QStringList strConfigList=strConfig.split("\r\n");

    for(int i=0;i<strConfigList.length();i++)
    {
        QString tmpStr=strConfigList[i];            //  读取文件中的一行
        //不是#号开头，并且包含一个=号的，算作是有效字符串
        if(!tmpStr.startsWith("#")&&tmpStr.contains("="))       //如果这一行字符串中不是以“#”开头，并且字符串包含“=”；注意优先级
        {
            strConfig=strConfig.replace(tmpStr,getReplaceString(tmpStr));       //没看懂，为什么要替换
        }
    }
    QString tmpKeyName="";
    QString tmpKeyValue="";
    if(fileName==sTimeParameterConfigFileName)
    {
        /*时间参数开始 */
        tmpKeyName="m_iMatchingMin";//时间参数 匹配时间 单位 分钟
        tmpKeyValue = ui->txtPiPeiTime->text();
        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
        {
            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
        }
        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

        tmpKeyName="m_iIntervalMin";//时间参数 时次间隔 单位 分钟
        tmpKeyValue = ui->txtShiCiJianGe->text();
        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
        {
            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
        }
        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

        tmpKeyName="m_iDelayMin";//时间参数 时间延迟 单位 分钟
        tmpKeyValue = ui->txtShiJianYanShi->text();
        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
        {
            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
        }
        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);
        /*时间参数结束*/
    }
    else if(fileName==sDataHandlerConfigFileName)
    {
        /*数据处理*/
        tmpKeyName="m_ctlSaveSigGridData";//数据处理  保存单站格点数据
        tmpKeyValue = boolParse(ui->chbDanJiZhan_2->isChecked());
        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
        {
            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
        }
        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

        tmpKeyName="m_ctlUseQC";//数据处理  基数据质量控制
        tmpKeyValue = boolParse(ui->chbZuHeFanShe_2->isChecked());
        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
        {
            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
        }
        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

        tmpKeyName="m_bSaveQcData";//数据处理  保存质量控制基数据
        tmpKeyValue = boolParse(ui->chbHuiBoDingGao_2->isChecked());
        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
        {
            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
        }
        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

        tmpKeyName="m_bSaveNetCDFData";//数据处理  输出NetCDF格式数据
        tmpKeyValue = boolParse(ui->chbZhuiZhiLeiJi_2->isChecked());
        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
        {
            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
        }
        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

        tmpKeyName="m_bDeleteOldBaseData";//数据处理  删除已处理的基数据
        tmpKeyValue = boolParse(ui->chbHuiBoWeiZhi_2->isChecked());
        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
        {
            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
        }
        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

        tmpKeyName="m_bSaveGridVelocity";//数据处理  速度格点化
        tmpKeyValue = boolParse(ui->chbSuDuGeDianHua->isChecked());
        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
        {
            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
        }
        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

        tmpKeyName="m_ProdRemainDays";//数据处理  产品保留天数
        tmpKeyValue = QString::number(ui->spinBox->value(),10);
        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
        {
            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
        }
        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);
        /*数据处理结束*/
    }
//    else if(fileName==sProductGenerateConfigFileName)
//    {
//        /*产品生成开始 */

//        tmpKeyName="m_bChkCrProd";//产品生成 组合反射率
//        tmpKeyValue = boolParse(ui->chbZuHeFanShe->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bChkEtProd";//产品生成 回波顶高
//        tmpKeyValue = boolParse(ui->chbHuiBoDingGao->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bChkVilProd";//产品生成 垂直液态水含量
//        tmpKeyValue = boolParse(ui->chbZhuiZhiLeiJi->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bChkForeProd";//产品生成 回波位置预报
//        tmpKeyValue = boolParse(ui->chbHuiBoWeiZhi->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bChkEchoMovProd";//产品生成 回波移动预报
//        tmpKeyValue = boolParse(ui->chbHuiBoYiDong->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bChkCtProd";//产品生成 云类型识别
//        tmpKeyValue = boolParse(ui->chbYunLeiXingShiBie->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bChkQpeProd";//产品生成 降水产品
//        tmpKeyValue = boolParse(ui->chbJiangShuiChanPin->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);
//        /*产品生成结束*/

//    }
//    else if(fileName==sHeightAndResolutionConfigFileName)
//    {
//        /*高度与分辨率*/
//        tmpKeyName="m_fLonRes";//高度与分辨率  水平分辨率  经度 度
//        tmpKeyValue = QString::number(ui->doubleSpinBox->value());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_fLatRes";//高度与分辨率 水平分辨率  纬度 度
//        tmpKeyValue = QString::number(ui->doubleSpinBox_2->value());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bSelLevel1";//高度与分辨率 高度层 海拔 米
//        tmpKeyValue = boolParse(ui->haiba_300->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bSelLevel2";//高度与分辨率 高度层 海拔 米
//        tmpKeyValue = boolParse(ui->haiba_500->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bSelLevel3";//高度与分辨率 高度层 海拔 米
//        tmpKeyValue = boolParse(ui->haiba_800->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bSelLevel4";//高度与分辨率 高度层 海拔 米
//        tmpKeyValue = boolParse(ui->haiba_1000->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bSelLevel5";//高度与分辨率 高度层 海拔 米
//        tmpKeyValue = boolParse(ui->haiba_1200->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bSelLevel6";//高度与分辨率 高度层 海拔 米
//        tmpKeyValue = boolParse(ui->haiba_1500->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bSelLevel7";//高度与分辨率 高度层 海拔 米
//        tmpKeyValue = boolParse(ui->haiba_2000->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bSelLevel8";//高度与分辨率 高度层 海拔 米
//        tmpKeyValue = boolParse(ui->haiba_2500->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bSelLevel9";//高度与分辨率 高度层 海拔 米
//        tmpKeyValue = boolParse(ui->haiba_3000->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bSelLevel10";//高度与分辨率 高度层 海拔 米
//        tmpKeyValue = boolParse(ui->haiba_3500->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bSelLevel11";//高度与分辨率 高度层 海拔 米
//        tmpKeyValue = boolParse(ui->haiba_4000->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bSelLevel12";//高度与分辨率 高度层 海拔 米
//        tmpKeyValue = boolParse(ui->haiba_4500->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bSelLevel13";//高度与分辨率 高度层 海拔 米
//        tmpKeyValue = boolParse(ui->haiba_5000->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bSelLevel14";//高度与分辨率 高度层 海拔 米
//        tmpKeyValue = boolParse(ui->haiba_5500->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bSelLevel15";//高度与分辨率 高度层 海拔 米
//        tmpKeyValue = boolParse(ui->haiba_6000->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bSelLevel16";//高度与分辨率 高度层 海拔 米
//        tmpKeyValue = boolParse(ui->haiba_6500->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bSelLevel17";//高度与分辨率 高度层 海拔 米
//        tmpKeyValue = boolParse(ui->haiba_7000->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bSelLevel18";//高度与分辨率 高度层 海拔 米
//        tmpKeyValue = boolParse(ui->haiba_7500->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bSelLevel19";//高度与分辨率 高度层 海拔 米
//        tmpKeyValue = boolParse(ui->haiba_8000->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bSelLevel20";//高度与分辨率 高度层 海拔 米
//        tmpKeyValue = boolParse(ui->haiba_8500->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bSelLevel21";//高度与分辨率 高度层 海拔 米
//        tmpKeyValue = boolParse(ui->haiba_9000->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bSelLevel22";//高度与分辨率 高度层 海拔 米
//        tmpKeyValue = boolParse(ui->haiba_9500->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bSelLevel23";//高度与分辨率 高度层 海拔 米
//        tmpKeyValue = boolParse(ui->haiba_10000->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bSelLevel24";//高度与分辨率 高度层 海拔 米
//        tmpKeyValue = boolParse(ui->haiba_10500->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bSelLevel25";//高度与分辨率 高度层 海拔 米
//        tmpKeyValue = boolParse(ui->haiba_11000->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bSelLevel26";//高度与分辨率 高度层 海拔 米
//        tmpKeyValue = boolParse(ui->haiba_11500->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bSelLevel27";//高度与分辨率 高度层 海拔 米
//        tmpKeyValue = boolParse(ui->haiba_12000->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bSelLevel28";//高度与分辨率 高度层 海拔 米
//        tmpKeyValue = boolParse(ui->haiba_12500->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bSelLevel29";//高度与分辨率 高度层 海拔 米
//        tmpKeyValue = boolParse(ui->haiba_13000->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bSelLevel30";//高度与分辨率 高度层 海拔 米
//        tmpKeyValue = boolParse(ui->haiba_13500->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bSelLevel31";//高度与分辨率 高度层 海拔 米
//        tmpKeyValue = boolParse(ui->haiba_14000->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bSelLevel32";//高度与分辨率 高度层 海拔 米
//        tmpKeyValue = boolParse(ui->haiba_14500->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bSelLevel33";//高度与分辨率 高度层 海拔 米
//        tmpKeyValue = boolParse(ui->haiba_15000->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bSelLevel34";//高度与分辨率 高度层 海拔 米
//        tmpKeyValue = boolParse(ui->haiba_16000->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bSelLevel35";//高度与分辨率 高度层 海拔 米
//        tmpKeyValue = boolParse(ui->haiba_16500->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bSelLevel36";//高度与分辨率 高度层 海拔 米
//        tmpKeyValue = boolParse(ui->haiba_17000->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bSelLevel37";//高度与分辨率 高度层 海拔 米
//        tmpKeyValue = boolParse(ui->haiba_18000->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bSelLevel38";//高度与分辨率 高度层 海拔 米
//        tmpKeyValue = boolParse(ui->haiba_19000->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bSelLevel39";//高度与分辨率 高度层 海拔 米
//        tmpKeyValue = boolParse(ui->haiba_20000->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

//        tmpKeyName="m_bSelLevel40";//高度与分辨率 高度层 海拔 米
//        tmpKeyValue = boolParse(ui->haiba_21000->isChecked());
//        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
//        {
//            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
//        }
//        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);
//        /*高度与分辨率*/
//    }
    else if(fileName==sDataPathConfigFileName)
    {
        /*数据路径*/
        tmpKeyName="m_ProductPath";//输出数据 产品
        tmpKeyValue=ui->txtOutputProduct->text();
        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
        {
            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
        }
        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);

        tmpKeyName="m_TemDataPath";//输入数据 临时数据
        tmpKeyValue=ui->txtOutputTmp->text();
        if(!strConfig.contains(sReplaceOperate+tmpKeyName))
        {
            strConfig+="\r\n"+tmpKeyName+"="+sReplaceOperate+tmpKeyName;
        }
        strConfig=strConfig.replace(sReplaceOperate+tmpKeyName,tmpKeyValue);
        /*数据路径结束*/
    }
    writeFile(fileName,strConfig,false);   //写入相对应的文件
}

/************************************************************产品生成***********************************************/
void MainWindow::saveProductGenerateConfig(QString fileName)
{
    /*产品生成开始 */
    QString strConfig;
    QString tmpKeyName="";
    QString tmpKeyValue="";

    tmpKeyName="m_bChkCrProd";//产品生成 组合反射率
    tmpKeyValue=boolParse(ui->chbZuHeFanShe->isChecked());
    strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;


    tmpKeyName="m_bChkEtProd";//产品生成 回波顶高
    tmpKeyValue = boolParse(ui->chbHuiBoDingGao->isChecked());
    strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;

    tmpKeyName="m_bChkVilProd";//产品生成 垂直液态水含量
    tmpKeyValue = boolParse(ui->chbZhuiZhiLeiJi->isChecked());
    strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;

    tmpKeyName="m_bChkForeProd";//产品生成 回波位置预报
    tmpKeyValue = boolParse(ui->chbHuiBoWeiZhi->isChecked());
    strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;

    tmpKeyName="m_bChkEchoMovProd";//产品生成 回波移动预报
    tmpKeyValue = boolParse(ui->chbHuiBoYiDong->isChecked());
    strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;

    tmpKeyName="m_bChkCtProd";//产品生成 云类型识别
    tmpKeyValue = boolParse(ui->chbYunLeiXingShiBie->isChecked());
    strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;

    tmpKeyName="m_bChkQpeProd";//产品生成 降水产品
    tmpKeyValue = boolParse(ui->chbJiangShuiChanPin->isChecked());
    strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    /*产品生成结束*/

    writeHeightFile(fileName,strConfig,false);
}

/***************************************高度与分辨率*******************************************************/
void MainWindow::saveHeightConfig(QString fileName)
{
    //QString strConfig=readFile(fileName);;
    QString strConfig;
    QString tmpKeyName="";
    QString tmpKeyValue="";
    /*高度与分辨率*/
    tmpKeyName="m_fLonRes";//高度与分辨率  水平分辨率  经度 度
    tmpKeyValue = QString::number(ui->doubleSpinBox->value());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
        strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }

    tmpKeyName="m_fLatRes";//高度与分辨率 水平分辨率  纬度 度
    tmpKeyValue = QString::number(ui->doubleSpinBox_2->value());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
        strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }

    tmpKeyName="m_bSelLevel1";//高度与分辨率 高度层 海拔 米
    tmpKeyValue = boolParse(ui->haiba_300->isChecked());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
        strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }

    tmpKeyName="m_bSelLevel2";//高度与分辨率 高度层 海拔 米
    tmpKeyValue = boolParse(ui->haiba_500->isChecked());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
        strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }

    tmpKeyName="m_bSelLevel3";//高度与分辨率 高度层 海拔 米
    tmpKeyValue = boolParse(ui->haiba_800->isChecked());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
        strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }

    tmpKeyName="m_bSelLevel4";//高度与分辨率 高度层 海拔 米
    tmpKeyValue = boolParse(ui->haiba_1000->isChecked());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
        strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }

    tmpKeyName="m_bSelLevel5";//高度与分辨率 高度层 海拔 米
    tmpKeyValue = boolParse(ui->haiba_1200->isChecked());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
        strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }

    tmpKeyName="m_bSelLevel6";//高度与分辨率 高度层 海拔 米
    tmpKeyValue = boolParse(ui->haiba_1500->isChecked());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
        strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }

    tmpKeyName="m_bSelLevel7";//高度与分辨率 高度层 海拔 米
    tmpKeyValue = boolParse(ui->haiba_2000->isChecked());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
        strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }


    tmpKeyName="m_bSelLevel8";//高度与分辨率 高度层 海拔 米
    tmpKeyValue = boolParse(ui->haiba_2500->isChecked());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
        strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }


    tmpKeyName="m_bSelLevel9";//高度与分辨率 高度层 海拔 米
    tmpKeyValue = boolParse(ui->haiba_3000->isChecked());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
        strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }


    tmpKeyName="m_bSelLevel10";//高度与分辨率 高度层 海拔 米
    tmpKeyValue = boolParse(ui->haiba_3500->isChecked());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
       strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }


    tmpKeyName="m_bSelLevel11";//高度与分辨率 高度层 海拔 米
    tmpKeyValue = boolParse(ui->haiba_4000->isChecked());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
       strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }


    tmpKeyName="m_bSelLevel12";//高度与分辨率 高度层 海拔 米
    tmpKeyValue = boolParse(ui->haiba_4500->isChecked());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
        strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }


    tmpKeyName="m_bSelLevel13";//高度与分辨率 高度层 海拔 米
    tmpKeyValue = boolParse(ui->haiba_5000->isChecked());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
        strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }


    tmpKeyName="m_bSelLevel14";//高度与分辨率 高度层 海拔 米
    tmpKeyValue = boolParse(ui->haiba_5500->isChecked());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
        strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }


    tmpKeyName="m_bSelLevel15";//高度与分辨率 高度层 海拔 米
    tmpKeyValue = boolParse(ui->haiba_6000->isChecked());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
        strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }


    tmpKeyName="m_bSelLevel16";//高度与分辨率 高度层 海拔 米
    tmpKeyValue = boolParse(ui->haiba_6500->isChecked());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
        strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }


    tmpKeyName="m_bSelLevel17";//高度与分辨率 高度层 海拔 米
    tmpKeyValue = boolParse(ui->haiba_7000->isChecked());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
        strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }


    tmpKeyName="m_bSelLevel18";//高度与分辨率 高度层 海拔 米
    tmpKeyValue = boolParse(ui->haiba_7500->isChecked());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
        strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }


    tmpKeyName="m_bSelLevel19";//高度与分辨率 高度层 海拔 米
    tmpKeyValue = boolParse(ui->haiba_8000->isChecked());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
        strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }


    tmpKeyName="m_bSelLevel20";//高度与分辨率 高度层 海拔 米
    tmpKeyValue = boolParse(ui->haiba_8500->isChecked());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
        strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }


    tmpKeyName="m_bSelLevel21";//高度与分辨率 高度层 海拔 米
    tmpKeyValue = boolParse(ui->haiba_9000->isChecked());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
        strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }


    tmpKeyName="m_bSelLevel22";//高度与分辨率 高度层 海拔 米
    tmpKeyValue = boolParse(ui->haiba_9500->isChecked());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
        strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }


    tmpKeyName="m_bSelLevel23";//高度与分辨率 高度层 海拔 米
    tmpKeyValue = boolParse(ui->haiba_10000->isChecked());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
        strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }


    tmpKeyName="m_bSelLevel24";//高度与分辨率 高度层 海拔 米
    tmpKeyValue = boolParse(ui->haiba_10500->isChecked());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
        strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }


    tmpKeyName="m_bSelLevel25";//高度与分辨率 高度层 海拔 米
    tmpKeyValue = boolParse(ui->haiba_11000->isChecked());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
        strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }


    tmpKeyName="m_bSelLevel26";//高度与分辨率 高度层 海拔 米
    tmpKeyValue = boolParse(ui->haiba_11500->isChecked());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
        strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }


    tmpKeyName="m_bSelLevel27";//高度与分辨率 高度层 海拔 米
    tmpKeyValue = boolParse(ui->haiba_12000->isChecked());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
        strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }


    tmpKeyName="m_bSelLevel28";//高度与分辨率 高度层 海拔 米
    tmpKeyValue = boolParse(ui->haiba_12500->isChecked());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
        strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }


    tmpKeyName="m_bSelLevel29";//高度与分辨率 高度层 海拔 米
    tmpKeyValue = boolParse(ui->haiba_13000->isChecked());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
        strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }


    tmpKeyName="m_bSelLevel30";//高度与分辨率 高度层 海拔 米
    tmpKeyValue = boolParse(ui->haiba_13500->isChecked());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
        strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }


    tmpKeyName="m_bSelLevel31";//高度与分辨率 高度层 海拔 米
    tmpKeyValue = boolParse(ui->haiba_14000->isChecked());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
        strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }


    tmpKeyName="m_bSelLevel32";//高度与分辨率 高度层 海拔 米
    tmpKeyValue = boolParse(ui->haiba_14500->isChecked());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
        strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }


    tmpKeyName="m_bSelLevel33";//高度与分辨率 高度层 海拔 米
    tmpKeyValue = boolParse(ui->haiba_15000->isChecked());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
        strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }


    tmpKeyName="m_bSelLevel34";//高度与分辨率 高度层 海拔 米
    tmpKeyValue = boolParse(ui->haiba_16000->isChecked());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
        strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }


    tmpKeyName="m_bSelLevel35";//高度与分辨率 高度层 海拔 米
    tmpKeyValue = boolParse(ui->haiba_16500->isChecked());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
        strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }


    tmpKeyName="m_bSelLevel36";//高度与分辨率 高度层 海拔 米
    tmpKeyValue = boolParse(ui->haiba_17000->isChecked());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
        strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }


    tmpKeyName="m_bSelLevel37";//高度与分辨率 高度层 海拔 米
    tmpKeyValue = boolParse(ui->haiba_18000->isChecked());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
        strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }


    tmpKeyName="m_bSelLevel38";//高度与分辨率 高度层 海拔 米
    tmpKeyValue = boolParse(ui->haiba_19000->isChecked());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
        strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }


    tmpKeyName="m_bSelLevel39";//高度与分辨率 高度层 海拔 米
    tmpKeyValue = boolParse(ui->haiba_20000->isChecked());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
        strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }


    tmpKeyName="m_bSelLevel40";//高度与分辨率 高度层 海拔 米
    tmpKeyValue = boolParse(ui->haiba_21000->isChecked());
    if(!strConfig.contains(sReplaceOperate+tmpKeyName))
    {
        strConfig+="\r\n"+tmpKeyName+"="+tmpKeyValue;
    }

    /*高度与分辨率*/
    writeHeightFile(fileName,strConfig,false);
}


void MainWindow::loadTableContent(QString strRadarFileName)
{
    QStringList header;
    header<<"站号"<<"站名"<<"经度（度）"<<"纬度（度）"<<"海拔高度（米）"<<"雷达型号"<<"范围（千米）"<<"数据路径";
    ui->tableWidget->setColumnCount(8);
    ui->tableWidget->setShowGrid(false);
    ui->tableWidget->setColumnWidth(0,50);
    ui->tableWidget->setHorizontalHeaderLabels(header);
    ui->tableWidget->setRowCount(0);

    QString tableContent=readFile(strRadarFileName);

    //QString tableContent=readFile(sRadarFileName);
    //先按照换行分割
    QStringList tableContentList = tableContent.split("\r\n");
    for(int i=0;i<tableContentList.size();i++)
    {
        //QMessageBox::information(this,"",words.at(i));
        QString tableContentItem=tableContentList[i];
        if(tableContentItem!=NULL&&!tableContentItem.startsWith("#"))
        {
            //QMessageBox::information(this,"",tmpStr);
            //按=号分割
            QStringList tableRowContentListByEqual = tableContentItem.split("=");
            int nRowNum=ui->tableWidget->rowCount();
            ui->tableWidget->insertRow(nRowNum);
            if(tableRowContentListByEqual.length()>0)
            {
                ui->tableWidget->setItem(nRowNum,0,new QTableWidgetItem(tableRowContentListByEqual[0].trimmed()));
            }
            if(tableRowContentListByEqual.length()>1)
            {
                //按照空格分割
                QStringList tableRowContentListBySpace=ReplaceSpaceAndTab(tableRowContentListByEqual[1]).split(" ");
                for(int j=0;j<tableRowContentListBySpace.count();j++)
                {
                    ui->tableWidget->setItem(nRowNum,j+1,new QTableWidgetItem(tableRowContentListBySpace[j].trimmed()));
                }
            }
        }
    }
    ui->tableWidget->show();
}

/*****************************************************保存表格里的雷达信息*******************************************************/
void MainWindow::saveTableContent(QString fileName)
{
    //加载table.ini文件->不存在，新建一个，存在话就覆盖
    //遍历table的信息，组织数据，写到文件中

    QString tableStr="";

    //QString strTableContent=readFile(sRadarFileName);
    QString strTableContent=readFile(fileName);

    QStringList strTableContentList=strTableContent.split("\r\n");

    //处理第一个行，如果第一行是#号开头的，就保留第一个行
    if(strTableContentList.count()>0)
    {
        QString firstRow=strTableContentList[0];
        if(firstRow.startsWith("#"))
        {
            tableStr+=firstRow+"\r\n";
        }
    }

    //遍历表格
    for(int i=0;i<ui->tableWidget->rowCount();i++)
    {
        for(int j=0;j<ui->tableWidget->columnCount();j++)
        {
            if (ui->tableWidget->item(i,j) != NULL )
            {
                //第一列特殊处理
                if(j==0)
                {
                    tableStr.append(ui->tableWidget->item(i,j)->text()+"=");
                }
                else
                {
                    tableStr.append("    "+ui->tableWidget->item(i,j)->text());
                }
            }
        }
        tableStr.append("\r\n");
    }
    //writeFile(sRadarFileName,tableStr,false);
    writeFile(fileName,tableStr,false);
}

/********************************************************显示文件内容*******************************************************/
//显示文件内容
void MainWindow::ShowInfo(QHash<QString,QString> list,QString fileName)
{
    QString tmpKeyName="";
    if(fileName==sTimeParameterConfigFileName)
    {
        /*时间参数开始 */
        tmpKeyName="m_iMatchingMin";//时间参数 匹配时间 单位 分钟
        ui->txtPiPeiTime->setText(list[tmpKeyName]);

        tmpKeyName="m_iIntervalMin";//时间参数 时次间隔 单位 分钟
        ui->txtShiCiJianGe->setText(list[tmpKeyName]);



        tmpKeyName="m_iDelayMin";//时间参数 时间延迟 单位 分钟
        ui->txtShiJianYanShi->setText(list[tmpKeyName]);
        /*时间参数结束*/
    }
    else if(fileName==sDataHandlerConfigFileName)
    {
        /*数据处理*/
        tmpKeyName="m_ctlSaveSigGridData";//数据处理  保存单站格点数据
        ui->chbDanJiZhan_2->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_ctlUseQC";//数据处理  基数据质量控制
        ui->chbZuHeFanShe_2->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSaveQcData";//数据处理  保存质量控制基数据
        ui->chbHuiBoDingGao_2->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSaveNetCDFData";//数据处理  输出NetCDF格式数据
        ui->chbZhuiZhiLeiJi_2->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bDeleteOldBaseData";//数据处理  删除已处理的基数据
        ui->chbHuiBoWeiZhi_2->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSaveGridVelocity";//数据处理  速度格点化
        ui->chbSuDuGeDianHua->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_ProdRemainDays";//数据处理  产品保留天数
        ui->spinBox->setValue(list[tmpKeyName].toInt());
        /*数据处理结束*/
    }
    else if(fileName==sProductGenerateConfigFileName)
    {
        /*产品生成开始 */
        tmpKeyName="m_bChkCrProd";//产品生成 组合反射率
        ui->chbZuHeFanShe->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bChkEtProd";//产品生成 回波顶高
        ui->chbHuiBoDingGao->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bChkVilProd";//产品生成 垂直液态水含量
        ui->chbZhuiZhiLeiJi->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bChkForeProd";//产品生成 回波位置预报
        ui->chbHuiBoWeiZhi->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bChkEchoMovProd";//产品生成 回波移动预报
        ui->chbHuiBoYiDong->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bChkCtProd";//产品生成 云类型识别
        ui->chbYunLeiXingShiBie->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bChkQpeProd";//产品生成 降水产品
        ui->chbJiangShuiChanPin->setChecked(parseBool(list[tmpKeyName]));

        /*产品生成结束*/
    }
    else if(fileName==sHeightAndResolutionConfigFileName)
    {

        /*高度与分辨率*/
        tmpKeyName="m_fLonRes";//高度与分辨率  水平分辨率  经度 度
        ui->doubleSpinBox->setValue(list[tmpKeyName].toDouble());

        tmpKeyName="m_fLatRes";//高度与分辨率 水平分辨率  纬度 度
        ui->doubleSpinBox_2->setValue(list[tmpKeyName].toDouble());

        tmpKeyName="m_bSelLevel1";//高度与分辨率 高度层 海拔 300米
        ui->haiba_300->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSelLevel2";//高度与分辨率 高度层 海拔 500米
        ui->haiba_500->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSelLevel3";//高度与分辨率 高度层 海拔 800米
        ui->haiba_800->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSelLevel4";//高度与分辨率 高度层 海拔 1000米
        ui->haiba_1000->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSelLevel5";//高度与分辨率 高度层 海拔 1200米
        ui->haiba_1200->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSelLevel6";//高度与分辨率 高度层 海拔 1500米
        ui->haiba_1500->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSelLevel7";//高度与分辨率 高度层 海拔 2000米
        ui->haiba_2000->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSelLevel8";//高度与分辨率 高度层 海拔 2500米
        ui->haiba_2500->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSelLevel9";//高度与分辨率 高度层 海拔 3000米
        ui->haiba_3000->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSelLevel10";//高度与分辨率 高度层 海拔 3500米
        ui->haiba_3500->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSelLevel11";//高度与分辨率 高度层 海拔 4000米
        ui->haiba_4000->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSelLevel12";//高度与分辨率 高度层 海拔 4500米
        ui->haiba_4500->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSelLevel13";//高度与分辨率 高度层 海拔 5000米
        ui->haiba_5000->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSelLevel14";//高度与分辨率 高度层 海拔 5500米
        ui->haiba_5500->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSelLevel15";//高度与分辨率 高度层 海拔 6000米
        ui->haiba_6000->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSelLevel16";//高度与分辨率 高度层 海拔 6500米
        ui->haiba_6500->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSelLevel17";//高度与分辨率 高度层 海拔 7000米
        ui->haiba_7000->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSelLevel18";//高度与分辨率 高度层 海拔 7500米
        ui->haiba_7500->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSelLevel19";//高度与分辨率 高度层 海拔 8000米
        ui->haiba_8000->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSelLevel20";//高度与分辨率 高度层 海拔 8500米
        ui->haiba_8500->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSelLevel21";//高度与分辨率 高度层 海拔 9000米
        ui->haiba_9000->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSelLevel22";//高度与分辨率 高度层 海拔 9500米
        ui->haiba_9500->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSelLevel23";//高度与分辨率 高度层 海拔 10000米
        ui->haiba_10000->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSelLevel24";//高度与分辨率 高度层 海拔 10500米
        ui->haiba_10500->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSelLevel25";//高度与分辨率 高度层 海拔 11000米
        ui->haiba_11000->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSelLevel26";//高度与分辨率 高度层 海拔 11500米
        ui->haiba_11500->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSelLevel27";//高度与分辨率 高度层 海拔 12000米
        ui->haiba_12000->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSelLevel28";//高度与分辨率 高度层 海拔 12500米
        ui->haiba_12500->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSelLevel29";//高度与分辨率 高度层 海拔 13000米
        ui->haiba_13000->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSelLevel30";//高度与分辨率 高度层 海拔 13500米
        ui->haiba_13500->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSelLevel31";//高度与分辨率 高度层 海拔 14000米
        ui->haiba_14000->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSelLevel32";//高度与分辨率 高度层 海拔 14500米
        ui->haiba_14500->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSelLevel33";//高度与分辨率 高度层 海拔 15000米
        ui->haiba_15000->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSelLevel34";//高度与分辨率 高度层 海拔 16000米
        ui->haiba_16000->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSelLevel35";//高度与分辨率 高度层 海拔 16500米
        ui->haiba_16500->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSelLevel36";//高度与分辨率 高度层 海拔 17000米
        ui->haiba_17000->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSelLevel37";//高度与分辨率 高度层 海拔 18000米
        ui->haiba_18000->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSelLevel38";//高度与分辨率 高度层 海拔 19000米
        ui->haiba_19000->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSelLevel39";//高度与分辨率 高度层 海拔 20000米
        ui->haiba_20000->setChecked(parseBool(list[tmpKeyName]));

        tmpKeyName="m_bSelLevel40";//高度与分辨率 高度层 海拔 21000米
        ui->haiba_21000->setChecked(parseBool(list[tmpKeyName]));
        /*高度与分辨率*/
    }
    else if(fileName==sDataPathConfigFileName)
    {
        /*数据路径*/
        tmpKeyName="m_ProductPath";//输出数据 产品
        ui->txtOutputProduct->setText(list[tmpKeyName]);

        tmpKeyName="m_TemDataPath";//输出数据 临时数据
        ui->txtOutputTmp->setText(list[tmpKeyName]);
        /*数据路径结束*/
    }
}

/************************************************没用********************************************************/
void MainWindow::startRaderProcess()
{
    QString runPath = QCoreApplication::applicationDirPath();

    for(int i=0;i<ui->tableWidget->rowCount();i++)
    {
        QString raderName=ui->tableWidget->item(i,0)->text();
        //raderName=sRaderFilePath+raderName;



        startProcess(runPath+"/QTRader"+raderName+" "+runPath);
    }
    startProcess(runPath+"/MosaicControl");

}

//获取满足条件的文件
QList<QString> MainWindow::GetFitConditionFile(QList<QString> srcList)
{
    int nYear=currentMyDateTime.date().year();
    int nMonth=currentMyDateTime.date().month();
    int nDay=currentMyDateTime.date().day();

    QString currentStartDateStr=QString("%1-%2-%3 00:00:00")
            .arg(nYear).arg(nMonth).arg(nDay);

    QString currentEndDateStr=QString("%1-%2-%3 23:59:59")
            .arg(nYear).arg(nMonth).arg(nDay);


    QDateTime dtMin = QDateTime::fromString(currentStartDateStr, "yyyy-MM-dd hh:mm:ss");

    QDateTime dtMax = QDateTime::fromString(currentEndDateStr, "yyyy-MM-dd hh:mm:ss");


    int spanFirstTime=g_mainTimerMSpan-g_mainTimerNSpan;
    int spanSecondTime=g_mainTimerMSpan+g_mainTimerNSpan;



    QList<QDateTime> listStartTime;
    QList<QDateTime> listEndTime;

    QDateTime firstTime=dtMin.addMSecs(spanFirstTime);
    QDateTime lastTime=dtMin.addMSecs(spanSecondTime);

    while(firstTime<dtMax)
    {
        listStartTime.append(firstTime);
        listEndTime.append(lastTime);
        firstTime=firstTime.addMSecs(g_mainTimerMSpan);
        lastTime=lastTime.addMSecs(g_mainTimerMSpan);
    }


    QDateTime myDateStart;
    QDateTime myDateEnd;

    for(int i=0;i<listStartTime.count();i++)
    {
        if(currentMyDateTime>=listStartTime.at(i)&&currentMyDateTime<=listEndTime.at(i))
        {
            myDateStart=listStartTime.at(i);
            myDateEnd=listEndTime.at(i);
            break;
        }
    }

    QString myDateStartStr=myDateStart.toString("yyyyMMddhhmmss");
    QString myDateEndStr=myDateEnd.toString("yyyyMMddhhmmss");


    QList<QString> tmpFileNameList;
    for(int i=0;i<srcList.count();i++)
    {
        QString srcName=srcList[i];

        fileInfo aFileInfo=getFileInfo(srcName);

        QString aStr=aFileInfo.fileName.replace("CinradMosaic","").replace(".txt","");

        QDateTime qtTmpDate=QDateTime::fromString(aStr, "yyyyMMddhhmmss");

        if(qtTmpDate>=myDateStart&&qtTmpDate<=myDateEnd)
        {
            tmpFileNameList.append(aStr);
        }
    }

    QList<QString> desList;

    for(int i=0;i<tmpFileNameList.count();i++)
    {
        for(int j=0;j<srcList.count();j++)
        {
            if(srcList[j].contains(tmpFileNameList[i]))
            {
                desList.append(srcList[j]);
                break;
            }
        }
    }
    return desList;
}

//开启定时器
void MainWindow::startMyTimer()
{
    //QMessageBox::information(this,tr("Information消息框"),tr("123"));
    QTimer *timer = new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(timertick()));

    g_mainTimerMSpan= ui->txtPiPeiTime->text().toInt()*1000*60;
    g_mainTimerNSpan= ui->txtShiCiJianGe->text().toInt()*1000*60;
    timer->start(g_mainTimerMSpan);
}

//倒计时函数
void MainWindow::lastTimerTick()
{
    QString sss;
    sss.append("还有");
    sss.append(QString::number(raderTickSpan));
    sss.append("秒定时器启动");

    //ss.ap"还有"+lastTimerMSpan+"秒定时器启动";
    ui->lblMainTimerTip->setText(sss);
    raderTickSpan--;
}


//计时器函数
void MainWindow::timertick()
{
    QString m_strRunPath = QCoreApplication::applicationDirPath();
    //currentMyDateTime=QDateTime::currentDateTime();
    currentMyDateTime=QDateTime::fromString("20140526000600", "yyyyMMddhhmmss");

    raderTickSpan=60*LastTimeSpan;
    QString sFileName="";

    if(hasFile(sFileName))
    {
        //遍历制定目录下所有文件
        //获取满足条件的文件
        //循环调用进程
        QList<QString> listFilePathList=readDir(sDataFilePath);
        QList<QString> rightFilePathList=GetFitConditionFile(listFilePathList);
        QList<QString> rightFileOnlyName;


        for(int i=0;i<rightFilePathList.count();i++)
        {
            //fileInfo aFileInfo=getFileInfo("F:\C++\Project\QC_QPE\build-QC_QPE-Desktop_Qt_5_2_1_MinGW_32bit-Debug\debug\testFile\CinradMosaic20140518022821.txt");
            fileInfo aFileInfo=getFileInfo(rightFilePathList[i]);

            rightFileOnlyName.append(aFileInfo.fileName);
            QString fileStr="定时器执行---调用进程,文件名:"+aFileInfo.fileName+" ,创建时间:"+aFileInfo.createDate+",修改时间:"+aFileInfo.updateDate;

            ui->listWidget->addItem(new QListWidgetItem(fileStr));

            //对文件名做个处理
            QString tmpStr=sStartProcessName+" "+sDataTestFilePath+aFileInfo.fileName;
            startProcess(m_strRunPath+"/"+tmpStr);
        }

        if(taskOperateCurrentIndex>2)
        {
            taskOperateCurrentIndex=0;
        }
        else
        {
            if(taskOperateInfo[taskOperateCurrentIndex].count()>0)
            {
                taskOperateInfo[taskOperateCurrentIndex].clear();
            }
            taskOperateInfo[taskOperateCurrentIndex++]=rightFileOnlyName;
        }
    }
}


//判断是否存在文件
bool MainWindow::hasFile(QString sFileName)
{
    return true;
}

//开启进程
void MainWindow::startProcess(QString sFileName)
{
    //QProcess *myProcess = new QProcess();
    myProcess = new QProcess();
    myProcess->start(sFileName);
}

//工具函数
bool MainWindow::parseBool(QString tmpString)
{
    if(tmpString=="true")
    {
        return true;
    }
    else
    {
        return false;
    }
}

QString MainWindow::boolParse(bool tmpBool)
{
    if(tmpBool)
    {
        return "true";
    }
    else
    {
        return "false";
    }
}

QHash<QString,QString>MainWindow:: parseHeigh(QString strHeigh){
    QHash<QString,QString> list;
    QStringList tmpHeights =  strHeigh.split(',');
    for(int ii=0;ii<tmpHeights.length();ii++){

    }
    QString tmpKeyName="";
    QString tmpKeyValue="";
}

QString MainWindow::saveHeigh(QHash<QString,QString> heightList){

}

QString MainWindow::ColumnParse(int coulumnID)
{
    switch(coulumnID)
    {
    case 0:
        return "/zhanhao";
    case 1:
        return "/zhanming";
    case 2:
        return "/jingdu";
    case 3:
        return "/weidu";
    case 4:
        return "/haiba";
    case 5:
        return "/leidaxinghao";
    case 6:
        return "/fanwei";
    case 7:
        return "/shujulujing";
    }
}

QString MainWindow::getStardString(QString str)
{
    QString resultStr=str;
    if(str.contains("#"))
    {
        QStringList strList = str.split("#");
        resultStr=strList[0];
    }
    return resultStr.trimmed();
}

/*******************************************************替换字符串*********************************************************/
//替换字符串
QString MainWindow::getReplaceString(QString str)
{
    QString beforeStr="";
    QString endStr="";

    QString keyValue=str.split("=")[0];             //为什么是【0】，这与下面else的beforeStr有区别吗

    QString desStr=str;

    if(str.contains("#"))
    {
        int nIndex=str.indexOf("#");                    //“#”的位置
        int nEqualIndex=str.indexOf("=");

        beforeStr=str.mid(0,nEqualIndex+1);
        endStr=str.mid(nIndex+1,str.length()-nIndex-1);
        desStr=beforeStr+sReplaceOperate+keyValue+endStr;
    }
    else
    {
        int nEqualIndex=str.indexOf("=");
        beforeStr=str.mid(0,nEqualIndex+1);             //“=”之前的字符
        desStr=beforeStr+sReplaceOperate+keyValue;      //sReplaceOperate="!@#$%"
    }
    return desStr;
}

//去掉the ASCII characters '\t', '\n', '\v', '\f', '\r', and ' '.
QString MainWindow::ReplaceSpaceAndTab(QString str)
{
    str=str.simplified();
    return str;
}

/*******************************************************写文件操作*********************************************************/
//写文件
void MainWindow::writeFile(QString fileName,QString content,bool isAppend)
{
    QFile contentfile(fileName);

    if(isAppend==true)
    {
        if(!contentfile.open(QIODevice::WriteOnly|QIODevice::Append))//如果不能读，也不能添加
        {
            QMessageBox::critical(this, tr("Error"), tr("无法打开文件"));
            return;
        }
    }
    else                                                              //isAppend==false
    {
        if(!contentfile.open(QIODevice::WriteOnly | QIODevice::Text))  //高度文件总是重复写入，是不是这的问题
        {
            QMessageBox::critical(this, tr("Error"), tr("无法打开文件"));
            return;
        }
    }
    QTextStream stream(&contentfile);

    if(isAppend)
    {
        QString ss="";
        ss.append("\r\n").append(content);
        stream <<ss;
    }
    else
    {
        stream << content;
    }
    stream.flush();
    contentfile.close();
}

/**********************************************写文件操作---zhy*********************************************************/
void MainWindow::writeHeightFile(QString fileName,QString content,bool isAppend)
{
    QDir myDir;
    QString m_strRunPath = QCoreApplication::applicationDirPath();
    myDir.remove(m_strRunPath+"/"+fileName);
    QFile contentfile(fileName);
    QTextStream stream(&contentfile);
    if(isAppend==true)
    {
        if(!contentfile.open(QFile::WriteOnly|QFile::Truncate|QFile::Text))
        {
            QMessageBox::critical(this, tr("Error"), tr("无法打开文件"));
            return;
        }
        QString ss="";
        ss.append("\r\n").append(content);
        stream <<ss;
    }
    else                                                              //isAppend==false
    {
        if(!contentfile.open(QFile::WriteOnly|QFile::Truncate))
        {
            QMessageBox::critical(this, tr("Error"), tr("无法打开文件"));
            return;
        }

        stream << content;
    }
    stream.flush();
    contentfile.close();
}

/*******************************************************读文件*********************************************************/
//读文件
QString MainWindow::readFile(QString fileName)
{
    if (!fileName.isEmpty())
    {
        //读文件,如果不存在，再创建
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly))
        {
            file.open(QIODevice::WriteOnly | QIODevice::Text);
            file.close();
        }
        QFile Endfile(fileName);
        if (!Endfile.open(QIODevice::ReadOnly))
        {
            QMessageBox::critical(this, tr("Error"), tr("无法打开文件"));
            return "";
        }
        QTextStream in(&Endfile);
        QString fileContent=in.readAll();
        Endfile.close();
        return fileContent;
    }
}

/*******************************************************获取目录下的所有文件*****************************************************/
//获取目录下的所有文件
QList<QString> MainWindow::readDir(QString dirName)
{
    //判断路径是否存在
    QList<QString> string_list;

    QDir dir(dirName);
    if(!dir.exists())
    {
        return string_list;
    }
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    QFileInfoList list = dir.entryInfoList();

    int file_count = list.count();
    if(file_count <= 0)
    {
        return string_list;
    }


    for(int i=0; i<file_count; i++)
    {
        QFileInfo file_info = list.at(i);
        QString absolute_file_path = file_info.absoluteFilePath();
        string_list.append(absolute_file_path);
    }
    return string_list;
}

void MainWindow::writeConfig(QString fileName,QHash<QString,QString> configList)
{
    //Qt中使用QSettings类读写ini文件
    //QSettings构造函数的第一个参数是ini文件的路径,第二个参数表示针对ini文件,第三个参数可以缺省
    QSettings *configIniWrite = new QSettings(fileName, QSettings::IniFormat);
    QHashIterator<QString,QString> i(configList);
    for(;i.hasNext();){
        configIniWrite->setValue(i.key(),i.next().value());
    }
    delete configIniWrite;
}

/*************************************************将开始、截止日期时间写入ini文件**************************************************/
void MainWindow::inputTimeDate(QString fileName)
{
    QString txtMyStartDateTime;
    QString txtMyEndDateTime;

    txtMyStartDateTime = "StartDateTime="+ui->startDateTimeEdit->dateTime().toString("yyyyMMddHHmmss")+"\r\n";
    txtMyEndDateTime = "EndDateTime="+ui->endDateTimeEdit->dateTime().toString("yyyyMMddHHmmss");
    QString content;
    content = txtMyStartDateTime + txtMyEndDateTime;
    writeFile(fileName,content,false);
}

/*************************************************读取开始、截止日期时间ini文件*****************************************************/
void MainWindow::setTimeDate(QString fileName)
{
    QString strConfig = readFile(fileName);

    if(strConfig.isNull())//如果ini文件中是空
    {
        return;
    }
    else
    {
        QStringList strList = strConfig.split("\r\n");
        QStringList strFirstLine = strList[0].split("=");
        QDateTime m_StartDateTime=QDateTime(QDate(strFirstLine[1].mid(0,4).toInt(),strFirstLine[1].mid(4,2).toInt(),strFirstLine[1].mid(6,2).toInt()),QTime(strFirstLine[1].mid(8,2).toInt(),strFirstLine[1].mid(10,2).toInt(),strFirstLine[1].mid(12,2).toInt()));
        ui->startDateTimeEdit->setDateTime(m_StartDateTime);

        QStringList strSecondLine = strList[1].split("=");
        QDateTime m_EndDateTime=QDateTime(QDate(strSecondLine[1].mid(0,4).toInt(),strSecondLine[1].mid(4,2).toInt(),strSecondLine[1].mid(6,2).toInt()),QTime(strSecondLine[1].mid(8,2).toInt(),strSecondLine[1].mid(10,2).toInt(),strSecondLine[1].mid(12,2).toInt()));
        ui->endDateTimeEdit->setDateTime(m_EndDateTime);
    }
}

/*******************************************************析构函数*********************************************************/
MainWindow::~MainWindow()
{
    delete ui;
}

/*******************************************************退出*********************************************************/
void MainWindow::on_action_8_triggered()
{
    qDebug()<<chooseProcess->isOpen();
    if(chooseProcess->isOpen())
    {
        chooseProcess->close();
    }
    if(addProcess->isOpen())
    {
        addProcess->close();
    }
    if(shiShiProcess->isOpen())
    {
        shiShiProcess->close();
    }
    if(liShiProcess->isOpen())
    {
        liShiProcess->close();
    }
    //这里要将后续的所有程序全部关闭
    //this->close();
   // this->destroy(true,true);
    exit(0);
}

/*******************************************************停止*********************************************************/
void MainWindow::on_actionStop_triggered()
{
    if(shiShiProcess->isOpen())
    {
        shiShiProcess->close();
        iconShishiChangeCount=0;
        systemTrayIcon->setIcon(QIcon("trayIcon.png"));
        ui->action_3->setEnabled(true);
        ui->action_2->setEnabled(true);
        action_ShiShiFenXi->setEnabled(true);
        action_LiShiFenXi->setEnabled(true);
        shiShiTitleTimer->stop();

        systemTrayIcon->setToolTip("大区域天气雷达数据QC及组网QPE系统");
        setWindowTitle(tr("大区域天气雷达数据QC及组网QPE系统"));
        systemTrayIcon->showMessage(QString("QC_QPE"), QString("实时模式停止"));
    }

    if(liShiProcess->isOpen())
    {
        liShiProcess->close();
        iconShishiChangeCount=0;
        systemTrayIcon->setIcon(QIcon("trayIcon.png"));
        ui->action_3->setEnabled(true);//在进行历史分析的时候,会让“历史分析”菜单不可操作,整个过程结束后,使“历史分析”菜单再次可以操作
        ui->action_2->setEnabled(true);
        action_ShiShiFenXi->setEnabled(true);
        action_LiShiFenXi->setEnabled(true);
        titleTimer->stop();

        systemTrayIcon->setToolTip("大区域天气雷达数据QC及组网QPE系统");
        setWindowTitle(tr("大区域天气雷达数据QC及组网QPE系统"));
        systemTrayIcon->showMessage(QString("QC_QPE"), QString("分析模式停止"));
    }
}

/*******************************************************隐藏*********************************************************/
void MainWindow::on_Hide_triggered()
{
    //在初始化的时候,要读取ini中的值,从而将对号赋值“显示”或者“隐藏”
    //这里要点击这个按钮,ini中的值设为false,同时后面显示对号
    ui->Show->setText("显示");
    ui->Hide->setText("隐藏 √");
    QSettings settings("./developer.ini", QSettings::IniFormat);
    settings.setValue("FormOption/bShowForm","false");
}

/*******************************************************显示*********************************************************/
void MainWindow::on_Show_triggered()
{
    ui->Hide->setText("隐藏");
    ui->Show->setText("显示 √");
    QSettings settings("./developer.ini", QSettings::IniFormat);
    settings.setValue("FormOption/bShowForm","true");
}
/************************************************"显示"或者"隐藏"初始化****************************************************/
void MainWindow::showOrHide()
{
    QSettings settings("./developer.ini", QSettings::IniFormat);
    QString value = settings.value("FormOption/bShowForm",false).toString();
    qDebug()<<value<<"value";
    if(value=="true")
    {
        ui->Hide->setText("隐藏");
        ui->Show->setText("显示 √");
    }
    else if(value=="false")
    {
        ui->Show->setText("显示");
        ui->Hide->setText("隐藏 √");
    }
}

/******************************************************关闭窗口时,隐藏处理*******************************************************/
void MainWindow::closeEvent(QCloseEvent *event)
{
        event->ignore();
        hide();
}

/******************************************************修改最小化*******************************************************/
//void MainWindow::changeEvent(QEvent *event)
//{
//    if(event->type()==QEvent::WindowStateChange)
//    {
//        if(windowState() & Qt::WindowMinimized)
//        {
//            hide();
//            //mSystemTrayIcon->show();
//        }
//    }
//}

/******************************************************托盘*******************************************************/
void MainWindow::myTrayIcon()
{
    systemTrayIcon = new QSystemTrayIcon(this);
    systemTrayIcon->setIcon(QIcon("trayIcon.png"));
    systemTrayIcon->setVisible(true);
    systemTrayIcon->showMessage(QString("QC_QPE"), QString("大区域天气雷达数据QC及组网QPE系统"));
    systemTrayIcon->setToolTip("大区域天气雷达数据QC及组网QPE系统");
    //systemTrayIcon->toolTip();

    //创建托盘项
    action_show = new QAction(this);
    action_quit = new QAction(this);
    action_minimize = new QAction(this);
    action_ShiShiFenXi = new QAction(this);
    action_LiShiFenXi = new QAction(this);
    action_Stop = new QAction(this);

    action_ShiShiFenXi->setText("运行");
    action_LiShiFenXi->setText("分析");
    action_Stop->setText("停止");
    action_minimize->setText("最小化");
    action_show->setText("显示");
    action_quit->setText("退出");

    QMenu *trayIconMenu = new QMenu(this);

    trayIconMenu->addAction(action_ShiShiFenXi);
    trayIconMenu->addAction(action_LiShiFenXi);
    trayIconMenu->addAction(action_Stop);
    //trayIconMenu->addAction(action_minimize);
    trayIconMenu->addAction(action_show);
    trayIconMenu->addAction(action_quit);

    systemTrayIcon->setContextMenu(trayIconMenu);//右键菜单

    connect(action_ShiShiFenXi,SIGNAL(triggered()),this,SLOT(on_action_2_triggered()));//运行
    connect(action_LiShiFenXi,SIGNAL(triggered()),this,SLOT(on_action_3_triggered()));//分析
    connect(action_Stop,SIGNAL(triggered()),this,SLOT(on_actionStop_triggered()));//停止
    connect(action_show,SIGNAL(triggered()),this,SLOT(showNormal()));//显示
    connect(action_quit,SIGNAL(triggered()),this,SLOT(on_action_8_triggered()));//退出
    //connect(action_minimize,SIGNAL(triggered()),this,SLOT(showMinimized()));//最小化
    connect(systemTrayIcon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(iconIsActived(QSystemTrayIcon::ActivationReason)));
}

void MainWindow::iconIsActived(QSystemTrayIcon::ActivationReason reason)
{
   switch(reason)
   {
   //双击托盘显示窗口
   case QSystemTrayIcon::DoubleClick:
   {
     showNormal();
     break;
   }

   default:
   break;
   }
}

//保存雷达区域
void MainWindow::saveRadarRegion()
{
    QSettings settings("./radarRegion.ini", QSettings::IniFormat);
    QString strRadarRegion;
    strRadarRegion = ui->lineEdit->text().trimmed();
    if(ui->lineEdit->text()=="")
    {
        qDebug()<<"雷达区域为空";
        settings.setValue("Region/strRadarRegion",strRadarRegion);
    }
    else
    {
        qDebug()<<strRadarRegion;
        settings.setValue("Region/strRadarRegion",strRadarRegion);
    }
}

//显示雷达区域
void MainWindow::showRadarRegion()
{
    QSettings settings("./radarRegion.ini", QSettings::IniFormat);
    QString strRadarRegion = settings.value("Region/strRadarRegion",NULL).toString();
    if(strRadarRegion == NULL)
    {
        qDebug()<<"雷达区域NULL";
        return;
    }
    else
    {
        ui->lineEdit->setText(strRadarRegion);
    }
}
