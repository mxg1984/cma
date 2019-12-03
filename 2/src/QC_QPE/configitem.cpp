#include "configitem.h"
#include <QCoreApplication>
#include <QFile>
#include <QtCore/QCoreApplication>
#include <QSettings>


ConfigItem::ConfigItem(QObject *parent = 0):QObject(parent)
{
    ConfigName="";
    ConfigValue="";
    DefaultValue="0";
    GroupName="";
}
ConfigItem::~ConfigItem(){

}

QList<ConfigItem> ConfigItem::loadConfig(QString fileName){

    QList<ConfigItem> list;
     QString tmpValue="";
       QSettings *configIniRead = new QSettings(fileName, QSettings::IniFormat);
    //从文件中加载配置项，
     if(list.length()<1){
         ConfigItem  *item;
         ConfigItem item1;
         list<<item1;
         item=&item1;
         item->GroupName="RunStatus";
         item->ConfigName="Use Last Status";
         tmpValue="/"+item->GroupName+"/"+item->ConfigName;
         item->DefaultValue="0";
         item->ConfigValue=configIniRead->value(tmpValue).toString();
         if(item->ConfigValue.length()<1){
           item->ConfigValue=item->DefaultValue;
         }
         ConfigItem item2;
         list<<item2;
         item=&item2;
         item = new ConfigItem;
          item->GroupName="RunStatus";
          item->ConfigName="Last Status";
         tmpValue="/"+item->GroupName+"/"+item->ConfigName;
          item->DefaultValue="1";
          item->ConfigValue=configIniRead->value(tmpValue).toString();
          if(item->ConfigValue.length()<1){
            item->ConfigValue=item->DefaultValue;
          }

     }
      delete configIniRead;
     return list;
}

 void ConfigItem::saveConfig(QString fileName,QList<ConfigItem> list){

     QString tmpGroupName="";
     //Qt中使用QSettings类读写ini文件
        //QSettings构造函数的第一个参数是ini文件的路径,第二个参数表示针对ini文件,第三个参数可以缺省
        QSettings *configIniWrite = new QSettings(fileName, QSettings::IniFormat);

     for(int ii=0;ii<list.length();ii++){
         tmpGroupName="";
         tmpGroupName="/"+list[ii].GroupName+"/"+list[ii].ConfigName;
         if(list[ii].ConfigValue.length()<1){
              //向ini文件的第一个节写入内容,ip节下的第一个参数
          configIniWrite->setValue(tmpGroupName,list[ii].DefaultValue);
         }
         else{
           configIniWrite->setValue(tmpGroupName,list[ii].ConfigValue);
         }
     }
      delete configIniWrite;
 }

