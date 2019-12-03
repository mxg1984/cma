#ifndef TABLEDIALOG_H
#define TABLEDIALOG_H

#include <QDialog>
#include <QTableWidget>

namespace Ui {
class tableDialog;
}

class tableDialog : public QDialog
{
    Q_OBJECT

signals:
    void sendData(QString data);

public:
    explicit tableDialog(QWidget *parent = 0);
    ~tableDialog();

private slots:
    void loadTableContent();
    void writeFile(QString fileName,QString content,bool isAppend);
    QString readFile(QString fileName);

    void on_btnSure_clicked();

    void on_btnCancle_clicked();

private:
    Ui::tableDialog *ui;
};



#endif // TABLEDIALOG_H
