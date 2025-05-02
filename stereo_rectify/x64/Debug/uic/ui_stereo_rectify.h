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
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
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
    QAction *actiond;
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
    QPushButton *Crop;
    QPushButton *computeRange;
    QPushButton *disLine;
    QTextBrowser *Param_Browser_2;
    QFrame *line1;
    QFrame *line2;
    QFrame *line3;
    QFrame *line4;
    QLineEdit *pSizeW;
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;
    QLineEdit *pSizeH;
    QLabel *label_4;
    QLabel *label_5;
    QLineEdit *sqSize;
    QLabel *label_6;
    QPushButton *Confirm_chess;
    QPushButton *stereo;
    QPushButton *next_img;
    QPushButton *test1;
    QPushButton *CloudCompute;
    QMenuBar *menuBar;
    QMenu *menu;
    QMenu *menustereo_disparity;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *stereo_rectifyClass)
    {
        if (stereo_rectifyClass->objectName().isEmpty())
            stereo_rectifyClass->setObjectName(QString::fromUtf8("stereo_rectifyClass"));
        stereo_rectifyClass->resize(1040, 506);
        actiond = new QAction(stereo_rectifyClass);
        actiond->setObjectName(QString::fromUtf8("actiond"));
        centralWidget = new QWidget(stereo_rectifyClass);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        Calib_Image = new QPushButton(centralWidget);
        Calib_Image->setObjectName(QString::fromUtf8("Calib_Image"));
        Calib_Image->setGeometry(QRect(10, 20, 121, 31));
        QIcon icon;
        icon.addFile(QString::fromUtf8("G:/Photo/f73f - \345\211\257\346\234\254(1).ico"), QSize(), QIcon::Normal, QIcon::Off);
        Calib_Image->setIcon(icon);
        calibration = new QPushButton(centralWidget);
        calibration->setObjectName(QString::fromUtf8("calibration"));
        calibration->setGeometry(QRect(10, 50, 61, 31));
        import_paramter = new QPushButton(centralWidget);
        import_paramter->setObjectName(QString::fromUtf8("import_paramter"));
        import_paramter->setGeometry(QRect(70, 50, 61, 31));
        rectification = new QPushButton(centralWidget);
        rectification->setObjectName(QString::fromUtf8("rectification"));
        rectification->setGeometry(QRect(10, 110, 121, 31));
        rectification->setAutoFillBackground(false);
        Test_Image = new QPushButton(centralWidget);
        Test_Image->setObjectName(QString::fromUtf8("Test_Image"));
        Test_Image->setGeometry(QRect(10, 80, 121, 31));
        Test_Image->setIcon(icon);
        progressBar = new QProgressBar(centralWidget);
        progressBar->setObjectName(QString::fromUtf8("progressBar"));
        progressBar->setGeometry(QRect(20, 150, 361, 16));
        progressBar->setValue(24);
        image_L = new QLabel(centralWidget);
        image_L->setObjectName(QString::fromUtf8("image_L"));
        image_L->setGeometry(QRect(20, 180, 480, 270));
        textBrowser = new QTextBrowser(centralWidget);
        textBrowser->setObjectName(QString::fromUtf8("textBrowser"));
        textBrowser->setGeometry(QRect(140, 20, 241, 121));
        image_R = new QLabel(centralWidget);
        image_R->setObjectName(QString::fromUtf8("image_R"));
        image_R->setGeometry(QRect(520, 180, 480, 270));
        Param_Browser = new QTextBrowser(centralWidget);
        Param_Browser->setObjectName(QString::fromUtf8("Param_Browser"));
        Param_Browser->setGeometry(QRect(470, 20, 331, 151));
        show_rectify = new QPushButton(centralWidget);
        show_rectify->setObjectName(QString::fromUtf8("show_rectify"));
        show_rectify->setGeometry(QRect(390, 150, 71, 21));
        Crop = new QPushButton(centralWidget);
        Crop->setObjectName(QString::fromUtf8("Crop"));
        Crop->setGeometry(QRect(390, 0, 71, 21));
        computeRange = new QPushButton(centralWidget);
        computeRange->setObjectName(QString::fromUtf8("computeRange"));
        computeRange->setGeometry(QRect(390, 30, 71, 21));
        disLine = new QPushButton(centralWidget);
        disLine->setObjectName(QString::fromUtf8("disLine"));
        disLine->setGeometry(QRect(390, 60, 71, 21));
        Param_Browser_2 = new QTextBrowser(centralWidget);
        Param_Browser_2->setObjectName(QString::fromUtf8("Param_Browser_2"));
        Param_Browser_2->setGeometry(QRect(810, 20, 221, 151));
        line1 = new QFrame(centralWidget);
        line1->setObjectName(QString::fromUtf8("line1"));
        line1->setGeometry(QRect(20, 234, 980, 3));
        line1->setFrameShape(QFrame::HLine);
        line1->setFrameShadow(QFrame::Sunken);
        line2 = new QFrame(centralWidget);
        line2->setObjectName(QString::fromUtf8("line2"));
        line2->setGeometry(QRect(20, 288, 980, 3));
        line2->setFrameShape(QFrame::HLine);
        line2->setFrameShadow(QFrame::Sunken);
        line3 = new QFrame(centralWidget);
        line3->setObjectName(QString::fromUtf8("line3"));
        line3->setGeometry(QRect(20, 342, 980, 3));
        line3->setFrameShape(QFrame::HLine);
        line3->setFrameShadow(QFrame::Sunken);
        line4 = new QFrame(centralWidget);
        line4->setObjectName(QString::fromUtf8("line4"));
        line4->setGeometry(QRect(20, 396, 980, 3));
        line4->setFrameShape(QFrame::HLine);
        line4->setFrameShadow(QFrame::Sunken);
        pSizeW = new QLineEdit(centralWidget);
        pSizeW->setObjectName(QString::fromUtf8("pSizeW"));
        pSizeW->setGeometry(QRect(70, 0, 21, 16));
        QFont font;
        font.setFamily(QString::fromUtf8("Arial"));
        font.setPointSize(11);
        pSizeW->setFont(font);
        label = new QLabel(centralWidget);
        label->setObjectName(QString::fromUtf8("label"));
        label->setGeometry(QRect(10, 0, 61, 20));
        label_2 = new QLabel(centralWidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setGeometry(QRect(630, 0, 61, 20));
        label_2->setMinimumSize(QSize(61, 20));
        label_2->setLineWidth(1);
        label_3 = new QLabel(centralWidget);
        label_3->setObjectName(QString::fromUtf8("label_3"));
        label_3->setGeometry(QRect(890, 0, 61, 20));
        label_3->setLineWidth(1);
        pSizeH = new QLineEdit(centralWidget);
        pSizeH->setObjectName(QString::fromUtf8("pSizeH"));
        pSizeH->setGeometry(QRect(100, 0, 21, 16));
        pSizeH->setFont(font);
        label_4 = new QLabel(centralWidget);
        label_4->setObjectName(QString::fromUtf8("label_4"));
        label_4->setGeometry(QRect(90, 0, 21, 20));
        label_5 = new QLabel(centralWidget);
        label_5->setObjectName(QString::fromUtf8("label_5"));
        label_5->setGeometry(QRect(130, 0, 61, 20));
        sqSize = new QLineEdit(centralWidget);
        sqSize->setObjectName(QString::fromUtf8("sqSize"));
        sqSize->setGeometry(QRect(180, 0, 21, 16));
        sqSize->setFont(font);
        label_6 = new QLabel(centralWidget);
        label_6->setObjectName(QString::fromUtf8("label_6"));
        label_6->setGeometry(QRect(200, 0, 61, 20));
        Confirm_chess = new QPushButton(centralWidget);
        Confirm_chess->setObjectName(QString::fromUtf8("Confirm_chess"));
        Confirm_chess->setGeometry(QRect(220, 0, 71, 16));
        stereo = new QPushButton(centralWidget);
        stereo->setObjectName(QString::fromUtf8("stereo"));
        stereo->setGeometry(QRect(390, 90, 71, 21));
        next_img = new QPushButton(centralWidget);
        next_img->setObjectName(QString::fromUtf8("next_img"));
        next_img->setGeometry(QRect(1010, 180, 31, 61));
        test1 = new QPushButton(centralWidget);
        test1->setObjectName(QString::fromUtf8("test1"));
        test1->setGeometry(QRect(1010, 250, 31, 61));
        CloudCompute = new QPushButton(centralWidget);
        CloudCompute->setObjectName(QString::fromUtf8("CloudCompute"));
        CloudCompute->setGeometry(QRect(390, 120, 71, 21));
        stereo_rectifyClass->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(stereo_rectifyClass);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1040, 18));
        menu = new QMenu(menuBar);
        menu->setObjectName(QString::fromUtf8("menu"));
        menustereo_disparity = new QMenu(menuBar);
        menustereo_disparity->setObjectName(QString::fromUtf8("menustereo_disparity"));
        stereo_rectifyClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(stereo_rectifyClass);
        mainToolBar->setObjectName(QString::fromUtf8("mainToolBar"));
        stereo_rectifyClass->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(stereo_rectifyClass);
        statusBar->setObjectName(QString::fromUtf8("statusBar"));
        stereo_rectifyClass->setStatusBar(statusBar);

        menuBar->addAction(menu->menuAction());
        menuBar->addAction(menustereo_disparity->menuAction());
        menu->addSeparator();

        retranslateUi(stereo_rectifyClass);

        QMetaObject::connectSlotsByName(stereo_rectifyClass);
    } // setupUi

    void retranslateUi(QMainWindow *stereo_rectifyClass)
    {
        stereo_rectifyClass->setWindowTitle(QApplication::translate("stereo_rectifyClass", "stereo_rectify", nullptr));
        actiond->setText(QApplication::translate("stereo_rectifyClass", "d", nullptr));
        Calib_Image->setText(QApplication::translate("stereo_rectifyClass", "\351\200\211\346\213\251\346\240\207\345\256\232\345\233\276\345\203\217\346\226\207\344\273\266\345\244\271", nullptr));
        calibration->setText(QApplication::translate("stereo_rectifyClass", "\345\274\200\345\247\213\346\240\207\345\256\232", nullptr));
        import_paramter->setText(QApplication::translate("stereo_rectifyClass", "\345\257\274\345\205\245\345\217\202\346\225\260", nullptr));
        rectification->setText(QApplication::translate("stereo_rectifyClass", "\345\274\200\345\247\213\347\237\253\346\255\243", nullptr));
        Test_Image->setText(QApplication::translate("stereo_rectifyClass", "\351\200\211\346\213\251\347\237\253\346\255\243\345\233\276\345\203\217\346\226\207\344\273\266\345\244\271", nullptr));
        image_L->setText(QApplication::translate("stereo_rectifyClass", "img_L", nullptr));
        image_R->setText(QApplication::translate("stereo_rectifyClass", "img_R", nullptr));
        show_rectify->setText(QApplication::translate("stereo_rectifyClass", "\345\261\225\347\244\272\347\237\253\346\255\243", nullptr));
        Crop->setText(QApplication::translate("stereo_rectifyClass", "\350\243\201\345\210\207", nullptr));
        computeRange->setText(QApplication::translate("stereo_rectifyClass", "\350\256\241\347\256\227range", nullptr));
        disLine->setText(QApplication::translate("stereo_rectifyClass", "\346\230\276\347\244\272\346\250\252\346\235\241", nullptr));
        pSizeW->setText(QApplication::translate("stereo_rectifyClass", "19", nullptr));
        label->setText(QApplication::translate("stereo_rectifyClass", "\346\243\213\347\233\230\346\240\274\350\247\222\347\202\271\346\225\260", nullptr));
        label_2->setText(QApplication::translate("stereo_rectifyClass", "Cameras", nullptr));
        label_3->setText(QApplication::translate("stereo_rectifyClass", "Stereo", nullptr));
        pSizeH->setText(QApplication::translate("stereo_rectifyClass", "14", nullptr));
        label_4->setText(QApplication::translate("stereo_rectifyClass", " *", nullptr));
        label_5->setText(QApplication::translate("stereo_rectifyClass", "\346\243\213\347\233\230\346\240\274\345\256\275\345\272\246", nullptr));
        sqSize->setText(QApplication::translate("stereo_rectifyClass", "15", nullptr));
        label_6->setText(QApplication::translate("stereo_rectifyClass", " mm", nullptr));
        Confirm_chess->setText(QApplication::translate("stereo_rectifyClass", "\347\241\256\350\256\244\346\243\213\347\233\230\345\217\202\346\225\260", nullptr));
        stereo->setText(QApplication::translate("stereo_rectifyClass", "\350\247\206\345\267\256\350\256\241\347\256\227", nullptr));
        next_img->setText(QApplication::translate("stereo_rectifyClass", "\344\270\213\347\273\204", nullptr));
        test1->setText(QApplication::translate("stereo_rectifyClass", "test1", nullptr));
        CloudCompute->setText(QApplication::translate("stereo_rectifyClass", "\347\202\271\344\272\221\350\275\254\346\215\242", nullptr));
        menu->setTitle(QApplication::translate("stereo_rectifyClass", "Zhuoyu_stereo_calibration v2.0", nullptr));
        menustereo_disparity->setTitle(QApplication::translate("stereo_rectifyClass", "stereo_disparity_v1.0", nullptr));
    } // retranslateUi

};

namespace Ui {
    class stereo_rectifyClass: public Ui_stereo_rectifyClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_STEREO_RECTIFY_H
