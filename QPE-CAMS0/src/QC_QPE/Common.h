#ifndef COMMON_H
#define COMMON_H
#endif // COMMON_H


#include <QObject>
#include <QFile>
#include <QFileInfo>
#include "qconfigdata.h"

static fileInfo getFileInfo(QString filePath)
{
    QFileInfo aQFileInfo(filePath);

    fileInfo aFileInfo;
    if(aQFileInfo.exists())
    {
        aFileInfo.fileName=aQFileInfo.fileName();
        aFileInfo.createDate=aQFileInfo.created().toString("yyyy年MM月dd日ddddhh:mm:ss");
        aFileInfo.updateDate=aQFileInfo.lastModified().toString("yyyy年MM月dd日ddddhh:mm:ss");
    }
    else
    {
        aFileInfo.fileName="abc";
        aFileInfo.createDate="2014-03-03";
        aFileInfo.updateDate="2014-08-08";
    }
    return aFileInfo;
}


