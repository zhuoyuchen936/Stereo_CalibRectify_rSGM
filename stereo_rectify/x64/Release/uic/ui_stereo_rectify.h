/********************************************************************************
** Form generated from reading UI file 'stereo_rectify.ui'
**
** Created by: Qt User Interface Compiler version 5.12.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_STEREO_RECTIFY_H
#define UI_STEREO_RECTIFY_H

#include <QtCore/QVariant>
#include <QtGui/QIcon>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_stereo_rectifyClass
{
public:
    QWidget *centralWidget;
    QPushButton *Calib_Image;
    QPushButton *calibration;
    QPushButton *import_paramter;
    QPushButton *rectification;
    QPushButton *Test_Image;
    QProgressBar *progressBar;
    QLabel *image_L;
    QTextBrowser *textBrowser;
    QLabel *image_R;
    QTextBrowser *Param_Browser;
    QPushButton *show_rectify;
    QMenuBar *menuBar;
    QMenu *menu;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *stereo_rectifyClass)
    {
        if (stereo_rectifyClass->objectName().isEmpty())
            stereo_rectifyClass->setObjectName(QString::fromUtf8("stereo_rectifyClass"));
        stereo_rectifyClass->resize(835, 429);
        centralWidget = new QWidget(stereo_rectifyClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        Calib_Image = new QPushButton(centralWidget);
        Calib_Image->setObjectName(QString::fromUtf8("Calib_Image"));
        Calib_Image->setGeometry(QRect(10, 10, 121, 31));
        QIcon icon;
        icon.addFile(QString::fromUtf8("G:/Photo/f73f - \345\211\257\346\234\254(1).ico"), QSize(), QIcon::Normal, QIcon::Off);
        Calib_Image->setIcon(icon);
        calibration = new QPushButton(centralWidget);
        calibration->setObjectName(QString::fromUtf8("calibration"));
        calibration->setGeometry(QRect(10, 40, 61, 31));
        import_paramter = new QPushButton(centralWidget);
        import_paramter->setObjectName(QString::fromUtf8("import_paramter"));
        import_paramter->setGeometry(QRect(70, 40, 61, 31));
        rectification = new QPushButton(centralWidget);
        rectification->setObjectName(QString::fromUtf8("rectification"));
        rectification->setGeometry(QRect(10, 100, 121, 31));
        Test_Image = new QPushButton(centralWidget);
        Test_Image->setObjectName(QString::fromUtf8("Test_Image"));
        Test_Image->setGeometry(QRect(10, 70, 121, 31));
        Test_Image->setIcon(icon);
        progressBar = new QProgressBar(centralWidget);
        progressBar->setObjectName(QString::fromUtf8("progressBar"));
        progressBar->setGeometry(QRect(20, 140, 291, 16));
        progressBar->setValue(24);
        image_L = new QLabel(centralWidget);
        image_L->setObjectName(QString::fromUtf8("image_L"));
        image_L->setGeometry(QRect(20, 170, 384, 216));
        textBrowser = new QTextBrowser(centralWidget);
        textBrowser->setObjectName(QString::fromUtf8("textBrowser"));
        textBrowser->setGeometry(QRect(140, 10, 241, 121));
        image_R = new QLabel(centralWidget);
        image_R->setObjectName(QString::fromUtf8("image_R"));
        image_R->setGeometry(QRect(430, 170, 384, 216));
        Param_Browser = new QTextBrowser(centralWidget);
        Param_Browser->setObjectName(QString::fromUtf8("Param_Browser"));
        Param_Browser->setGeometry(QRect(390, 10, 411, 151));
        show_rectify = new QPushButton(centralWidget);
        show_rectify->setObjectName(QString::fromUtf8("show_rectify"));
        show_rectify->setGeometry(QRect(320, 140, 61, 21));
        stereo_rectifyClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(stereo_rectifyClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 835, 18));
        menu = new QMenu(menuBar);
        menu->setObjectName(QString::fromUtf8("menu"));
        stereo_rectifyClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(stereo_rectifyClass);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        stereo_rectifyClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(stereo_rectifyClass);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        stereo_rectifyClass->setStatusBar(statusBar);

        menuBar->addAction(menu->menuAction());
        menu->addSeparator();

        retranslateUi(stereo_rectifyClass);

        QMetaObject::connectSlotsByName(stereo_rectifyClass);
    } // setupUi

    void retranslateUi(QMainWindow *stereo_rectifyClass)
    {
        stereo_rectifyClass->setWindowTitle(QApplication::translate("stereo_rectifyClass", "stereo_rectify", nullptr));
        Calib_Image->setText(QApplication::translate("stereo_rectifyClass", "\351\200\211\346\213\251\346\240\207\345\256\232\345\233\276\345\203\217\346\226\207\344\273\266\345\244\271", nullptr));
        calibration->setText(QApplication::translate("stereo_rectifyClass", "\345\274\200\345\247\213\346\240\207\345\256\232", nullptr));
        import_paramter->setText(QApplication::translate("stereo_rectifyClass", "\345\257\274\345\205\245\346\240\207\345\256\232\345\217\202\346\225\260", nullptr));
        rectification->setText(QApplication::translate("stereo_rectifyClass", "\345\274\200\345\247\213\347\237\253\346\255\243", nullptr));
        Test_Image->setText(QApplication::translate("stereo_rectifyClass", "\351\200\211\346\213\251\346\234\252\347\237\253\346\255\243\345\233\276\345\203\217\346\226\207\344\273\266\345\244\271", nullptr));
        image_L->setText(QApplication::translate("stereo_rectifyClass", "TextLabel", nullptr));
        image_R->setText(QApplication::translate("stereo_rectifyClass", "TextLabel", nullptr));
        show_rectify->setText(QApplication::translate("stereo_rectifyClass", "\345\261\225\347\244\272\347\237\253\346\255\243", nullptr));
        menu->setTitle(QApplication::translate("stereo_rectifyClass", "\346\240\207\345\256\232+\347\237\253\346\255\243", nullptr));
    } // retranslateUi

};

namespace Ui {
    class stereo_rectifyClass: public Ui_stereo_rectifyClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_STEREO_RECTIFY_H
