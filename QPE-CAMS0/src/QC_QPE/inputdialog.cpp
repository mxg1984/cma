#include "inputdialog.h"
#include <QFile>
#include <QDebug>
#include <QStringList>
#include <QTextStream>
#include <QFileDialog>
#include <QCoreApplication>

const static QString sRadarFileName="Radar.ini";
const static QString sRadarListFileName="RadarList.ini";
const static QString sReplaceOperate="!@#$%";
const static QString sTemporaryRadarFileName="TemporaryRadarList.ini";
QString sViewMode="Insert";

inputDialog::inputDialog(QWidget *parent) :
    QDialog(parent)
{
    setWindowTitle(tr("雷达站参数"));

    //站号
    lblStationNum =new QLabel;
    lblStationNum->setText(tr("站号："));
    txtStationNum =new QLineEdit;
    lblStationNumTip=new QLabel;
    lblStationNumTip->setText(tr("(1,2,3,...)"));

    //站名
    lblStationName=new QLabel;
    lblStationName->setText(tr("站名："));
    txtStationName =new QLineEdit;

    //经度
    lblLatNum=new QLabel;
    lblLatNum->setText(tr("经度："));
    txtLatNum =new QLineEdit;
    lblLatNumTip=new QLabel;
    lblLatNumTip->setText(tr("度"));

    //纬度
    lblLngNum=new QLabel;
    lblLngNum->setText(tr("纬度："));
    txtLngNum =new QLineEdit;
    lblLngNumTip=new QLabel;
    lblLngNumTip->setText(tr("度"));

    //海拔高度
    lblHeightNum=new QLabel;
    lblHeightNum->setText(tr("海拔高度："));
    txtHeightNum =new QLineEdit;
    lblHeightNumTip=new QLabel;
    lblHeightNumTip->setText(tr("米"));

    //雷达类型
    lblLeiDaType=new QLabel;
    lblLeiDaType->setText(tr("雷达类型："));
    txtLeiDaType =new QLineEdit;

    //观测范围
    lblFanWei=new QLabel;
    lblFanWei->setText(tr("观测范围："));
    txtFanWei =new QLineEdit;
    lblFanWeiTip=new QLabel;
    lblFanWeiTip->setText(tr("千米"));

    //数据路径
    lblDataPath=new QLabel;
    lblDataPath->setText(tr("数据路径："));
    txtDataPath =new QLineEdit;
    dirButton = new QPushButton("&选择", this);

    centorLayout =new QGridLayout();
    centorLayout->addWidget(lblStationNum,0,0);
    centorLayout->addWidget(txtStationNum,0,1);
    centorLayout->addWidget(lblStationNumTip,0,2);
    centorLayout->addWidget(lblStationName,1,0);
    centorLayout->addWidget(txtStationName,1,1);
    centorLayout->addWidget(lblLatNum,2,0);
    centorLayout->addWidget(txtLatNum,2,1);
    centorLayout->addWidget(lblLatNumTip,2,2);
    centorLayout->addWidget(lblLngNum,3,0);
    centorLayout->addWidget(txtLngNum,3,1);
    centorLayout->addWidget(lblLngNumTip,3,2);
    centorLayout->addWidget(lblHeightNum,4,0);
    centorLayout->addWidget(txtHeightNum,4,1);
    centorLayout->addWidget(lblHeightNumTip,4,2);
    centorLayout->addWidget(lblLeiDaType,5,0);
    centorLayout->addWidget(txtLeiDaType,5,1);
    centorLayout->addWidget(lblFanWei,6,0);
    centorLayout->addWidget(txtFanWei,6,1);
    centorLayout->addWidget(lblFanWeiTip,6,2);
    centorLayout->addWidget(lblDataPath,7,0);
    centorLayout->addWidget(txtDataPath,7,1);
    centorLayout->addWidget(dirButton,7,2);
    centorLayout->setMargin(15);
    centorLayout->setSpacing(10);

    btnSure =new QPushButton;
    btnSure->setText(tr("确定"));
    btnCancel =new QPushButton;
    btnCancel->setText(tr("取消"));

    queryLayout = new QHBoxLayout();
    queryLayout->addWidget(btnSure);
    queryLayout->addWidget(btnCancel);

    mainLayout=new QGridLayout(this);
    mainLayout->addLayout(centorLayout,0,0);
    mainLayout->addLayout(queryLayout,1,0);

    connect(btnSure,SIGNAL(clicked()),this,SLOT(SaveData()));
    connect(btnCancel,SIGNAL(clicked()),this,SLOT(CancelData()));
    connect(dirButton,SIGNAL(clicked()),this,SLOT(chooseDir()));
}

/*********************************************************选择雷达站数据路径********************************************************/
void inputDialog::chooseDir()
{
    QString s = QFileDialog::getExistingDirectory(this,"浏览",QDir::currentPath());

    if(s == NULL)
    {
        txtDataPath->setText(txtDataPath->text());
    }
    else
    {
        txtDataPath->setText(s);
    }
}

QString inputDialog::ColumnParse(int coulumnID)
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

void inputDialog::SaveData()
{
    QString showContent="";
    if(sViewMode=="Insert")     //添加
    {
        //showContent=readFile(sRadarFileName);
        QString m_strRunPath = QCoreApplication::applicationDirPath();
        qDebug()<<m_strRunPath;
        showContent=readFile(m_strRunPath+"/Temp/"+sTemporaryRadarFileName);
        showContent.append("\r\n"+txtStationNum->text()+"=")
                   .append("    "+txtStationName->text())
                   .append("    "+txtLatNum->text())
                   .append("    "+txtLngNum->text())
                   .append("    "+txtHeightNum->text())
                   .append("    "+txtLeiDaType->text())
                   .append("    "+txtFanWei->text())
                   .append("    "+txtDataPath->text());
    }
    else        //修改
    {
       // showContent=readFile(sRadarFileName);
        QString m_strRunPath = QCoreApplication::applicationDirPath();
        qDebug()<<m_strRunPath;
        showContent=readFile(m_strRunPath+"/Temp/"+sTemporaryRadarFileName);

        QStringList strDataList=showContent.split("\r\n");
        for(int i=0;i<strDataList.count();i++)
        {
            if(!strDataList[i].startsWith("#"))
            {
                QString tmpString=strDataList[i].split("=")[0].trimmed();
                if(txtStationNum->text().trimmed()==tmpString)
                {
                    //第一列特殊处理
                    QString rowContent="";
                    rowContent.append(txtStationNum->text()+"=");
                    rowContent.append("    "+txtStationName->text());
                    rowContent.append("    "+txtLatNum->text());
                    rowContent.append("    "+txtLngNum->text());
                    rowContent.append("    "+txtHeightNum->text());
                    rowContent.append("    "+txtLeiDaType->text());
                    rowContent.append("    "+txtFanWei->text());
                    rowContent.append("    "+txtDataPath->text());
                    showContent=showContent.replace(strDataList[i],rowContent);
                    break;
                }
            }
        }
    }
    emit sendData(showContent);
    this->close();
}

void inputDialog::CancelData()
{
    this->close();
}

void inputDialog::receiveMainWindowData(QString data)
{
    //QMessageBox::information(this,tr("大区域天气雷达数据QC及组网QPE系统"),data);
    LoadView(data);
}

//加载界面
void inputDialog::LoadView(QString data)
{
    QStringList strDataList=data.split(sReplaceOperate);
    sViewMode="Edit";
    if(strDataList.length()>0)
    {
        txtStationNum->setText(strDataList[0]);
    }
    if(strDataList.length()>1)
    {
        txtStationName->setText(strDataList[1]);
    }
    if(strDataList.length()>2)
    {
        txtLatNum->setText(strDataList[2]);
    }
    if(strDataList.length()>3)
    {
        txtLngNum->setText(strDataList[3]);
    }
    if(strDataList.length()>4)
    {
        txtHeightNum->setText(strDataList[4]);
    }
    if(strDataList.length()>5)
    {
        txtLeiDaType->setText(strDataList[5]);
    }
    if(strDataList.length()>6)
    {
        txtFanWei->setText(strDataList[6]);
    }
    if(strDataList.length()>7)
    {
        txtDataPath->setText(strDataList[7]);
    }
}

//加载文件

//操作文件
void inputDialog::writeFile(QString fileName,QString content,bool isAppend)
{
    QFile contentfile(fileName);

    if(isAppend==true)
    {
        if(!contentfile.open(QIODevice::WriteOnly|QIODevice::Append))
        {
            QMessageBox::critical(this, tr("Error"), tr("无法打开文件"));
            return;
        }
    }
    else
    {
        if(!contentfile.open(QIODevice::WriteOnly | QIODevice::Text))
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

QString inputDialog::readFile(QString fileName)
{
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (!file.open(QIODevice::ReadOnly)) {
            QMessageBox::critical(this, tr("Error"), tr("无法打开文件"));
            return "";
        }
        QTextStream in(&file);
        QString fileContent=in.readAll();
        file.close();
        return fileContent;
    }
}

