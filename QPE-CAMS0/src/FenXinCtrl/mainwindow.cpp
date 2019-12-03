#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QMessageBox>
#include <QStringList>
#include <QTextStream>
#include <QDebug>
#include <QDir>
#include <QTime>
#include <QDate>
#include <QDateTime>
#include <QProcess>
#include <QTextCodec>
#include <QSettings>
#include <QThread>
//#include <QLineEdit>

const static QString TIME_LIST_FILENAME="QC_QPE_FenXiCtrlTimeList.txt";
const static QString PROCESS_NAME_QC_QPE_FEN_XI="QC_QPE_FenXi";
const static QString sRadarFileName="Radar.ini";
const static QString sDataPathConfigFileName="dataPathConfig.ini";
const static QString sDateTimeFileName = "inputTimeDate.ini";
const static QString sTimeParameterConfigName="timeParameterConfig.ini";


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    setWindowTitle(tr("大区域天气雷达数据QC及组网QPE系统 分析(历史数据)模式控制"));
    m_strRunPath = QCoreApplication::applicationDirPath();
    qDebug()<<m_strRunPath;

    m_iWorkLoadCnt=0;       // 已经启动处理的fenxi进程数
    m_iWorkDoneCnt=0;       // 已经处理完的fenxi进程数数
    m_iTimerCnt=0;          // 当前已花费时间
    m_iPreWorkStartPoint=0; // 前一个时段的work启动的时间点
    m_iAllWorksDoneTimerCnt=0;// 所有任务结束后的计时

    m_bAllWorksLoaded=false;    //所有时段都已经启处理
    m_bAllWorksDone=false;      //所有时段都已经处理完成

    QSettings developerIni("./developer.ini", QSettings::IniFormat);
    m_bQuickMode=developerIni.value("ConcurrencyControl/bQuickMode","false").toBool();
    //是否启动“快速并发模式”，同时处理多个时段
    m_RunWorkMax=developerIni.value("ConcurrencyControl/iRunWorkMax","2").toInt();
    //“快速并发模式”启用时，允许同时处理的时段数目上限
    m_WaitInterval=8;
    //“快速并发模式”启用时，两次自动调用需要等待的时间间隔(秒)
    if(m_bQuickMode==true)
    {
        ui-> pushButtonQM->setText("QuickMode(已启动)");
    }




    m_dtMinDateTime=QDateTime(QDate(2020,12,31),QTime(23,59,59));
    m_dtMaxDateTime=QDateTime(QDate(1900,1,1),QTime(0,0,0));
    CreateTimeSequenceIni();

 //---1.获取雷达站信息----
    GetWorkRadarInfo();
    //获取雷达原始数据的路径
    //遍历所有文件获取文件的日期范围
    //获取分析时间范围（文件名时间范围）
    qDebug()<<"-----------------------------------------------";
    qDebug()<<"File MinTime:"<<m_dtMinDateTime;
    qDebug()<<"File MaxTime:"<<m_dtMaxDateTime;

//---2.获取时间交集----
    //输入的时间范围
    QDateTime dtMinDateTime2;
    QDateTime dtMaxDateTime2;

    QString strStartDateTime;
    QString strEndDateTime;

    getInputDateTime(&strStartDateTime,&strEndDateTime);

    dtMinDateTime2 = QDateTime(QDate(strStartDateTime.mid(0,4).toInt(),strStartDateTime.mid(4,2).toInt(),strStartDateTime.mid(6,2).toInt()),QTime(strStartDateTime.mid(8,2).toInt(),strStartDateTime.mid(10,2).toInt(),strStartDateTime.mid(12,2).toInt()));
    dtMaxDateTime2 = QDateTime(QDate(strEndDateTime.mid(0,4).toInt(),strEndDateTime.mid(4,2).toInt(),strEndDateTime.mid(6,2).toInt()),QTime(strEndDateTime.mid(8,2).toInt(),strEndDateTime.mid(10,2).toInt(),strEndDateTime.mid(12,2).toInt()));
    qDebug()<<"Input MinTime:"<<dtMinDateTime2;
    qDebug()<<"Input MaxTime:"<<dtMaxDateTime2;
    //    dtMinDateTime2=QDateTime(QDate(2007,8,17),QTime(12,0,0));
    //    dtMaxDateTime2=QDateTime(QDate(2007,8,20),QTime(12,0,0));

    //最终交集的时间范围
    QDateTime dtMinDateTimeRslt;
    QDateTime dtMaxDateTimeRslt;

    dtMinDateTimeRslt=QDateTime(QDate(1900,1,1),QTime(0,0,0));
    dtMaxDateTimeRslt=QDateTime(QDate(1900,1,1),QTime(0,0,0));

    GetNewDateTimeFanWei(m_dtMinDateTime,m_dtMaxDateTime,dtMinDateTime2,dtMaxDateTime2,dtMinDateTimeRslt,dtMaxDateTimeRslt);


    qDebug()<<"Rslt MinTime:"<<dtMinDateTimeRslt;
    qDebug()<<"Rslt MaxTime:"<<dtMaxDateTimeRslt;

    ui->listWidgetInfo->addItem(new QListWidgetItem("数据文件区间:  \t"+
                                                    m_dtMinDateTime.toString("yyyy-MM-dd HH:mm:ss")+"\t"+
                                                    m_dtMaxDateTime.toString("yyyy-MM-dd HH:mm:ss")));
    ui->listWidgetInfo->addItem(new QListWidgetItem("时间输入区间:  \t"+
                                                    dtMinDateTime2.toString("yyyy-MM-dd HH:mm:ss")+"\t"+
                                                    dtMaxDateTime2.toString("yyyy-MM-dd HH:mm:ss")));
    ui->listWidgetInfo->addItem(new QListWidgetItem("二者交集区间:  \t"+
                                                    dtMinDateTimeRslt.toString("yyyy-MM-dd HH:mm:ss")+"\t"+
                                                    dtMaxDateTimeRslt.toString("yyyy-MM-dd HH:mm:ss")));


    if(dtMinDateTimeRslt==dtMaxDateTimeRslt)
    {
        ui->listWidgetInfo->addItem(new QListWidgetItem("输入时间有误，时间区间没有交集！"));
        QMessageBox::warning(this, tr("Error"), tr("输入时间有误，时间区间没有交集！"));
        SetEndFlag();
        exit(1);
    }


//    dtMinDateTimeRslt=QDateTime(QDate(2007,8,18),QTime(12,0,0));
//    dtMaxDateTimeRslt=QDateTime(QDate(2007,8,18),QTime(13,0,0));

//---3.根据时间范围获取有效时间段集合timeList----

    int m=6;
    int n=3;
    int k=0;
    QString fileName = m_strRunPath+"/"+sTimeParameterConfigName;
    ReadMNK(fileName,&m,&n,&k);
    GenerateTimeList(dtMinDateTimeRslt,dtMaxDateTimeRslt,m,n,k);

//---4.根据TimeList中的每一个时段启动QC_QPE_fenxi----

    ui->listWidgetInfo->addItem(new QListWidgetItem("准备开始"));

    on_pushButtonWork_clicked();//调用第一个一个QC_QPE分析

    timerWork= new QTimer(this);
    connect(timerWork,SIGNAL(timeout()),this,SLOT(TimerWork()));//用定时器不断查询是否启动下一个QC_QPE
    timerWork->start(1000);

}

void MainWindow::ReadMNK(QString fileName,int *M, int *N, int *K)       //读取.ini文件中的M,N,K
{
    QFile mFile(fileName);
    if(!mFile.open(QFile::ReadOnly|QFile::Text))
    {
        qDebug()<<"can not open for reading";
        ui->listWidgetInfo->addItem(new QListWidgetItem("没有找到时间参数配置文件！"));
        *M=12;
        *N=2;
        *K=0;
        return;
    }

    QString parameter[10];
    QString myText[10];
    QStringList timeParameterConfig;
    QTextStream in(&mFile);
    for(int j=0;j<10;j++)//提取.ini文件中的M,N,K放入parameter数组
    {
        myText[j] = in.readLine();
        if(!myText[j].isNull())
        {
            for(int i =myText[j].size()-1;i>=0;i--)
            {
                if(myText[j].data()[i]=='=')
                    break;
                else
                {
                    parameter[j]=myText[j].data()[i]+parameter[j];
                }
            }
            timeParameterConfig.append(parameter[j]);
        }
    }
    for(int k=0;k<10;k++)//去掉parameter数组前面的空值
    {
        if(parameter[k].isNull())
        {
            for(int h=k;h<10;h++)
            {
                parameter[h]=parameter[h+1];
            }
        }
    }

    qDebug()<<"timeParameterConfig::"<<timeParameterConfig.at(0)<<timeParameterConfig.at(1)<<timeParameterConfig.at(2);
    *M=parameter[0].toInt();
    *N=parameter[1].toInt();
    *K=parameter[2].toInt();
    mFile.flush();
    mFile.close();
}

void MainWindow::getInputDateTime(QString *strMyStartDateTime,QString *strMyEndDateTime)
{
    QString fileName = m_strRunPath+"/"+sDateTimeFileName;
    QString strConfig = readFile(fileName);
    if(strConfig.isNull())//如果ini文件中是空
    {
        *strMyStartDateTime ="";
        *strMyEndDateTime = "";
    }
    else
    {
        QStringList strList = strConfig.split("\r\n");
        QStringList strFirstLine = strList[0].split("=");
        *strMyStartDateTime = strFirstLine[1].trimmed();
        QStringList strSecondLine = strList[1].split("=");
        *strMyEndDateTime = strSecondLine[1].trimmed();
    }


}

MainWindow::~MainWindow()
{
    delete ui;
}

//开启进程
void MainWindow::startProcess(QString sFileName)
{
    QProcess *myProcess = new QProcess();
    myProcess->start(sFileName);
}

void MainWindow::GenerateTimeList(QDateTime dtMinDateTime,QDateTime dtMaxDateTime,int m,int n,int k)
{
//   dtDateTimeStart=QDateTime(dtMinDateTime.date(),QTime(0,0,0));

//    qDebug()<<"Start Time:"<<dtDateTimeStart;
    QTime tStart=QTime(0,m-n,0);
    QTime tMid=QTime(0,m,0);
    QTime tEnd=QTime(0,m+n+k,0);

    QDateTime dtDateTimeStart;
    QDateTime dtDateTimeMid;
    QDateTime dtDateTimeEnd;

    dtDateTimeStart=QDateTime(dtMinDateTime.date(),tStart);
    dtDateTimeMid=QDateTime(dtMinDateTime.date(),tMid);
    dtDateTimeEnd=QDateTime(dtMinDateTime.date(),tEnd);

    QString strFileName=m_strRunPath+"/"+ TIME_LIST_FILENAME;//"/QC_QPE_FenXiCtrlTimeList.txt";
    QDir myDir;
    myDir.remove(strFileName);

    QFile contentfile(strFileName);
    QTextStream stream(&contentfile);
    QString strText;

    contentfile.open(QIODevice::WriteOnly|QIODevice::Append);

    while(true)
    {
        if(dtDateTimeEnd>dtMaxDateTime)
        {
            break;
        }

        if(dtDateTimeStart>=dtMinDateTime)
        {
            qDebug()<<"Start1:"<<dtDateTimeStart<<"Mid:"<<dtDateTimeMid<<"End:"<<dtDateTimeEnd;
            strText=dtDateTimeStart.toString("yyyyMMddHHmmss")+"|"+dtDateTimeEnd.toString("yyyyMMddHHmmss")+"|"+dtDateTimeMid.toString("yyyyMMddHHmmss");
            m_strWorkTimeList.append(strText);
            strText=strText+"\r\n";
            stream << strText;
        }
        dtDateTimeStart=dtDateTimeStart.addSecs(m*60);
        dtDateTimeMid=dtDateTimeMid.addSecs(m*60);
        dtDateTimeEnd=dtDateTimeEnd.addSecs(m*60);

    }

    stream.flush();
    contentfile.close();
}

void MainWindow::GetNewDateTimeFanWei(QDateTime dtMinDateTime1,QDateTime dtMaxDateTime1,QDateTime dtMinDateTime2,QDateTime dtMaxDateTime2,QDateTime& dtMinDateTimeRslt,QDateTime& dtMaxDateTimeRslt)
{
    ;
    if(dtMaxDateTime1<dtMinDateTime2)
    {//没有交集
 //       int k=1;
        return;
    }
    if(dtMaxDateTime2<dtMinDateTime1)
    {//没有交集
//        int k=1;
        return;
    }
    if(dtMinDateTime1<dtMinDateTime2)
    {//小的里面取大的
        dtMinDateTimeRslt=dtMinDateTime2;
    }else
    {
        dtMinDateTimeRslt=dtMinDateTime1;
    }

    if(dtMaxDateTime1<dtMaxDateTime2)
    {//大的里面取小的
        dtMaxDateTimeRslt=dtMaxDateTime1;
    }else
    {
        dtMaxDateTimeRslt=dtMaxDateTime2;
    }
}
//获取目录下的所有文件
void MainWindow::readDir(QString dirName)//最终获取文件中最大和最小日期时间
{
    //判断路径是否存在
//    QList<QString> string_list;

    QDir dir(dirName);
    if(!dir.exists())
    {
//        return string_list;
        return;
    }
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    QFileInfoList list = dir.entryInfoList();

    int file_count = list.count();
    if(file_count <= 0)
    {
 //       return string_list;
        return ;
    }


    QString strAbsolute_file_path;
    QString strfileName;
    QStringList strFileInfoList;
    QDate fileDate ;
    QTime fileTime;

    QDateTime   fileDateTime;

    for(int i=0; i<file_count; i++)
    {
        QFileInfo file_info = list.at(i);
        strAbsolute_file_path = file_info.absoluteFilePath();
        strfileName=file_info.fileName();
//        string_list.append(absolute_file_path);
//        qDebug()<<strAbsolute_file_path;
//        qDebug()<<strfileName;
        strFileInfoList=strfileName.split("_");
        for(int j=0; j<strFileInfoList.size(); j++)
        {
//            qDebug()<<strFileInfoList[j];
        }

        //strFileInfoList[4]是数据文件名上的时间日期
        int iYear=strFileInfoList[4].mid(0,4).toInt();
        int iMonth=strFileInfoList[4].mid(4,2).toInt();
        int iDay=strFileInfoList[4].mid(6,2).toInt();

        int iHour=strFileInfoList[4].mid(8,2).toInt();
        int iMinute=strFileInfoList[4].mid(10,2).toInt();
        int iSecond=strFileInfoList[4].mid(12,2).toInt();

        fileDate=QDate(iYear,iMonth,iDay);
        fileTime=QTime(iHour,iMinute,iSecond);
        fileDateTime=QDateTime(fileDate,fileTime);

        if(fileDateTime<m_dtMinDateTime)
        {
            m_dtMinDateTime=fileDateTime;
        }
        if(fileDateTime>m_dtMaxDateTime)
        {
            m_dtMaxDateTime=fileDateTime;
        }
//        qDebug()<<dtMinDateTime;
//        qDebug()<<dtMaxDateTime;

    }
}

QString MainWindow::readFile(QString fileName)
{
    QString fileContent;
    if (!fileName.isEmpty())
    {
        //读文件,如果不存在，再创建
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly))
        {
 //           file.open(QIODevice::WriteOnly | QIODevice::Text);
  //          file.close();
            return fileContent;
        }
        QFile Endfile(fileName);
        if (!Endfile.open(QIODevice::ReadOnly))
        {
            QMessageBox::critical(this, tr("Error"), tr("无法打开文件"));
            return "";
        }
        QTextStream in(&Endfile);
        fileContent=in.readAll();
        Endfile.close();
//        return fileContent;
    }
    return fileContent;
}

QString MainWindow::ReplaceSpaceAndTab(QString str)
{
    str=str.replace("="," ");
    str=str.simplified();
    return str;
}
void MainWindow::GetWorkRadarInfo()//获取雷达站信息，最终向readDir（）中传递数据文件路径
{

    QString strFileName=m_strRunPath+"/"+sRadarFileName;
    qDebug()<<strFileName;
    QString tableContent=readFile(strFileName);
    //先按照换行分割
    m_strWorkRadarList = tableContent.split("\r\n");

    QStringList strRadarInfoList;
    for(int i=0;i<m_strWorkRadarList.size();i++)
    {
        //QMessageBox::information(this,"",words.at(i));
/*        QString tableContentItem=m_strWorkRadarList[i];
        qDebug()<<tableContentItem;
        tableContentItem=ReplaceSpaceAndTab(tableContentItem);
        qDebug()<<tableContentItem;
*/
        m_strWorkRadarList[i]=ReplaceSpaceAndTab(m_strWorkRadarList[i]);
        qDebug()<<m_strWorkRadarList[i];//"Z9592 厦门 118.08 24.48 185 SA 300 E:\气科院\Sepact\Z9592"
        strRadarInfoList= m_strWorkRadarList[i].split(" ");
//        for(int j=0;j<strRadarInfoList.size();j++)
//        {
//          qDebug()<<strRadarInfoList[j];
//        }
        if(strRadarInfoList.size()>6)
        {
            CheckAndCreateDir(strRadarInfoList[1]);
            //readDir是为了获得最大最小时间！
            readDir(strRadarInfoList[7]);//strRadarInfoList[7]为数据文件路径 "E:\气科院\Sepact\Z9592"
        }
    }

}

void MainWindow::CheckAndCreateDir(QString strDirName)
{
    QString strTempPath=ReadTempPath();
    if(strTempPath=="")
    {
        strTempPath="D:/SMOSAICDATA/";;
    }else
    {
        strTempPath=strTempPath+"/SMOSAICDATA/";;
    }
 //   QString strPath="D:/SMOSAICDATA/"+strDirName;
    QString strPath=strTempPath+strDirName;

    QDir mDir;
    if(!mDir.exists(strPath))
    {
        mDir.mkpath(strPath);
    }
}
QString MainWindow::ReadTempPath()
{
    QString strRetVal="";
    QString strTemDataPathKey="m_TemDataPath=";
    QString strFileName=m_strRunPath+"/"+sDataPathConfigFileName;
    int iPos;
    qDebug()<<strFileName;
    QString tableContent=readFile(strFileName);
    //先按照换行分割
    QStringList strFileTextList= tableContent.split("\r\n");
    for(int j=0;j<strFileTextList.size();j++)
    {
       iPos=strFileTextList.at(j).indexOf(strTemDataPathKey);
       if(iPos==0)
       {
           strRetVal=strFileTextList.at(j).mid(iPos+strTemDataPathKey.length());
           iPos=strRetVal.indexOf("\r");
           if(iPos>0)
           {
               strRetVal=strRetVal.left(iPos);
           }
//           strRetVal=strRetVal+"/";
       }
    }

    return strRetVal;
}

//载入下一个时段，启动QC_QPE_fenxi进行处理
void MainWindow::on_pushButtonWork_clicked()
{
    QString strFileName;
    if(!m_bAllWorksLoaded)//启动一个QC_QPE_FenXi.exe
    {
        //启动一个新的work(Load another work)
        strFileName="\""+m_strRunPath+"/"+PROCESS_NAME_QC_QPE_FEN_XI+"\""+
                " "+m_strWorkTimeList.at(m_iWorkLoadCnt)+" "+QString::number(m_iWorkLoadCnt+1);//m_strWorkTimeList.at(m_iWorkCnt)是作为参数传递的时间段，
        startProcess(strFileName);//QC_QPE_FenXi.exe
        m_iPreWorkStartPoint=m_iTimerCnt;//记录上一个work启动的时间

        //显示载入新的Work时候的信息:
        QDateTime currentMyDateTime=QDateTime::currentDateTime();
        strFileName =currentMyDateTime.toString("yyyy-MM-dd hh:mm:ss")+QString("[%1]").arg(m_iWorkLoadCnt+1)+strFileName;
        ui->listWidgetInfo->addItem(new QListWidgetItem(strFileName));

        m_iWorkLoadCnt++;

        if(m_iWorkLoadCnt==m_strWorkTimeList.size())
        {
            m_bAllWorksLoaded=true;
            ui->listWidgetInfo->addItem(new QListWidgetItem(QString(" All works loaded. (Total:%1)").arg(m_iWorkLoadCnt)) );
        }
    }
}

void MainWindow::TimerWork()
{
    //    timerWork->stop();
    m_iTimerCnt++;

    if(!m_bAllWorksDone)
    {
        QString strText=QString("等待 Mosaic_Contrl(%1s)").arg(m_iTimerCnt)
                +QString(" [Work Loaded:%1]").arg(m_iWorkLoadCnt)
                +QString(" [Work Finished:%1]").arg(m_iWorkDoneCnt);
        ui->lineEdit->setText(strText);

        if(CheckFileIsExist(m_strRunPath,"/Temp/QC_QPE_FenXi_Mosaic_Contrl_EndFlag.txt"))//QC_QPE_FenXi_Mosaic_Contrl.exe结束标志
        {
            on_pushButtonWork_clicked();//载入下一个任务（启动fenxi处理下一个时段）
            m_iWorkDoneCnt++;
            //  m_iTimerCnt=0;
        }

        if(m_bQuickMode)
        {
            if(((m_iTimerCnt - m_iPreWorkStartPoint) > m_WaitInterval) && (m_iWorkLoadCnt-m_iWorkDoneCnt<m_RunWorkMax))
            {//距离上一次启动15s以后，自动再load一个任务
                //如果当前正在运行的任务过多，则不再load新任务
                on_pushButtonWork_clicked();
            }
        }
    }
    else m_iAllWorksDoneTimerCnt++;
    if(m_bAllWorksLoaded)//all works loaded{
    {
        if(!m_bAllWorksDone)
        {
            if(CheckWorksDone()||NoMosaicCtrl())
            {
                QString strText="[历史分析]结束!";
                ui->lineEdit->setText(strText);
                QDateTime currentMyDateTime=QDateTime::currentDateTime();
                ui->listWidgetInfo->addItem(new QListWidgetItem(currentMyDateTime.toString("yyyy-MM-dd hh:mm:ss")+" All works done !"));
                //m_iTimerCnt=0;
                m_bAllWorksDone=true;
                SetEndFlag();
            }
        }
        else// if(m_bAllWorksDone)
        {
            if(m_iAllWorksDoneTimerCnt>=5)         exit(0);//10s 后自动退出
        }
    }// if(m_bAllWorksLoaded)

    //    timerWork->start(1000);

}

bool MainWindow::CheckFileIsExist(QString strPath,QString strFileName)//查看目标文件是否存在
{
    bool bRet=false;
    QString strAllName;
    strAllName=strPath+strFileName;
    QDir mDir;
    if(mDir.exists(strAllName))
    {
        bRet=true;
        mDir.remove(strAllName);
    }
    QThread::msleep(200);

    return bRet;
}

bool MainWindow::CheckWorksDone()
{
    if(  m_iWorkDoneCnt==m_strWorkTimeList.size())
        return true;
    return false;
}

bool MainWindow::NoMosaicCtrl()
{//若当前进程列表已经没有NoMosaicCtrl

    return false;
}

void MainWindow::SetEndFlag()
{
    char  endflag[256]="";
    sprintf(endflag,"./Temp/FenXiCtrl_Complete.txt");
    QFile endflagFile(endflag);
    if (endflagFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        endflagFile.close();
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), tr("FenXiCtrl_Complete.txt 生成失败"));
    }
}

void MainWindow::on_pushButtonQM_clicked()
{
    m_bQuickMode=!m_bQuickMode;
    if(m_bQuickMode==true)
    {
        ui-> pushButtonQM->setText("QuickMode(已启动)");
    }
    else
    {
        ui-> pushButtonQM->setText("QuickMode(未启动)");
    }
}
void MainWindow::closeEvent(QCloseEvent *event)
{
   SetEndFlag();
}

void MainWindow::CreateTimeSequenceIni()
{
    QSettings settings("Temp/TimeSequence.ini", QSettings::IniFormat); // 当前目录的INI文件

    settings.setValue("RealMosaic",0);
    settings.setValue("TrecProd", 0);
    settings.setValue("QPEProd", 0);
    settings.setValue("MrefDerivProd", 0);
}
