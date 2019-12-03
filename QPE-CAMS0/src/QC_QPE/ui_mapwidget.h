/********************************************************************************
** Form generated from reading UI file 'mapwidget.ui'
**
** Created by: Qt User Interface Compiler version 5.2.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAPWIDGET_H
#define UI_MAPWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGraphicsView>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_mapwidget
{
public:
    QGraphicsView *graphicsView;
    QPushButton *pushButtonOK;
    QPushButton *pushButtonCancel;
    QTextEdit *textEdit;
    QWidget *widget;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLineEdit *lineEditLon;
    QLabel *label_2;
    QLineEdit *lineEditLat;

    void setupUi(QDialog *mapwidget)
    {
        if (mapwidget->objectName().isEmpty())
            mapwidget->setObjectName(QStringLiteral("mapwidget"));
        mapwidget->resize(666, 518);
        graphicsView = new QGraphicsView(mapwidget);
        graphicsView->setObjectName(QStringLiteral("graphicsView"));
        graphicsView->setGeometry(QRect(10, 10, 531, 431));
        pushButtonOK = new QPushButton(mapwidget);
        pushButtonOK->setObjectName(QStringLiteral("pushButtonOK"));
        pushButtonOK->setGeometry(QRect(440, 470, 75, 23));
        pushButtonCancel = new QPushButton(mapwidget);
        pushButtonCancel->setObjectName(QStringLiteral("pushButtonCancel"));
        pushButtonCancel->setGeometry(QRect(540, 470, 75, 23));
        textEdit = new QTextEdit(mapwidget);
        textEdit->setObjectName(QStringLiteral("textEdit"));
        textEdit->setGeometry(QRect(550, 120, 104, 171));
        widget = new QWidget(mapwidget);
        widget->setObjectName(QStringLiteral("widget"));
        widget->setGeometry(QRect(60, 465, 211, 22));
        horizontalLayout = new QHBoxLayout(widget);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        horizontalLayout->setContentsMargins(0, 0, 0, 0);
        label = new QLabel(widget);
        label->setObjectName(QStringLiteral("label"));
        label->setScaledContents(false);

        horizontalLayout->addWidget(label);

        lineEditLon = new QLineEdit(widget);
        lineEditLon->setObjectName(QStringLiteral("lineEditLon"));

        horizontalLayout->addWidget(lineEditLon);

        label_2 = new QLabel(widget);
        label_2->setObjectName(QStringLiteral("label_2"));

        horizontalLayout->addWidget(label_2);

        lineEditLat = new QLineEdit(widget);
        lineEditLat->setObjectName(QStringLiteral("lineEditLat"));

        horizontalLayout->addWidget(lineEditLat);


        retranslateUi(mapwidget);

        QMetaObject::connectSlotsByName(mapwidget);
    } // setupUi

    void retranslateUi(QDialog *mapwidget)
    {
        mapwidget->setWindowTitle(QApplication::translate("mapwidget", "Dialog", 0));
        pushButtonOK->setText(QApplication::translate("mapwidget", "OK", 0));
        pushButtonCancel->setText(QApplication::translate("mapwidget", "Cancel", 0));
        label->setText(QApplication::translate("mapwidget", "\347\273\217\345\272\246\357\274\232", 0));
        label_2->setText(QApplication::translate("mapwidget", "\347\272\254\345\272\246\357\274\232", 0));
    } // retranslateUi

};

namespace Ui {
    class mapwidget: public Ui_mapwidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAPWIDGET_H
