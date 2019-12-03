/********************************************************************************
** Form generated from reading UI file 'tabledialog.ui'
**
** Created by: Qt User Interface Compiler version 5.2.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_TABLEDIALOG_H
#define UI_TABLEDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableWidget>

QT_BEGIN_NAMESPACE

class Ui_tableDialog
{
public:
    QTableWidget *tableWidget;
    QPushButton *btnSure;
    QPushButton *btnCancle;

    void setupUi(QDialog *tableDialog)
    {
        if (tableDialog->objectName().isEmpty())
            tableDialog->setObjectName(QStringLiteral("tableDialog"));
        tableDialog->resize(400, 300);
        tableWidget = new QTableWidget(tableDialog);
        tableWidget->setObjectName(QStringLiteral("tableWidget"));
        tableWidget->setGeometry(QRect(10, 10, 381, 241));
        tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tableWidget->setSelectionMode(QAbstractItemView::MultiSelection);
        tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
        btnSure = new QPushButton(tableDialog);
        btnSure->setObjectName(QStringLiteral("btnSure"));
        btnSure->setGeometry(QRect(200, 260, 75, 23));
        btnCancle = new QPushButton(tableDialog);
        btnCancle->setObjectName(QStringLiteral("btnCancle"));
        btnCancle->setGeometry(QRect(300, 260, 75, 23));

        retranslateUi(tableDialog);

        QMetaObject::connectSlotsByName(tableDialog);
    } // setupUi

    void retranslateUi(QDialog *tableDialog)
    {
        tableDialog->setWindowTitle(QApplication::translate("tableDialog", "\351\233\267\350\276\276\347\253\231\351\200\211\346\213\251", 0));
        btnSure->setText(QApplication::translate("tableDialog", "\347\241\256\345\256\232", 0));
        btnCancle->setText(QApplication::translate("tableDialog", "\345\217\226\346\266\210", 0));
    } // retranslateUi

};

namespace Ui {
    class tableDialog: public Ui_tableDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_TABLEDIALOG_H
