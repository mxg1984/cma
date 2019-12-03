#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QDir>
#include <QDateTime>
#include <QProcess>
#include <QTimer>
#include <QDebug>
#include <QtAlgorithms>
#include <QThread>
#include <QSettings>

#if defined (WIN32)|| defined(WIN64)
    #include <windows.h>
    #include<tchar.h>
    #include<tlhelp32.h>
#elif defined (UNIX)

#else
    #include <dirent.h>             // 提供目录流操作函数
    #include <sys/stat.h>           // 提供属性操作函数
    #include <sys/types.h>          // 提供mode_t 类型
    #include  <unistd.h>
    #include <signal.h>
    #include <sys/wait.h>

#endif



const static QString strRadarIni="Radar.ini";
int g_iBaseDataQCLaunched=0;        //已被调用的BaseDataQC数目；
int g_iBaseDataQCFinished=0;          //已结束的BaseDataQC数目；
int g_iBaseDataQCMax=8;                 // 允许同时运行的BaseDataQC数目上限；

#ifndef _WIN32
void sig_chld_handle(int  )
{	//跟事件驱动的处理函数一样，会有一个隐式参数
    //pid_t pid;
    //int stat;                              //to store child process's endding way
    //pid=wait(&stat);
    wait(NULL);            // handle the zombie
    g_iBaseDataQCFinished++;
    //    char buf[256]="";
    //    CLogWriter  logwriter;
    //    sprintf(buf,"signo:%d child%d pid:%d terminated",signo,g_iBaseDataQCFinished,pid);
    //    logwriter.WriteMsg(buf);
}
#endif



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    int iCnt=0;//等待次数计数
    m_AppName="QC_QPE_fenxi";
    strMsg="";
    m_strRunPath = QCoreApplication::applicationDirPath();
    setWindowTitle(tr("大区域天气雷达数据QC及组网QPE系统_分析进程"));
#ifndef _WIN32
    signal(SIGCHLD,sig_chld_handle);//处理子进程结束信号
#endif

//1.解析命令行参数     "20140330103300|20140330103900|20140330103600" 1
//                    "20140330104500|20140330105100|20140330104800" 1
//                    "20140330000800|20140330001800|20140330001300" 1
    QStringList args=qApp->arguments();//获取的FenXi_Ctrl.exe中传递过来的参数,一般会传递两个参数（1）QC_QPE_FenXi.exe.(2)时间段
    ui->listWidgetInfo->addItem(new QListWidgetItem("调用本程序时传递的参数:"));
    ui->listWidgetInfo->addItem(new QListWidgetItem(args[1] +" " +args[2] ));
    //F:\Release_1121\QC_QPE_FenXi "20140330103300|20140330103900|20140330103600" 1

    //设置工作的时间区间
    if(args.size()>2)
    {
        SetTimeFanWei(args[1]);//解析传入的时间参数
    }else
    {
        QMessageBox::critical(this,"Error","QC_QPE_Fenxi:No Proper Ruuning Arguments!");
        exit(0);
    }
    QString strCmdPara="";
    for(int i=1; i<args.size(); i++)
    {
        strCmdPara+=" ";
        strCmdPara+=args[i];
    }
    logWriter.WriteStatusMsg("",false);
    strMsg="Load Task: "+m_AppName+" "+strCmdPara;
    logWriter.WriteStatusMsg(strMsg.toStdString());


    m_strBaseDataQCrsltFenXiTxt=m_strRunPath+"/Temp/BaseDataQC_rslt_FenXi_"+m_dtMidDateTime.toString("yyyyMMddHHmmss")+".txt";
    m_strQCQPEFenXiRsltTxt= m_strRunPath+"/Temp/QC_QPE_FenXi_Rslt_"+m_dtMidDateTime.toString("yyyyMMddHHmmss")+".txt";
    QString strRunCtrlProcess="\""+m_strRunPath+"/QC_QPE_FenXi_Mosaic_Contrl\" "+m_dtMidDateTime.toString("yyyyMMddHHmmss")+" "+args[2];

    QDir myDir;
    myDir.remove(m_strBaseDataQCrsltFenXiTxt);
    myDir.remove(m_strQCQPEFenXiRsltTxt);
    m_strAllFileNameWrite="";

//2.打开配置文件,获取雷达配置数据
    GetWorkRadarInfo();//所有雷达站点数据保存在 m_strWorkRadarList

//3.调用BaseDataQC.exe

    //读取允许的最大并发数,对BaseDataQC并发控制
    QSettings developerIni("./developer.ini", QSettings::IniFormat);
    g_iBaseDataQCMax=developerIni.value("ConcurrencyControl/iBaseDataQCMax","8").toInt();

    ui->listWidgetInfo->addItem(new QListWidgetItem("\n调用BaseDataQC: "));
    for(int i=0;i<m_strWorkRadarList.size();i++)//链表中的size()是雷达站数 （似乎m_strWorkRadarList[0]==""）
    {
        StartOneRadarWorkProess(m_strWorkRadarList[i]);//在某个雷达站数据目录下寻找时间区间内的最优数据文件,调用baseDataQC处理
    }   


//4. 生成FenXiRslt.txt,调用QC_QPE_FenXi_Mosaic_Contrl
   if(m_strAllFileNameWrite.trimmed()!="")
   {
       //创建FenXiRslt.txt 记录被fenxi调用baseDataQC处理的数据项
       writeFile(m_strQCQPEFenXiRsltTxt,m_strAllFileNameWrite,false);
       startProcess(strRunCtrlProcess);
       ui->listWidgetInfo->addItem(new QListWidgetItem("\n调用MosaicCtrl: "));
       ui->listWidgetInfo->addItem(new QListWidgetItem(strRunCtrlProcess));
   }
//5. 直接进行本时段结束处理
   else {
/// 如果当前时段没有任何需要处理的数据，不调用MosaicControl,由fenxi完成本该MosaicCtrl完成的 同步时序控制 和 endflag生成。

       int iTaskNumber=args[2].toInt();
       QString sFileName=m_strRunPath+"/Temp/TimeSequence.ini";
       QSettings * qstTimeSequenceIni=new QSettings(sFileName, QSettings::IniFormat); // 当前目录的INI文件
       qstTimeSequenceIni->setValue("RealMosaic",iTaskNumber);
       qstTimeSequenceIni->setValue("MrefDerivProd",iTaskNumber);
       qstTimeSequenceIni->setValue("QPEProd",iTaskNumber);
       qstTimeSequenceIni->setValue("TrecProd",iTaskNumber);
       delete qstTimeSequenceIni;

       //创建Endflag
       iCnt=0;
       while(QFile::exists(m_strRunPath+"/Temp/QC_QPE_FenXi_Mosaic_Contrl_EndFlag.txt") && iCnt<10)//为了防止覆盖上一个Mosaic写出来的EndFlag
       {
           QThread::msleep(500);
           iCnt++;
       }
       QFile contentfile(QString(m_strRunPath+"/Temp/QC_QPE_FenXi_Mosaic_Contrl_EndFlag.txt"));
       contentfile.open(QIODevice::WriteOnly|QIODevice::Append);
       contentfile.close();
       iCnt=0;
       while(! QFile::exists(m_strRunPath+"/Temp/QC_QPE_FenXi_Mosaic_Contrl_EndFlag.txt")  && iCnt<20)
       {
           QThread::sleep(100);
           iCnt++;
       }
   }

//   QSettings settings("./developer.ini", QSettings::IniFormat);
//   bool bShowForm=settings.value("FormOption/bShowForm","false").toBool();
//   if(bShowForm)//bShowForm
//   {
//       QTimer *timer = new QTimer(this);
//       connect(timer,SIGNAL(timeout()),this,SLOT(TimerWork()));
//       m_iCntQuit=46; //调用结束后退出需要的时间s
//       timer->start(1000);
//   }
//   else
       exit(0);//不显示界面了


}

//-----------------------------------------其他函数-----------------------------------------

int MainWindow::GetBaseDataQCNum(void)//遍历进程列表，获取BaseDataQC数目
{
    int iPCnt=0;

#if defined (WIN32)|| defined(WIN64)
    //1. 声明Win32进程实体
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(pe32); // 在使用这个结构之前，先设置它的大小

    //2. 给系统内的所有进程拍一个快照
    HANDLE hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if(hProcessSnap == INVALID_HANDLE_VALUE)
    {
        printf(" CreateToolhelp32Snapshot调用失败！ \n");
        return 0;
    }
    //3. 遍历进程快照
    BOOL bMore = ::Process32First(hProcessSnap, &pe32);
    while(bMore)
    {
        if(!wcscmp(pe32.szExeFile,L"BaseDataQC.exe") )
            iPCnt++;
        bMore = ::Process32Next(hProcessSnap, &pe32);
    }
    //4. 清除snapshot对象
    ::CloseHandle(hProcessSnap);
#elif defined (UNIX)

#else
            iPCnt=g_iBaseDataQCLaunched-g_iBaseDataQCFinished;

//        //1.define related variable
//        DIR *dp;             // 定义子目录流指针
//        struct dirent *entry;//保存每次遍历得到的项基本信息
//        struct stat statbuf; // 保存每次遍历得到的项的详细信息（存储详细信息:文件名、扩展名等等）
//        //2.opendir
//        if((dp = opendir("/proc")) == NULL) // 打开目录
//        {
//            puts("can't open dir.");
//            return 0;
//        }
//        //3.遍历目录下每一项
//        int cnt=0;
//        while((entry = readdir(dp)) != NULL)
//        {
//            lstat(entry->d_name, &statbuf);
//            if( (S_IFDIR &statbuf.st_mode) == 0 )   continue ;  //不是目录
//            const char *s = entry->d_name;
//            while(*s && *s>='0' && *s<='9') s++;
//            if (*s) continue;                           //目录名不是纯数字
//            // printf("%d Dir:%s\n",cnt,entry->d_name);///
//            char filePath[260]="/proc/";
//            char buf[260]="";
//            strcat(filePath,entry->d_name); // 获得完整路径
//            strcat(filePath,"/comm");
//            FILE *fp=fopen(filePath,"r");   // /proc/[0-9]*/comm
//            if(fp==NULL)
//            {
//                printf("fopen %s failed",filePath);
//            }
//            else
//            {
//                fscanf(fp,"%s",buf);//fgets(thisProcess,sizeof(thisProcess)-1,fp);//会带入\n导致strcmp比较失败
//                //printf("processName:%s\n",buf);///print
//                if(strcmp(buf,"BaseDataQC")==0)
//                    iPCnt++;
//            }
//            fclose(fp);
//            cnt++;
//        }
//        //4.close dir stream
//        closedir(dp);


#endif

        return iPCnt;
}

void MainWindow::StartOneRadarWorkProess(QString strInText)//启动一个BaseDataQC进程
{
    //先按照换行分割
    QStringList strWorkRadarInfo = strInText.split(" ");//传递过来一个雷达站的所有信息，然后分割
    if(strWorkRadarInfo.size()<8)   return;//一个雷达站包括8个基本信息
    QString strRadarID,strRadarName,strRadarType,strRadarJingDu,strRadarWeiDu,strRadarGaoDu,strRadarFanWei,strDirName;
    strRadarID=strWorkRadarInfo[0];
    strRadarName=strWorkRadarInfo[1];
    strRadarJingDu=strWorkRadarInfo[2];
    strRadarWeiDu=strWorkRadarInfo[3];
    strRadarGaoDu=strWorkRadarInfo[4];
    strRadarType=strWorkRadarInfo[5];
    strRadarFanWei = strWorkRadarInfo[6];
    strDirName  = strWorkRadarInfo[7];//数据路径( F:/RadarData/GD-20140331/660_QC)
    //读取数据路径中文件数，比如读取E:/qixiangju/20141014/气科院/原始数据/GD-20140331/660_QC下的压缩文件数据
    QList<QString> listFilePathList=readDir(strDirName);
    //寻找最优数据文件
    m_iTmp= m_dtMaxDateTime.toString("yyyyMMddHHmmss").mid(8,6).toInt()-m_dtMinDateTime.toString("yyyyMMddHHmmss").mid(8,6).toInt();
    for(int j=0;j<listFilePathList.count();j++)//寻找路径下，文件名包含的日期在m_dtMinDateTime和m_dtMaxDateTime之间的最优文件
    {
        QFileInfo file_info = listFilePathList.at(j);
        QString strFileName=file_info.fileName();
        CheckFileTimeIsWorkTime(strFileName);//检查某雷达站点数据路径下第j个文件是否是"最优文件"
    }
    //循环结束，得到m_dtOptimumFileName;若不存在最优文件，m_dtOptimumFileName=="/r/n" // QMessageBox::critical(this, tr("Error"), m_dtOptimumFileName);

    //调用BaseDataQc处理最优文件
    if(m_dtOptimumFileName.trimmed()!="")///保证当前时段内有需要处理的数据文件
    {
        m_strAllFileNameWrite=m_strAllFileNameWrite+m_dtOptimumFileName+"\r\n";//记录调用信息
        QString strCmdLn="./BaseDataQC 00 "+strRadarID.mid(2)+" "+strRadarName+" "+strRadarType+" "+strRadarJingDu+" "+strRadarWeiDu+" "+strRadarGaoDu+" "+strRadarFanWei+" "+strDirName+"/"+m_dtOptimumFileName+" "+m_dtMidDateTime.toString("yyyyMMddHHmmss");
        //第一个参数00无意义？
        int Cnt=0;
        int iQCNum =GetBaseDataQCNum();//获取QC数目
        while( (iQCNum >= g_iBaseDataQCMax ) && (Cnt<30) )//最多运行g_iBaseDataQCMax个BaseDataQC.exe
        {
            iQCNum =GetBaseDataQCNum();
            Cnt++;
            //strMsg=QString("BaseDataQC :")+QString("%1").arg(iQCNum);
            //logWriter.WriteErrorMsg(strMsg.toStdString());
            //QMessageBox::information(this, tr("BaseDataQC进程数"), QString("%1").arg(iQCNum));
            QThread::msleep(2000);
        }
        startProcess(strCmdLn);
//        system(strCmdLn.toStdString().c_str());
        ui->listWidgetInfo->addItem(new QListWidgetItem(strCmdLn));
        m_dtOptimumFileName="";
        g_iBaseDataQCLaunched++;
    }///否则，不调用BaseDataQC
 }

//判断压缩文件上的日期时间是不是在最大和最小日期时间之间
bool MainWindow::CheckFileTimeIsWorkTime(QString strFileName)
{
    bool bRet=false;
    if(strFileName.indexOf(".bz2")<0)//判断文件名是不是为带有“.bz2”,带有的为0
    {
        return bRet;
    }
    QStringList strFileInfo=strFileName.split("_");
    //strFileInfo[4]为压缩文件名上带有日期时间的一段
    QDateTime qtFileCreateTime=QDateTime(QDate(strFileInfo[4].mid(0,4).toInt(),strFileInfo[4].mid(4,2).toInt(),strFileInfo[4].mid(6,2).toInt()),QTime(strFileInfo[4].mid(8,2).toInt(),strFileInfo[4].mid(10,2).toInt(),strFileInfo[4].mid(12,2).toInt()));
    int iMidDateTime = m_dtMidDateTime.toString("yyyyMMddHHmmss").mid(8,6).toInt();

    int iFileCreatTime = 0;
    if((qtFileCreateTime>=m_dtMinDateTime)&&(qtFileCreateTime<=m_dtMaxDateTime))
    {
        iFileCreatTime = qtFileCreateTime.toString("yyyyMMddHHmmss").mid(8,6).toInt();
        if(qAbs(iFileCreatTime-iMidDateTime)<=qAbs(m_iTmp))
        {
            m_iTmp = qAbs(iFileCreatTime-iMidDateTime);
            m_dtOptimumFileName = strFileName;
        }
        bRet=true;
    }

    return bRet;
}

void MainWindow::SetTimeFanWei(QString strInText)//将一个时间段分解成三个日期时间
{
    QStringList strTextList = strInText.split("|");
    m_dtMinDateTime=QDateTime(QDate(strTextList[0].mid(0,4).toInt(),strTextList[0].mid(4,2).toInt(),strTextList[0].mid(6,2).toInt()),QTime(strTextList[0].mid(8,2).toInt(),strTextList[0].mid(10,2).toInt(),strTextList[0].mid(12,2).toInt()));
    m_dtMaxDateTime=QDateTime(QDate(strTextList[1].mid(0,4).toInt(),strTextList[1].mid(4,2).toInt(),strTextList[1].mid(6,2).toInt()),QTime(strTextList[1].mid(8,2).toInt(),strTextList[1].mid(10,2).toInt(),strTextList[1].mid(12,2).toInt()));
    m_dtMidDateTime=QDateTime(QDate(strTextList[2].mid(0,4).toInt(),strTextList[2].mid(4,2).toInt(),strTextList[2].mid(6,2).toInt()),QTime(strTextList[2].mid(8,2).toInt(),strTextList[2].mid(10,2).toInt(),strTextList[2].mid(12,2).toInt()));
}

void MainWindow::GetWorkRadarInfo()//获得雷达的配置数据，将每个雷达站的基本信息（站号，站名，经度，纬度，数据路径等）保存在m_strWorkRadarList里
{
    QString strFileName=m_strRunPath+"/"+strRadarIni;
    qDebug()<<strFileName;
    QString tableContent=readFile(strFileName);
    //先按照换行分割
    m_strWorkRadarList = tableContent.split("\r\n");
    for(int i=0;i<m_strWorkRadarList.size();i++)
    {
        m_strWorkRadarList[i]=ReplaceSpaceAndTab(m_strWorkRadarList[i]);
        qDebug()<<m_strWorkRadarList[i];
    }
}

//void MainWindow::TimerWork()
//{
//     QString strCmdLn;
//    if(m_iCntQuit<=0)
//    {
//        exit(0);
//    }else
//    {
//        strCmdLn=QString("%1").arg(m_iCntQuit)+" 秒后退出......";
//        this->setWindowTitle(strCmdLn);
//        m_iCntQuit--;
//    }
//}

QString MainWindow::ReplaceSpaceAndTab(QString str)//去掉字符串中的“=”
{
    str=str.replace("="," ");
    str=str.simplified();//去掉字符串中'\t', '\n', '\v', '\f', '\r',  ' '.
    return str;
}

MainWindow::~MainWindow()
{
    delete ui;
}

//读文件
QString MainWindow::readFile(QString fileName)
{
    QString fileContent;
        if (!fileName.isEmpty()) {
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
            QMessageBox::critical(this, tr("Error"), m_AppName+tr("无法打开文件")+fileName);

            return "";
        }
        QTextStream in(&Endfile);
        fileContent=in.readAll();
        Endfile.close();
     }
     return fileContent;
}

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
//开启进程
void MainWindow::startProcess(QString sFileName)
{
    QProcess * myProcess = new QProcess();
    myProcess->start(sFileName);
    QThread::msleep(100);
}

//操作文件，写文件
void MainWindow::writeFile(QString fileName,QString content,bool isAppend)
{
    QFile contentfile(fileName);

    if(isAppend==true)
    {
        if(!contentfile.open(QIODevice::WriteOnly|QIODevice::Append))
        {
            QMessageBox::critical(this, tr("Error"),  m_AppName+tr("无法打开文件")+fileName);
            return;
        }
    }
    else
    {
        if(!contentfile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QMessageBox::critical(this, tr("Error"),  m_AppName+tr("无法打开文件")+fileName);
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
