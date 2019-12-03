#ifndef INPUTDIALOG_H
#define INPUTDIALOG_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QDialog>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QSettings>
#include <QMessageBox>

class inputDialog : public QDialog
{
    Q_OBJECT
public:
    explicit inputDialog(QWidget *parent = 0);


signals:
    void sendData(QString data);
public slots:

private slots:
    void SaveData();
    QString ColumnParse(int coulumnID);
    void receiveMainWindowData(QString);
    void LoadView(QString);
    void CancelData();
    //文件操作
    void writeFile(QString fileName,QString content,bool isAppend);
    QString readFile(QString fileName);
    void chooseDir();
private:
    QLabel *lblStationNum;
    QLineEdit *txtStationNum;
    QLabel *lblStationNumTip;
    QLabel *lblStationName;
    QLineEdit *txtStationName;
    QLabel *lblLatNum;
    QLineEdit *txtLatNum;
    QLabel *lblLatNumTip;
    QLabel *lblLngNum;
    QLineEdit *txtLngNum;
    QLabel *lblLngNumTip;
    QLabel *lblHeightNum;
    QLineEdit *txtHeightNum;
    QLabel *lblHeightNumTip;
    QLabel *lblLeiDaType;
    QLineEdit *txtLeiDaType;
    QLabel *lblFanWei;
    QLineEdit *txtFanWei;
    QLabel *lblFanWeiTip;
    QLabel *lblDataPath;
    QLineEdit *txtDataPath;
    QPushButton *btnSure;
    QPushButton *btnCancel;
    QGridLayout *centorLayout;
    QGridLayout *mainLayout;
    QHBoxLayout *queryLayout;
    QPushButton *dirButton;
};


#endif // INPUTDIALOG_H
