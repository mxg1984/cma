#ifndef QCONFIGDATA_H
#define QCONFIGDATA_H

#include <QObject>

struct QConfigData{
    QString GroupName;
    QString ConfigKey;
    QString ConfigValue;
    QString DefaultValue;
    QConfigData* Children;
};
struct uiElement{
    QString ParentContainID;
    QString CompentID;
    int Visiable;
    int Enable;
    QString CompentValue;
    QString CompentText;
    QString Remark;
    uiElement* Children;
};

struct fileInfo{
    QString fileName;
    QString createDate;
    QString updateDate;
    QString filePath;
};


//摘抄的结构体
struct _BASEDATAINFO{
    //	char strDataType[10];	//雷达数据类型
    char  strFilePath[256];		//文件路径及名称
    long  lJulDate;				//儒略日
    long  lSeconds;				//观测开始时间
    int  iSiteID;				//雷达站在本区域组网中的标号
    //	int  iIndexInList;			//文件序号
    //	unsigned __int64 ulDataTime;//
};

#endif

