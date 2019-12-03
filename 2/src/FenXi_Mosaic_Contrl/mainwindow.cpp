#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QProcess>
#include <QDateTime>
#include <QDir>
#include <QDebug>
#include <QFile>
#include <QSettings>
#include <QThread>
//#include <QThread>



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
//    this->show();
    setWindowTitle(tr("Mosaic Control"));
    //setWindowTitle(tr("MosaicControl ( 监控BaseDataQC, 控制启动realMosaic, MrefDeriv, Trec, QPE )"));
    m_runPath = QCoreApplication::applicationDirPath();//ui->listWidgetInfo->addItem(new QListWidgetItem(m_runPath));
    m_AppName="MosaicControl";
    QStringList args=qApp->arguments();
    QDir myDir;
    QString strFileFenXiRslt;



    ///    realdini;
    bool CR=false;
    bool ET=false;
    bool VIL=false;
    bool EchoMov=false;
    bool Fore=false;
    bool QPE=false;
    m_runGenMrefDeivProd=false;//是否需要启动该模块
    m_runGenTrecProd=false;
    m_runGenQPEProd=false;
    m_MrefDerivProdReady=false;//此模块是否已经处理ready？
    m_TrecProdReady=false;
    m_QpeProdReady=false;
    QSettings settings("productGenerateConfig.ini", QSettings::IniFormat);
    CR=settings.value("m_bChkCrProd","false").toBool();
    ET=settings.value("m_bChkEtProd","false").toBool();
    VIL=settings.value("m_bChkVilProd","false").toBool();
    EchoMov=settings.value("m_bChkEchoMovProd","false").toBool();
    Fore=settings.value("m_bChkForeProd","false").toBool();
    QPE=settings.value("m_bChkQpeProd","false").toBool();
    if(CR||ET||VIL)     m_runGenMrefDeivProd=true;//没有选择生成相关产品则不启动相关模块，直接认为其已经处理完成
    else                m_MrefDerivProdReady=true;//若没有勾选了任何mref导出产品(ET,CR,VIL)，就不运行GenMrefDerivProd模块，直接认为该模块处理结束
    if(EchoMov||Fore)   m_runGenTrecProd=true;
    else                m_TrecProdReady=true;
    if(QPE)             m_runGenQPEProd=true;
    else                m_QpeProdReady=true;

//    for(int i=0;i<args.size();i++)
//    {
//        qDebug()<<args[i];
//        if(i>=1)
//            ui->listWidgetInfo->addItem(new QListWidgetItem(args[i]));
//    }

    m_strMidDateTime="";

    if(args.size()>2)// 附加的CMD参数：20140330105400 1
    {
        m_strMidDateTime=args[1];
        strFileFenXiRslt = m_runPath+"/Temp/QC_QPE_FenXi_Rslt_"+m_strMidDateTime+".txt";
        m_iTaskNumber=args[2].toInt();
        setWindowTitle(tr("Mosaic Control [")+args[2]+"]");
        ui->listWidgetInfo->addItem(new QListWidgetItem("当前处理的时间段:"+m_strMidDateTime+" TaskNum:"+args[2]));

    }else
    {
        QMessageBox::critical(this,"Error","MosaicControl: No proper Running Arguments!");
        SetEndFlag();
        exit(1);
    }

    RealMosaic_Endflag = m_runPath+"/Temp/RealMosaic_"+m_strMidDateTime+"_EndFlag.txt";
    // /Temp/RealMosaic_20140330000500_EndFlag.txt
    GenMrefDerivProd_Endflag = m_runPath+"/Temp/GenMrefDerivProd_"+m_strMidDateTime.mid(2,10)+"_EndFlag.txt";
    // /Temp/GenMrefDerivProd_1403300005_EndFlag.txt
    GenTrecProd_Endflag = m_runPath+"/Temp/GenTrecProd_"+m_strMidDateTime.mid(2,10)+"_EndFlag.txt";
    // /Temp/GenTrecProd__1403300005_EndFlag.txt
    GenQpeProd_Endflag = m_runPath+"/Temp/GenQPEProd_"+m_strMidDateTime.mid(2,10)+"_EndFlag.txt";
    // /Temp/GenQpeProd__1403300005_EndFlag.txt

//    QFile::remove(m_runPath+"/Temp/QC_QPE_FenXi_Mosaic_Contrl_EndFlag.txt");//可能勿删除上一时段的！

    //读取FenXi_Rslt信息
    QString tableContent=readFile(strFileFenXiRslt);

    if(tableContent.trimmed()=="")//如果当前时间段内没有数据要处理
    {
        //QMessageBox::critical(this,"Error","当前时间段内没有数据！");
        SetEndFlag();
        exit(1);
    }

    m_PreparWorkFileName = tableContent.split("\r\n");

    ui->listWidgetInfo->addItem(new QListWidgetItem(""));
    ui->listWidgetInfo->addItem(new QListWidgetItem("BaseDataQC处理的数据："));
    for(int i=0;i<m_PreparWorkFileName.size()-1;i++)
    {
        ui->listWidgetInfo->addItem(new QListWidgetItem(m_PreparWorkFileName[i]));
    }


    //读取dataPathConfig,删除mref xx.dat文件
    tableContent=readFile(m_runPath+"/dataPathConfig.ini");
    QStringList dataPath = tableContent.split("\r\n");
    m_strMrefPath="";
    bool bGetPath=false;
    int idx=-1;
    for(int i=0;i<dataPath.size();i++)
    {
        idx=dataPath.at(i).indexOf("m_ProductPath");
        if(idx!=-1)
        {
            if((idx=dataPath.at(i).indexOf("="))!=-1)
            {
                m_strMrefPath=dataPath.at(i).mid(idx+1);
                bGetPath=true;
            }
        }
    }
    if(bGetPath==false)
    {
        QMessageBox::critical(this, tr("Error"), tr("配置文件dataPathConfig.ini错误！"));
        SetEndFlag();
        exit(0);
    }
    else
    {
        m_strMrefPath+=+"/mref/"+m_strMidDateTime.mid(2,10)+".dat";
        myDir.remove(m_strMrefPath);
    }


    //初始化Timer以及任务标志
    m_iWorkPeriod=1;    //1 当前的处理阶段（1:等待baseDataQC, 2等待RealMosaic 3等待MrefDerived、Trec、QPe）
    m_iReadyCnt=0;      //m_bReady==true后，经过的周期数
    m_iMatchCnt=0;      //匹配次数
    m_iNotFindCnt=0; //每个阶段(case1 case2 case3)，搜索模块结束标志失败的次数 （主要用于设置等待上限）
    m_bReady=false; //每个case的任务是否匹配成功标志
//    m_taskOver=false;//所有任务完成标志
    timerWork= new QTimer(this);
    connect(timerWork,SIGNAL(timeout()),this,SLOT(TimerWork()));
    timerWork->start(1000);
    m_iCntTimer=3;  //每隔3s后匹配一次
}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::TimerWork()
{
    if(m_iCntTimer<=0)//每定时3次满，检查是否任务完成
    {
        //        timerWork->stop();
        QString sFileName=m_runPath+"/Temp/TimeSequence.ini";
        QSettings qstTimeSequenceIni(sFileName, QSettings::IniFormat); // 当前目录的INI文件
        switch (m_iWorkPeriod)
        {
        case 1://1 判断baseDataQC是否处理结束
            if(!m_bReady)//检查rslt是否匹配
            {
                m_bReady=CheckAllFile();
                m_iNotFindCnt++;
                if(m_iNotFindCnt>100){///等待basedataQC时间上限：100(次)*3s==300s
                    ui->listWidgetInfo->addItem(new QListWidgetItem("等待BaseDataQC超时!"));
                    m_bReady=true;
                }
            }
            else
            {

                m_iNotFindCnt=0;
                m_iReadyCnt++;
                if(m_iReadyCnt==1)
                {
                    ui->listWidgetInfo->addItem(new QListWidgetItem("BaseDataQC处理结束"));
                    ui->listWidgetInfo->addItem(new QListWidgetItem(""));
                }
                if(m_iReadyCnt>=1)//2s (缩短以优化时间？)
                {
                    int iTaskFinished=qstTimeSequenceIni.value("RealMosaic",-2).toInt();
//                    setWindowTitle(QString::number(iTaskFinished));
                    if( iTaskFinished + 1 == m_iTaskNumber||m_iReadyCnt>=20)//如果上一个时段的RealMosaic已经结束
                    {
                        InvokeRealMosaic();//匹配成功，调用realMosaic.exe
                        m_iWorkPeriod=2;//case2
                        m_iReadyCnt=0;
                        m_bReady=false;

                    }
                }
            }
            break;

        case 2://2 判断RealMosaic是否处理结束
            if(!m_bReady)
            {
                m_bReady= (QFile::exists(m_strMrefPath) &&  QFile::exists(RealMosaic_Endflag));
                //mref下的.dat和realMosaicendflag.txt同时存在，认为realMosaic结束。
                if(m_bReady )
                {
                    ui->listWidgetInfo->addItem(new QListWidgetItem("RealMosaic处理结束"));
                    ui->listWidgetInfo->addItem(new QListWidgetItem(" "));
                    QFile::remove(RealMosaic_Endflag);
                    qstTimeSequenceIni.setValue("RealMosaic",m_iTaskNumber);
                    m_iWorkPeriod=3;//case 3
                    m_bReady=false;
                    m_iReadyCnt=0;

               }
                m_iNotFindCnt++;///等待RealMosaic时间上限：50(次)*3s
                if(m_iNotFindCnt>100) {
                    ui->listWidgetInfo->addItem(new QListWidgetItem("等待RealMosaic超时!"));

                    qstTimeSequenceIni.setValue("RealMosaic",m_iTaskNumber);
                    m_iWorkPeriod=3;//case 3
                    m_iReadyCnt=0;
                }
            }
            else
            {
                m_iReadyCnt++;
            }
            break;

        case 3://3 判断GenMrefDerivProd,GenTrecProd,xxx是否结束
            if(!m_bReady)
            {
                //产品生成处理开始
                if(m_runGenMrefDeivProd){//模块不调用，直接把m_runxx设为false
                    if(qstTimeSequenceIni.value("MrefDerivProd",-2).toInt()== m_iTaskNumber-1)
                    {
                        sFileName="\""+m_runPath+"/GenMrefDerivProd"+ "\" "+m_strMrefPath;
                        startProcess(sFileName);
                        ui->listWidgetInfo->addItem(new QListWidgetItem("启动GenMrefDerivProd "+sFileName));
                        m_runGenMrefDeivProd=false;//调用过就不再调用
                    }
                }

                if(m_runGenTrecProd){
                    if(qstTimeSequenceIni.value("TrecProd",-2).toInt()== m_iTaskNumber-1)
                    {
                            sFileName="\""+m_runPath+"/GenTrecProd"+"\" "+ m_strMrefPath;
                        startProcess(sFileName);
                        ui->listWidgetInfo->addItem(new QListWidgetItem("启动GenTrecProd "+sFileName));
                        m_runGenTrecProd=false;
                    }
                }
                if(m_runGenQPEProd){
                    if(qstTimeSequenceIni.value("QPEProd",-2).toInt()== m_iTaskNumber-1)
                    {
                        sFileName="\""+m_runPath+"/GenQPEProd"+"\" "+ m_strMrefPath;
                        startProcess(sFileName);
                        ui->listWidgetInfo->addItem(new QListWidgetItem("启动GenQPEProd "+sFileName));
                        m_runGenQPEProd=false;
                    }
                }
                //判断是否处理结束
                if(!m_MrefDerivProdReady){//模块不调用，直接把m_xxReady设为true
                    if(QFile::exists(GenMrefDerivProd_Endflag))
                    {
                        ui->listWidgetInfo->addItem(new QListWidgetItem("GenMrefDerivProd 处理结束 "));
                        m_MrefDerivProdReady=true;
                        QFile::remove(GenMrefDerivProd_Endflag);
                        QThread::msleep(100);
                        qstTimeSequenceIni.setValue("MrefDerivProd",m_iTaskNumber);
                    }
                }
                if(!m_TrecProdReady){
                    if(QFile::exists(GenTrecProd_Endflag))
                    {
                        ui->listWidgetInfo->addItem(new QListWidgetItem("GenTrecProd 处理结束 "));
                        m_TrecProdReady=true;
                        QFile::remove(GenTrecProd_Endflag);
                        QThread::msleep(100);
                        qstTimeSequenceIni.setValue("TrecProd",m_iTaskNumber);
                    }
                }
                if(!m_QpeProdReady){
                    if(QFile::exists(GenQpeProd_Endflag))
                    {
                        ui->listWidgetInfo->addItem(new QListWidgetItem("GenQPEProd 处理结束 "));
                        m_QpeProdReady=true;
                        QFile::remove(GenQpeProd_Endflag);
                        QThread::msleep(100);
                        qstTimeSequenceIni.setValue("QPEProd",m_iTaskNumber);
                    }
                }
                m_bReady = (m_MrefDerivProdReady  && m_TrecProdReady && m_QpeProdReady);
                m_iNotFindCnt++;
                if(m_iNotFindCnt>100) ///等待后续模块 时间上限：100(次)*3s
                {
                    m_bReady=true;
                    qstTimeSequenceIni.setValue("QPEProd",m_iTaskNumber);
                    qstTimeSequenceIni.setValue("TrecProd",m_iTaskNumber);
                    qstTimeSequenceIni.setValue("MrefDerivProd",m_iTaskNumber);
                }
            }
            else
            {
                m_iReadyCnt++;
                m_iNotFindCnt=0;
            }
            break;
        }
        //        timerWork->start();
        m_iCntTimer=3;//再次从3开始，倒数3秒检测
        m_iMatchCnt++;//匹配次数
    }
    else//m_iCntTimer>=0
    {
        QString stText;
        if(m_bReady && m_iWorkPeriod==3)
        {
           m_iCntTimer=1;//倒数1s 退出
           stText ="已经匹配成功! "+QString("%1").arg(m_iCntTimer)+" 秒后退出......";
            if(m_iReadyCnt>=1){
                m_bReady=false;
                SetEndFlag();
                exit(0);
            }
        }else
        {
            stText =QString("尝试匹配次数[%1]").arg(m_iMatchCnt)+QString(" %1 秒后再次匹配......").arg(m_iCntTimer)+QString(" [当前任务：Work%1]").arg(m_iWorkPeriod);
        }
        //        ui->listWidgetInfo->addItem(new QListWidgetItem(stText));
        ui->lineEdit->setText(stText);
        m_iCntTimer--;
    }
}

void MainWindow::InvokeRealMosaic()
{
    QDateTime currentMyDateTime=QDateTime::currentDateTime();

    QString strNowTime=currentMyDateTime.toString("yyyy MM dd hh mm ss");

    //           QString sFileName= m_runPath+"/RealMosaic.exe "+strDateTime[0]+" "+strDateTime[1]+" "+strDateTime[2]+" "+strDateTime[3]+" "+strDateTime[4]+" "+strDateTime[5];
    QString sFileName;
    if(m_strMidDateTime.length()==14)
    {
        sFileName="./RealMosaic "+m_strMidDateTime.mid(0,4)+" "+m_strMidDateTime.mid(4,2)+" "+m_strMidDateTime.mid(6,2)+" "+m_strMidDateTime.mid(8,2)+" "+m_strMidDateTime.mid(10,2)+" "+m_strMidDateTime.mid(12,2);
//         sFileName= "\""+m_runPath+"/RealMosaic "+"\""+m_strMidDateTime.mid(0,4)+" "+m_strMidDateTime.mid(4,2)+" "+m_strMidDateTime.mid(6,2)+" "+m_strMidDateTime.mid(8,2)+" "+m_strMidDateTime.mid(10,2)+" "+m_strMidDateTime.mid(12,2);
    }else
    {
        sFileName= "\""+m_runPath+"/RealMosaic "+"\""+strNowTime;
    }
    //    ui->listWidgetInfo->addItem(new QListWidgetItem(sFileName));
    startProcess(sFileName);
    ui->listWidgetInfo->addItem(new QListWidgetItem("启动RealMosaic "+sFileName));

    QDir myDir;
    sFileName= m_runPath+"/Temp/QC_QPE_FenXi_Rslt_"+m_strMidDateTime+".txt";
    myDir.remove(sFileName);


}

bool MainWindow::CheckAllFile()
{
    bool bRet=true;
    QString strfileName=m_runPath+"/Temp/BaseDataQC_rslt_FenXi_"+m_strMidDateTime+".txt";
    QString strWorkDoneContent=readFile(strfileName);

    QList<QString>strWorkDoneFileName  = strWorkDoneContent.split("\r\n");
    /*    for(int i=0;i<strWorkDoneFileName.size();i++)
    {
//        ui->listWidgetInfo->addItem(new QListWidgetItem(strWorkDoneFileName[i]));
        for(int j=0;j<)
    }
*/
    for(int i=0;i<m_PreparWorkFileName.size();i++)
    {
        //       ui->listWidgetInfo->addItem(new QListWidgetItem("****************8"));
        //       ui->listWidgetInfo->addItem(new QListWidgetItem(m_PreparWorkFileName[i]));
        QString strTemp=m_PreparWorkFileName[i].left(m_PreparWorkFileName[i].length()-2);
        if(strTemp.length()<=0)
        {
            continue;
            // break;
        }
        bool bFindFlag=false;
        for(int j=0;j<strWorkDoneFileName.size();j++)
        {
            //      ui->listWidgetInfo->addItem(new QListWidgetItem(m_PreparWorkFileName[i]));
            //           ui->listWidgetInfo->addItem(new QListWidgetItem(strWorkDoneFileName[j]));

            int iPos=strWorkDoneFileName[j].indexOf(strTemp);
            //          int iPos=strWorkDoneFileName[j].indexOf("Z_RADR_I_Z9200_20140925103800_O_DOR_SA_CAP.bin.bz2");
            if(iPos>0)
            {
                bFindFlag=true;
                break;
            }
            //            ui->listWidgetInfo->addItem(new QListWidgetItem("-----------------------"));

        }

        if(bFindFlag==false)
        {
            bRet=false;
            break;
        }
    }

    return bRet;

}

//读文件函数
QString MainWindow::readFile(QString fileName)
{
    QString fileContent;
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
            QMessageBox::critical(this, tr("Error"), m_AppName+tr("无法打开文件")+fileName);
            return "";
        }
        QTextStream in(&Endfile);
        fileContent=in.readAll();
        Endfile.close();
    }
    return fileContent;
}

//开启进程函数
void MainWindow::startProcess(QString sFileName)
{
    QProcess *myProcess = new QProcess();
    myProcess->start(sFileName);
    //delete myProcess;//不能delete! delete会导致指向的进程被关闭
}

//操作文件
bool MainWindow::writeFile(QString fileName)
{
    QFile contentfile(fileName);
    contentfile.open(QIODevice::WriteOnly|QIODevice::Append);
    contentfile.close();
    return true;
}

void MainWindow::SetEndFlag(void)
{
    int iCnt=0;
    while(QFile::exists(m_runPath+"/Temp/QC_QPE_FenXi_Mosaic_Contrl_EndFlag.txt") && iCnt<10)//为了防止覆盖上一个Mosaic写出来的EndFlag
    {
        QThread::msleep(500);
        iCnt++;
    }
    writeFile(QString(m_runPath+"/Temp/QC_QPE_FenXi_Mosaic_Contrl_EndFlag.txt"));
    iCnt=0;
    while(! QFile::exists(m_runPath+"/Temp/QC_QPE_FenXi_Mosaic_Contrl_EndFlag.txt")  && iCnt<20)
    {
        QThread::sleep(100);
        iCnt++;
    }
//      QThread::msleep(100);
}
