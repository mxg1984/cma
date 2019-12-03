#ifndef CONFIGITEM_H
#define CONFIGITEM_H

#include <QObject>

class ConfigItem : public QObject
{
    Q_OBJECT
public:
    explicit ConfigItem(QObject *parent = 0);
    ~ConfigItem();
signals:

public slots:
   QString ConfigName;
   QString ConfigValue;
   QString DefaultValue;
   QString GroupName;
   QList<ConfigItem> loadConfig(QString fileName);
   void saveConfig(QString fileName,QList<ConfigItem> list);
};

#endif // CONFIGITEM_H
