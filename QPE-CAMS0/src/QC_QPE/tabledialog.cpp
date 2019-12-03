#include "tabledialog.h"
#include "ui_tabledialog.h"
#include <QFile>
#include <QMessageBox>
#include <QTextStream>


const static QString sRadarFileName="Radar.ini";
const static QString sRadarListFileName="RadarList.ini";

tableDialog::tableDialog(QWidget *parent) :QDialog(parent),ui(new Ui::tableDialog)
{
    ui->setupUi(this);
    loadTableContent();
}

tableDialog::~tableDialog()
{
    delete ui;
}

//业务
void tableDialog::loadTableContent()
{
    QStringList header;
    header<<"站号"<<"站名"<<"经度（度）"<<"纬度（度）"<<"海拔高度（米）"<<"雷达型号"<<"范围（千米）"<<"数据路径";
    ui->tableWidget->setColumnCount(8);
    ui->tableWidget->setShowGrid(false);
    ui->tableWidget->setColumnWidth(0,50);
    ui->tableWidget->setHorizontalHeaderLabels(header);
    ui->tableWidget->setRowCount(0);


    QString tableContent=readFile(sRadarListFileName);//读取雷达列表
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
                QStringList tableRowContentListBySpace=tableRowContentListByEqual[1].simplified().split(" ");
                for(int j=0;j<tableRowContentListBySpace.count();j++)
                {
                    ui->tableWidget->setItem(nRowNum,j+1,new QTableWidgetItem(tableRowContentListBySpace[j].trimmed()));
                }
            }
        }
    }
    ui->tableWidget->show();
}

//点击“重建”出来的对话框里的“确定”按钮
void tableDialog::on_btnSure_clicked()
{
    //读取所有的已经显示的数据的列头
    //获取所有选择列头，如果列头没有出现在上述列表中，将这条数据加入，如果存在，不做任何处理
    //关闭界面

    QItemSelectionModel *selections = ui->tableWidget->selectionModel();

    QList<int> listRow;
    QModelIndexList indexes = selections->selectedIndexes();
    QModelIndex index;
    foreach(index, indexes)
    {
        if(!listRow.contains(index.row()))
        {
            listRow.append(index.row());
        }
    }
    //QList<QTableWidgetItem*> items=ui->tableWidget->selectedItems();

    int rowCount=selections->selectedRows().count();

    QString tableStr="";

    for(int i=0;i<rowCount;i++)
    {
        int tmpRowNum=listRow.at(i);
        for(int j=0;j<ui->tableWidget->columnCount();j++)
        {
            //第一列特殊处理
            if(j==0)
            {
                if(ui->tableWidget->item(tmpRowNum,j)==NULL||(ui->tableWidget->item(tmpRowNum,j)&&ui->tableWidget->item(tmpRowNum,j)->text()==tr("")))
                {
                    tableStr.append("=");
                }
                else
                {
                   tableStr.append(ui->tableWidget->item(tmpRowNum,j)->text()+"=");
                }
            }
            else
            {
                if(ui->tableWidget->item(tmpRowNum,j)==NULL||(ui->tableWidget->item(tmpRowNum,j)&&ui->tableWidget->item(tmpRowNum,j)->text()==tr("")))
                {
                     tableStr.append("    ");
                }
                else
                {
                     tableStr.append("    "+ui->tableWidget->item(tmpRowNum,j)->text());
                }
            }
        }
        tableStr.append("\r\n");
    }
    emit sendData(tableStr);    //发射信息，并传递参数
    this->close();
}

void tableDialog::on_btnCancle_clicked()
{
    this->close();
}

//操作文件
void tableDialog::writeFile(QString fileName,QString content,bool isAppend)
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

QString tableDialog::readFile(QString fileName)
{
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
            QMessageBox::critical(this, tr("Error"), tr("无法打开文件"));
            return "";
        }
        QTextStream in(&Endfile);
        QString fileContent=in.readAll();
        Endfile.close();
        return fileContent;
    }
}
