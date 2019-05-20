/*
 * qTrassa - The program for the calculation of geometrical elements of the 
 * track the excavation of tunnels (surveyors).
 * Copyright (c) 2014-2019 Oleg Kosorukov
 *
 * qTrassa is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software. If not, see <http://www.gnu.org/licenses/>.
 */


#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "comboboxdelegate.h"
#include "qlineeditdelegate.h"
#include "geo.h"
#include "calcq.h"
#include "about.h"

#include <QtCore>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QSettings>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QMessageBox>
#include <QDebug>
#include <QtMath>
#include <QClipboard>
#include <QRegExp>
#include <QEvent>
#include <QKeyEvent>
#include <QFileDialog>
#include <QDesktopWidget>



QString openFile = "";

//        0 	1	2	3   4       5       6       7       8   9       10          11      12      13
//        №№	№R  X	Y	Hфакт	D(1+4)	B=1,75	R пр	ПК	Смещ	HпрУГР  	Hпр 	Rфакт	Откл.

struct ring {    
    int nkolca;   //Номер кольца
    int nr;       //Номер радиуса

    double  rx;   //x факт Радиуса
    double  ry;   //y факт Радиуса
    double  rh;   //h факт Радиуса

    double xkol;  //x центр кольца
    double ykol;  //y центр кольца
    double hkol;  //h центр кольца

    double  D1_4; //Диаметр измеренный
    double  B;    //Домер от УГР до центра кольца (1,75)
    double  R;    //R     проект

    QString rPK;    //ПК
    double  rsm_qz; //Смещ
    double  rh_ugr; //HпрУГР
    double  rf;     //Rфакт
    double  rdiff_r;//Отклонение
} ;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    readSettings();
    //qDebug()<< "Разделитель числа" << QString::number(0.1).at(1);

    QString ds = QString::number(0.1).at(1);

    QString spk="[0-9]{1,3}[+]{1,1}[0-9]{1,3}["+ ds + "]{1,1}[0-9]{1,4}$";
    QRegExp regPK(spk);
    ui->EditPKnach->setValidator(new QRegExpValidator(regPK,this) );
    ui->lineEdit_RazbivkaPKnach->setValidator(new QRegExpValidator(regPK,this) );
    ui->lineEdit_RazbivkaPKkon->setValidator(new QRegExpValidator(regPK,this) );

    QString s = "[?-]{0,1}[0-9]{1,6}[?"+ ds + "]{0,1}[0-9]{0,4}";
    QRegExp regXY(s);

    //StartTrassa
    ui->EditXnach->setValidator(new QRegExpValidator(regXY,this));
    ui->EditYnach->setValidator(new QRegExpValidator(regXY,this));
    ui->PointEdit_X->setValidator(new QRegExpValidator(regXY,this));
    ui->PointEdit_Y->setValidator(new QRegExpValidator(regXY,this));
    ui->EditStndPk->setValidator( new QIntValidator(20, 200, this));

    //tableTrassa
    ComboBoxDelegate *cComboBoxDelegate = new ComboBoxDelegate;
    ui->tableTrassa->setItemDelegateForColumn(0, cComboBoxDelegate);
    QLineEditDelegate *xkQLineEditDelegate = new QLineEditDelegate;
    ui->tableTrassa->setItemDelegateForColumn(1, xkQLineEditDelegate);
    QLineEditDelegate *ykQLineEditDelegate = new QLineEditDelegate;
    ui->tableTrassa->setItemDelegateForColumn(2, ykQLineEditDelegate);
    QLineEditDelegate *xckQLineEditDelegate = new QLineEditDelegate;
    ui->tableTrassa->setItemDelegateForColumn(3, xckQLineEditDelegate);
    QLineEditDelegate *yckQLineEditDelegate = new QLineEditDelegate;
    ui->tableTrassa->setItemDelegateForColumn(4, yckQLineEditDelegate);
    ui->tableTrassa->setItem(0,0,new QTableWidgetItem("Прямая"));
    ui->tableTrassa->setItem(0,1,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
    ui->tableTrassa->setItem(0,2,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
    ui->tableTrassa->setItem(0,3,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
    ui->tableTrassa->setItem(0,4,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
    ui->tableTrassa->setItem(0,5,new QTableWidgetItem("0+0.0000"));
    ui->tableTrassa->setItem(0,6,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
    ui->tableTrassa->setItem(0,7,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
    ui->tableTrassa->item(0,5)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
    ui->tableTrassa->item(0,6)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
    ui->tableTrassa->item(0,7)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);

    //tableNPK
    QLineEditDelegate *PkNPKQLineEditDelegate = new QLineEditDelegate;
    PkNPKQLineEditDelegate->setPK(false);
    PkNPKQLineEditDelegate->setCeloe(true);
    ui->tableNPK->setItemDelegateForColumn(0, PkNPKQLineEditDelegate);

    QLineEditDelegate *LNPKQLineEditDelegate = new QLineEditDelegate;
    ui->tableNPK->setItemDelegateForColumn(1, LNPKQLineEditDelegate);
    ui->tableNPK->setItem( 0,0,new QTableWidgetItem(QString::number( 0,   'f', 0 ))); //Pk
    ui->tableNPK->setItem( 0,1,new QTableWidgetItem(QString::number( 100,   'f', 4 ))); //L

    //tableTrassa_2 Переходка
    QLineEditDelegate *PkNachQLineEditDelegate = new QLineEditDelegate; PkNachQLineEditDelegate->setPK(true);
    ui->tableTrassa_2->setItemDelegateForColumn(0, PkNachQLineEditDelegate);
    QLineEditDelegate *LpNachQLineEditDelegate = new QLineEditDelegate;
    ui->tableTrassa_2->setItemDelegateForColumn(1, LpNachQLineEditDelegate);
    QLineEditDelegate *CNachQLineEditDelegate = new QLineEditDelegate;
    ui->tableTrassa_2->setItemDelegateForColumn(2, CNachQLineEditDelegate);
    QLineEditDelegate *qQLineEditDelegate = new QLineEditDelegate;
    ui->tableTrassa_2->setItemDelegateForColumn(3, qQLineEditDelegate);
    QLineEditDelegate *zQLineEditDelegate = new QLineEditDelegate;
    ui->tableTrassa_2->setItemDelegateForColumn(4, zQLineEditDelegate);
    QLineEditDelegate *PkKonQLineEditDelegate = new QLineEditDelegate; PkKonQLineEditDelegate->setPK(true);
    ui->tableTrassa_2->setItemDelegateForColumn(5, PkKonQLineEditDelegate);
    QLineEditDelegate *LpKonQLineEditDelegate = new QLineEditDelegate;
    ui->tableTrassa_2->setItemDelegateForColumn(6, LpKonQLineEditDelegate);
    QLineEditDelegate *CKonQLineEditDelegate = new QLineEditDelegate;
    ui->tableTrassa_2->setItemDelegateForColumn(7, CKonQLineEditDelegate);
    ui->tableTrassa_2->setItem(0,0,new QTableWidgetItem("0+0.0000")); //PkNach
    ui->tableTrassa_2->setItem(0,1,new QTableWidgetItem(QString::number( 0,   'f', 4 ))); //LpNach
    ui->tableTrassa_2->setItem(0,2,new QTableWidgetItem(QString::number( 0,   'f', 4 ))); //CNach
    ui->tableTrassa_2->setItem(0,3,new QTableWidgetItem(QString::number( 0,   'f', 4 ))); //q
    ui->tableTrassa_2->setItem(0,4,new QTableWidgetItem(QString::number( 0,   'f', 4 ))); //z
    ui->tableTrassa_2->setItem(0,5,new QTableWidgetItem("0+0.0000")); //PkKon
    ui->tableTrassa_2->setItem(0,6,new QTableWidgetItem(QString::number( 0,   'f', 4 )));//LpKon
    ui->tableTrassa_2->setItem(0,7,new QTableWidgetItem(QString::number( 0,   'f', 4 )));//CKon
    ui->tableTrassa_2->setItem(0,8,new QTableWidgetItem(QString::number( 0,   'f', 3 )));//h
    //tableProfil
    QLineEditDelegate *PkProfilQLineEditDelegate = new QLineEditDelegate; PkProfilQLineEditDelegate->setPK(true);
    ui->tableProfil->setItemDelegateForColumn(0, PkProfilQLineEditDelegate);
    QLineEditDelegate *HProfilQLineEditDelegate = new QLineEditDelegate;
    ui->tableProfil->setItemDelegateForColumn(1, HProfilQLineEditDelegate);
    QLineEditDelegate *RProfilQLineEditDelegate = new QLineEditDelegate;
    ui->tableProfil->setItemDelegateForColumn(2, RProfilQLineEditDelegate);
    ui->tableProfil->setItem( 0,0,new QTableWidgetItem("0+0.0000"));
    ui->tableProfil->setItem( 0,1,new QTableWidgetItem(QString::number( 0,   'f', 4 ))); //H
    ui->tableProfil->setItem( 0,2,new QTableWidgetItem(QString::number( 0,   'f', 4 ))); //R
    ui->tableProfil->setItem(0,3,new QTableWidgetItem(QString::number( 0,   'f', 4 ))); //T
    ui->tableProfil->setItem(0,4,new QTableWidgetItem(QString::number( 0,   'f', 4 ))); //i
    ui->tableProfil->setItem(0,5,new QTableWidgetItem(QString::number( 0,   'f', 4 ))); //L
    ui->tableProfil->item(0,3)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
    ui->tableProfil->item(0,4)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
    ui->tableProfil->item(0,5)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);


    QLineEditDelegate *XRaschetQLineEditDelegate = new QLineEditDelegate;
    ui->tableRaschet->setItemDelegateForColumn(1, XRaschetQLineEditDelegate);
    QLineEditDelegate *YRaschetQLineEditDelegate = new QLineEditDelegate;
    ui->tableRaschet->setItemDelegateForColumn(2, YRaschetQLineEditDelegate);
    ui->tableRaschet->setItem(0,3,new QTableWidgetItem("0+0.0000"));
    ui->tableRaschet->setItem(0,4,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
    ui->tableRaschet->setItem(0,5,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
    ui->tableRaschet->item(0,3)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
    ui->tableRaschet->item(0,4)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
    ui->tableRaschet->item(0,5)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);

    //tablePred_Raschet
    QLineEditDelegate *PkPred_RaschetQLineEditDelegate = new QLineEditDelegate; PkPred_RaschetQLineEditDelegate->setPK(true);
    ui->tablePred_Raschet->setItemDelegateForColumn(1, PkPred_RaschetQLineEditDelegate);
    QLineEditDelegate *SmPred_RaschetQLineEditDelegate = new QLineEditDelegate;
    ui->tablePred_Raschet->setItemDelegateForColumn(2, SmPred_RaschetQLineEditDelegate);
    ui->tablePred_Raschet->setItem(0,1,new QTableWidgetItem("0+0.0000"));                     //пк
    ui->tablePred_Raschet->setItem(0,2,new QTableWidgetItem(QString::number( 0,   'f', 4 ))); //Смещение
    ui->tablePred_Raschet->setItem(0,3,new QTableWidgetItem(QString::number( 0,   'f', 4 ))); //X
    ui->tablePred_Raschet->setItem(0,4,new QTableWidgetItem(QString::number( 0,   'f', 4 ))); //Y
    ui->tablePred_Raschet->setItem(0,5,new QTableWidgetItem(QString::number( 0,   'f', 4 ))); //H
    ui->tablePred_Raschet->item(0,3)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
    ui->tablePred_Raschet->item(0,4)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
    ui->tablePred_Raschet->item(0,5)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);

    //table_Kolca
    //    ui->table_Kolca->setItem(0,1,new QTableWidgetItem(""));
    //    ui->table_Kolca->setItem(0,2,new QTableWidgetItem(""));
    //    ui->table_Kolca->setItem(0,3,new QTableWidgetItem(""));
    //    ui->table_Kolca->setItem(0,4,new QTableWidgetItem(""));
    //    ui->table_Kolca->setItem(0,5,new QTableWidgetItem(""));
    //    ui->table_Kolca->setItem(0,6,new QTableWidgetItem(""));
    //    ui->table_Kolca->setItem(0,7,new QTableWidgetItem(""));

    //    ui->table_Kolca->setItem(0,8,new QTableWidgetItem(""));
    //    ui->table_Kolca->setItem(0,9,new QTableWidgetItem(""));
    //    ui->table_Kolca->setItem(0,10,new QTableWidgetItem(""));
    //    ui->table_Kolca->setItem(0,11,new QTableWidgetItem(""));
    //    ui->table_Kolca->setItem(0,12,new QTableWidgetItem(""));
    //    ui->table_Kolca->setItem(0,13,new QTableWidgetItem(""));
    ui->table_Kolca->item(0,8)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
    ui->table_Kolca->item(0,9)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
    ui->table_Kolca->item(0,10)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
    ui->table_Kolca->item(0,11)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
    ui->table_Kolca->item(0,12)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
    ui->table_Kolca->item(0,13)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);

    //ui->tabWidget->widget(2)->setDisabled(true);
    ui->tabWidget->setCurrentIndex(0);

    qDebug() << "Параметр запуска =" << QApplication::arguments() << QApplication::arguments().count();

    if (QApplication::arguments().count()>1){
        //Если открыт файл программой то
        //nameFile =
        qDebug()<< QApplication::arguments().at(1);
        fileOpen(QApplication::arguments().at(1),false);
    }

    //Сканируем папку с шаблонами
    on_pB_reload_template_clicked();

    statusBar()->showMessage("Работаем", 3000);

    //    //Тестируем COMПорты

    //    // Example use QSerialPortInfo
    //    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {
    //        qDebug() << "Name : " << info.portName();
    //        qDebug() << "Description : " << info.description();
    //        qDebug() << "Manufacturer: " << info.manufacturer();

    //        // Example use QSerialPort
    //        QSerialPort serial;
    //        serial.setPort(info);
    //        if (serial.open(QIODevice::ReadWrite))
    //            serial.close();
    //    }
    //    //Тестируем БлюТУЗПорты
    //    QBluetoothLocalDevice localDevice;
    //    QString localDeviceName;

    //    // Check if Bluetooth is available on this device
    //    if (localDevice.isValid()) {
    //        // Turn Bluetooth on
    //        localDevice.powerOn();
    //        // Read local device name
    //        localDeviceName = localDevice.name();
    //        // Make it visible to others
    //        localDevice.setHostMode(QBluetoothLocalDevice::HostDiscoverable);
    //        // Get connected devices
    //        QList<QBluetoothAddress> remotes;
    //        remotes = localDevice.connectedDevices();
    //        qDebug() << remotes.value(remotes.count())<<remotes.count();
    //    }
}

MainWindow::~MainWindow()
{
    writeSettings();
    qDebug()<<"Настройки сохранили и программу закрыли";
    delete ui;
}

//Выходим
void MainWindow::on_action_close_triggered()
{    
    close();
}

void MainWindow::on_action_Save_triggered()
{
    if (openFile =="") {
        on_actionSaveAs_triggered();
    } else {
        QMessageBox msgBox;
        msgBox.setText("Сохранение файла...");
        msgBox.setInformativeText("При сохранении файла все данные в файле будут заменены новыми, \n продолжить?");
        msgBox.setIcon(QMessageBox::Warning);
        QPushButton *yes = msgBox.addButton("Да", QMessageBox::ActionRole);
        msgBox.addButton(QObject::tr("Отмена"), QMessageBox::ActionRole);
        msgBox.setDefaultButton(yes);
        msgBox.exec();
        if(msgBox.clickedButton() != yes)        {
            statusBar()->showMessage("Сохранение файла отменено...", 3000);
            return;
        }
        saveFile(openFile);
    }

}

void MainWindow::on_action_open_triggered()
{       
    QString dir = QDir::home().absolutePath()+QDir::separator()+QStandardPaths::displayName(QStandardPaths::DocumentsLocation)+QDir::separator();
    if(QDir(dir).exists())
    {
        qDebug()<<"Есть Папка";
    } else {
        dir = QDir::home().absolutePath()+QDir::separator();
        qDebug()<<"Нету Папки";
    }
    QString nameFile;
    nameFile = QFileDialog::getOpenFileName(this,tr("Выберите файл проекта"),dir +"/Проект_QtTrassa.mtrassa",tr("Проект QtTrassa (*.mtrassa)") );
    if(nameFile.isEmpty()) {return;}
    fileOpen(nameFile,true);

    qDebug() << nameFile;

}

void MainWindow::on_action_new_triggered()
{
    //создать
    QMessageBox msgBox;
    msgBox.setText("Новый  файл...");
    msgBox.setInformativeText("При создании нового файла все данные в программе будут уничтожены, \n продолжить?");
    msgBox.setIcon(QMessageBox::Warning);
    QPushButton *yes = msgBox.addButton(tr("Да"), QMessageBox::ActionRole);
    msgBox.addButton(QObject::tr("Отмена"), QMessageBox::ActionRole);
    msgBox.setDefaultButton(yes);
    msgBox.exec();
    if(msgBox.clickedButton() != yes)
    {
        statusBar()->showMessage(tr("Создание нового файла отменено..."), 3000);
        return;
    }

    ui->tableTrassa->setRowCount(1);
    ui->tableTrassa->setItem(0,0,new QTableWidgetItem(""));
    ui->tableTrassa->setItem(0,1,new QTableWidgetItem(""));
    ui->tableTrassa->setItem(0,2,new QTableWidgetItem(""));
    ui->tableTrassa->setItem(0,3,new QTableWidgetItem(""));
    ui->tableTrassa->setItem(0,4,new QTableWidgetItem(""));
    ui->tableTrassa->setItem(0,5,new QTableWidgetItem(""));
    ui->tableTrassa->setItem(0,6,new QTableWidgetItem(""));
    ui->tableTrassa->setItem(0,7,new QTableWidgetItem(""));

    ui->tableTrassa_2->setRowCount(1);
    ui->tableTrassa_2->setItem(0,0,new QTableWidgetItem(""));
    ui->tableTrassa_2->setItem(0,1,new QTableWidgetItem(""));
    ui->tableTrassa_2->setItem(0,2,new QTableWidgetItem(""));
    ui->tableTrassa_2->setItem(0,3,new QTableWidgetItem(""));
    ui->tableTrassa_2->setItem(0,4,new QTableWidgetItem(""));
    ui->tableTrassa_2->setItem(0,5,new QTableWidgetItem(""));
    ui->tableTrassa_2->setItem(0,6,new QTableWidgetItem(""));
    ui->tableTrassa_2->setItem(0,7,new QTableWidgetItem(""));
    ui->tableTrassa_2->setItem(0,8,new QTableWidgetItem(""));

    ui->tableRaschet->setRowCount(1);
    ui->tableRaschet->setItem(0,0,new QTableWidgetItem(""));
    ui->tableRaschet->setItem(0,1,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
    ui->tableRaschet->setItem(0,2,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
    ui->tableRaschet->setItem(0,3,new QTableWidgetItem("0+0.0000"));
    ui->tableRaschet->setItem(0,4,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
    ui->tableRaschet->setItem(0,5,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
    ui->tableRaschet->item(0,3)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
    ui->tableRaschet->item(0,4)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
    ui->tableRaschet->item(0,5)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);

    ui->tableProfil->setRowCount(1);
    ui->tableProfil->setItem( 0,0,new QTableWidgetItem(""));
    ui->tableProfil->setItem( 0,1,new QTableWidgetItem(""));
    ui->tableProfil->setItem( 0,2,new QTableWidgetItem(""));
    ui->tableProfil->setItem( 0,3,new QTableWidgetItem(""));
    ui->tableProfil->setItem( 0,4,new QTableWidgetItem(""));
    ui->tableProfil->setItem( 0,5,new QTableWidgetItem(""));

    ui->tableNPK->setRowCount(1);
    ui->tableNPK->setItem( 0,0,new QTableWidgetItem(QString::number( 0,   'f', 0 ))); //Pk
    ui->tableNPK->setItem( 0,1,new QTableWidgetItem(QString::number( 100,   'f', 4 ))); //L

    ui->table_Kolca->setRowCount(1);
    ui->table_Kolca->setItem(0,0,new QTableWidgetItem(""));
    ui->table_Kolca->setItem(0,1,new QTableWidgetItem(""));
    ui->table_Kolca->setItem(0,2,new QTableWidgetItem(""));
    ui->table_Kolca->setItem(0,3,new QTableWidgetItem(""));
    ui->table_Kolca->setItem(0,4,new QTableWidgetItem(""));
    ui->table_Kolca->setItem(0,5,new QTableWidgetItem(""));
    ui->table_Kolca->setItem(0,6,new QTableWidgetItem(""));
    ui->table_Kolca->setItem(0,7,new QTableWidgetItem(""));
    ui->table_Kolca->setItem(0,8,new QTableWidgetItem(""));
    ui->table_Kolca->setItem(0,9,new QTableWidgetItem(""));
    ui->table_Kolca->setItem(0,10,new QTableWidgetItem(""));
    ui->table_Kolca->setItem(0,11,new QTableWidgetItem(""));
    ui->table_Kolca->setItem(0,12,new QTableWidgetItem(""));
    ui->table_Kolca->setItem(0,13,new QTableWidgetItem(""));

    ui->lineEdit_RazbivkaPKnach->setText("0+0.0000");
    ui->lineEdit_RazbivkaPKkon->setText("0+0.0000");


    openFile ="";

    QSettings settings("nesmit", "qTrassa");
    settings.beginGroup("MainWindow");
    setWindowTitle(settings.value("infoPRG").toString());
    settings.endGroup();

    statusBar()->showMessage(tr("File successfully create."), 3000);
}

//Save ConfigFile
void MainWindow::writeSettings() {
    QSettings settings("nesmit", "qTrassa");

    settings.beginGroup("MainWindow");
    settings.setValue("size", size());
    settings.setValue("pos", pos());
    settings.setValue("infoPRG", "qTrassa 0.52" );
    settings.setValue("splitter", ui->splitter->saveState());
    settings.endGroup();

    settings.beginGroup("TableTrassa");
    settings.setValue("tr_type",       ui->tableTrassa->columnWidth(0));
    settings.setValue("tr_Xk",         ui->tableTrassa->columnWidth(1));
    settings.setValue("tr_Yk",         ui->tableTrassa->columnWidth(2));
    settings.setValue("tr_Xck",        ui->tableTrassa->columnWidth(3));
    settings.setValue("tr_Yck",        ui->tableTrassa->columnWidth(4));
    settings.setValue("tr_Pkkon",      ui->tableTrassa->columnWidth(5));
    settings.setValue("tr_Luch",       ui->tableTrassa->columnWidth(6));
    settings.setValue("tr_R",          ui->tableTrassa->columnWidth(7));
    settings.endGroup();

    settings.beginGroup("TablePerehod");
    settings.setValue("pr_PKpnach",    ui->tableTrassa_2->columnWidth(0));
    settings.setValue("pr_Lpnach",     ui->tableTrassa_2->columnWidth(1));
    settings.setValue("pr_Cnach",      ui->tableTrassa_2->columnWidth(2));
    settings.setValue("pr_q",          ui->tableTrassa_2->columnWidth(3));
    settings.setValue("pr_z",          ui->tableTrassa_2->columnWidth(4));
    settings.setValue("pr_PKpkon",     ui->tableTrassa_2->columnWidth(5));
    settings.setValue("pr_Lpkon",      ui->tableTrassa_2->columnWidth(6));
    settings.setValue("pr_Ckon",       ui->tableTrassa_2->columnWidth(7));
    settings.setValue("pr_h",          ui->tableTrassa_2->columnWidth(8));
    settings.endGroup();

    settings.beginGroup("TableProfil");
    settings.setValue("h_pk",       ui->tableProfil->columnWidth(0));
    settings.setValue("h_H",          ui->tableProfil->columnWidth(1));
    settings.setValue("h_R",          ui->tableProfil->columnWidth(2));
    settings.setValue("h_T",          ui->tableProfil->columnWidth(3));
    settings.setValue("h_i",          ui->tableProfil->columnWidth(4));
    settings.setValue("h_L",          ui->tableProfil->columnWidth(5));
    settings.endGroup();

    settings.beginGroup("TableRaschet");
    settings.setValue("r_n",          ui->tableRaschet->columnWidth(0));
    settings.setValue("r_x",          ui->tableRaschet->columnWidth(1));
    settings.setValue("r_y",          ui->tableRaschet->columnWidth(2));
    settings.setValue("r_pk",         ui->tableRaschet->columnWidth(3));
    settings.setValue("r_sm",         ui->tableRaschet->columnWidth(4));
    settings.setValue("r_h",          ui->tableRaschet->columnWidth(5));
    settings.endGroup();

    settings.beginGroup("TableKolca");
    settings.setValue("k_nk",         ui->table_Kolca->columnWidth(0));
    settings.setValue("k_nr",         ui->table_Kolca->columnWidth(1));
    settings.setValue("k_x",          ui->table_Kolca->columnWidth(2));
    settings.setValue("k_y",          ui->table_Kolca->columnWidth(3));
    settings.setValue("k_h",          ui->table_Kolca->columnWidth(4));
    settings.setValue("k_d",          ui->table_Kolca->columnWidth(5));
    settings.setValue("k_b",          ui->table_Kolca->columnWidth(6));
    settings.setValue("k_r",          ui->table_Kolca->columnWidth(7));
    settings.setValue("k_pk",         ui->table_Kolca->columnWidth(8));
    settings.setValue("k_sm",         ui->table_Kolca->columnWidth(9));
    settings.setValue("k_hugr",       ui->table_Kolca->columnWidth(10));
    settings.setValue("k_hpr",        ui->table_Kolca->columnWidth(11));
    settings.setValue("k_rf",         ui->table_Kolca->columnWidth(12));
    settings.setValue("k_diff",       ui->table_Kolca->columnWidth(13));
    settings.endGroup();

    settings.beginGroup("TableNPK");
    settings.setValue("nPK",        ui->tableNPK->columnWidth(0));
    settings.setValue("nPKPlus",    ui->tableNPK->columnWidth(1));
    settings.endGroup();

    settings.beginGroup("tableWidget"); //Разбивка
    settings.setValue("razbivkaPK",       ui->tableWidget->columnWidth(0));
    settings.setValue("razbivkaPKusl",    ui->tableWidget->columnWidth(1));
    settings.setValue("razbivkaX",        ui->tableWidget->columnWidth(2));
    settings.setValue("razbivkaY",        ui->tableWidget->columnWidth(3));
    settings.setValue("razbivkaH",        ui->tableWidget->columnWidth(4));
    settings.endGroup();


}

//Load ConfigFile
void MainWindow::readSettings()
{
    QSettings settings("nesmit", "qTrassa");
    settings.beginGroup("MainWindow");
    resize(settings.value("size", QSize(400, 400)).toSize());
    move(settings.value  ("pos", QPoint(200, 200)).toPoint());
    ui->splitter->restoreState(settings.value("splitter").toByteArray());
    setWindowTitle(settings.value("infoPRG").toString());
    settings.endGroup();

    //Load Column Width TableTrassa
    settings.beginGroup("TableTrassa");
    if (settings.value("tr_type").toInt()      == 0) {ui->tableTrassa->setColumnWidth(0,55);}   else {ui->tableTrassa->setColumnWidth(0,(settings.value("tr_type").toInt()));}
    if (settings.value("tr_Xk").toInt()        == 0) {ui->tableTrassa->setColumnWidth(1,75);}   else {ui->tableTrassa->setColumnWidth(1,(settings.value("tr_Xk").toInt()));}
    if (settings.value("tr_Yk").toInt()        == 0) {ui->tableTrassa->setColumnWidth(2,75);}   else {ui->tableTrassa->setColumnWidth(2,(settings.value("tr_Yk").toInt()));}
    if (settings.value("tr_Xck").toInt()       == 0) {ui->tableTrassa->setColumnWidth(3,75);}   else {ui->tableTrassa->setColumnWidth(3,(settings.value("tr_Xck").toInt()));}
    if (settings.value("tr_Yck").toInt()       == 0) {ui->tableTrassa->setColumnWidth(4,75);}   else {ui->tableTrassa->setColumnWidth(4,(settings.value("tr_Yck").toInt()));}
    if (settings.value("tr_Pkkon").toInt()     == 0) {ui->tableTrassa->setColumnWidth(5,75);}   else {ui->tableTrassa->setColumnWidth(5,(settings.value("tr_Pkkon").toInt()));}
    if (settings.value("tr_Luch").toInt()      == 0) {ui->tableTrassa->setColumnWidth(6,61);}   else {ui->tableTrassa->setColumnWidth(6,(settings.value("tr_Luch").toInt()));}
    if (settings.value("tr_R").toInt()         == 0) {ui->tableTrassa->setColumnWidth(7,60);}   else {ui->tableTrassa->setColumnWidth(7,(settings.value("tr_R").toInt()));}
    settings.endGroup();

    settings.beginGroup("TablePerehod");
    if (settings.value("pr_PKpnach").toInt()   == 0) {ui->tableTrassa_2->setColumnWidth(0,75);}  else {ui->tableTrassa_2->setColumnWidth(0,(settings.value("pr_PKpnach").toInt()));}
    if (settings.value("pr_Lpnach").toInt()    == 0) {ui->tableTrassa_2->setColumnWidth(1,55);}  else {ui->tableTrassa_2->setColumnWidth(1,(settings.value("pr_Lpnach").toInt()));}
    if (settings.value("pr_Cnach").toInt()     == 0) {ui->tableTrassa_2->setColumnWidth(2,50);}  else {ui->tableTrassa_2->setColumnWidth(2,(settings.value("pr_Ckon").toInt()));}
    if (settings.value("pr_q").toInt()         == 0) {ui->tableTrassa_2->setColumnWidth(3,45);}  else {ui->tableTrassa_2->setColumnWidth(3,(settings.value("pr_q").toInt()));}
    if (settings.value("pr_z").toInt()         == 0) {ui->tableTrassa_2->setColumnWidth(4,45);}  else {ui->tableTrassa_2->setColumnWidth(4,(settings.value("pr_z").toInt()));}
    if (settings.value("pr_PKpkon").toInt()    == 0) {ui->tableTrassa_2->setColumnWidth(5,75);}  else {ui->tableTrassa_2->setColumnWidth(5,(settings.value("pr_PKpkon").toInt()));}
    if (settings.value("pr_Lpkon").toInt()     == 0) {ui->tableTrassa_2->setColumnWidth(6,55);}  else {ui->tableTrassa_2->setColumnWidth(6,(settings.value("pr_Lpkon").toInt()));}
    if (settings.value("pr_Ckon").toInt()      == 0) {ui->tableTrassa_2->setColumnWidth(7,50);}  else {ui->tableTrassa_2->setColumnWidth(7,(settings.value("pr_Ckon").toInt()));}
    if (settings.value("pr_h").toInt()         == 0) {ui->tableTrassa_2->setColumnWidth(8,45);}  else {ui->tableTrassa_2->setColumnWidth(8,(settings.value("pr_h").toInt()));}
    settings.endGroup();

    settings.beginGroup("TableProfil");
    if (settings.value("h_pk").toInt()        == 0) {ui->tableProfil->setColumnWidth(0,75);}  else {ui->tableProfil->setColumnWidth(0,(settings.value("h_pk").toInt()));}
    if (settings.value("h_H").toInt()         == 0) {ui->tableProfil->setColumnWidth(1,65);}  else {ui->tableProfil->setColumnWidth(1,(settings.value("h_H").toInt()));}
    if (settings.value("h_R").toInt()         == 0) {ui->tableProfil->setColumnWidth(2,65);}  else {ui->tableProfil->setColumnWidth(2,(settings.value("h_R").toInt()));}
    if (settings.value("h_T").toInt()         == 0) {ui->tableProfil->setColumnWidth(3,70);}  else {ui->tableProfil->setColumnWidth(3,(settings.value("h_T").toInt()));}
    if (settings.value("h_i").toInt()         == 0) {ui->tableProfil->setColumnWidth(4,50);}  else {ui->tableProfil->setColumnWidth(4,(settings.value("h_i").toInt()));}
    if (settings.value("h_L").toInt()         == 0) {ui->tableProfil->setColumnWidth(5,50);}  else {ui->tableProfil->setColumnWidth(5,(settings.value("h_L").toInt()));}
    settings.endGroup();

    settings.beginGroup("TableRaschet");
    if (settings.value("r_n").toInt()         == 0) {ui->tableRaschet->setColumnWidth(0,70);}  else {ui->tableRaschet->setColumnWidth(0,(settings.value("r_n").toInt()));}
    if (settings.value("r_x").toInt()         == 0) {ui->tableRaschet->setColumnWidth(1,65);}  else {ui->tableRaschet->setColumnWidth(1,(settings.value("r_x").toInt()));}
    if (settings.value("r_y").toInt()         == 0) {ui->tableRaschet->setColumnWidth(2,65);}  else {ui->tableRaschet->setColumnWidth(2,(settings.value("r_y").toInt()));}
    if (settings.value("r_pk").toInt()        == 0) {ui->tableRaschet->setColumnWidth(3,70);}  else {ui->tableRaschet->setColumnWidth(3,(settings.value("r_pk").toInt()));}
    if (settings.value("r_sm").toInt()        == 0) {ui->tableRaschet->setColumnWidth(4,50);}  else {ui->tableRaschet->setColumnWidth(4,(settings.value("r_sm").toInt()));}
    if (settings.value("r_h").toInt()         == 0) {ui->tableRaschet->setColumnWidth(5,50);}  else {ui->tableRaschet->setColumnWidth(5,(settings.value("r_h").toInt()));}
    settings.endGroup();

    settings.beginGroup("TableKolca");
    if (settings.value("k_nk").toInt()        == 0) {ui->table_Kolca->setColumnWidth(0,60);}   else {ui->table_Kolca->setColumnWidth(0,(settings.value("k_nk").toInt()));}
    if (settings.value("k_nr").toInt()        == 0) {ui->table_Kolca->setColumnWidth(1,60);}   else {ui->table_Kolca->setColumnWidth(1,(settings.value("k_nr").toInt()));}
    if (settings.value("k_x").toInt()         == 0) {ui->table_Kolca->setColumnWidth(2,60);}   else {ui->table_Kolca->setColumnWidth(2,(settings.value("k_x").toInt()));}
    if (settings.value("k_y").toInt()         == 0) {ui->table_Kolca->setColumnWidth(3,60);}   else {ui->table_Kolca->setColumnWidth(3,(settings.value("k_y").toInt()));}
    if (settings.value("k_h").toInt()         == 0) {ui->table_Kolca->setColumnWidth(4,60);}   else {ui->table_Kolca->setColumnWidth(4,(settings.value("k_h").toInt()));}
    if (settings.value("k_d").toInt()         == 0) {ui->table_Kolca->setColumnWidth(5,60);}   else {ui->table_Kolca->setColumnWidth(5,(settings.value("k_d").toInt()));}
    if (settings.value("k_b").toInt()         == 0) {ui->table_Kolca->setColumnWidth(6,45);}   else {ui->table_Kolca->setColumnWidth(6,(settings.value("k_b").toInt()));}
    if (settings.value("k_r").toInt()         == 0) {ui->table_Kolca->setColumnWidth(7,45);}   else {ui->table_Kolca->setColumnWidth(7,(settings.value("k_r").toInt()));}
    if (settings.value("k_pk").toInt()        == 0) {ui->table_Kolca->setColumnWidth(8,75);}   else {ui->table_Kolca->setColumnWidth(8,(settings.value("k_pk").toInt()));}
    if (settings.value("k_sm").toInt()        == 0) {ui->table_Kolca->setColumnWidth(9,75);}   else {ui->table_Kolca->setColumnWidth(9,(settings.value("k_sm").toInt()));}
    if (settings.value("k_hugr").toInt()      == 0) {ui->table_Kolca->setColumnWidth(10,55);}  else {ui->table_Kolca->setColumnWidth(10,(settings.value("k_hugr").toInt()));}
    if (settings.value("k_hpr").toInt()       == 0) {ui->table_Kolca->setColumnWidth(11,55);}  else {ui->table_Kolca->setColumnWidth(11,(settings.value("k_hpr").toInt()));}
    if (settings.value("k_rf").toInt()        == 0) {ui->table_Kolca->setColumnWidth(12,45);}  else {ui->table_Kolca->setColumnWidth(12,(settings.value("k_rf").toInt()));}
    if (settings.value("k_diff").toInt()      == 0) {ui->table_Kolca->setColumnWidth(13,45);}  else {ui->table_Kolca->setColumnWidth(13,(settings.value("k_diff").toInt()));}
    settings.endGroup();

    //Load Column Width TableNPK
    settings.beginGroup("TableNPK");
    if (settings.value("nPK").toInt()       == 0) {ui->tableNPK->setColumnWidth(0,37);}     else {ui->tableNPK->setColumnWidth(0,(settings.value("nPK").toInt()));}
    if (settings.value("nPKPlus").toInt()   == 0) {ui->tableNPK->setColumnWidth(1,51);}     else {ui->tableNPK->setColumnWidth(1,(settings.value("nPKPlus").toInt()));}
    settings.endGroup();

    //Load Column Width TableNPK
    settings.beginGroup("tableWidget"); //Разбивка
    if (settings.value("razbivkaPK").toInt()     == 0) {ui->tableWidget->setColumnWidth(0,120);}     else {ui->tableWidget->setColumnWidth(0,(settings.value("razbivkaPK").toInt()));}
    if (settings.value("razbivkaPKusl").toInt()  == 0) {ui->tableWidget->setColumnWidth(1,120);}     else {ui->tableWidget->setColumnWidth(1,(settings.value("razbivkaPKusl").toInt()));}
    if (settings.value("razbivkaX").toInt()      == 0) {ui->tableWidget->setColumnWidth(2,120);}     else {ui->tableWidget->setColumnWidth(2,(settings.value("razbivkaX").toInt()));}
    if (settings.value("razbivkaY").toInt()      == 0) {ui->tableWidget->setColumnWidth(3,120);}     else {ui->tableWidget->setColumnWidth(3,(settings.value("razbivkaY").toInt()));}
    if (settings.value("razbivkaH").toInt()      == 0) {ui->tableWidget->setColumnWidth(4,120);}     else {ui->tableWidget->setColumnWidth(4,(settings.value("razbivkaH").toInt()));}
    settings.endGroup();
}

void MainWindow::on_Button_Run_clicked()
{
    geo zadacha;

    qDebug() << "Считываем длинну Стандартного пикета";
    zadacha.PkStnd = ui->EditStndPk->text().toDouble(); //Считываем длинну Стандартного пикета

    //Ввод таблицы неправильного пикета
    qDebug() << "Ввод таблицы неправильного пикета";
    for (int i=0;i<(ui->tableNPK->rowCount());i++){
        zadacha.appendNpk(ui->tableNPK->item(i,0)->text(),ui->tableNPK->item(i,1)->text());
    }

    //Ввод таблицы трасса
    qDebug() << "Ввод таблицы трасса";
    //QString KrLine,QString PKn,QString Xn,QString Yn,QString Xk,QString Yk,QString Xck,QString Yck)
    for (int i = 0; i < ui->tableTrassa->rowCount();i++ ){
        zadacha.appendTrassa(ui->tableTrassa->item(i,0)->text(), //type
                             ui->EditPKnach->text(), //ПК начала трассы
                             ui->EditXnach->text(),  //X начала трассы
                             ui->EditYnach->text(),  //Y начала трассы
                             ui->tableTrassa->item(i,1)->text(), //Xk
                             ui->tableTrassa->item(i,2)->text(), //Yk
                             ui->tableTrassa->item(i,3)->text(), //Xck
                             ui->tableTrassa->item(i,4)->text());//Yck
    }
    //Ввод таблицы Переходных кривых
    qDebug() << "Ввод таблицы Переходных кривых";
    for (int i = 0; i < ui->tableTrassa_2->rowCount();i++ ){
        zadacha.appendPerehod(ui->tableTrassa_2->item(i,0)->text(), //PKin
                              ui->tableTrassa_2->item(i,1)->text(), //Lin
                              ui->tableTrassa_2->item(i,2)->text(), //Cin
                              ui->tableTrassa_2->item(i,3)->text(), //q
                              ui->tableTrassa_2->item(i,4)->text(), //z
                              ui->tableTrassa_2->item(i,5)->text(), //PKout
                              ui->tableTrassa_2->item(i,6)->text(), //Lout
                              ui->tableTrassa_2->item(i,7)->text()); //Cout
    }
    //Ввод таблицы Профиля
    for (int i = 0; i < ui->tableProfil->rowCount();i++ ){
        zadacha.appendProfil(ui->tableProfil->item(i,0)->text(),
                             ui->tableProfil->item(i,1)->text(),
                             ui->tableProfil->item(i,2)->text());
    }
    qDebug() << "Предрасчет";
    zadacha.predRaschet();


    //Считаем точки
    bool xOk;
    bool yOk;

    double xkoor = ui->PointEdit_X->text().toDouble(&xOk);
    double ykoor = ui->PointEdit_Y->text().toDouble(&yOk);
    QString PKpoint  = "0";
    double SMpoint  = 0;
    QString RL = "L";

    double SMpointZ  = 0;
    double SMpointZQ = 0;
    double Hpoint = 0;

    if ((xOk == true) &&(yOk == true)){
        zadacha.raschetPkSm(xkoor,
                            ykoor,
                            PKpoint,
                            SMpoint,
                            RL);
        qDebug() << "Пк=" << PKpoint << "Смещение=" << SMpoint << RL;


        zadacha.raschetPer(PKpoint,
                           SMpoint,
                           RL,
                           SMpointZ,
                           SMpointZQ);
    }

    qDebug() << "----------------Укладочная----------------";
    zadacha.raschetH(PKpoint,Hpoint);
    qDebug() << "----------------Укладочная----------------";
    qDebug() << Hpoint;

    ui->label_7->setText(RL +"_Пк=" + PKpoint);
    ui->label_8->setText(RL +"_См=" + QString::number(SMpoint,'f',3));
    ui->label_9->setText(RL +"_См z=" + QString::number(SMpointZ,'f',3));
    ui->label_10->setText(RL +"_См z+q=" + QString::number(SMpointZQ,'f',3));
    ui->label_12->setText("H=" + QString::number(Hpoint,'f',3));

}

void MainWindow::on_EditXnach_returnPressed()
{
    double s = ui->EditXnach->text().toDouble();
    ui->EditXnach->setText( QString::number( s, 'f', 4 ));
    ui->EditYnach->setFocus();
    ui->EditYnach->selectAll();
}

void MainWindow::on_EditYnach_returnPressed()
{
    double s = ui->EditYnach->text().toDouble();
    ui->EditYnach->setText( QString::number( s, 'f', 4 ));
    ui->EditStndPk->setFocus();
    ui->EditStndPk->selectAll();
}

void MainWindow::on_EditPKnach_returnPressed()
{
    QString pk= ui->EditPKnach->text().split("+").value(0);
    double pkplus = ui->EditPKnach->text().split("+").value(1).toDouble();
    ui->EditPKnach->setText(pk + "+" + QString::number( pkplus, 'f', 4 ));
    ui->EditXnach->setFocus();
    ui->EditXnach->selectAll();

}


void MainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(this,"aboutQt");
}


//Закладка Расчет
void MainWindow::on_pushButton_Raschet_run_clicked()
{
    geo zadacha;
    qDebug() << "Считываем длинну Стандартного пикета";
    zadacha.PkStnd = ui->EditStndPk->text().toDouble(); //Считываем длинну Стандартного пикета
    //Ввод таблицы неправильного пикета
    qDebug() << "Ввод таблицы неправильного пикета";
    for (int i=0;i<(ui->tableNPK->rowCount());i++){
        zadacha.appendNpk(ui->tableNPK->item(i,0)->text(),ui->tableNPK->item(i,1)->text());
    }
    //Ввод таблицы трасса
    qDebug() << "Ввод таблицы трасса";
    //QString KrLine,QString PKn,QString Xn,QString Yn,QString Xk,QString Yk,QString Xck,QString Yck)
    for (int i = 0; i < ui->tableTrassa->rowCount();i++ ){
        zadacha.appendTrassa(ui->tableTrassa->item(i,0)->text(), //type
                             ui->EditPKnach->text(), //ПК начала трассы
                             ui->EditXnach->text(),  //X начала трассы
                             ui->EditYnach->text(),  //Y начала трассы
                             ui->tableTrassa->item(i,1)->text(), //Xk
                             ui->tableTrassa->item(i,2)->text(), //Yk
                             ui->tableTrassa->item(i,3)->text(), //Xck
                             ui->tableTrassa->item(i,4)->text());//Yck
    }
    //Ввод таблицы Переходных кривых
    qDebug() << "Ввод таблицы Переходных кривых";
    for (int i = 0; i < ui->tableTrassa_2->rowCount();i++ ){
        zadacha.appendPerehod(ui->tableTrassa_2->item(i,0)->text(), //PKin
                              ui->tableTrassa_2->item(i,1)->text(), //Lin
                              ui->tableTrassa_2->item(i,2)->text(), //Cin
                              ui->tableTrassa_2->item(i,3)->text(), //q
                              ui->tableTrassa_2->item(i,4)->text(), //z
                              ui->tableTrassa_2->item(i,5)->text(), //PKout
                              ui->tableTrassa_2->item(i,6)->text(), //Lout
                              ui->tableTrassa_2->item(i,7)->text()); //Cout
    }

    //Ввод таблицы Профиля
    for (int i = 0; i < ui->tableProfil->rowCount();i++ ){
        zadacha.appendProfil(ui->tableProfil->item(i,0)->text(),
                             ui->tableProfil->item(i,1)->text(),
                             ui->tableProfil->item(i,2)->text());
    }

    qDebug() << "Предрасчет";
    zadacha.predRaschet();


    //Считаем точки
    bool xOk;
    bool yOk;

    double xkoor = 0;
    double ykoor = 0;
    QString PKpoint  = "0";
    double SMpoint  = 0;
    double SMpointZ  = 0;
    double SMpointZQ = 0;
    double Hpoint = 0;

    QString RL = "L";

    //Условные координаты
    double x_usl_koor = ui->lineEdit_usl_x->text().toDouble(&xOk);
    double y_usl_koor = ui->lineEdit_usl_y->text().toDouble(&yOk);
    if ((xOk != true) || (yOk != true)){
        x_usl_koor = 0;
        y_usl_koor = 0;
    }
    qDebug()<<x_usl_koor<<xOk<< "  "<<y_usl_koor<<yOk;

    for (int i = 0; i < ui->tableRaschet->rowCount();i++ ){
        if (ui->checkBox_usl->isChecked()==true ){
            xkoor = ui->tableRaschet->item(i,1)->text().toDouble(&xOk)+x_usl_koor;
            ykoor = ui->tableRaschet->item(i,2)->text().toDouble(&yOk)+y_usl_koor;
        } else {
            xkoor = ui->tableRaschet->item(i,1)->text().toDouble(&xOk);
            ykoor = ui->tableRaschet->item(i,2)->text().toDouble(&yOk);
        }
        qDebug()<< "Условка " <<xkoor<<ykoor;
        PKpoint  = "0";
        SMpoint   = 0;
        SMpointZ  = 0;
        SMpointZQ = 0;
        Hpoint    = 0;

        if ((xOk == true) && (yOk == true)){
            zadacha.raschetPkSm(xkoor,
                                ykoor,
                                PKpoint,
                                SMpoint,
                                RL);

            zadacha.raschetPer(PKpoint,
                               SMpoint,
                               RL,
                               SMpointZ,
                               SMpointZQ);

            zadacha.raschetH(PKpoint,Hpoint);

            ui->tableRaschet->setItem(i,3,new QTableWidgetItem(PKpoint));
            switch (ui->TypeOsComboBox->currentIndex()) {
            case 0: //Разбивочная ось
                ui->tableRaschet->setItem(i,4,new QTableWidgetItem(QString::number( SMpoint,   'f', 4 )));
                break;

            case 1: //Ось пути +Z
                if (SMpointZ!=0){
                    ui->tableRaschet->setItem(i,4,new QTableWidgetItem(QString::number( SMpointZ,   'f', 4 )));
                }else{
                    ui->tableRaschet->setItem(i,4,new QTableWidgetItem(QString::number( SMpoint,   'f', 4 )));
                }
                break;

            case 2: //Ось тоннеля +Z +Q
                if (SMpointZQ!=0){
                    ui->tableRaschet->setItem(i,4,new QTableWidgetItem(QString::number( SMpointZQ,   'f', 4 )));
                }else{
                    ui->tableRaschet->setItem(i,4,new QTableWidgetItem(QString::number( SMpoint,   'f', 4 )));

                }
                break;

            default:
                ui->tableRaschet->setItem(i,4,new QTableWidgetItem(QString::number( SMpoint,   'f', 4 )));
                break;
            }
            ui->tableRaschet->setItem(i,5,new QTableWidgetItem(QString::number( Hpoint,   'f', 4 )));

            ui->tableRaschet->item(i,3)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled); //Пк
            ui->tableRaschet->item(i,4)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled); //Смещение
            ui->tableRaschet->item(i,5)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled); //Отметка с укладочной схемы
        }
    }
    statusBar()->showMessage(tr("Расчет выполнен"), 3000);
}//on_pushButton_Raschet_run_clicked()




void MainWindow::on_action_about_program_triggered()
{
    //    QMessageBox::about(this,"О программе", "<p>qTrassa METRO 0.5 betta</p>" "\n 26.01.2016г.");
    QDesktopWidget desktop;
    QRect rect = desktop.availableGeometry(desktop.primaryScreen()); // прямоугольник с размерами экрана
    QPoint center = rect.center(); //координаты центра экрана



    About *about_m = new About;
    int x = center.x() - (about_m->width()/2);  // учитываем половину ширины окна
    int y = center.y() - (about_m->height()/2); // .. половину высоты
    center.setX(x);
    center.setY(y);
    about_m->move(center);
    about_m->setModal(true);
    //    QSettings settings("nesmit", "qTrassa");
    //    settings.beginGroup("MainWindow");
    //    ui->label_2->setText("О программе: "+settings.value("infoPRG").toString());
    //    settings.endGroup();
    //    ui->setupUi(this);
    about_m->exec();
}

void MainWindow::on_tableRaschet_customContextMenuRequested(const QPoint &pos)
{
    QMenu *menu=new QMenu(this);
    menu->addAction(new QAction("Вставить строку выше", this));
    menu->addAction(new QAction("Вставить строку ниже", this));
    menu->addAction(new QAction("Удалить строку", this));
    menu->addSeparator();
    menu->addAction(new QAction("Копировать все (для Excel)", this));
    menu->addAction(new QAction("Вставить все (для Excel)", this));
    menu->addSeparator();
    menu->addAction(new QAction("Очистить", this));

    QString ds = QString::number(0.1).at(1);

    QAction* selectedItem= menu->exec(ui->tableRaschet->viewport()->mapToGlobal(pos));
    if (selectedItem)
    {
        QClipboard *clipboard = QApplication::clipboard();
        QString s = "";
        qDebug()<<selectedItem->text();
        int cRow = ui->tableRaschet->selectionModel()->currentIndex().row();
        if (selectedItem->text()=="Очистить") {
            ui->tableRaschet->setRowCount(1);
            ui->tableRaschet->setItem(0,0,new QTableWidgetItem(""));
            ui->tableRaschet->setItem(0,1,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
            ui->tableRaschet->setItem(0,2,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
            ui->tableRaschet->setItem(0,3,new QTableWidgetItem("0+0.0000"));
            ui->tableRaschet->setItem(0,4,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
            ui->tableRaschet->setItem(0,5,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
            ui->tableRaschet->item(0,0)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled);
            ui->tableRaschet->item(0,1)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled);
            ui->tableRaschet->item(0,2)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled);
            ui->tableRaschet->item(0,3)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
            ui->tableRaschet->item(0,4)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
            ui->tableRaschet->item(0,5)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
        }
        if (selectedItem->text()=="Копировать все (для Excel)") {
            for (int i = 0; i < ui->tableRaschet->rowCount();i++ ){
                s += ui->tableRaschet->item(i,0)->text()+"\t";
                s += ui->tableRaschet->item(i,1)->text()+"\t";
                s += ui->tableRaschet->item(i,2)->text()+"\t";
                s += ui->tableRaschet->item(i,3)->text()+"\t";
                s += ui->tableRaschet->item(i,4)->text()+"\t";
                s += ui->tableRaschet->item(i,5)->text()+"\n";
                ui->tableRaschet->item(i,0)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled);
                ui->tableRaschet->item(i,1)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled);
                ui->tableRaschet->item(i,2)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled);
                ui->tableRaschet->item(i,3)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
                ui->tableRaschet->item(i,4)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
                ui->tableRaschet->item(i,5)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
            }
            clipboard->setText(s);
        }

        if (selectedItem->text()=="Вставить все (для Excel)") {
            s = clipboard->text();
            if (ds==",") s.replace(".",",");
            if (ds==".") s.replace(",",".");

            QString stroka = "";
            ui->tableRaschet->setRowCount(1);
            ui->tableRaschet->setItem(0,0,new QTableWidgetItem(""));
            ui->tableRaschet->setItem(0,1,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
            ui->tableRaschet->setItem(0,2,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
            ui->tableRaschet->setItem(0,3,new QTableWidgetItem("0+0.0000"));
            ui->tableRaschet->setItem(0,4,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
            ui->tableRaschet->setItem(0,5,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
            ui->tableRaschet->item(0,3)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
            ui->tableRaschet->item(0,4)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
            ui->tableRaschet->item(0,5)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
            for (int i = 0; i < s.split("\n").count()-1;i++ ){
                stroka = s.split("\n").at(i);
                //                qDebug()<<stroka.split("\t").count();
                if (stroka.split("\t").count()==3)
                {
                    if (i>0) ui->tableRaschet->insertRow(i);
                    ui->tableRaschet->setItem(i,0,new QTableWidgetItem(stroka.split("\t").at(0))); //Name
                    ui->tableRaschet->setItem(i,1,new QTableWidgetItem(stroka.split("\t").at(1))); //X
                    ui->tableRaschet->setItem(i,2,new QTableWidgetItem(stroka.split("\t").at(2))); //Y
                    ui->tableRaschet->setItem(i,3,new QTableWidgetItem("0+0.0000"));
                    ui->tableRaschet->setItem(i,4,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
                    ui->tableRaschet->setItem(i,5,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
                    ui->tableRaschet->item(i,3)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                    ui->tableRaschet->item(i,4)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                    ui->tableRaschet->item(i,5)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                    //                qDebug() <<i<<s.split("\n").at(i)<< "Результат:" << pointname<<pointx<<pointy;
                }
            }
        }

        if (selectedItem->text()=="Удалить строку") ui->tableRaschet->removeRow(cRow);
        if (selectedItem->text()=="Вставить строку выше") {
            ui->tableRaschet->insertRow(cRow);
            ui->tableRaschet->setItem(cRow,0,new QTableWidgetItem(""));
            ui->tableRaschet->setItem(cRow,1,new QTableWidgetItem(""));
            ui->tableRaschet->setItem(cRow,2,new QTableWidgetItem(""));
            ui->tableRaschet->setItem(cRow,3,new QTableWidgetItem("0+0.0000"));
            ui->tableRaschet->setItem(cRow,4,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
            ui->tableRaschet->setItem(cRow,5,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
        }
        if (selectedItem->text()=="Вставить строку ниже") {
            ui->tableRaschet->insertRow(cRow+1 );
            ui->tableRaschet->setItem(cRow+1,0,new QTableWidgetItem(""));
            ui->tableRaschet->setItem(cRow+1,1,new QTableWidgetItem(""));
            ui->tableRaschet->setItem(cRow+1,2,new QTableWidgetItem(""));
            ui->tableRaschet->setItem(cRow+1,3,new QTableWidgetItem("0+0.0000"));
            ui->tableRaschet->setItem(cRow+1,4,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
            ui->tableRaschet->setItem(cRow+1,5,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
        }

        QStringList label;
        for (int i = 0; i < ui->tableRaschet->rowCount();i++ ){
            label.append(QString::number(i+1));
        }
        ui->tableRaschet->setVerticalHeaderLabels(label);
    }
}

void MainWindow::on_tablePred_Raschet_customContextMenuRequested(const QPoint &pos)
{
    QMenu *menu=new QMenu(this);
    menu->addAction(new QAction("Вставить строку выше", this));
    menu->addAction(new QAction("Вставить строку ниже", this));
    menu->addAction(new QAction("Удалить строку", this));
    menu->addSeparator();
    menu->addAction(new QAction("Копировать все (для Excel)", this));
    menu->addAction(new QAction("Вставить все (для Excel)", this));
    menu->addSeparator();
    menu->addAction(new QAction("Очистить", this));

    QString ds = QString::number(0.1).at(1);

    QAction* selectedItem= menu->exec(ui->tablePred_Raschet->viewport()->mapToGlobal(pos));
    if (selectedItem)
    {
        QClipboard *clipboard = QApplication::clipboard();
        QString s = "";
        qDebug()<<selectedItem->text();
        int cRow = ui->tablePred_Raschet->selectionModel()->currentIndex().row();
        if (selectedItem->text()=="Очистить") {
            ui->tablePred_Raschet->setRowCount(1);
            ui->tablePred_Raschet->setItem(0,0,new QTableWidgetItem(""));
            ui->tablePred_Raschet->setItem(0,1,new QTableWidgetItem("0+0.0000"));                       //ПК
            ui->tablePred_Raschet->setItem(0,2,new QTableWidgetItem(QString::number( 0,   'f', 4 )));   //См
            ui->tablePred_Raschet->setItem(0,3,new QTableWidgetItem(QString::number( 0,   'f', 4 )));   //X
            ui->tablePred_Raschet->setItem(0,4,new QTableWidgetItem(QString::number( 0,   'f', 4 )));   //Y
            ui->tablePred_Raschet->setItem(0,5,new QTableWidgetItem(QString::number( 0,   'f', 4 )));   //H
            ui->tablePred_Raschet->item(0,0)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled);
            ui->tablePred_Raschet->item(0,1)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled);
            ui->tablePred_Raschet->item(0,2)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled);
            ui->tablePred_Raschet->item(0,3)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
            ui->tablePred_Raschet->item(0,4)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
            ui->tablePred_Raschet->item(0,5)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
        }
        if (selectedItem->text()=="Копировать все (для Excel)") {
            for (int i = 0; i < ui->tablePred_Raschet->rowCount();i++ ){
                ui->tablePred_Raschet->item(i,0)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled);
                ui->tablePred_Raschet->item(i,1)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled);
                ui->tablePred_Raschet->item(i,2)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled);
                ui->tablePred_Raschet->item(i,3)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
                ui->tablePred_Raschet->item(i,4)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
                ui->tablePred_Raschet->item(i,5)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
                s += ui->tablePred_Raschet->item(i,0)->text()+"\t";
                s += ui->tablePred_Raschet->item(i,1)->text()+"\t";
                s += ui->tablePred_Raschet->item(i,2)->text()+"\t";
                s += ui->tablePred_Raschet->item(i,3)->text()+"\t";
                s += ui->tablePred_Raschet->item(i,4)->text()+"\t";
                s += ui->tablePred_Raschet->item(i,5)->text()+"\n";
            }
            clipboard->setText(s);
        }

        if (selectedItem->text()=="Вставить все (для Excel)") {
            s = clipboard->text();
            if (ds==",") s.replace(".",",");
            if (ds==".") s.replace(",",".");

            QString stroka = "";
            ui->tablePred_Raschet->setRowCount(1);
            ui->tablePred_Raschet->setItem(0,0,new QTableWidgetItem(""));
            ui->tablePred_Raschet->setItem(0,1,new QTableWidgetItem("0+0.0000"));                       //ПК
            ui->tablePred_Raschet->setItem(0,2,new QTableWidgetItem(QString::number( 0,   'f', 4 )));   //См
            ui->tablePred_Raschet->setItem(0,3,new QTableWidgetItem(QString::number( 0,   'f', 4 )));   //X
            ui->tablePred_Raschet->setItem(0,4,new QTableWidgetItem(QString::number( 0,   'f', 4 )));   //Y
            ui->tablePred_Raschet->setItem(0,5,new QTableWidgetItem(QString::number( 0,   'f', 4 )));   //H
            ui->tablePred_Raschet->item(0,0)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled);
            ui->tablePred_Raschet->item(0,1)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled);
            ui->tablePred_Raschet->item(0,2)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled);
            ui->tablePred_Raschet->item(0,3)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
            ui->tablePred_Raschet->item(0,4)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
            ui->tablePred_Raschet->item(0,5)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);

            for (int i = 0; i < s.split("\n").count()-1;i++ ){
                stroka = s.split("\n").at(i);
                //                qDebug()<<stroka.split("\t").count();
                if (stroka.split("\t").count()==3)
                {
                    if (i>0) ui->tablePred_Raschet->insertRow(i);
                    ui->tablePred_Raschet->setItem(i,0,new QTableWidgetItem(stroka.split("\t").at(0)));         //Name
                    ui->tablePred_Raschet->setItem(i,1,new QTableWidgetItem(stroka.split("\t").at(1)));         //Пк
                    ui->tablePred_Raschet->setItem(i,2,new QTableWidgetItem(stroka.split("\t").at(2)));         //См
                    ui->tablePred_Raschet->setItem(i,3,new QTableWidgetItem(QString::number( 0,   'f', 4 )));   //X
                    ui->tablePred_Raschet->setItem(i,4,new QTableWidgetItem(QString::number( 0,   'f', 4 )));   //Y
                    ui->tablePred_Raschet->setItem(i,5,new QTableWidgetItem(QString::number( 0,   'f', 4 )));   //H
                    ui->tablePred_Raschet->item(i,0)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled);
                    ui->tablePred_Raschet->item(i,1)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled);
                    ui->tablePred_Raschet->item(i,2)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled);
                    ui->tablePred_Raschet->item(i,3)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                    ui->tablePred_Raschet->item(i,4)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                    ui->tablePred_Raschet->item(i,5)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                }
            }
        }

        if (selectedItem->text()=="Удалить строку") ui->tablePred_Raschet->removeRow(cRow);
        if (selectedItem->text()=="Вставить строку выше") {
            ui->tablePred_Raschet->insertRow(cRow);
            ui->tablePred_Raschet->setItem(cRow,0,new QTableWidgetItem(""));
            ui->tablePred_Raschet->setItem(cRow,1,new QTableWidgetItem(""));
            ui->tablePred_Raschet->setItem(cRow,2,new QTableWidgetItem(""));
            ui->tablePred_Raschet->setItem(cRow,3,new QTableWidgetItem("0+0.0000"));
            ui->tablePred_Raschet->setItem(cRow,4,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
            ui->tablePred_Raschet->setItem(cRow,5,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
        }
        if (selectedItem->text()=="Вставить строку ниже") {
            ui->tablePred_Raschet->insertRow(cRow+1 );
            ui->tablePred_Raschet->setItem(cRow+1,0,new QTableWidgetItem(""));
            ui->tablePred_Raschet->setItem(cRow+1,1,new QTableWidgetItem(""));
            ui->tablePred_Raschet->setItem(cRow+1,2,new QTableWidgetItem(""));
            ui->tablePred_Raschet->setItem(cRow+1,3,new QTableWidgetItem("0+0.0000"));
            ui->tablePred_Raschet->setItem(cRow+1,4,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
            ui->tablePred_Raschet->setItem(cRow+1,5,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
        }

        QStringList label;
        for (int i = 0; i < ui->tablePred_Raschet->rowCount();i++ )
            label.append(QString::number(i+1));
        ui->tablePred_Raschet->setVerticalHeaderLabels(label);
    }
}



void MainWindow::on_Button_Primer_clicked()
{
    on_action_demo_triggered();
}

void MainWindow::addRowtableTrassa(int row)
{
    ui->tableTrassa->insertRow(row+1);
    ui->tableTrassa->setItem(row+1,0,new QTableWidgetItem("Прямая"));
    ui->tableTrassa->setItem(row+1,1,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
    ui->tableTrassa->setItem(row+1,2,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
    ui->tableTrassa->setItem(row+1,3,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
    ui->tableTrassa->setItem(row+1,4,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
    ui->tableTrassa->setItem(row+1,5,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
    ui->tableTrassa->setItem(row+1,6,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
    ui->tableTrassa->setItem(row+1,7,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
    ui->tableTrassa->item(row+1,5)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
    ui->tableTrassa->item(row+1,6)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
    ui->tableTrassa->item(row+1,7)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
}



void MainWindow::on_tableTrassa_customContextMenuRequested(const QPoint &pos)
{
    QMenu *menu=new QMenu(this);
    menu->addAction(new QAction("Добавить", this));
    menu->addAction(new QAction("Удалить", this));
    menu->addSeparator();
    menu->addAction(new QAction("Очистить", this));

    QAction* selectedItem = menu->exec(ui->tableTrassa->viewport()->mapToGlobal(pos));

    int cRow = ui->tableTrassa->selectionModel()->currentIndex().row();

    if (selectedItem)
    {
        if (selectedItem->text()=="Добавить") {
            addRowtableTrassa(cRow);
        }
        if (selectedItem->text()=="Удалить") {
            if (ui->tableTrassa->rowCount()>1){
                ui->tableTrassa->removeRow(cRow);
                QStringList label;
                for (int i = 0; i < ui->tableTrassa->rowCount();i++ ){
                    label.append(QString::number(i+1));
                }
                ui->tableTrassa->setVerticalHeaderLabels(label);
            }
        }
        if (selectedItem->text()=="Очистить") {
            ui->tableTrassa->setRowCount(1);
            ui->tableTrassa->setItem(0,0,new QTableWidgetItem(""));
            ui->tableTrassa->setItem(0,1,new QTableWidgetItem(""));
            ui->tableTrassa->setItem(0,2,new QTableWidgetItem(""));
            ui->tableTrassa->setItem(0,3,new QTableWidgetItem(""));
            ui->tableTrassa->setItem(0,4,new QTableWidgetItem(""));
            ui->tableTrassa->setItem(0,5,new QTableWidgetItem(""));
            ui->tableTrassa->setItem(0,6,new QTableWidgetItem(""));
            ui->tableTrassa->setItem(0,7,new QTableWidgetItem(""));
        }
    }
}

void MainWindow::addRowtableTrassa_2(int row)
{
    ui->tableTrassa_2->insertRow(row+1);
    ui->tableTrassa_2->setItem(row+1,0,new QTableWidgetItem("0+0.0000")); //PkNach
    ui->tableTrassa_2->setItem(row+1,1,new QTableWidgetItem(QString::number( 0,   'f', 4 )));   //LpNach
    ui->tableTrassa_2->setItem(row+1,2,new QTableWidgetItem(QString::number( 0,   'f', 4 )));   //CNach
    ui->tableTrassa_2->setItem(row+1,3,new QTableWidgetItem(QString::number( 0,   'f', 4 )));   //q
    ui->tableTrassa_2->setItem(row+1,4,new QTableWidgetItem(QString::number( 0,   'f', 4 )));   //z
    ui->tableTrassa_2->setItem(row+1,5,new QTableWidgetItem("0+0.0000"));                       //PkKon
    ui->tableTrassa_2->setItem(row+1,6,new QTableWidgetItem(QString::number( 0,   'f', 4 )));   //LpKon
    ui->tableTrassa_2->setItem(row+1,7,new QTableWidgetItem(QString::number( 0,   'f', 4 )));   //CKon
    ui->tableTrassa_2->setItem(row+1,8,new QTableWidgetItem(QString::number( 0,   'f', 3 )));   //h
    QStringList label;
    for (int i = 0; i < ui->tableTrassa_2->rowCount();i++ ){
        label.append(QString::number(i+1));
    }
    ui->tableTrassa_2->setVerticalHeaderLabels(label);
}

void MainWindow::on_tableTrassa_2_customContextMenuRequested(const QPoint &pos)
{
    QMenu *menu=new QMenu(this);
    menu->addAction(new QAction("Добавить", this));
    menu->addAction(new QAction("Удалить", this));
    menu->addSeparator();
    menu->addAction(new QAction("Очистить", this));
    menu->addSeparator();
    menu->addAction(new QAction("Вычислить q", this));
    menu->addAction(new QAction("Вычислить z", this));

    QAction* selectedItem= menu->exec(ui->tableTrassa_2->viewport()->mapToGlobal(pos));
    int row = ui->tableTrassa_2->selectionModel()->currentIndex().row();
    int cRow = ui->tableTrassa_2->rowCount();
    double L;
    double C;
    double z;
    double tz;
    if (selectedItem)
    {
        qDebug()<<"чтото не так:"<<selectedItem->text();

        if (selectedItem->text()=="Добавить") {
            addRowtableTrassa_2(row);
        }

        if (selectedItem->text()=="Удалить") {
            if (cRow>1)
            {
                ui->tableTrassa_2->removeRow(row);
                QStringList label;
                for (int i = 0; i < ui->tableTrassa_2->rowCount();i++ ){
                    label.append(QString::number(i+1));
                }
                ui->tableTrassa_2->setVerticalHeaderLabels(label);
            }
        }

        if (selectedItem->text()=="Очистить") {
            ui->tableTrassa_2->setRowCount(1);
            ui->tableTrassa_2->setItem(0,0,new QTableWidgetItem(""));
            ui->tableTrassa_2->setItem(0,1,new QTableWidgetItem(""));
            ui->tableTrassa_2->setItem(0,2,new QTableWidgetItem(""));
            ui->tableTrassa_2->setItem(0,3,new QTableWidgetItem(""));
            ui->tableTrassa_2->setItem(0,4,new QTableWidgetItem(""));
            ui->tableTrassa_2->setItem(0,5,new QTableWidgetItem(""));
            ui->tableTrassa_2->setItem(0,6,new QTableWidgetItem(""));
            ui->tableTrassa_2->setItem(0,7,new QTableWidgetItem(""));
            ui->tableTrassa_2->setItem(0,8,new QTableWidgetItem(""));
        }

        if (selectedItem->text()=="Вычислить z") {
            qDebug()<<"чтото не так z";
            if (ui->tableTrassa_2->item(row,1)->text().toDouble() !=0 && ui->tableTrassa_2->item(row,2)->text().toDouble() !=0
                    && ui->tableTrassa_2->item(row,1)->text().toDouble() !=0 && ui->tableTrassa_2->item(row,2)->text().toDouble() !=0  ){
                L=ui->tableTrassa_2->item(row,1)->text().toDouble();
                C=ui->tableTrassa_2->item(row,2)->text().toDouble();
                if (L!=0&&C!=0){
                    z=       (pow(L,3)      /   (24*C)) +
                            ((17*pow(L,7))  /   (2688*pow(C,3)));
                } else {
                    z=0;
                }

                L=ui->tableTrassa_2->item(row,6)->text().toDouble();
                C=ui->tableTrassa_2->item(row,7)->text().toDouble();
                if (L!=0&&C!=0){
                    tz=       (pow(L,3)      /   (24*C)) +
                            ((17*pow(L,7))  /   (2688*pow(C,3)));
                } else {
                    tz=0;
                }

                //                if (ui->tableTrassa_2->item(row,4)->text()!=""){
                QMessageBox msgBox;
                msgBox.setIcon(QMessageBox::Question);
                msgBox.setText("Текущее значение z = "+ui->tableTrassa_2->item(row,4)->text());
                msgBox.setInformativeText(tr("Вычислены: z1=")+QString::number( z,   'f', 4 )+
                                          tr(" и z2=")+QString::number( z,   'f', 4 )+
                                          tr(" Заменить это значение на среденее ?"));

                msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
                msgBox.setDefaultButton(QMessageBox::Cancel);
                int ret = msgBox.exec();
                switch (ret) {
                case QMessageBox::Ok:
                    ui->tableTrassa_2->setItem(row,4,new QTableWidgetItem(QString::number( (z+tz)/2,   'f', 4 )));   //z
                    break;
                }
                //                }
            }
        }

        if (selectedItem->text()=="Вычислить q") {
            QDesktopWidget desktop;
            QRect rect = desktop.availableGeometry(desktop.primaryScreen()); // прямоугольник с размерами экрана
            QPoint center = rect.center(); //координаты центра экрана

            CalcQ *calc_q = new CalcQ(this);
            int x = center.x() - (calc_q->width()/2);  // учитываем половину ширины окна
            int y = center.y() - (calc_q->height()/2); // .. половину высоты
            center.setX(x);
            center.setY(y);
            calc_q->move(center);
            calc_q->setModal(true);
            calc_q->calc();
            calc_q->exec();
            QMessageBox msgBox;
            if (calc_q->q !=0 ){
                qDebug()<<"Вычислил:"<<calc_q->q;
                if (ui->tableTrassa_2->item(row,3)->text()!=""){
                    msgBox.setText("Текущее значение q = "+ui->tableTrassa_2->item(row,3)->text());
                    msgBox.setInformativeText("Заменить это значение на "+QString::number( calc_q->q,   'f', 4 )+" ?");
                    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
                    msgBox.setDefaultButton(QMessageBox::Cancel);
                    int ret = msgBox.exec();
                    switch (ret) {
                    case QMessageBox::Ok:
                        ui->tableTrassa_2->setItem(row,3,new QTableWidgetItem(QString::number( calc_q->q,   'f', 4 )));   //q
                        break;
                    case QMessageBox::Cancel:
                        qDebug()<<"Нажали канцел";
                        break;
                    }
                }
            }else{
                qDebug()<< "calc_q" <<calc_q->q;
                //ui->tableTrassa_2->setItem(row,3,new QTableWidgetItem(QString::number( calc_q->q,   'f', 4 )));   //q
            }

        }

    }
}

void MainWindow::on_tableProfil_customContextMenuRequested(const QPoint &pos){
    QMenu *menu=new QMenu(this);
    menu->addAction(new QAction("Добавить", this));
    menu->addAction(new QAction("Удалить", this));
    menu->addSeparator();
    menu->addAction(new QAction("Очистить", this));

    QAction* selectedItem= menu->exec(ui->tableProfil->viewport()->mapToGlobal(pos));
    int row = ui->tableProfil->selectionModel()->currentIndex().row();
    int cRow = ui->tableProfil->rowCount();

    if (selectedItem)
    {
        if (selectedItem->text()=="Добавить") {
            ui->tableProfil->insertRow(row+1);
            ui->tableProfil->setItem( row+1,0,new QTableWidgetItem(""));
            ui->tableProfil->setItem( row+1,1,new QTableWidgetItem(""));
            ui->tableProfil->setItem( row+1,2,new QTableWidgetItem(""));
            ui->tableProfil->setItem( row+1,3,new QTableWidgetItem(""));
            ui->tableProfil->setItem( row+1,4,new QTableWidgetItem(""));
            ui->tableProfil->setItem( row+1,5,new QTableWidgetItem(""));
        }

        if (selectedItem->text()=="Удалить") {
            if (cRow>1)
            {
                ui->tableProfil->removeRow(row);
                QStringList label;
                for (int i = 0; i < ui->tableProfil->rowCount();i++ ){
                    label.append(QString::number(i+1));
                }
                ui->tableProfil->setVerticalHeaderLabels(label);
            }
        }

        if (selectedItem->text()=="Очистить") {
            ui->tableProfil->setRowCount(1);
            ui->tableProfil->setItem( 0,0,new QTableWidgetItem(""));
            ui->tableProfil->setItem( 0,1,new QTableWidgetItem(""));
            ui->tableProfil->setItem( 0,2,new QTableWidgetItem(""));
            ui->tableProfil->setItem( 0,3,new QTableWidgetItem(""));
            ui->tableProfil->setItem( 0,4,new QTableWidgetItem(""));
            ui->tableProfil->setItem( 0,5,new QTableWidgetItem(""));
        }
    }
}

void MainWindow::on_tableNPK_customContextMenuRequested(const QPoint &pos){
    QMenu *menu=new QMenu(this);
    menu->addAction(new QAction("Добавить", this));
    menu->addAction(new QAction("Удалить", this));
    menu->addSeparator();
    menu->addAction(new QAction("Очистить", this));

    QAction* selectedItem= menu->exec(ui->tableNPK->viewport()->mapToGlobal(pos));
    int row = ui->tableNPK->selectionModel()->currentIndex().row();
    int cRow = ui->tableNPK->rowCount();

    if (selectedItem)
    {
        if (selectedItem->text()=="Добавить") {
            ui->tableNPK->insertRow(row+1);
            ui->tableNPK->setItem( row+1,0,new QTableWidgetItem(QString::number( 0,   'f', 0 ))); //Pk
            ui->tableNPK->setItem( row+1,1,new QTableWidgetItem(QString::number( 100,   'f', 4 ))); //L
        }

        if (selectedItem->text()=="Удалить") {
            if (cRow>1)
            {
                ui->tableNPK->removeRow(row);
                QStringList label;
                for (int i = 0; i < ui->tableNPK->rowCount();i++ ){
                    label.append(QString::number(i+1));
                }
                ui->tableNPK->setVerticalHeaderLabels(label);
            }
        }

        if (selectedItem->text()=="Очистить") {
            ui->tableNPK->setRowCount(1);
            ui->tableNPK->setItem( 0,0,new QTableWidgetItem(QString::number( 0,   'f', 0 ))); //Pk
            ui->tableNPK->setItem( 0,1,new QTableWidgetItem(QString::number( 100,   'f', 4 ))); //L
        }

    }
}





void MainWindow::on_Button_Predraschet_clicked()
{
    geo zadacha;

    qDebug() << "Считываем длинну Стандартного пикета";
    zadacha.PkStnd = ui->EditStndPk->text().toDouble(); //Считываем длинну Стандартного пикета


    //Ввод таблицы неправильного пикета
    qDebug() << "Ввод таблицы неправильного пикета";
    for (int i=0;i<(ui->tableNPK->rowCount());i++){
        zadacha.appendNpk(ui->tableNPK->item(i,0)->text(),ui->tableNPK->item(i,1)->text());
    }

    //Ввод таблицы трасса
    qDebug() << "Ввод таблицы трасса";
    //QString KrLine,QString PKn,QString Xn,QString Yn,QString Xk,QString Yk,QString Xck,QString Yck)
    //    QString tS;
    for (int i = 0; i < ui->tableTrassa->rowCount();i++ ){
        //Проверяем, сооответствие введенных данных.
        ui->tableTrassa->item(i,1)->setText(QString::number(ui->tableTrassa->item(i,1)->text().toDouble(),'f',4)); //Xk
        ui->tableTrassa->item(i,2)->setText(QString::number(ui->tableTrassa->item(i,2)->text().toDouble(),'f',4)); //Yk
        if (ui->tableTrassa->item(i,0)->text()=="Прямая") {
            ui->tableTrassa->item(i,3)->setText(QString::number(0,'f',4)); //Xck
            ui->tableTrassa->item(i,4)->setText(QString::number(0,'f',4)); //Yck
        }
        if (ui->tableTrassa->item(i,0)->text()=="Кривая") {
            ui->tableTrassa->item(i,3)->setText(QString::number(ui->tableTrassa->item(i,3)->text().toDouble(),'f',4)); //Xck
            ui->tableTrassa->item(i,4)->setText(QString::number(ui->tableTrassa->item(i,4)->text().toDouble(),'f',4)); //Yck
        }

        zadacha.appendTrassa(ui->tableTrassa->item(i,0)->text(), //type
                             ui->EditPKnach->text(), //ПК начала трассы
                             ui->EditXnach->text(),  //X начала трассы
                             ui->EditYnach->text(),  //Y начала трассы
                             ui->tableTrassa->item(i,1)->text(), //Xk
                             ui->tableTrassa->item(i,2)->text(), //Yk
                             ui->tableTrassa->item(i,3)->text(), //Xck
                             ui->tableTrassa->item(i,4)->text());//Yck
    }
    //Ввод таблицы Переходных кривых
    qDebug() << "Ввод таблицы Переходных кривых";
    for (int i = 0; i < ui->tableTrassa_2->rowCount();i++ ){
        //Проверяем, сооответствие введенных данных.
        ui->tableTrassa_2->item(i,1)->setText(QString::number(ui->tableTrassa_2->item(i,1)->text().toDouble(),'f',4)); //Lin
        ui->tableTrassa_2->item(i,2)->setText(QString::number(ui->tableTrassa_2->item(i,2)->text().toDouble(),'f',4)); //Cin
        ui->tableTrassa_2->item(i,3)->setText(QString::number(ui->tableTrassa_2->item(i,3)->text().toDouble(),'f',4)); //q
        ui->tableTrassa_2->item(i,4)->setText(QString::number(ui->tableTrassa_2->item(i,4)->text().toDouble(),'f',4)); //z
        ui->tableTrassa_2->item(i,6)->setText(QString::number(ui->tableTrassa_2->item(i,6)->text().toDouble(),'f',4)); //Lout
        ui->tableTrassa_2->item(i,7)->setText(QString::number(ui->tableTrassa_2->item(i,7)->text().toDouble(),'f',4)); //Cout
        ui->tableTrassa_2->item(i,8)->setText(QString::number(ui->tableTrassa_2->item(i,8)->text().toDouble(),'f',3)); //h

        zadacha.appendPerehod(ui->tableTrassa_2->item(i,0)->text(), //PKin
                              ui->tableTrassa_2->item(i,1)->text(), //Lin
                              ui->tableTrassa_2->item(i,2)->text(), //Cin
                              ui->tableTrassa_2->item(i,3)->text(), //q
                              ui->tableTrassa_2->item(i,4)->text(), //z
                              ui->tableTrassa_2->item(i,5)->text(), //PKout
                              ui->tableTrassa_2->item(i,6)->text(), //Lout
                              ui->tableTrassa_2->item(i,7)->text()); //Cout
    }

    //Ввод таблицы Профиля
    for (int i = 0; i < ui->tableProfil->rowCount();i++ ){
        //Проверяем, сооответствие введенных данных.
        ui->tableProfil->item(i,1)->setText(QString::number(ui->tableProfil->item(i,1)->text().toDouble(),'f',4)); //H
        ui->tableProfil->item(i,2)->setText(QString::number(ui->tableProfil->item(i,2)->text().toDouble(),'f',4)); //R

        zadacha.appendProfil(ui->tableProfil->item(i,0)->text(),  //PK
                             ui->tableProfil->item(i,1)->text(),  //H
                             ui->tableProfil->item(i,2)->text()); //R
    }
    qDebug() << "Предрасчет";
    zadacha.predRaschet();
    //Считываем результат из таблицы трассы
    for (int i = 0; i < ui->tableTrassa->rowCount();i++ ){
        ui->tableTrassa->item(i,5)->setText(zadacha.GetPK2L(zadacha.TrassaPKnach.value(i),zadacha.TrassaL.value(i).toDouble(),zadacha.PkStnd,zadacha.Npk,zadacha.NpL));  //ПК
        ui->tableTrassa->item(i,6)->setText(zadacha.TrassaL.value(i));       //L
        ui->tableTrassa->item(i,7)->setText(zadacha.TrassaRadius.value(i));  //R
    }
    //Считываем результат из таблицы профиля
    for (int i = 0; i < ui->tableProfil->rowCount();i++ ){
        //ui->tableTrassa_2->setItem(row+1,2,);   //CNach
        if (ui->tableProfil->item(i,1)->text().toDouble()==0){
            ui->tableProfil->setItem(i,1,new QTableWidgetItem(zadacha.ProfilH.value(i)));  //Отметка
        }
        ui->tableProfil->setItem(i,3,new QTableWidgetItem(zadacha.ProfilT.value(i)));  //Тангенс
        ui->tableProfil->setItem(i,4,new QTableWidgetItem(zadacha.Profili.value(i)));  //Уклон
        ui->tableProfil->setItem(i,5,new QTableWidgetItem(zadacha.ProfilL.value(i)));  //Радиус
        ui->tableProfil->item(i,3)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
        ui->tableProfil->item(i,4)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
        ui->tableProfil->item(i,5)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
    }

    //Для разбивки трассы
    if (ui->lineEdit_RazbivkaPKnach->text() =="0+0.0000") ui->lineEdit_RazbivkaPKnach->setText(ui->EditPKnach->text());
    QString pkKon;
    if (ui->lineEdit_RazbivkaPKkon->text()  =="0+0.0000") {
        pkKon = ui->tableTrassa->item(ui->tableTrassa->rowCount()-1,5)->text().split("+").at(0)
                + "+" +
                QString::number(ui->tableTrassa->item(ui->tableTrassa->rowCount()-1,5)->text().split("+").at(1).toDouble()-0.001,'f',3);
        ui->lineEdit_RazbivkaPKkon->setText(pkKon);
    }
}


void MainWindow::on_table_Kolca_customContextMenuRequested(const QPoint &pos)
{
    QMenu *menu=new QMenu(this);
    menu->addAction(new QAction("Добавить", this));
    menu->addAction(new QAction("Удалить", this));
    menu->addSeparator();
    menu->addAction(new QAction("Копировать все (для Excel)", this));
    menu->addAction(new QAction("Вставить все (для Excel)", this));
    menu->addSeparator();
    menu->addAction(new QAction("Очистить", this));

    QAction* selectedItem= menu->exec(ui->table_Kolca->viewport()->mapToGlobal(pos));
    int row  = ui->table_Kolca->selectionModel()->currentIndex().row();
    int cRow = ui->table_Kolca->rowCount();

    if (selectedItem)
    {
        qDebug()<<selectedItem->text();
        if (selectedItem->text()=="Добавить") {
            ui->table_Kolca->insertRow(row+1);

            ui->table_Kolca->setItem(0,8,new QTableWidgetItem(""));
            ui->table_Kolca->setItem(0,9,new QTableWidgetItem(""));
            ui->table_Kolca->setItem(0,10,new QTableWidgetItem(""));
            ui->table_Kolca->setItem(0,11,new QTableWidgetItem(""));
            ui->table_Kolca->setItem(0,12,new QTableWidgetItem(""));
            ui->table_Kolca->setItem(0,13,new QTableWidgetItem(""));
            ui->table_Kolca->item(0,8)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
            ui->table_Kolca->item(0,9)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
            ui->table_Kolca->item(0,10)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
            ui->table_Kolca->item(0,11)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
            ui->table_Kolca->item(0,12)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
            ui->table_Kolca->item(0,13)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
        }

        if (selectedItem->text()=="Удалить") {
            if (cRow>1)
            {
                ui->table_Kolca->removeRow(row);
                QStringList label;
                for (int i = 0; i < ui->table_Kolca->rowCount();i++ ){
                    label.append(QString::number(i+1));
                }
                ui->table_Kolca->setVerticalHeaderLabels(label);
            }
        }
        QString ds = QString::number(0.1).at(1);
        QClipboard *clipboard = QApplication::clipboard();
        QString s = "";

        //        0 	1	2	3   4       5       6       7       8   9       10          11      12      13
        //        №№	№R  X	Y	Hфакт	D(1+4)	B=1,75	R пр	ПК	Смещ	HпрУГР  	Hпр 	Rфакт	Откл.

        if (selectedItem->text()=="Копировать все (для Excel)") {
            for (int i = 0; i < ui->table_Kolca->rowCount();i++ ){
                ui->table_Kolca->item(i,8)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
                ui->table_Kolca->item(i,9)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
                ui->table_Kolca->item(i,10)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
                ui->table_Kolca->item(i,11)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
                ui->table_Kolca->item(i,12)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
                ui->table_Kolca->item(i,13)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
                s += ui->table_Kolca->item(i,0)->text()+"\t";
                s += ui->table_Kolca->item(i,1)->text()+"\t";
                s += ui->table_Kolca->item(i,2)->text()+"\t";
                s += ui->table_Kolca->item(i,3)->text()+"\t";
                s += ui->table_Kolca->item(i,4)->text()+"\t";
                s += ui->table_Kolca->item(i,5)->text()+"\t";
                s += ui->table_Kolca->item(i,6)->text()+"\t";
                s += ui->table_Kolca->item(i,7)->text()+"\t";
                s += ui->table_Kolca->item(i,8)->text()+"\t";
                s += ui->table_Kolca->item(i,9)->text()+"\t";
                s += ui->table_Kolca->item(i,10)->text()+"\t";
                s += ui->table_Kolca->item(i,11)->text()+"\t";
                s += ui->table_Kolca->item(i,12)->text()+"\t";
                s += ui->table_Kolca->item(i,13)->text()+"\n";
            }
            clipboard->setText(s);
        }
        //        0 	1	2	3   4       5       6       7       8   9       10          11      12      13
        //        №№	№R  X	Y	Hфакт	D(1+4)	B=1,75	R пр	ПК	Смещ	HпрУГР  	Hпр 	Rфакт	Откл.

        if (selectedItem->text()=="Вставить все (для Excel)") {
            s = clipboard->text();
            if (ds==",") {s.replace(".",",");} else {s.replace(",",".");}
            QString stroka = "";
            //            QString nameR = "";

            ui->table_Kolca->setRowCount(1);
            for (int i = 0; i < s.split("\n").count()-1;i++ ){
                stroka = s.split("\n").at(i);
                qDebug()<<stroka.split("\t").count();
                if (stroka.split("\t").count()==8)
                {
                    if (i>0){
                        ui->table_Kolca->insertRow(i);
                    }
                    ui->table_Kolca->setItem(i,0,new QTableWidgetItem(stroka.split("\t").at(0))); //NKolca
                    ui->table_Kolca->setItem(i,1,new QTableWidgetItem(stroka.split("\t").at(1))); //NR
                    ui->table_Kolca->setItem(i,2,new QTableWidgetItem(stroka.split("\t").at(2))); //X
                    ui->table_Kolca->setItem(i,3,new QTableWidgetItem(stroka.split("\t").at(3))); //Y
                    ui->table_Kolca->setItem(i,4,new QTableWidgetItem(stroka.split("\t").at(4))); //H
                    ui->table_Kolca->setItem(i,5,new QTableWidgetItem(stroka.split("\t").at(5))); //D(1+4)
                    ui->table_Kolca->setItem(i,6,new QTableWidgetItem(stroka.split("\t").at(6))); //B=1.75
                    ui->table_Kolca->setItem(i,7,new QTableWidgetItem(stroka.split("\t").at(7))); //Rпр
                    ui->table_Kolca->setItem(i,8,new QTableWidgetItem(""));
                    ui->table_Kolca->setItem(i,9,new QTableWidgetItem(""));
                    ui->table_Kolca->setItem(i,10,new QTableWidgetItem(""));
                    ui->table_Kolca->setItem(i,11,new QTableWidgetItem(""));
                    ui->table_Kolca->setItem(i,12,new QTableWidgetItem(""));
                    ui->table_Kolca->setItem(i,13,new QTableWidgetItem(""));
                    ui->table_Kolca->item(i,13)->setTextColor(QColor(0,0,0));
                    ui->table_Kolca->item(i,8)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                    ui->table_Kolca->item(i,9)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                    ui->table_Kolca->item(i,10)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                    ui->table_Kolca->item(i,11)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                    ui->table_Kolca->item(i,12)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                    ui->table_Kolca->item(i,13)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                } else {
                    statusBar()->showMessage(tr("Ошибка! Вставляемых столбцов должно быть = 8"), 5000);
                }

            }
        }

        if (selectedItem->text()=="Очистить") {
            ui->table_Kolca->setRowCount(1);
            ui->table_Kolca->setItem(0,0,new QTableWidgetItem(""));
            ui->table_Kolca->setItem(0,1,new QTableWidgetItem(""));
            ui->table_Kolca->setItem(0,2,new QTableWidgetItem(""));
            ui->table_Kolca->setItem(0,3,new QTableWidgetItem(""));
            ui->table_Kolca->setItem(0,4,new QTableWidgetItem(""));
            ui->table_Kolca->setItem(0,5,new QTableWidgetItem(""));
            ui->table_Kolca->setItem(0,6,new QTableWidgetItem(""));
            ui->table_Kolca->setItem(0,7,new QTableWidgetItem(""));
            ui->table_Kolca->setItem(0,8,new QTableWidgetItem(""));
            ui->table_Kolca->setItem(0,9,new QTableWidgetItem(""));
            ui->table_Kolca->setItem(0,10,new QTableWidgetItem(""));
            ui->table_Kolca->setItem(0,11,new QTableWidgetItem(""));
            ui->table_Kolca->setItem(0,12,new QTableWidgetItem(""));
            ui->table_Kolca->setItem(0,13,new QTableWidgetItem(""));
            ui->table_Kolca->item(0,8)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
            ui->table_Kolca->item(0,9)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
            ui->table_Kolca->item(0,10)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
            ui->table_Kolca->item(0,11)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
            ui->table_Kolca->item(0,12)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
            ui->table_Kolca->item(0,13)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);

        }
    }
}




//Предрасчет
void MainWindow::on_pushButton_Pred_Raschet_run_clicked()
{
    geo zadacha;
    qDebug() << "Считываем длинну Стандартного пикета";
    zadacha.PkStnd = ui->EditStndPk->text().toDouble(); //Считываем длинну Стандартного пикета

    //Ввод таблицы неправильного пикета
    qDebug() << "Ввод таблицы неправильного пикета";
    for (int i=0;i<(ui->tableNPK->rowCount());i++){
        zadacha.appendNpk(ui->tableNPK->item(i,0)->text(),ui->tableNPK->item(i,1)->text());
    }

    //Ввод таблицы трасса
    qDebug() << "Ввод таблицы трасса";
    //QString KrLine,QString PKn,QString Xn,QString Yn,QString Xk,QString Yk,QString Xck,QString Yck)
    for (int i = 0; i < ui->tableTrassa->rowCount();i++ ){
        zadacha.appendTrassa(ui->tableTrassa->item(i,0)->text(), //type
                             ui->EditPKnach->text(), //ПК начала трассы
                             ui->EditXnach->text(),  //X начала трассы
                             ui->EditYnach->text(),  //Y начала трассы
                             ui->tableTrassa->item(i,1)->text(), //Xk
                             ui->tableTrassa->item(i,2)->text(), //Yk
                             ui->tableTrassa->item(i,3)->text(), //Xck
                             ui->tableTrassa->item(i,4)->text());//Yck
    }

    //Ввод таблицы Переходных кривых
    qDebug() << "Ввод таблицы Переходных кривых";
    for (int i = 0; i < ui->tableTrassa_2->rowCount();i++ ){
        zadacha.appendPerehod(ui->tableTrassa_2->item(i,0)->text(), //PKin
                              ui->tableTrassa_2->item(i,1)->text(), //Lin
                              ui->tableTrassa_2->item(i,2)->text(), //Cin
                              ui->tableTrassa_2->item(i,3)->text(), //q
                              ui->tableTrassa_2->item(i,4)->text(), //z
                              ui->tableTrassa_2->item(i,5)->text(), //PKout
                              ui->tableTrassa_2->item(i,6)->text(), //Lout
                              ui->tableTrassa_2->item(i,7)->text()); //Cout
    }

    //Ввод таблицы Профиля
    for (int i = 0; i < ui->tableProfil->rowCount();i++ ){
        zadacha.appendProfil(ui->tableProfil->item(i,0)->text(),
                             ui->tableProfil->item(i,1)->text(),
                             ui->tableProfil->item(i,2)->text());
    }

    qDebug() << "Предрасчет";
    zadacha.predRaschet();


    //Считаем точки
    bool xOk;
    bool yOk;

    double  xkoor;
    double  ykoor;
    QString PKpoint   = "0";
    double  SMpoint;
    double  SMpointZ;
    double  SMpointZQ;
    double  Hpoint;

    //QString RL = "L";

    //Условные координаты
    double x_usl_koor = ui->lineEdit_usl_x_2->text().toDouble(&xOk);
    double y_usl_koor = ui->lineEdit_usl_y_2->text().toDouble(&yOk);
    if ((xOk != true) || (yOk != true)){
        x_usl_koor = 0;
        y_usl_koor = 0;
    }

    //checkBox_usl_2

    if (ui->checkBox_usl_2->isChecked()==false ){
        x_usl_koor = 0;
        y_usl_koor = 0;    }

    qDebug()<<x_usl_koor<<xOk<< "  "<<y_usl_koor<<yOk;
    for (int i = 0; i < ui->tablePred_Raschet->rowCount();i++ ){
        PKpoint  = "0";
        SMpoint   = 0;
        SMpointZ  = 0;
        SMpointZQ = 0;
        Hpoint    = 0;
        xkoor     = 0;
        ykoor     = 0;

        PKpoint = ui->tablePred_Raschet->item(i,1)->text();

        switch (ui->TypeOsComboBox_2->currentIndex()) {
        case 0: //Разбивочная ось
            SMpoint   = 0.00001 + ui->tablePred_Raschet->item(i,2)->text().toDouble();
            break;
        case 1: //Ось пути +Z
            SMpointZ  = 0.00001 + ui->tablePred_Raschet->item(i,2)->text().toDouble();
            break;
        case 2: //Ось тоннеля +Z +Q
            SMpointZQ = 0.00001 + ui->tablePred_Raschet->item(i,2)->text().toDouble();
            break;
        }

        zadacha.raschetXY(PKpoint,SMpoint,SMpointZ,SMpointZQ,xkoor,ykoor);
        zadacha.raschetH(PKpoint,Hpoint);

        if ((xkoor!=0) && (ykoor!=0)) {
            ui->tablePred_Raschet->setItem(i,3,new QTableWidgetItem(QString::number( xkoor+x_usl_koor,   'f', 4 )));
            ui->tablePred_Raschet->setItem(i,4,new QTableWidgetItem(QString::number( ykoor+y_usl_koor,   'f', 4 )));
        }else {
            ui->tablePred_Raschet->setItem(i,3,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
            ui->tablePred_Raschet->setItem(i,4,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
        }
        ui->tablePred_Raschet->setItem(i,5,new QTableWidgetItem(QString::number( Hpoint,   'f', 4 )));
        ui->tablePred_Raschet->item(i,3)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled); // X
        ui->tablePred_Raschet->item(i,4)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled); // Y
        ui->tablePred_Raschet->item(i,5)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled); //Отметка с укладочной схемы
    }
    statusBar()->showMessage(tr("Предрасчет выполнен"), 3000);
}


void MainWindow::on_pushButton_RaschetColca_clicked()
{
    geo zadacha;
    qDebug() << "Считываем длинну Стандартного пикета";
    zadacha.PkStnd = ui->EditStndPk->text().toDouble(); //Считываем длинну Стандартного пикета
    //Ввод таблицы неправильного пикета
    qDebug() << "Ввод таблицы неправильного пикета";
    for (int i=0;i<(ui->tableNPK->rowCount());i++){
        zadacha.appendNpk(ui->tableNPK->item(i,0)->text(),ui->tableNPK->item(i,1)->text());
    }
    //Ввод таблицы трасса
    qDebug() << "Ввод таблицы трасса";
    //QString KrLine,QString PKn,QString Xn,QString Yn,QString Xk,QString Yk,QString Xck,QString Yck)
    for (int i = 0; i < ui->tableTrassa->rowCount();i++ ){
        zadacha.appendTrassa(ui->tableTrassa->item(i,0)->text(), //type
                             ui->EditPKnach->text(), //ПК начала трассы
                             ui->EditXnach->text(),  //X начала трассы
                             ui->EditYnach->text(),  //Y начала трассы
                             ui->tableTrassa->item(i,1)->text(), //Xk
                             ui->tableTrassa->item(i,2)->text(), //Yk
                             ui->tableTrassa->item(i,3)->text(), //Xck
                             ui->tableTrassa->item(i,4)->text());//Yck
    }
    //Ввод таблицы Переходных кривых
    qDebug() << "Ввод таблицы Переходных кривых";
    for (int i = 0; i < ui->tableTrassa_2->rowCount();i++ ){
        zadacha.appendPerehod(ui->tableTrassa_2->item(i,0)->text(), //PKin
                              ui->tableTrassa_2->item(i,1)->text(), //Lin
                              ui->tableTrassa_2->item(i,2)->text(), //Cin
                              ui->tableTrassa_2->item(i,3)->text(), //q
                              ui->tableTrassa_2->item(i,4)->text(), //z
                              ui->tableTrassa_2->item(i,5)->text(), //PKout
                              ui->tableTrassa_2->item(i,6)->text(), //Lout
                              ui->tableTrassa_2->item(i,7)->text()); //Cout
    }

    //Ввод таблицы Профиля
    for (int i = 0; i < ui->tableProfil->rowCount();i++ ){
        zadacha.appendProfil(ui->tableProfil->item(i,0)->text(),
                             ui->tableProfil->item(i,1)->text(),
                             ui->tableProfil->item(i,2)->text());
    }

    qDebug() << "Предрасчет";
    zadacha.predRaschet();


    //Считаем точки
    bool xOk;
    bool yOk;

    double xkoor = 0;
    double ykoor = 0;
    QString PKpoint  = "0";
    double SMpoint  = 0;
    double SMpointZ  = 0;
    double SMpointZQ = 0;
    double Hpoint = 0;

    QString RL = "L";

    QVector<ring> rings;
    ring xring;


    //Условные координаты
    double x_usl_koor = ui->lineEdit_usl_x_3->text().toDouble(&xOk);
    double y_usl_koor = ui->lineEdit_usl_y_3->text().toDouble(&yOk);
    if ((xOk != true) || (yOk != true)){
        x_usl_koor = 0;
        y_usl_koor = 0;
    }
    qDebug()<<x_usl_koor<<xOk<< "  "<<y_usl_koor<<yOk;


    for (int i = 0; i < ui->table_Kolca->rowCount();i++ ){
        if (ui->checkBox_usl_3->isChecked()==true ){
            xkoor = ui->table_Kolca->item(i,2)->text().toDouble(&xOk)+x_usl_koor;
            ykoor = ui->table_Kolca->item(i,3)->text().toDouble(&yOk)+y_usl_koor;
        } else {
            xkoor = ui->table_Kolca->item(i,2)->text().toDouble(&xOk);
            ykoor = ui->table_Kolca->item(i,3)->text().toDouble(&yOk);
        }

        PKpoint  = "0";
        SMpoint   = 0;
        SMpointZ  = 0;
        SMpointZQ = 0;
        Hpoint    = 0;

        //        0 	1	2	3   4       5       6       7       8   9       10          11      12      13
        //        №№	№R  X	Y	Hфакт	D(1+4)	B=1,75	R пр	ПК	Смещ	HпрУГР  	Hпр 	Rфакт	Откл.

        if ((xOk == true) && (yOk == true)){
            zadacha.raschetPkSm(xkoor,
                                ykoor,
                                PKpoint,
                                SMpoint,
                                RL);
            qDebug() << xkoor << ykoor << PKpoint << SMpoint;

            if(PKpoint!="0"){
                zadacha.raschetPer(PKpoint,
                                   SMpoint,
                                   RL,
                                   SMpointZ,
                                   SMpointZQ);

                zadacha.raschetH(PKpoint,Hpoint);
                xring.nkolca = ui->table_Kolca->item(i,0)->text().toInt(&xOk);    //Номер кольца
                xring.nr     = ui->table_Kolca->item(i,1)->text().toInt(&xOk);    //Номер радиуса

                xring.rx     = ui->table_Kolca->item(i,2)->text().toDouble(&xOk); //x факт Радиуса
                xring.ry     = ui->table_Kolca->item(i,3)->text().toDouble(&xOk); //y факт Радиуса
                xring.rh     = ui->table_Kolca->item(i,4)->text().toDouble(&yOk); //h факт Радиуса

                switch (xring.nr) {
                case 1:
                    xring.D1_4   = ui->table_Kolca->item(i,5)->text().toDouble(&xOk); //Диаметр измеренный
                    break;
                case 2:
                    xring.D1_4   = ui->table_Kolca->item(i,5)->text().toDouble(&xOk); //Диаметр измеренный
                    break;
                case 3:
                    xring.D1_4   = ui->table_Kolca->item(i,5)->text().toDouble(&xOk); //Диаметр измеренный
                    break;
                case 4:
                    xring.D1_4   = ui->table_Kolca->item(i,5)->text().toDouble(&xOk); //Диаметр измеренный
                    break;
                default:
                    xring.D1_4   = 0;
                    break;
                }

                xring.B      = ui->table_Kolca->item(i,6)->text().toDouble(&xOk); //Домер от УГР до центра кольца (1,75)
                xring.R      = ui->table_Kolca->item(i,7)->text().toDouble(&xOk);  //R     проект

                xring.rPK     = PKpoint;                                                             // ПК
                if (SMpointZQ!=0) {xring.rsm_qz = SMpointZQ; }else{if (SMpointZ!=0){xring.rsm_qz = SMpointZ;} else {xring.rsm_qz = SMpoint;} }  //Смещение от оси тоннеля
                xring.rh_ugr  = Hpoint;                                                              // H УГР
                xring.rf      = sqrt(pow((xring.rh-(xring.rh_ugr+xring.B)),2)+pow(xring.rsm_qz,2));  // Rфакт
                xring.rdiff_r = (xring.rf - xring.R)*1000;                                           // Отклонение

                ui->table_Kolca->setItem(i,8 ,new QTableWidgetItem(PKpoint));
                ui->table_Kolca->setItem(i,9 ,new QTableWidgetItem(QString::number( xring.rsm_qz,              'f', 4 )));           //смещение от оси тоннеля
                ui->table_Kolca->setItem(i,10,new QTableWidgetItem(QString::number( xring.rh_ugr,              'f', 4 )));           // H УГР
                ui->table_Kolca->setItem(i,11,new QTableWidgetItem(QString::number( xring.rh_ugr+xring.B,      'f', 4 )));           // Hпроект
                ui->table_Kolca->setItem(i,12,new QTableWidgetItem(QString::number( xring.rf,                  'f', 4 )));           // Rфакт
                ui->table_Kolca->setItem(i,13,new QTableWidgetItem(QString::number( xring.rdiff_r,             'f', 0 )));           // Отклонение
                if (xring.rdiff_r<50&&xring.rdiff_r>-50){
                    ui->table_Kolca->item(i,13)->setTextColor(QColor(0,0,0));
                }else{
                    ui->table_Kolca->item(i,13)->setTextColor(Qt::red);
                    //ui->table_Kolca->item(i,13)->setFont(new QFont::~QFont());
                }
                ui->table_Kolca->item(i,8)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                ui->table_Kolca->item(i,9)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                ui->table_Kolca->item(i,10)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                ui->table_Kolca->item(i,11)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                ui->table_Kolca->item(i,12)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                ui->table_Kolca->item(i,13)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);

                rings.append(xring);
            }
        }
    }
    statusBar()->showMessage(tr("Расчет колец выполнен"), 3000);
}


void MainWindow::on_pushButton_Otchet_kolca_clicked()
{
    geo zadacha;
    qDebug() << "Считываем длинну Стандартного пикета";
    zadacha.PkStnd = ui->EditStndPk->text().toDouble(); //Считываем длинну Стандартного пикета
    //Ввод таблицы неправильного пикета
    qDebug() << "Ввод таблицы неправильного пикета";
    for (int i=0;i<(ui->tableNPK->rowCount());i++){
        zadacha.appendNpk(ui->tableNPK->item(i,0)->text(),ui->tableNPK->item(i,1)->text());
    }
    //Ввод таблицы трасса
    qDebug() << "Ввод таблицы трасса";
    //QString KrLine,QString PKn,QString Xn,QString Yn,QString Xk,QString Yk,QString Xck,QString Yck)
    for (int i = 0; i < ui->tableTrassa->rowCount();i++ ){
        zadacha.appendTrassa(ui->tableTrassa->item(i,0)->text(), //type
                             ui->EditPKnach->text(), //ПК начала трассы
                             ui->EditXnach->text(),  //X начала трассы
                             ui->EditYnach->text(),  //Y начала трассы
                             ui->tableTrassa->item(i,1)->text(), //Xk
                             ui->tableTrassa->item(i,2)->text(), //Yk
                             ui->tableTrassa->item(i,3)->text(), //Xck
                             ui->tableTrassa->item(i,4)->text());//Yck
    }
    //Ввод таблицы Переходных кривых
    qDebug() << "Ввод таблицы Переходных кривых";
    for (int i = 0; i < ui->tableTrassa_2->rowCount();i++ ){
        zadacha.appendPerehod(ui->tableTrassa_2->item(i,0)->text(), //PKin
                              ui->tableTrassa_2->item(i,1)->text(), //Lin
                              ui->tableTrassa_2->item(i,2)->text(), //Cin
                              ui->tableTrassa_2->item(i,3)->text(), //q
                              ui->tableTrassa_2->item(i,4)->text(), //z
                              ui->tableTrassa_2->item(i,5)->text(), //PKout
                              ui->tableTrassa_2->item(i,6)->text(), //Lout
                              ui->tableTrassa_2->item(i,7)->text()); //Cout
    }

    //Ввод таблицы Профиля
    for (int i = 0; i < ui->tableProfil->rowCount();i++ ){
        zadacha.appendProfil(ui->tableProfil->item(i,0)->text(),
                             ui->tableProfil->item(i,1)->text(),
                             ui->tableProfil->item(i,2)->text());
    }

    qDebug() << "Предрасчет";
    zadacha.predRaschet();


    //Считаем точки
    bool xOk;
    bool yOk;

    double xkoor = 0;
    double ykoor = 0;
    QString PKpoint  = "0";
    double SMpoint  = 0;
    double SMpointZ  = 0;
    double SMpointZQ = 0;
    double Hpoint = 0;

    double X_KOL = 0;
    double Y_KOL = 0;
    double H_KOL = 0;


    double R1_KOL = 0;
    double R2_KOL = 0;

    QString RL = "L";

    QVector<ring> rings;
    ring xring;

    //Условные координаты
    double x_usl_koor = ui->lineEdit_usl_x_3->text().toDouble(&xOk);
    double y_usl_koor = ui->lineEdit_usl_y_3->text().toDouble(&yOk);
    if ((xOk != true) || (yOk != true)){
        x_usl_koor = 0;
        y_usl_koor = 0;
    }
    //qDebug()<<x_usl_koor<<xOk<< "  "<<y_usl_koor<<yOk;


    for (int i = 0; i < ui->table_Kolca->rowCount();i++ ){
        if (ui->checkBox_usl_3->isChecked()==true ){
            xkoor = ui->table_Kolca->item(i,2)->text().toDouble(&xOk)+x_usl_koor;
            ykoor = ui->table_Kolca->item(i,3)->text().toDouble(&yOk)+y_usl_koor;
        } else {
            xkoor = ui->table_Kolca->item(i,2)->text().toDouble(&xOk);
            ykoor = ui->table_Kolca->item(i,3)->text().toDouble(&yOk);
        }

        PKpoint  = "0";
        SMpoint   = 0;
        SMpointZ  = 0;
        SMpointZQ = 0;
        Hpoint    = 0;


        //        0 	1	2	3   4       5       6       7       8   9       10          11      12      13
        //        №№	№R  X	Y	Hфакт	D(1+4)	B=1,75	R пр	ПК	Смещ	HпрУГР  	Hпр 	Rфакт	Откл.

        if ((xOk == true) && (yOk == true)){
            zadacha.raschetPkSm(xkoor,
                                ykoor,
                                PKpoint,
                                SMpoint,
                                RL);

            if(PKpoint!="0"){
                zadacha.raschetPer(PKpoint,
                                   SMpoint,
                                   RL,
                                   SMpointZ,
                                   SMpointZQ);

                zadacha.raschetH(PKpoint,Hpoint);

                xring.nkolca = ui->table_Kolca->item(i,0)->text().toInt(&xOk);    //Номер кольца
                xring.nr     = ui->table_Kolca->item(i,1)->text().toInt(&xOk);    //Номер радиуса

                xring.rx     = ui->table_Kolca->item(i,2)->text().toDouble(&xOk); //x факт Радиуса
                xring.ry     = ui->table_Kolca->item(i,3)->text().toDouble(&xOk); //y факт Радиуса
                xring.rh     = ui->table_Kolca->item(i,4)->text().toDouble(&yOk); //h факт Радиуса


                switch (xring.nr) {
                case 1:
                    xring.D1_4   = ui->table_Kolca->item(i,5)->text().toDouble(&xOk); //Диаметр измеренный
                    X_KOL = ui->table_Kolca->item(i,2)->text().toDouble(&xOk); //x факт Радиуса
                    Y_KOL = ui->table_Kolca->item(i,3)->text().toDouble(&xOk); //y факт Радиуса
                    break;
                case 2:
                    xring.D1_4   = ui->table_Kolca->item(i,5)->text().toDouble(&xOk); //Диаметр измеренный
                    break;
                case 3:
                    xring.D1_4   = ui->table_Kolca->item(i,5)->text().toDouble(&xOk); //Диаметр измеренный
                    H_KOL        = ui->table_Kolca->item(i,4)->text().toDouble(&yOk); //h факт Радиуса
                    break;
                case 4:
                    xring.D1_4   = ui->table_Kolca->item(i,5)->text().toDouble(&xOk); //Диаметр измеренный
                    break;
                case 5:
                    xring.D1_4   = 0;
                    xring.xkol = (X_KOL + ui->table_Kolca->item(i,2)->text().toDouble(&xOk))/2;
                    xring.ykol = (Y_KOL + ui->table_Kolca->item(i,3)->text().toDouble(&xOk))/2;
                    break;
                case 7:
                    xring.D1_4   = 0;
                    xring.hkol = (H_KOL + ui->table_Kolca->item(i,4)->text().toDouble(&yOk)) / 2;
                    break;

                default:
                    xring.D1_4   = 0;
                    break;
                }

                xring.B      = ui->table_Kolca->item(i,6)->text().toDouble(&xOk); //Домер от УГР до центра кольца (1,75)
                xring.R      = ui->table_Kolca->item(i,7)->text().toDouble(&xOk);  //R     проект

                xring.rPK     = PKpoint;                                                             // ПК
                if (SMpointZQ!=0) {xring.rsm_qz = SMpointZQ; }else{if (SMpointZ!=0){xring.rsm_qz = SMpointZ;} else {xring.rsm_qz = SMpoint;} }  //Смещение от оси тоннеля
                xring.rh_ugr  = Hpoint;                                                              // H УГР
                xring.rf      = sqrt(pow((xring.rh-(xring.rh_ugr+xring.B)),2)+pow(xring.rsm_qz,2));  // Rфакт
                xring.rdiff_r = (xring.rf - xring.R)*1000;                                           // Отклонение

                ui->table_Kolca->setItem(i,8 ,new QTableWidgetItem(PKpoint));
                ui->table_Kolca->setItem(i,9 ,new QTableWidgetItem(QString::number( xring.rsm_qz,              'f', 4 )));           //смещение от оси тоннеля
                ui->table_Kolca->setItem(i,10,new QTableWidgetItem(QString::number( xring.rh_ugr,              'f', 4 )));           // H УГР
                ui->table_Kolca->setItem(i,11,new QTableWidgetItem(QString::number( xring.rh_ugr+xring.B,      'f', 4 )));           // Hпроект
                ui->table_Kolca->setItem(i,12,new QTableWidgetItem(QString::number( xring.rf,                  'f', 4 )));           // Rфакт
                ui->table_Kolca->setItem(i,13,new QTableWidgetItem(QString::number( xring.rdiff_r,             'f', 0 )));           // Отклонение
                if (xring.rdiff_r<50&&xring.rdiff_r>-50){
                    ui->table_Kolca->item(i,13)->setTextColor(QColor(0,0,0));
                }else{
                    ui->table_Kolca->item(i,13)->setTextColor(Qt::red);
                    //ui->table_Kolca->item(i,13)->setFont(new QFont::~QFont());
                }
                ui->table_Kolca->item(i,8)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                ui->table_Kolca->item(i,9)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                ui->table_Kolca->item(i,10)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                ui->table_Kolca->item(i,11)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                ui->table_Kolca->item(i,12)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                ui->table_Kolca->item(i,13)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);

                rings.append(xring);
            }
        }
    }
    statusBar()->showMessage(tr("Расчет выполнен, создаем протокол колец"), 3000);
    //    1 A
    //    2 B
    //    3 C
    //    4 D
    //    5 E
    //    6 F
    //    7 G
    //    8 H

    QVector<QString> headerDoc;
    QVector<QString> bodyDoc;
    QVector<QString> bodyDocClear;
    QVector<QString> bodyDocTemp;
    QVector<QString> footDoc;

    QString lineLoad;
    QString lineSave;
    double RA = 0;
    double RB = 0;
    double RC = 0;
    double RD = 0;
    double RE = 0;
    double RF = 0;
    double RG = 0;
    double RH = 0;

    double RA_Eism = 0;
    double RB_Fism = 0;
    double RC_Gism = 0;
    double RD_Hism = 0;
    double ddif = 0;
    QString tstroka;

    int step=0;

#ifdef Q_OS_WIN
    QString file = pathDirTemplate + QDir::separator() + ui->comboBox_template->itemText(ui->comboBox_template->currentIndex()) + ".xml";
    qDebug()<<"Шаблон взят"<<file ;
    QFile templatefile( QDir::toNativeSeparators(file));
#endif

#ifdef Q_OS_LINUX
    //QFile templatefile( QCoreApplication::applicationDirPath() + "/templateFull.xml");
    QString file = pathDirTemplate + QDir::separator() + ui->comboBox_template->itemText(ui->comboBox_template->currentIndex()) + ".xml";
    qDebug()<<"Шаблон взят"<<file ;
    QFile templatefile( QDir::toNativeSeparators(file));
#endif

    if (!templatefile.open(QIODevice::ReadOnly | QIODevice::Text))  {
        qDebug()<<"Файл templateFull.xml не открывается...";
    } else {
        // file opened successfully
        QTextStream tamplate( &templatefile );        // use a text stream
        tamplate.setCodec("UTF-8");           // until end of file...
        while ( !tamplate.atEnd() ) {
            // read and parse the command line
            lineLoad = tamplate.readLine();         // line of text excluding '\n'

            if (lineLoad.contains("<wx:sect>")&& step==0) {
                headerDoc.append(lineLoad.split("<wx:sect>").at(0));
                bodyDoc.append("<wx:sect>");
                bodyDoc.append(lineLoad.split("<wx:sect>").at(1));
                lineLoad = tamplate.readLine();         // line of text excluding '\n'
                step=1;
            }else{
                if (step==0) headerDoc.append(lineLoad);
            }
            if (lineLoad.contains("</wx:sect></w:body>")&& step==1) {
                if (!lineLoad.contains("<wx:sect>")){
                    bodyDoc.append(lineLoad.split("</w:body>").at(0));
                    footDoc.append("</w:body></w:wordDocument>");
                    step=2;}
            }else{
                if (step==1) bodyDoc.append(lineLoad);
            }
        }
        templatefile.close(); // Close the file
    }

    step=0;
    for (int i = 0; i < bodyDoc.count();i++ ){
        if(step==0){
            if (bodyDoc.value(i).contains("<w:binData")){
                bodyDocClear.append(bodyDoc.value(i).split("<w:binData").at(0));
                step=1;
                //qDebug()<<step<<bodyDoc.value(i).split("<w:binData").at(0);
            } else {
                bodyDocClear.append(bodyDoc.value(i));
                //qDebug()<<step<<"пишем";
            }
        }else{
            if (bodyDoc.value(i).contains("</w:binData>")){
                tstroka = bodyDoc.value(i).split("</w:binData>").at(1);
                step=0;
                if (tstroka.contains("<w:binData")){
                    tstroka=tstroka.split("<w:binData").at(0);
                    step=1;
                }
                bodyDocClear.append(tstroka);
                //qDebug()<<step<<tstroka.split("<w:binData").at(0);
            }
            //qDebug()<<step<<"пропуск";
        }
    }

#ifdef Q_OS_WIN
    QFile fileSave( "C:/Windows/Temp/OtchetFull.xml");
#else
    QFile fileSave(QCoreApplication::applicationDirPath() + "/OtchetFull.xml");
#endif

    if (!fileSave.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug()<<"Файл OtchetFull.xml не открывается...";
        return;
    } else {
        QTextStream out(&fileSave);
        out.setCodec("UTF-8");

        for (int i = 0; i < headerDoc.count();i++ ){
            out << headerDoc.value(i)<<"\r\n";
        }

        //Поросчет фактических диаметров.
        for (int k = 0; k < rings.count();k++ ){
            if (rings.value(k).nr==7) {
                //                    R_KOL = qSqrt(  (rings.value(k).xkol-rings.value(k).rx)*(rings.value(k).xkol-rings.value(k).rx)+
                //                                   +(rings.value(k).ykol-rings.value(k).ry)*(rings.value(k).ykol-rings.value(k).ry)+
                //                                   +(rings.value(k).hkol-rings.value(k).rh)*(rings.value(k).hkol-rings.value(k).rh));

                //1-5
                //2-6
                //3-7
                //4-8

                X_KOL = rings.value(k-2).xkol;
                Y_KOL = rings.value(k-2).ykol;
                H_KOL = rings.value(k).hkol;
                //1-5
                R1_KOL = qSqrt(  (X_KOL-rings.value(k-6).rx)*(X_KOL-rings.value(k-6).rx)+
                                +(Y_KOL-rings.value(k-6).ry)*(Y_KOL-rings.value(k-6).ry)+
                                +(H_KOL-rings.value(k-6).rh)*(H_KOL-rings.value(k-6).rh));

                R2_KOL = qSqrt(  (X_KOL-rings.value(k-2).rx)*(X_KOL-rings.value(k-2).rx)+
                                +(Y_KOL-rings.value(k-2).ry)*(Y_KOL-rings.value(k-2).ry)+
                                +(H_KOL-rings.value(k-2).rh)*(H_KOL-rings.value(k-2).rh));
                rings[k-6].D1_4=R1_KOL+R2_KOL;
                //2-6
                R1_KOL = qSqrt(  (X_KOL-rings.value(k-5).rx)*(X_KOL-rings.value(k-5).rx)+
                                +(Y_KOL-rings.value(k-5).ry)*(Y_KOL-rings.value(k-5).ry)+
                                +(H_KOL-rings.value(k-5).rh)*(H_KOL-rings.value(k-5).rh));

                R2_KOL = qSqrt(  (X_KOL-rings.value(k-1).rx)*(X_KOL-rings.value(k-1).rx)+
                                +(Y_KOL-rings.value(k-1).ry)*(Y_KOL-rings.value(k-1).ry)+
                                +(H_KOL-rings.value(k-1).rh)*(H_KOL-rings.value(k-1).rh));
                rings[k-5].D1_4=R1_KOL+R2_KOL;

                //3-7
                R1_KOL = qSqrt(   (X_KOL-rings.value(k-4).rx)*(X_KOL-rings.value(k-4).rx)+
                                 +(Y_KOL-rings.value(k-4).ry)*(Y_KOL-rings.value(k-4).ry)+
                                 +(H_KOL-rings.value(k-4).rh)*(H_KOL-rings.value(k-4).rh));

                R2_KOL = qSqrt(   (X_KOL-rings.value(k).rx)*(X_KOL-rings.value(k).rx)+
                                 +(Y_KOL-rings.value(k).ry)*(Y_KOL-rings.value(k).ry)+
                                 +(H_KOL-rings.value(k).rh)*(H_KOL-rings.value(k).rh));
                rings[k-4].D1_4=R1_KOL+R2_KOL;

                //4-8
                R1_KOL = qSqrt(  (X_KOL-rings.value(k-3).rx)*(X_KOL-rings.value(k-3).rx)+
                                +(Y_KOL-rings.value(k-3).ry)*(Y_KOL-rings.value(k-3).ry)+
                                +(H_KOL-rings.value(k-3).rh)*(H_KOL-rings.value(k-3).rh));

                R2_KOL = qSqrt(  (X_KOL-rings.value(k+1).rx)*(X_KOL-rings.value(k+1).rx)+
                                +(Y_KOL-rings.value(k+1).ry)*(Y_KOL-rings.value(k+1).ry)+
                                +(H_KOL-rings.value(k+1).rh)*(H_KOL-rings.value(k+1).rh));
                rings[k-3].D1_4=R1_KOL+R2_KOL;


                qDebug() << "Номер сечения: " << rings.value(k).nr << rings.value(k-2).xkol << rings.value(k-2).ykol << rings.value(k).hkol;
            }


        }//Поросчет фактических диаметров.


        for (int k = 0; k < rings.count();k++ ){
            if (k==0) {
                bodyDocTemp=bodyDoc;
            } else{
                if (rings.value(k).nr==1) {
                    out << "<w:p wsp:rsidR=\"0072705B\" wsp:rsidRDefault=\"00BB2810\" wsp:rsidP=\"00C15578\"><w:pPr><w:spacing w:after=\"0\" w:line=\"240\" w:line-rule=\"auto\" /><w:rPr><w:rFonts w:fareast=\"Times New Roman\" /><w:sz w:val=\"20\" /><w:sz-cs w:val=\"20\" /><w:lang w:fareast=\"RU\" /></w:rPr></w:pPr><w:r><w:rPr><w:rFonts w:fareast=\"Times New Roman\" /><w:sz w:val=\"20\" /><w:sz-cs w:val=\"20\" /><w:lang w:fareast=\"RU\" /></w:rPr><w:br w:type=\"page\" /></w:r></w:p>";
                    bodyDocTemp=bodyDocClear;
                    RA=0;
                    RB=0;
                    RC=0;
                    RD=0;
                    RE=0;
                    RF=0;
                    RG=0;
                    RH=0;
                    RA_Eism=0;
                    RB_Fism=0;
                    RC_Gism=0;
                    RD_Hism=0;
                }
            }

            //ПИШЕМ ПРОТОКОЛ КОЛЬЦА
            for (int i = 0; i < bodyDocTemp.count();i++ ){
                lineSave = bodyDocTemp.value(i);

                lineSave = lineSave.replace("nkolca" ,QString::number(rings.value(k).nkolca));
                tstroka=rings.value(k).rPK.split("+").at(0)+"+"+QString::number(rings.value(k).rPK.split("+").at(1).toDouble(),'f',2);
                lineSave = lineSave.replace("PKkolca",tstroka);
                lineSave = lineSave.replace("DiamPr",QString::number(rings.value(k).R*2,'f',3));
                lineSave = lineSave.replace("R_pr",QString::number(rings.value(k).R,'f',3));

                switch (rings.value(k).nr) {
                case 1:{
                    //A
                    lineSave = lineSave.replace("radiusAx",QString::number(rings.value(k).rx,'f',3));
                    lineSave = lineSave.replace("radiusAy",QString::number(rings.value(k).ry,'f',3));
                    lineSave = lineSave.replace("radiusAh",QString::number(rings.value(k).rh,'f',3));
                    lineSave = lineSave.replace("radiusAf",QString::number(rings.value(k).rf,'f',3));
                    RA=rings.value(k).rf;
                    lineSave = lineSave.replace("radiusAdiff_r",QString::number(rings.value(k).rdiff_r,'f',0));

                    //                    R_KOL = qSqrt(  (rings.value(k).xkol-rings.value(k).rx)*(rings.value(k).xkol-rings.value(k).rx)+
                    //                                   +(rings.value(k).ykol-rings.value(k).ry)*(rings.value(k).ykol-rings.value(k).ry)+
                    //                                   +(rings.value(k).hkol-rings.value(k).rh)*(rings.value(k).hkol-rings.value(k).rh));

                    lineSave = lineSave.replace("dA_Eizm" ,QString::number(rings.value(k).D1_4,'f',3));
                    RA_Eism=rings.value(k).D1_4;
                    ddif = (rings.value(k).D1_4-rings.value(k).R*2)*1000;
                    lineSave = lineSave.replace("dA_Ediff",QString::number(ddif,'f',0));
                    if(ddif<50&&ddif>-50){
                        lineSave = lineSave.replace("dA_Eover","нет");
                    }else{
                        if (ddif>0){ddif-=50;}else{ddif+=50;}
                        lineSave = lineSave.replace("dA_Eover",QString::number(ddif,'f',0));
                    }
                    break;
                }
                case 2:{
                    //B
                    lineSave = lineSave.replace("radiusBx",QString::number(rings.value(k).rx,'f',3));
                    lineSave = lineSave.replace("radiusBy",QString::number(rings.value(k).ry,'f',3));
                    lineSave = lineSave.replace("radiusBh",QString::number(rings.value(k).rh,'f',3));
                    lineSave = lineSave.replace("radiusBf",QString::number(rings.value(k).rf,'f',3));
                    RB=rings.value(k).rf;
                    lineSave = lineSave.replace("radiusBdiff_r",QString::number(rings.value(k).rdiff_r,'f',0));

                    lineSave = lineSave.replace("dB_Fizm" ,QString::number(rings.value(k).D1_4,'f',3));
                    RB_Fism=rings.value(k).D1_4;
                    ddif = (rings.value(k).D1_4-rings.value(k).R*2)*1000;
                    lineSave = lineSave.replace("dB_Fdiff",QString::number(ddif,'f',0));
                    if(ddif<50&&ddif>-50){
                        lineSave = lineSave.replace("dB_Fover","нет");
                    }else{
                        if (ddif>0){ddif-=50;}else{ddif+=50;}
                        lineSave = lineSave.replace("dB_Fover",QString::number(ddif,'f',0));
                    }
                    break;
                }
                case 3:{
                    //C
                    lineSave = lineSave.replace("radiusCx",QString::number(rings.value(k).rx,'f',3));
                    lineSave = lineSave.replace("radiusCy",QString::number(rings.value(k).ry,'f',3));
                    lineSave = lineSave.replace("radiusCh",QString::number(rings.value(k).rh,'f',3));
                    lineSave = lineSave.replace("radiusCf",QString::number(rings.value(k).rf,'f',3));
                    RC=rings.value(k).rf;
                    lineSave = lineSave.replace("radiusCdiff_r",QString::number(rings.value(k).rdiff_r,'f',0));

                    lineSave = lineSave.replace("dC_Gizm" ,QString::number(rings.value(k).D1_4,'f',3));
                    RC_Gism=rings.value(k).D1_4;
                    ddif = (rings.value(k).D1_4-rings.value(k).R*2)*1000;
                    lineSave = lineSave.replace("dC_Gdiff",QString::number(ddif,'f',0));
                    if(ddif<50&&ddif>-50){
                        lineSave = lineSave.replace("dC_Gover","нет");
                    }else{
                        if (ddif>0){ddif-=50;}else{ddif+=50;}
                        lineSave = lineSave.replace("dC_Gover",QString::number(ddif,'f',0));
                    }
                    break;
                }
                case 4:{
                    //D
                    lineSave = lineSave.replace("radiusDx",QString::number(rings.value(k).rx,'f',3));
                    lineSave = lineSave.replace("radiusDy",QString::number(rings.value(k).ry,'f',3));
                    lineSave = lineSave.replace("radiusDh",QString::number(rings.value(k).rh,'f',3));
                    lineSave = lineSave.replace("radiusDf",QString::number(rings.value(k).rf,'f',3));
                    RD=rings.value(k).rf;
                    lineSave = lineSave.replace("radiusDdiff_r",QString::number(rings.value(k).rdiff_r,'f',0));

                    lineSave = lineSave.replace("dD_Hizm" ,QString::number(rings.value(k).D1_4,'f',3));
                    RD_Hism=rings.value(k).D1_4;
                    ddif = (rings.value(k).D1_4-rings.value(k).R*2)*1000;
                    lineSave = lineSave.replace("dD_Hdiff",QString::number(ddif,'f',0));
                    if(ddif<50&&ddif>-50){
                        lineSave = lineSave.replace("dD_Hover","нет");
                    }else{
                        if (ddif>0){ddif-=50;}else{ddif+=50;}
                        lineSave = lineSave.replace("dD_Hover",QString::number(ddif,'f',0));
                    }
                    break;
                }
                case 5:{
                    //E
                    lineSave = lineSave.replace("radiusEx",QString::number(rings.value(k).rx,'f',3));
                    lineSave = lineSave.replace("radiusEy",QString::number(rings.value(k).ry,'f',3));
                    lineSave = lineSave.replace("radiusEh",QString::number(rings.value(k).rh,'f',3));
                    lineSave = lineSave.replace("radiusEf",QString::number(rings.value(k).rf,'f',3));
                    lineSave = lineSave.replace("radiusEdiff_r",QString::number(rings.value(k).rdiff_r,'f',0));
                    RE=rings.value(k).rf;
                    lineSave = lineSave.replace("dA_Ec",QString::number((RA+RE),'f',3));
                    lineSave = lineSave.replace("dA_Eddiff",QString::number((RA_Eism-(RA+RE))*1000,'f',0));
                    lineSave = lineSave.replace("xkolca",QString::number(rings.value(k).xkol,'f',3));
                    lineSave = lineSave.replace("ykolca",QString::number(rings.value(k).ykol,'f',3));
                    break;
                }
                case 6:{
                    //F
                    lineSave = lineSave.replace("radiusFx",QString::number(rings.value(k).rx,'f',3));
                    lineSave = lineSave.replace("radiusFy",QString::number(rings.value(k).ry,'f',3));
                    lineSave = lineSave.replace("radiusFh",QString::number(rings.value(k).rh,'f',3));
                    lineSave = lineSave.replace("radiusFf",QString::number(rings.value(k).rf,'f',3));
                    lineSave = lineSave.replace("radiusFdiff_r",QString::number(rings.value(k).rdiff_r,'f',0));
                    RF=rings.value(k).rf;
                    lineSave = lineSave.replace("dB_Fc",QString::number((RB+RF),'f',3));
                    lineSave = lineSave.replace("dB_Fddiff",QString::number(((RB_Fism -(RB+RF))*1000),'f',0));
                    break;
                }
                case 7:{
                    //G
                    lineSave = lineSave.replace("radiusGx",QString::number(rings.value(k).rx,'f',3));
                    lineSave = lineSave.replace("radiusGy",QString::number(rings.value(k).ry,'f',3));
                    lineSave = lineSave.replace("radiusGh",QString::number(rings.value(k).rh,'f',3));
                    lineSave = lineSave.replace("radiusGf",QString::number(rings.value(k).rf,'f',3));
                    lineSave = lineSave.replace("radiusGdiff_r",QString::number(rings.value(k).rdiff_r,'f',0));
                    RG=rings.value(k).rf;
                    lineSave = lineSave.replace("dC_Gc",QString::number((RC+RG),'f',3));
                    lineSave = lineSave.replace("dC_Gddiff",QString::number((RC_Gism-(RC+RG))*1000,'f',0));
                    lineSave = lineSave.replace("hkolca",QString::number(rings.value(k).hkol,'f',3));

                    break;
                }
                case 8:{
                    //H
                    lineSave = lineSave.replace("radiusHx",QString::number(rings.value(k).rx,'f',3));
                    lineSave = lineSave.replace("radiusHy",QString::number(rings.value(k).ry,'f',3));
                    lineSave = lineSave.replace("radiusHh",QString::number(rings.value(k).rh,'f',3));
                    lineSave = lineSave.replace("radiusHf",QString::number(rings.value(k).rf,'f',3));
                    lineSave = lineSave.replace("radiusHdiff_r",QString::number(rings.value(k).rdiff_r,'f',0));
                    RH=rings.value(k).rf;
                    lineSave = lineSave.replace("dD_Hc",QString::number((RD+RH),'f',3));
                    lineSave = lineSave.replace("dD_Hddiff",QString::number((RD_Hism-(RD+RH))*1000,'f',0));

                    ddif=((RE-RA)/2)*1000;
                    lineSave = lineSave.replace("xydiff" ,QString::number(ddif ,'f',0));
                    if(ddif<50&&ddif>-50){
                        lineSave = lineSave.replace("xyoverdiff","нет");
                    }else{
                        if (ddif>0){ddif-=50;}else{ddif+=50;}
                        lineSave = lineSave.replace("xyoverdiff",QString::number(ddif,'f',0));
                    }
                    ddif=( (RC-RG)/2  )*1000;
                    lineSave = lineSave.replace("zdiff"  ,QString::number( ddif ,'f',0));
                    if(ddif<50&&ddif>-50){
                        lineSave = lineSave.replace("zoverdiff","нет");
                    }else{
                        if (ddif>0){ddif-=50;}else{ddif+=50;}
                        lineSave = lineSave.replace("zoverdiff",QString::number(ddif,'f',0));
                    }
                    break;
                }
                }  //switch


                bodyDocTemp.replace(i,lineSave); //фиксируем во временном протоколе измененную строку.
            } // ЗАПИСАЛИ ПРОТОКОЛ КОЛЬЦА
            if (rings.value(k).nr==8) {         //фиксируем во временном протоколе все изменения.
                for (int i = 0; i < bodyDocTemp.count();i++ ){
                    out << bodyDocTemp.value(i)<<"\r\n";
                }
            }
        }

        for (int i = 0; i < footDoc.count();i++ ){
            out << footDoc.value(i)<<"\r\n";
        }
        fileSave.close();

#ifdef Q_OS_WIN
        QSettings settings("HKEY_CLASSES_ROOT\\Applications\\winword.exe\\shell\\edit\\command",QSettings::NativeFormat);
        QString s = settings.value("Default").toString();
        s=s.split('"').value(1);
        QStringList arguments;
        arguments << "C:/Windows/Temp/OtchetFull.xml";

        QProcess *myProcess = new QProcess();
        myProcess->start(QDir::toNativeSeparators(s), arguments);
#endif

    }

}


void MainWindow::fileOpen(QString fileName,bool ask){
    int countNpk  = 0;
    int countPlan = 0;
    int countPerehod = 0;
    int countProfil  = 0;

    //Открыть
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        statusBar()->showMessage(tr("Ошибка открытия файла"), 3000);
        return;
    } else {
        if (ask){
            QMessageBox msgBox;
            msgBox.setText("Открытие файла...");
            //To-Do
            msgBox.setInformativeText("При открытии файла все данные в программе будут заменены новыми, \n продолжить?");
            msgBox.setIcon(QMessageBox::Warning);
            QPushButton *yes = msgBox.addButton(tr("Да"), QMessageBox::ActionRole);
            msgBox.addButton(QObject::tr("Отмена"), QMessageBox::ActionRole);
            msgBox.setDefaultButton(yes);
            msgBox.exec();
            if(msgBox.clickedButton() != yes)
            {
                statusBar()->showMessage(tr("Открытие файла отменено..."), 3000);
                return;
            }
        }

        //тело
        QXmlStreamReader xml(&file);
        while(!xml.atEnd() && !xml.hasError()) {

            if (xml.isEndElement()){
                xml.readNext();
                break;
            }

            while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "Trassa"))
            {
                if (xml.tokenType() == QXmlStreamReader::StartElement)
                {


                    if (xml.name() == "StartTrassa") {
                        ui->EditPKnach->setText(xml.attributes().value("pk").toString());
                        ui->EditXnach->setText(xml.attributes().value("x").toString());
                        ui->EditYnach->setText(xml.attributes().value("y").toString());
                        ui->EditStndPk->setText(xml.attributes().value("Lpk").toString());
                        xml.readNext();
                    }

                    if (xml.name() == "Npk") {
                        ui->tableNPK->setRowCount(countNpk);
                        ui->tableNPK->insertRow(countNpk);
                        ui->tableNPK->setItem(countNpk,0,new QTableWidgetItem(xml.attributes().value("NNpk").toString()));
                        ui->tableNPK->setItem(countNpk,1,new QTableWidgetItem(xml.attributes().value("NpL").toString()));
                        countNpk+=1;
                        xml.readNext();
                    }

                    if (xml.name() == "TrassaPlan") {
                        ui->tableTrassa->setRowCount(countPlan);
                        ui->tableTrassa->insertRow(countPlan);
                        ui->tableTrassa->setItem(countPlan,0,new QTableWidgetItem(xml.attributes().value("Type").toString()));
                        ui->tableTrassa->setItem(countPlan,1,new QTableWidgetItem(xml.attributes().value("Xk").toString()));
                        ui->tableTrassa->setItem(countPlan,2,new QTableWidgetItem(xml.attributes().value("Yk").toString()));
                        ui->tableTrassa->setItem(countPlan,3,new QTableWidgetItem(xml.attributes().value("Xck").toString()));
                        ui->tableTrassa->setItem(countPlan,4,new QTableWidgetItem(xml.attributes().value("Yck").toString()));
                        ui->tableTrassa->setItem(countPlan,5,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
                        ui->tableTrassa->setItem(countPlan,6,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
                        ui->tableTrassa->setItem(countPlan,7,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
                        ui->tableTrassa->item(countPlan,5)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                        ui->tableTrassa->item(countPlan,6)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                        ui->tableTrassa->item(countPlan,7)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                        countPlan+=1;
                        xml.readNext();
                    }

                    if (xml.name() == "TrassaPerehod") {
                        ui->tableTrassa_2->setRowCount(countPerehod);
                        ui->tableTrassa_2->insertRow(countPerehod);
                        ui->tableTrassa_2->setItem(countPerehod,0,new QTableWidgetItem(xml.attributes().value("PkNach").toString()));
                        ui->tableTrassa_2->setItem(countPerehod,1,new QTableWidgetItem(xml.attributes().value("LpNach").toString()));
                        ui->tableTrassa_2->setItem(countPerehod,2,new QTableWidgetItem(xml.attributes().value("CNach").toString()));
                        ui->tableTrassa_2->setItem(countPerehod,3,new QTableWidgetItem(xml.attributes().value("q").toString()));
                        ui->tableTrassa_2->setItem(countPerehod,4,new QTableWidgetItem(xml.attributes().value("z").toString()));
                        ui->tableTrassa_2->setItem(countPerehod,5,new QTableWidgetItem(xml.attributes().value("PkKon").toString()));
                        ui->tableTrassa_2->setItem(countPerehod,6,new QTableWidgetItem(xml.attributes().value("LpKon").toString()));
                        ui->tableTrassa_2->setItem(countPerehod,7,new QTableWidgetItem(xml.attributes().value("CKon").toString()));
                        ui->tableTrassa_2->setItem(countPerehod,8,new QTableWidgetItem(xml.attributes().value("h").toString()));
                        countPerehod+=1;
                        xml.readNext();
                    }

                    if (xml.name() == "TrassaProfil") {
                        ui->tableProfil->setRowCount(countProfil);
                        ui->tableProfil->insertRow(countProfil);
                        ui->tableProfil->setItem(countProfil,0,new QTableWidgetItem(xml.attributes().value("PkNach").toString()));
                        ui->tableProfil->setItem(countProfil,1,new QTableWidgetItem(xml.attributes().value("H").toString()));
                        ui->tableProfil->setItem(countProfil,2,new QTableWidgetItem(xml.attributes().value("R").toString()));
                        ui->tableProfil->setItem(countProfil,3,new QTableWidgetItem(QString::number( 0,   'f', 4 ))); //T
                        ui->tableProfil->setItem(countProfil,4,new QTableWidgetItem(QString::number( 0,   'f', 4 ))); //i
                        ui->tableProfil->setItem(countProfil,5,new QTableWidgetItem(QString::number( 0,   'f', 4 ))); //L
                        ui->tableProfil->item(countProfil,3)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                        ui->tableProfil->item(countProfil,4)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                        ui->tableProfil->item(countProfil,5)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                        countProfil+=1;
                        xml.readNext();
                    }
                }
                xml.readNext();
            }
        }
        //конец
        file.close();
    }

    openFile = fileName;

    ui->lineEdit_RazbivkaPKnach->setText("0+0.0000");
    ui->lineEdit_RazbivkaPKkon->setText("0+0.0000");

    on_Button_Predraschet_clicked();

    QSettings settings("nesmit", "qTrassa");
    settings.beginGroup("MainWindow");
    setWindowTitle(settings.value("infoPRG").toString() +"  Проект: " + fileName);
    settings.endGroup();

    statusBar()->showMessage(tr("Файл открыт."), 3000);

}

void MainWindow::saveFile(QString fileName){
    //    QIODevice::ReadOnly    Открыть файл для чтения
    //    QIODevice::WriteOnly   Открыть файл для записи (таким методом можно просто создать файл)
    //    QIODevice::ReadWrite   Открыть файл для чтения и записи
    //    QIODevice::Append      Открыть файл для дополнения файла в конец
    //Сохраняем
    //QFile file( QCoreApplication::applicationDirPath() +"/demo.mtrassa");

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)){
        statusBar()->showMessage(tr("Ошибка записи в файл"), 3000);
    } else {
        QXmlStreamWriter stream(&file);
        stream.setAutoFormatting(true);
        stream.writeStartDocument();
        //тело
        stream.writeStartElement("Trassa");

        stream.writeStartElement("StartTrassa");
        stream.writeAttribute("version", "0.1");
        stream.writeAttribute("pk",  ui->EditPKnach->text());
        stream.writeAttribute("x",   ui->EditXnach->text());
        stream.writeAttribute("y",   ui->EditYnach->text());
        stream.writeAttribute("Lpk", ui->EditStndPk->text());
        stream.writeEndElement(); // StartTrassa

        for (int i=0;i<(ui->tableNPK->rowCount());i++){
            stream.writeStartElement("Npk");
            stream.writeAttribute("id", QString::number(i));
            stream.writeAttribute("NNpk", ui->tableNPK->item(i,0)->text() ) ;    //Номер  неправильного пикета
            stream.writeAttribute("NpL" , ui->tableNPK->item(i,1)->text() ) ;     //Длинна неправильного пикета
            stream.writeEndElement(); // npk
        }

        for (int i = 0; i < ui->tableTrassa->rowCount();i++ ){
            stream.writeStartElement("TrassaPlan");
            stream.writeAttribute("id", QString::number(i));
            stream.writeAttribute("Type", ui->tableTrassa->item(i,0)->text() );
            stream.writeAttribute("Xk",   ui->tableTrassa->item(i,1)->text() );
            stream.writeAttribute("Yk",   ui->tableTrassa->item(i,2)->text() );
            stream.writeAttribute("Xck",  ui->tableTrassa->item(i,3)->text() );
            stream.writeAttribute("Yck",  ui->tableTrassa->item(i,4)->text() );
            stream.writeEndElement(); // TrassaPlan
        }

        for (int i = 0; i < ui->tableTrassa_2->rowCount();i++ ){
            stream.writeStartElement("TrassaPerehod");
            stream.writeAttribute("id", QString::number(i));
            stream.writeAttribute("PkNach", ui->tableTrassa_2->item(i,0)->text() );
            stream.writeAttribute("LpNach", ui->tableTrassa_2->item(i,1)->text() );
            stream.writeAttribute("CNach",  ui->tableTrassa_2->item(i,2)->text() );
            stream.writeAttribute("q",      ui->tableTrassa_2->item(i,3)->text() );
            stream.writeAttribute("z",      ui->tableTrassa_2->item(i,4)->text() );
            stream.writeAttribute("PkKon",  ui->tableTrassa_2->item(i,5)->text() );
            stream.writeAttribute("LpKon",  ui->tableTrassa_2->item(i,6)->text() );
            stream.writeAttribute("CKon",   ui->tableTrassa_2->item(i,7)->text() );
            stream.writeAttribute("h",   ui->tableTrassa_2->item(i,8)->text() );
            stream.writeEndElement(); // TrassaPerehod
        }

        for (int i = 0; i < ui->tableProfil->rowCount();i++ ){
            stream.writeStartElement("TrassaProfil");
            stream.writeAttribute("id", QString::number(i));
            stream.writeAttribute("PkNach", ui->tableProfil->item(i,0)->text() );
            stream.writeAttribute("H",      ui->tableProfil->item(i,1)->text() );
            stream.writeAttribute("R",      ui->tableProfil->item(i,2)->text() );
            stream.writeEndElement(); // TrassaProfil
        }

        stream.writeEndElement(); // Trassa

        //конец
        stream.writeEndDocument();
        file.close();

        QSettings settings("nesmit", "qTrassa");
        settings.beginGroup("MainWindow");
        setWindowTitle(settings.value("infoPRG").toString() +"  Проект: " + fileName);
        settings.endGroup();

        statusBar()->showMessage(tr("Файл сохранен"), 3000);
    }
}

void MainWindow::on_action_demo_triggered()
{
    int countNpk  = 0;
    int countPlan = 0;
    int countPerehod = 0;
    int countProfil  = 0;
    int countKolca  = 0;
    int countRaschet  = 0;
    int countPredRaschet  = 0;
    //Открыть
    //    QFile file( QCoreApplication::applicationDirPath() +"/demo.mtrassa");
    //    QFile file(fileName);
    QFile file(":/demo.mtrassa");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        statusBar()->showMessage(tr("Ошибка открытия файла"), 3000);
        return;
    } else {
        //тело
        QXmlStreamReader xml(&file);
        while(!xml.atEnd() && !xml.hasError()) {

            if (xml.isEndElement()){
                xml.readNext();
                break;
            }

            while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "Trassa"))
            {
                if (xml.tokenType() == QXmlStreamReader::StartElement)
                {


                    if (xml.name() == "StartTrassa") {
                        ui->EditPKnach->setText(xml.attributes().value("pk").toString());
                        ui->EditXnach->setText(xml.attributes().value("x").toString());
                        ui->EditYnach->setText(xml.attributes().value("y").toString());
                        ui->EditStndPk->setText(xml.attributes().value("Lpk").toString());
                        xml.readNext();
                    }

                    if (xml.name() == "Npk") {
                        ui->tableNPK->setRowCount(countNpk);
                        ui->tableNPK->insertRow(countNpk);
                        ui->tableNPK->setItem(countNpk,0,new QTableWidgetItem(xml.attributes().value("NNpk").toString()));
                        ui->tableNPK->setItem(countNpk,1,new QTableWidgetItem(xml.attributes().value("NpL").toString()));
                        countNpk+=1;
                        xml.readNext();
                    }

                    if (xml.name() == "TrassaPlan") {
                        ui->tableTrassa->setRowCount(countPlan);
                        ui->tableTrassa->insertRow(countPlan);
                        ui->tableTrassa->setItem(countPlan,0,new QTableWidgetItem(xml.attributes().value("Type").toString()));
                        ui->tableTrassa->setItem(countPlan,1,new QTableWidgetItem(xml.attributes().value("Xk").toString()));
                        ui->tableTrassa->setItem(countPlan,2,new QTableWidgetItem(xml.attributes().value("Yk").toString()));
                        ui->tableTrassa->setItem(countPlan,3,new QTableWidgetItem(xml.attributes().value("Xck").toString()));
                        ui->tableTrassa->setItem(countPlan,4,new QTableWidgetItem(xml.attributes().value("Yck").toString()));
                        ui->tableTrassa->setItem(countPlan,5,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
                        ui->tableTrassa->setItem(countPlan,6,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
                        ui->tableTrassa->setItem(countPlan,7,new QTableWidgetItem(QString::number( 0,   'f', 4 )));
                        ui->tableTrassa->item(countPlan,5)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                        ui->tableTrassa->item(countPlan,6)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                        ui->tableTrassa->item(countPlan,7)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                        countPlan+=1;
                        xml.readNext();
                    }

                    if (xml.name() == "TrassaPerehod") {
                        ui->tableTrassa_2->setRowCount(countPerehod);
                        ui->tableTrassa_2->insertRow(countPerehod);
                        ui->tableTrassa_2->setItem(countPerehod,0,new QTableWidgetItem(xml.attributes().value("PkNach").toString()));
                        ui->tableTrassa_2->setItem(countPerehod,1,new QTableWidgetItem(xml.attributes().value("LpNach").toString()));
                        ui->tableTrassa_2->setItem(countPerehod,2,new QTableWidgetItem(xml.attributes().value("CNach").toString()));
                        ui->tableTrassa_2->setItem(countPerehod,3,new QTableWidgetItem(xml.attributes().value("q").toString()));
                        ui->tableTrassa_2->setItem(countPerehod,4,new QTableWidgetItem(xml.attributes().value("z").toString()));
                        ui->tableTrassa_2->setItem(countPerehod,5,new QTableWidgetItem(xml.attributes().value("PkKon").toString()));
                        ui->tableTrassa_2->setItem(countPerehod,6,new QTableWidgetItem(xml.attributes().value("LpKon").toString()));
                        ui->tableTrassa_2->setItem(countPerehod,7,new QTableWidgetItem(xml.attributes().value("CKon").toString()));
                        ui->tableTrassa_2->setItem(countPerehod,8,new QTableWidgetItem(xml.attributes().value("h").toString()));
                        countPerehod+=1;
                        xml.readNext();
                    }

                    if (xml.name() == "TrassaProfil") {
                        ui->tableProfil->setRowCount(countProfil);
                        ui->tableProfil->insertRow(countProfil);
                        ui->tableProfil->setItem(countProfil,0,new QTableWidgetItem(xml.attributes().value("PkNach").toString()));
                        ui->tableProfil->setItem(countProfil,1,new QTableWidgetItem(xml.attributes().value("H").toString()));
                        ui->tableProfil->setItem(countProfil,2,new QTableWidgetItem(xml.attributes().value("R").toString()));
                        ui->tableProfil->setItem(countProfil,3,new QTableWidgetItem(QString::number( 0,   'f', 4 ))); //T
                        ui->tableProfil->setItem(countProfil,4,new QTableWidgetItem(QString::number( 0,   'f', 4 ))); //i
                        ui->tableProfil->setItem(countProfil,5,new QTableWidgetItem(QString::number( 0,   'f', 4 ))); //L
                        ui->tableProfil->item(countProfil,3)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                        ui->tableProfil->item(countProfil,4)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                        ui->tableProfil->item(countProfil,5)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                        countProfil+=1;
                        xml.readNext();
                    }


                    //DemoKolca
                    if (xml.name() == "DemoKolca") {
                        ui->table_Kolca->setRowCount(countKolca);
                        ui->table_Kolca->insertRow(countKolca);

                        ui->table_Kolca->setItem(countKolca,0,new QTableWidgetItem(xml.attributes().value("NK").toString()));
                        ui->table_Kolca->setItem(countKolca,1,new QTableWidgetItem(xml.attributes().value("NR").toString()));
                        ui->table_Kolca->setItem(countKolca,2,new QTableWidgetItem(xml.attributes().value("XK").toString()));
                        ui->table_Kolca->setItem(countKolca,3,new QTableWidgetItem(xml.attributes().value("YK").toString()));
                        ui->table_Kolca->setItem(countKolca,4,new QTableWidgetItem(xml.attributes().value("HK").toString()));
                        ui->table_Kolca->setItem(countKolca,5,new QTableWidgetItem(xml.attributes().value("DK").toString()));
                        ui->table_Kolca->setItem(countKolca,6,new QTableWidgetItem(xml.attributes().value("BK").toString()));
                        ui->table_Kolca->setItem(countKolca,7,new QTableWidgetItem(xml.attributes().value("RK").toString()));

                        ui->table_Kolca->setItem(countKolca,8,new QTableWidgetItem(QString::number( 0,   'f', 0 )));
                        ui->table_Kolca->setItem(countKolca,9,new QTableWidgetItem(QString::number( 0,   'f', 0 )));
                        ui->table_Kolca->setItem(countKolca,10,new QTableWidgetItem(QString::number( 0,   'f', 0 )));
                        ui->table_Kolca->setItem(countKolca,11,new QTableWidgetItem(QString::number( 0,   'f', 0 )));
                        ui->table_Kolca->setItem(countKolca,12,new QTableWidgetItem(QString::number( 0,   'f', 0 )));
                        ui->table_Kolca->setItem(countKolca,13,new QTableWidgetItem(QString::number( 0,   'f', 0 )));

                        ui->table_Kolca->item(countKolca,8)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                        ui->table_Kolca->item(countKolca,9)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                        ui->table_Kolca->item(countKolca,10)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                        ui->table_Kolca->item(countKolca,11)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                        ui->table_Kolca->item(countKolca,12)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                        ui->table_Kolca->item(countKolca,13)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);

                        countKolca+=1;
                        xml.readNext();
                    }

                    //DemoRaschet
                    if (xml.name() == "DemoRaschet") {
                        ui->tableRaschet->setRowCount(countRaschet);
                        ui->tableRaschet->insertRow(countRaschet);
                        ui->tableRaschet->setItem(countRaschet,0,new QTableWidgetItem(xml.attributes().value("NameR").toString()));
                        ui->tableRaschet->setItem(countRaschet,1,new QTableWidgetItem(xml.attributes().value("XR").toString()));
                        ui->tableRaschet->setItem(countRaschet,2,new QTableWidgetItem(xml.attributes().value("YR").toString()));
                        ui->tableRaschet->setItem(countRaschet,3,new QTableWidgetItem(QString::number( 0,   'f', 0 ))); //Pk
                        ui->tableRaschet->setItem(countRaschet,4,new QTableWidgetItem(QString::number( 0,   'f', 4 ))); //Sm
                        ui->tableRaschet->setItem(countRaschet,5,new QTableWidgetItem(QString::number( 0,   'f', 4 ))); //H
                        ui->tableRaschet->item(countRaschet,3)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                        ui->tableRaschet->item(countRaschet,4)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                        ui->tableRaschet->item(countRaschet,5)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                        countRaschet+=1;
                        xml.readNext();
                    }

                    //DemoPredRas
                    if (xml.name() == "DemoPredRas") {
                        ui->tablePred_Raschet->setRowCount(countPredRaschet);
                        ui->tablePred_Raschet->insertRow(countPredRaschet);
                        ui->tablePred_Raschet->setItem(countPredRaschet,0,new QTableWidgetItem(xml.attributes().value("NamePR").toString()));
                        ui->tablePred_Raschet->setItem(countPredRaschet,1,new QTableWidgetItem(xml.attributes().value("PKPR").toString()));
                        ui->tablePred_Raschet->setItem(countPredRaschet,2,new QTableWidgetItem(xml.attributes().value("SMPR").toString()));
                        ui->tablePred_Raschet->setItem(countPredRaschet,3,new QTableWidgetItem(QString::number( 0,   'f', 4 ))); //X
                        ui->tablePred_Raschet->setItem(countPredRaschet,4,new QTableWidgetItem(QString::number( 0,   'f', 4 ))); //Y
                        ui->tablePred_Raschet->setItem(countPredRaschet,5,new QTableWidgetItem(QString::number( 0,   'f', 4 ))); //H
                        ui->tablePred_Raschet->item(countPredRaschet,3)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                        ui->tablePred_Raschet->item(countPredRaschet,4)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                        ui->tablePred_Raschet->item(countPredRaschet,5)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                        countPredRaschet+=1;
                        xml.readNext();
                    }
                    //                TypeOsComboBox
                    //                DemoRaschetOS
                    //DemoPredRas
                    if (xml.name() == "DemoKolcaOS") {
                        if (xml.attributes().value("uslK").toString()=="1"){
                            ui->checkBox_usl_3->setChecked(true);
                        } else {
                            ui->checkBox_usl_3->setChecked(false);
                        }
                        ui->lineEdit_usl_x_3->setText(xml.attributes().value("XuslK").toString());
                        ui->lineEdit_usl_y_3->setText(xml.attributes().value("YuslK").toString());
                        xml.readNext();
                    }

                    //DemoRaschetOS
                    if (xml.name() == "DemoRaschetOS") {
                        if (xml.attributes().value("uslR").toString()=="1"){
                            ui->checkBox_usl->setChecked(true);
                        } else {
                            ui->checkBox_usl->setChecked(false);
                        }
                        ui->TypeOsComboBox->setCurrentIndex(xml.attributes().value("OsR").toInt());
                        ui->lineEdit_usl_x->setText(xml.attributes().value("XuslR").toString());
                        ui->lineEdit_usl_y->setText(xml.attributes().value("YuslR").toString());
                        xml.readNext();
                    }


                    //DemoPredRasOS
                    if (xml.name() == "DemoPredRasOS") {
                        if (xml.attributes().value("uslPR").toString()=="1"){
                            ui->checkBox_usl_2->setChecked(true);
                        } else {
                            ui->checkBox_usl_2->setChecked(false);
                        }
                        ui->TypeOsComboBox_2->setCurrentIndex(xml.attributes().value("OsPR").toInt());
                        ui->lineEdit_usl_x_2->setText(xml.attributes().value("XuslPR").toString());
                        ui->lineEdit_usl_y_2->setText(xml.attributes().value("YuslPR").toString());
                        xml.readNext();
                    }



                }
                xml.readNext();
            }
        }
        //конец
        file.close();
    }
    ui->lineEdit_RazbivkaPKnach->setText("0+0.0000");
    ui->lineEdit_RazbivkaPKkon->setText("0+0.0000");

    statusBar()->showMessage(tr("Демо-файл  открыт."), 3000);
    on_Button_Predraschet_clicked();
}

void MainWindow::on_actionSaveAs_triggered()
{    
    QString dir = QDir::home().absolutePath()+QDir::separator()+QStandardPaths::displayName(QStandardPaths::DocumentsLocation)+QDir::separator();
    if(QDir(dir).exists())
    {
        qDebug()<<"Есть Папка";
    } else {
        dir = QDir::home().absolutePath()+QDir::separator();
        qDebug()<<"Нету Папки";
    }
    QString nameFile;
    if (openFile.isEmpty()){
        nameFile   = QFileDialog::getSaveFileName(this,tr("Выберите файл проекта"),dir+tr("Проект"),tr("Проект_QtTrassa (*.mtrassa)") );
    } else {
        nameFile   = QFileDialog::getSaveFileName(this,tr("Выберите файл проекта"),openFile,tr("Проект_QtTrassa (*.mtrassa)") );
    }

    QFileInfo inf(nameFile);

    if (inf.suffix().isEmpty()){
        nameFile.append(".mtrassa");
    } else {
        qDebug()<<"Расширение есть"<<inf.suffix();
    }
    if(nameFile.isEmpty())    {
        return;
    } else{
        qDebug()<<nameFile;
        saveFile(nameFile);
    }
}

void MainWindow::on_action_settings_triggered()
{    
    QMessageBox msgBox;
    msgBox.setText("Настройки...");
    msgBox.setInformativeText("Настроек пока нет.");
    msgBox.setIcon(QMessageBox::Warning);
    QPushButton *yes = msgBox.addButton(tr("Да"), QMessageBox::ActionRole);
    msgBox.addButton(QObject::tr("Отмена"), QMessageBox::ActionRole);
    msgBox.setDefaultButton(yes);
    msgBox.exec();

}

void MainWindow::readListTemplete(QString dir)
{
    ui->comboBox_template->clear();
    pathDirTemplate=QDir::toNativeSeparators(QDir(dir).absolutePath());
    dir=QDir::toNativeSeparators(dir);

    QString file;
    //Проверка файла на наличие, если нет копируем из основной программы
    QString path_templateFull(QDir::toNativeSeparators(QCoreApplication::applicationDirPath()
                                                       +QDir::separator()
                                                       + "templateFull.xml"));
    qDebug()<<"Нативные сепораторы path_templateFull" << path_templateFull;
    QString path_templateFullinDocuments = QDir::toNativeSeparators(dir
                                                                    + QDir::separator()
                                                                    + "templateFull.xml");
    qDebug()<<"Нативные сепораторы path_templateFullinDocuments" << path_templateFull;

    if (!QDir(path_templateFullinDocuments).exists()){
        QFile::copy(path_templateFull,path_templateFullinDocuments);
    }
    //Читаем список файлов.
    QDirIterator qdi(dir,
                     QStringList() << "*.xml",
                     QDir::NoSymLinks | QDir::Files);
    while (qdi.hasNext()) {
        file = qdi.next();
        qDebug()<<"Файл:"<<file<<"Dir"<<dir;
        ui->comboBox_template->addItem(QFileInfo(file).baseName());
    }
}


void MainWindow::on_pB_reload_template_clicked()
{
    QString dir;

#ifdef Q_OS_WIN
    dir = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).last()+ QDir::separator();
    qDebug()<<"Проверка под вендой"<<QDir(dir).absolutePath();
    if(QDir(dir).exists())
    {
        qDebug()<<"Есть Папка DocumentsLocation"<<dir;
        dir.append(QDir::separator());
        dir.append("qTrassaTemplate");
        if(QDir(dir).exists())
        {
            qDebug()<<"Есть папка"<<dir;
            readListTemplete(dir);
        } else {
            qDebug()<<"Нету папки"<<dir;
            //Создаем папку и файл
            QDir().mkpath(dir);
            readListTemplete(dir);
        }


    } else {
        //Для WindowsXP
        qDebug()<<"Нету Папки DocumentsLocation"<<dir;
        dir = QDir::home().absolutePath()
                +QDir::separator()
                +"Мои документы"
                +QDir::separator();
        if(QDir(dir).exists())
        {
            qDebug()<<"Есть папка Мои документы"<<dir;
            readListTemplete(dir);
        } else {
            qDebug()<<"Нету папки Мои документы"<<dir;
            //Создаем папку и файл
            QDir().mkpath(dir);
            readListTemplete(dir);
        }

    }

#endif

#ifdef Q_OS_LINUX
    dir = QDir::home().absolutePath()+QDir::separator()+QStandardPaths::displayName(QStandardPaths::DocumentsLocation)+QDir::separator();
    if(QDir(dir).exists())
    {
        qDebug()<<"Есть папка DocumentsLocation"<<dir;
        dir.append(QDir::separator());
        dir.append("qTrassaTemplate");
        dir.append(QDir::separator());
        if(QDir(dir).exists())
        {
            qDebug()<<"Linux есть папка qTrassaTemplate"<<dir;
            //читаем содержимое
            readListTemplete(dir);
        } else {
            qDebug()<<"Linux нету папки qTrassaTemplate"<<dir;
            //Создаем папку и файл
            QDir().mkpath(dir);
            readListTemplete(dir);
        }

    } else {
        qDebug()<<"Нету Папки DocumentsLocation ищем Документы"<<dir;
        dir = QDir::home().absolutePath()+QDir::separator()+tr("Документы")+QDir::separator();
        if(QDir(dir).exists())
        {
            qDebug()<<"Linux есть папка Документы"<<dir;
            dir.append("qTrassaTemplate");
            dir.append(QDir::separator());
            if(QDir(dir).exists())
            {
                qDebug()<<"Linux есть папка qTrassaTemplate"<<dir;
                //читаем содержимое
                readListTemplete(dir);
            } else {
                qDebug()<<"Linux нету папки qTrassaTemplate"<<dir;
                //Создаем папку и файл
                QDir().mkpath(dir);
                readListTemplete(dir);
            }
        } else {
            qDebug()<<"Linux нету папки Документы! Ну и пофиг, TO-DO"<<dir;
        }
    }
#endif




}

void MainWindow::on_pB_edit_template_clicked()
{
    qDebug()<<ui->comboBox_template->itemText(ui->comboBox_template->currentIndex());

#ifdef Q_OS_WIN
    QString file = pathDirTemplate + QDir::separator() + ui->comboBox_template->itemText(ui->comboBox_template->currentIndex()) + ".xml";
    qDebug()<<file ;

    QSettings settings("HKEY_CLASSES_ROOT\\Applications\\winword.exe\\shell\\edit\\command",QSettings::NativeFormat);
    QString s = settings.value("Default").toString();
    s=s.split('"').value(1);
    QStringList arguments;
    arguments << file;

    QProcess *myProcess = new QProcess();
    myProcess->start(QDir::toNativeSeparators(s), arguments);
#endif

#ifdef Q_OS_LINUX
    QString file = pathDirTemplate + QDir::separator()+ ui->comboBox_template->itemText(ui->comboBox_template->currentIndex()) + ".xml";
    qDebug()<<file<< "Путь Линукс";
#endif

}

void MainWindow::on_pushButton_Razbivka_clicked()
{
    ui->tableWidget->setRowCount(1);
    ui->tableWidget->setItem( 0,0,new QTableWidgetItem("")); //Pk
    ui->tableWidget->setItem( 0,1,new QTableWidgetItem("")); //Pkusl
    ui->tableWidget->setItem( 0,2,new QTableWidgetItem("")); //x
    ui->tableWidget->setItem( 0,3,new QTableWidgetItem("")); //y
    ui->tableWidget->setItem( 0,4,new QTableWidgetItem("")); //h

    geo zadacha;
    qDebug() << "Считываем длинну Стандартного пикета";
    zadacha.PkStnd = ui->EditStndPk->text().toDouble(); //Считываем длинну Стандартного пикета

    //Ввод таблицы неправильного пикета
    qDebug() << "Ввод таблицы неправильного пикета";
    for (int i=0;i<(ui->tableNPK->rowCount());i++){
        zadacha.appendNpk(ui->tableNPK->item(i,0)->text(),ui->tableNPK->item(i,1)->text());
    }

    //Ввод таблицы трасса
    qDebug() << "Ввод таблицы трасса";
    //QString KrLine,QString PKn,QString Xn,QString Yn,QString Xk,QString Yk,QString Xck,QString Yck)
    for (int i = 0; i < ui->tableTrassa->rowCount();i++ ){
        zadacha.appendTrassa(ui->tableTrassa->item(i,0)->text(), //type
                             ui->EditPKnach->text(), //ПК начала трассы
                             ui->EditXnach->text(),  //X начала трассы
                             ui->EditYnach->text(),  //Y начала трассы
                             ui->tableTrassa->item(i,1)->text(), //Xk
                             ui->tableTrassa->item(i,2)->text(), //Yk
                             ui->tableTrassa->item(i,3)->text(), //Xck
                             ui->tableTrassa->item(i,4)->text());//Yck
    }

    //Ввод таблицы Переходных кривых
    qDebug() << "Ввод таблицы Переходных кривых";
    for (int i = 0; i < ui->tableTrassa_2->rowCount();i++ ){
        zadacha.appendPerehod(ui->tableTrassa_2->item(i,0)->text(), //PKin
                              ui->tableTrassa_2->item(i,1)->text(), //Lin
                              ui->tableTrassa_2->item(i,2)->text(), //Cin
                              ui->tableTrassa_2->item(i,3)->text(), //q
                              ui->tableTrassa_2->item(i,4)->text(), //z
                              ui->tableTrassa_2->item(i,5)->text(), //PKout
                              ui->tableTrassa_2->item(i,6)->text(), //Lout
                              ui->tableTrassa_2->item(i,7)->text(), //Cout
                              ui->tableTrassa_2->item(i,8)->text()); //h
    }

    //Ввод таблицы Профиля
    for (int i = 0; i < ui->tableProfil->rowCount();i++ ){
        zadacha.appendProfil(ui->tableProfil->item(i,0)->text(),
                             ui->tableProfil->item(i,1)->text(),
                             ui->tableProfil->item(i,2)->text());
    }

    qDebug() << "Предрасчет";
    zadacha.predRaschet();

    //Считаем точки
    double  xkoor       = 0;
    double  ykoor       = 0;
    QString PKpoint     = "0";
    double  SMpoint     = 0;
    double  SMpointZ    = 0;
    double  SMpointZQ   = 0;
    double  Hpoint      = 0;
    double  h           = 0;
    double  q           = 0;
    double  z           = 0;


    PKpoint = ui->lineEdit_RazbivkaPKnach->text();
    switch (ui->TypeOsComboBox_3->currentIndex()) {
    case 0: //Разбивочная ось
        SMpoint   = 0.00001;
        SMpointZ  = 0.0000;
        SMpointZQ = 0.0000;
        break;
    case 1: //Ось пути +Z
        SMpoint   = 0.0000;
        SMpointZ  = 0.00001;
        SMpointZQ = 0.0000;
        break;
    case 2: //Ось тоннеля +Z +Q
        SMpoint   = 0.0000;
        SMpointZ  = 0.0000;
        SMpointZQ = 0.00001;
        break;
    }

    zadacha.raschetXY(PKpoint,SMpoint,SMpointZ,SMpointZQ,xkoor,ykoor,h,q,z);
    if (QString::number(xkoor,'f',3).toDouble()==0 &&
            QString::number(ykoor,'f',3).toDouble()==0){
        statusBar()->showMessage(tr("Разбивку трассы выполнить НЕВОЗМОЖНО! Координаты начала участка не вычислены." ), 3000);
    } else {
        //zadacha.raschetH(PKpoint,Hpoint);
        double lnach = zadacha.GetLpk2pk(zadacha.TrassaPKnach.value(0),
                                         PKpoint,
                                         zadacha.PkStnd,
                                         zadacha.Npk,
                                         zadacha.NpL);

        PKpoint = ui->lineEdit_RazbivkaPKkon->text();
        zadacha.raschetXY(PKpoint,SMpoint,SMpointZ,SMpointZQ,xkoor,ykoor,h,q,z);
        if (QString::number(xkoor,'f',3).toDouble()==0 &&
                QString::number(ykoor,'f',3).toDouble()==0){
            statusBar()->showMessage(tr("Разбивку трассы выполнить НЕВОЗМОЖНО! Координаты конца участка не вычислены."), 3000);
        } else {
            double lkon = zadacha.GetLpk2pk(zadacha.TrassaPKnach.value(0),
                                            PKpoint,
                                            zadacha.PkStnd,
                                            zadacha.Npk,
                                            zadacha.NpL);
            int count;
            QString PKnew = "0";
            double luch;

            int iPk;
            double iPkPlus;

            if (lnach<lkon){
                count=floor((lkon-lnach)/ui->doubleSpinBox_RazbivkaL->value());
                PKpoint = ui->lineEdit_RazbivkaPKnach->text();
                qDebug() << "ХреньО " << SMpoint << SMpointZ << SMpointZQ << xkoor << ykoor ;
                zadacha.raschetXY(PKpoint,SMpoint,SMpointZ,SMpointZQ,xkoor,ykoor,h,q,z);
                zadacha.raschetH(PKpoint,Hpoint);
                ui->tableWidget->setRowCount(count+1);
                ui->tableWidget->setItem(0,0,new QTableWidgetItem(ui->lineEdit_RazbivkaPKnach->text())); //Истинный
                ui->tableWidget->setItem(0,1,new QTableWidgetItem("0+0.0000"));
                ui->tableWidget->setItem(0,2,new QTableWidgetItem(QString::number( xkoor,   'f', 4 )));
                ui->tableWidget->setItem(0,3,new QTableWidgetItem(QString::number( ykoor,   'f', 4 )));
                ui->tableWidget->setItem(0,4,new QTableWidgetItem(QString::number( Hpoint,   'f', 4 )));
                ui->tableWidget->setItem(0,5,new QTableWidgetItem(QString::number( z,   'f', 4 )));
                ui->tableWidget->setItem(0,6,new QTableWidgetItem(QString::number( q,   'f', 4 )));
                ui->tableWidget->setItem(0,7,new QTableWidgetItem(QString::number( h,   'f', 4 )));
                iPk = 0;
                iPkPlus = 0;
                for (int i = 1; i < (count + 1);i++ ){
                    iPkPlus+= ui->doubleSpinBox_RazbivkaL->value();
                    if (iPkPlus >= zadacha.PkStnd) {
                        iPkPlus-= zadacha.PkStnd;
                        iPk++;
                    }
                    PKnew = zadacha.GetPK2L(ui->lineEdit_RazbivkaPKnach->text(),
                                            ui->doubleSpinBox_RazbivkaL->value()*i,
                                            zadacha.PkStnd,
                                            zadacha.Npk,
                                            zadacha.NpL);
                    qDebug() << "ХреньО " << SMpoint << SMpointZ << SMpointZQ << xkoor << ykoor ;
                    zadacha.raschetXY(PKnew,SMpoint,SMpointZ,SMpointZQ,xkoor,ykoor,h,q,z);
                    qDebug() << "ХреньЖ " << SMpoint << SMpointZ << SMpointZQ << xkoor << ykoor ;
                    zadacha.raschetH(PKnew,Hpoint);
                    ui->tableWidget->setItem(i,0,new QTableWidgetItem(PKnew)); //Истинный
                    ui->tableWidget->setItem(i,1,new QTableWidgetItem(QString::number(iPk,'f',0)+ "+" +QString::number(iPkPlus,'f',4)));
                    ui->tableWidget->setItem(i,2,new QTableWidgetItem(QString::number( xkoor,   'f', 4 )));
                    ui->tableWidget->setItem(i,3,new QTableWidgetItem(QString::number( ykoor,   'f', 4 )));
                    ui->tableWidget->setItem(i,4,new QTableWidgetItem(QString::number( Hpoint,  'f', 4 )));
                    ui->tableWidget->setItem(i,5,new QTableWidgetItem(QString::number( z,   'f', 4 )));
                    ui->tableWidget->setItem(i,6,new QTableWidgetItem(QString::number( q,   'f', 4 )));
                    ui->tableWidget->setItem(i,7,new QTableWidgetItem(QString::number( h,   'f', 4 )));

                }
            } else {
                luch=lnach-lkon;
                count=floor(luch/ui->doubleSpinBox_RazbivkaL->value());

                PKpoint = ui->lineEdit_RazbivkaPKnach->text();
                zadacha.raschetXY(PKpoint,SMpoint,SMpointZ,SMpointZQ,xkoor,ykoor,h,q,z);
                zadacha.raschetH(PKpoint,Hpoint);
                ui->tableWidget->setRowCount(count+1);
                ui->tableWidget->setItem(0,0,new QTableWidgetItem(ui->lineEdit_RazbivkaPKnach->text())); //Истинный
                ui->tableWidget->setItem(0,1,new QTableWidgetItem("0+0.0000"));
                ui->tableWidget->setItem(0,2,new QTableWidgetItem(QString::number( xkoor,   'f', 4 )));
                ui->tableWidget->setItem(0,3,new QTableWidgetItem(QString::number( ykoor,   'f', 4 )));
                ui->tableWidget->setItem(0,4,new QTableWidgetItem(QString::number( Hpoint,  'f', 4 )));
                ui->tableWidget->setItem(0,5,new QTableWidgetItem(QString::number( z,   'f', 4 )));
                ui->tableWidget->setItem(0,6,new QTableWidgetItem(QString::number( q,   'f', 4 )));
                ui->tableWidget->setItem(0,7,new QTableWidgetItem(QString::number( h,   'f', 4 )));

                iPk = 0;
                iPkPlus = 0;
                for (int i = 1; i < (count + 1);i++ ){
                    iPkPlus+= ui->doubleSpinBox_RazbivkaL->value();
                    if (iPkPlus >= zadacha.PkStnd) {
                        iPkPlus-= zadacha.PkStnd;
                        iPk++;
                    }
                    luch = luch - ui->doubleSpinBox_RazbivkaL->value();
                    PKnew = zadacha.GetPK2L(ui->lineEdit_RazbivkaPKkon->text(),
                                            luch,
                                            zadacha.PkStnd,
                                            zadacha.Npk,
                                            zadacha.NpL);
                    zadacha.raschetXY(PKnew,SMpoint,SMpointZ,SMpointZQ,xkoor,ykoor,h,q,z);
                    zadacha.raschetH(PKnew,Hpoint);
                    ui->tableWidget->setItem(i,0,new QTableWidgetItem(PKnew)); //Истинный
                    ui->tableWidget->setItem(i,1,new QTableWidgetItem(QString::number(iPk,'f',0)+ "+" +QString::number(iPkPlus,'f',4)));
                    ui->tableWidget->setItem(i,2,new QTableWidgetItem(QString::number( xkoor,   'f', 4 )));
                    ui->tableWidget->setItem(i,3,new QTableWidgetItem(QString::number( ykoor,   'f', 4 )));
                    ui->tableWidget->setItem(i,4,new QTableWidgetItem(QString::number( Hpoint,  'f', 4 )));
                    ui->tableWidget->setItem(i,5,new QTableWidgetItem(QString::number( z,   'f', 4 )));
                    ui->tableWidget->setItem(i,6,new QTableWidgetItem(QString::number( q,   'f', 4 )));
                    ui->tableWidget->setItem(i,7,new QTableWidgetItem(QString::number( h,   'f', 4 )));
                }

            }

            statusBar()->showMessage(tr("Разбивка трассы выполнена"), 3000);
            ui->pushButtonExport2DXF->setEnabled(true);

        }
    }

}

void MainWindow::on_tableWidget_customContextMenuRequested(const QPoint &pos){
    QMenu *menu=new QMenu(this);
    menu->addAction(new QAction("Копировать все", this));
    menu->addSeparator();
    menu->addAction(new QAction("Очистить", this));

    QAction* selectedItem= menu->exec(ui->tableWidget->viewport()->mapToGlobal(pos));
    if (selectedItem)
    {
        QClipboard *clipboard = QApplication::clipboard();
        QString s = "";
        if (selectedItem->text()=="Копировать все") {
            for (int i = 0; i < ui->tableWidget->rowCount();i++ ){
                s += ui->tableWidget->item(i,0)->text()+"\t";
                s += ui->tableWidget->item(i,1)->text()+"\t";
                s += ui->tableWidget->item(i,2)->text()+"\t";
                s += ui->tableWidget->item(i,3)->text()+"\t";
                s += ui->tableWidget->item(i,4)->text()+"\n";
            }
            clipboard->setText(s);
        }
        if (selectedItem->text()=="Очистить") {
            ui->tableWidget->setRowCount(1);
            ui->tableWidget->setItem( 0,0,new QTableWidgetItem("")); //Pk
            ui->tableWidget->setItem( 0,1,new QTableWidgetItem("")); //Pkusl
            ui->tableWidget->setItem( 0,2,new QTableWidgetItem("")); //x
            ui->tableWidget->setItem( 0,3,new QTableWidgetItem("")); //y
            ui->tableWidget->setItem( 0,4,new QTableWidgetItem("")); //h
        }
    }
}


void MainWindow::on_pushButton_Revert_clicked()
{
    QString nach = ui->lineEdit_RazbivkaPKnach->text();
    QString kon = ui->lineEdit_RazbivkaPKkon->text();
    ui->lineEdit_RazbivkaPKnach->setText(kon);
    ui->lineEdit_RazbivkaPKkon->setText(nach);
}

void MainWindow::saveDXF(QString fileSavePath)
{
    QString lineSave;
    QString nameLayer = "_no";

    switch (ui->TypeOsComboBox_3->currentIndex()) {
    case 0: //Разбивочная ось
        nameLayer = "_razbivOs";
        break;
    case 1: //Ось пути +Z
        nameLayer = "_osPuti";
        break;
    case 2: //Ось тоннеля +Z +Q
        nameLayer = "_osTonnelya";
        break;
    }


    QFile fileSave( fileSavePath);
    if (!fileSave.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;}
    QTextStream out(&fileSave);
    out.setCodec("CP1251");
    //HEADER
    lineSave = "0\r";       out << lineSave;
    lineSave = "SECTION\r"; out << lineSave;
    lineSave = "2\r"; out << lineSave;
    lineSave = "ENTITIES\r"; out << lineSave;
    for (int i = 0; i < ui->tableWidget->rowCount();i++ ){
        //POINT
        lineSave = "0\r"; out << lineSave;
        lineSave = "POINT\r"; out << lineSave;
        lineSave = "8\r"; out << lineSave;
        lineSave = "POINT"+nameLayer+ "\r"; out << lineSave;
        lineSave = "10\r"; out << lineSave;
        lineSave = ui->tableWidget->item(i,3)->text() + "\r"; out << lineSave;
        lineSave = "20\r"; out << lineSave;
        lineSave = ui->tableWidget->item(i,2)->text() + "\r"; out << lineSave;
        lineSave = "30\r"; out << lineSave;
        lineSave = ui->tableWidget->item(i,4)->text() + "\r"; out << lineSave;
        //TEXT
        lineSave = "0\r"; out << lineSave;
        lineSave = "TEXT\r"; out << lineSave;
        lineSave = "8\r"; out << lineSave;
        lineSave = "PK_usl"+nameLayer+ "\r"; out << lineSave;
        lineSave = "10\r"; out << lineSave;
        lineSave = ui->tableWidget->item(i,3)->text() + "\r"; out << lineSave;
        lineSave = "20\r"; out << lineSave;
        lineSave = ui->tableWidget->item(i,2)->text() + "\r"; out << lineSave;
        lineSave = "30\r"; out << lineSave;
        lineSave = "0.0\r"; out << lineSave;
        lineSave = "40\r"; out << lineSave;
        lineSave = "0.1\r"; out << lineSave;
        lineSave = "1\r"; out << lineSave;
        lineSave = ui->tableWidget->item(i,1)->text() + "\r"; out << lineSave;
        //TEXT
        lineSave = "0\r"; out << lineSave;
        lineSave = "TEXT\r"; out << lineSave;
        lineSave = "8\r"; out << lineSave;
        lineSave = "PK_true"+nameLayer+ "\r"; out << lineSave;
        lineSave = "10\r"; out << lineSave;
        lineSave = ui->tableWidget->item(i,3)->text() + "\r"; out << lineSave;
        lineSave = "20\r"; out << lineSave;
        lineSave = ui->tableWidget->item(i,2)->text() + "\r"; out << lineSave;
        lineSave = "30\r"; out << lineSave;
        lineSave = "0.0\r"; out << lineSave;
        lineSave = "40\r"; out << lineSave;
        lineSave = "0.1\r"; out << lineSave;
        lineSave = "1\r"; out << lineSave;
        lineSave = ui->tableWidget->item(i,0)->text() + "\r"; out << lineSave;

        lineSave = "11\r"; out << lineSave;
        lineSave = ui->tableWidget->item(i,3)->text() + "\r"; out << lineSave;
        lineSave = "21\r"; out << lineSave;
        lineSave = ui->tableWidget->item(i,2)->text() + "\r"; out << lineSave;
        lineSave = "31\r"; out << lineSave;
        lineSave = "0.0\r"; out << lineSave;

        lineSave = "73\r"; out << lineSave;
        lineSave = "3\r"; out << lineSave;


    }

    //POLYLINE
    lineSave = "0\r"; out << lineSave;
    lineSave = "POLYLINE\r"; out << lineSave;
    lineSave = "8\r"; out << lineSave;
    lineSave = "POLYLINE"+nameLayer+ "\r"; out << lineSave;
    lineSave = "66\r"; out << lineSave;
    lineSave = "1\r"; out << lineSave;
    lineSave = "10\r"; out << lineSave;
    lineSave = "0.0\r"; out << lineSave;
    lineSave = "20\r"; out << lineSave;
    lineSave = "0.0\r"; out << lineSave;
    lineSave = "30\r"; out << lineSave;
    lineSave = "0.0\r"; out << lineSave;

    for (int i = 0; i < ui->tableWidget->rowCount();i++ ){
        //VERTEX
        lineSave = "0\r"; out << lineSave;
        lineSave = "VERTEX\r"; out << lineSave;
        lineSave = "8\r"; out << lineSave;
        lineSave = "POLYLINE"+nameLayer+ "\r"; out << lineSave;
        lineSave = "10\r"; out << lineSave;
        lineSave = ui->tableWidget->item(i,3)->text() + "\r"; out << lineSave;
        lineSave = "20\r"; out << lineSave;
        lineSave = ui->tableWidget->item(i,2)->text() + "\r"; out << lineSave;
        lineSave = "30\r"; out << lineSave;
        lineSave = "0.0\r"; out << lineSave;
    }
    lineSave = "0\r"; out << lineSave;
    lineSave = "SEQEND\r"; out << lineSave;
    lineSave = "8\r"; out << lineSave;
    lineSave = "POLYLINE"+nameLayer+ "\r"; out << lineSave;

    //FOOT
    lineSave = "0\r"; out << lineSave;
    lineSave = "ENDSEC\r"; out << lineSave;
    lineSave = "0\r"; out << lineSave;
    lineSave = "EOF\r"; out << lineSave;

    fileSave.close();

}


void MainWindow::on_pushButtonExport2DXF_clicked()
{
    QString dir = QDir::home().absolutePath()+QDir::separator()+QStandardPaths::displayName(QStandardPaths::DocumentsLocation)+QDir::separator();
    if(QDir(dir).exists())
    {
        qDebug()<<"Есть Папка";
    } else {
        dir = QDir::home().absolutePath()+QDir::separator();
        qDebug()<<"Нету Папки";
    }
    QString nameFile;

    nameFile   = QFileDialog::getSaveFileName(this,tr("Выберите папку и файл для экспорта"),dir+tr("Схема"),tr("Разбивка_трассы (*.dxf)") );

    QFileInfo inf(nameFile);

    if(!nameFile.isEmpty())
    {
        if (inf.suffix().isEmpty()){
            nameFile.append(".dxf");
        }

        qDebug()<<nameFile;
        saveDXF(nameFile);

        if (ui->checkBox->isChecked()){

            QSettings setAcadVer("HKEY_CLASSES_ROOT\\.dxf\\OpenWithprogids",QSettings::NativeFormat);
            QString sAcadVer = "HKEY_CLASSES_ROOT\\" + setAcadVer.allKeys().at(0)+ "\\shell\\open\\command";

            //QSettings setAcadPath("HKEY_CLASSES_ROOT\\ACAD.B001.419\\shell\\open\\command",QSettings::NativeFormat);
            QSettings setAcadPath(sAcadVer,QSettings::NativeFormat);
            QString s = setAcadPath.value("Default").toString();
            s=s.split('"').value(1);
            QStringList arguments;
            arguments << nameFile;

            QProcess *myProcess = new QProcess();
            myProcess->start(QDir::toNativeSeparators(s), arguments);
        }

    }
    //saveDXF("C:/Windows/Temp/qTrassa.dxf");


    //        [HKEY_CLASSES_ROOT\.dxf\OpenWithprogids]
    //        "ACAD.B001.419"=" "
    //        [HKEY_CLASSES_ROOT\ACAD.B001.419\shell\open\command]
    //        @="\"C:\\Program Files\\Autodesk\\AutoCAD 2013\\acad.exe\""

}

void MainWindow::on_lineEdit_RazbivkaPKnach_textChanged(const QString &arg1)
{
    ui->pushButtonExport2DXF->setEnabled(false);
    qDebug()<<arg1;
}

void MainWindow::on_lineEdit_RazbivkaPKkon_textChanged(const QString &arg1)
{
    ui->pushButtonExport2DXF->setEnabled(false);
    qDebug()<<arg1;
}

void MainWindow::on_doubleSpinBox_RazbivkaL_valueChanged(const QString &arg1)
{
    ui->pushButtonExport2DXF->setEnabled(false);
    qDebug()<<arg1;
}

void MainWindow::on_TypeOsComboBox_3_currentIndexChanged(const QString &arg1)
{
    ui->pushButtonExport2DXF->setEnabled(false);
    qDebug()<<arg1;
}



void MainWindow::on_pushButton_Svodnaya_kolca_clicked()
{
    geo zadacha;
    qDebug() << "Считываем длинну Стандартного пикета";
    zadacha.PkStnd = ui->EditStndPk->text().toDouble(); //Считываем длинну Стандартного пикета
    //Ввод таблицы неправильного пикета
    qDebug() << "Ввод таблицы неправильного пикета";
    for (int i=0;i<(ui->tableNPK->rowCount());i++){
        zadacha.appendNpk(ui->tableNPK->item(i,0)->text(),ui->tableNPK->item(i,1)->text());
    }
    //Ввод таблицы трасса
    qDebug() << "Ввод таблицы трасса";
    //QString KrLine,QString PKn,QString Xn,QString Yn,QString Xk,QString Yk,QString Xck,QString Yck)
    for (int i = 0; i < ui->tableTrassa->rowCount();i++ ){
        zadacha.appendTrassa(ui->tableTrassa->item(i,0)->text(), //type
                             ui->EditPKnach->text(), //ПК начала трассы
                             ui->EditXnach->text(),  //X начала трассы
                             ui->EditYnach->text(),  //Y начала трассы
                             ui->tableTrassa->item(i,1)->text(), //Xk
                             ui->tableTrassa->item(i,2)->text(), //Yk
                             ui->tableTrassa->item(i,3)->text(), //Xck
                             ui->tableTrassa->item(i,4)->text());//Yck
    }
    //Ввод таблицы Переходных кривых
    qDebug() << "Ввод таблицы Переходных кривых";
    for (int i = 0; i < ui->tableTrassa_2->rowCount();i++ ){
        zadacha.appendPerehod(ui->tableTrassa_2->item(i,0)->text(), //PKin
                              ui->tableTrassa_2->item(i,1)->text(), //Lin
                              ui->tableTrassa_2->item(i,2)->text(), //Cin
                              ui->tableTrassa_2->item(i,3)->text(), //q
                              ui->tableTrassa_2->item(i,4)->text(), //z
                              ui->tableTrassa_2->item(i,5)->text(), //PKout
                              ui->tableTrassa_2->item(i,6)->text(), //Lout
                              ui->tableTrassa_2->item(i,7)->text()); //Cout
    }

    //Ввод таблицы Профиля
    for (int i = 0; i < ui->tableProfil->rowCount();i++ ){
        zadacha.appendProfil(ui->tableProfil->item(i,0)->text(),
                             ui->tableProfil->item(i,1)->text(),
                             ui->tableProfil->item(i,2)->text());
    }

    qDebug() << "Предрасчет";
    zadacha.predRaschet();


    //Считаем точки
    bool xOk;
    bool yOk;

    double xkoor = 0;
    double ykoor = 0;
    QString PKpoint  = "0";
    double SMpoint  = 0;
    double SMpointZ  = 0;
    double SMpointZQ = 0;
    double Hpoint = 0;

    QString RL = "L";

    QVector<ring> rings;
    ring xring;


    //Условные координаты
    double x_usl_koor = ui->lineEdit_usl_x_3->text().toDouble(&xOk);
    double y_usl_koor = ui->lineEdit_usl_y_3->text().toDouble(&yOk);
    if ((xOk != true) || (yOk != true)){
        x_usl_koor = 0;
        y_usl_koor = 0;
    }
    //qDebug()<<x_usl_koor<<xOk<< "  "<<y_usl_koor<<yOk;


    for (int i = 0; i < ui->table_Kolca->rowCount();i++ ){
        if (ui->checkBox_usl_3->isChecked()==true ){
            xkoor = ui->table_Kolca->item(i,2)->text().toDouble(&xOk)+x_usl_koor;
            ykoor = ui->table_Kolca->item(i,3)->text().toDouble(&yOk)+y_usl_koor;
        } else {
            xkoor = ui->table_Kolca->item(i,2)->text().toDouble(&xOk);
            ykoor = ui->table_Kolca->item(i,3)->text().toDouble(&yOk);
        }

        PKpoint  = "0";
        SMpoint   = 0;
        SMpointZ  = 0;
        SMpointZQ = 0;
        Hpoint    = 0;

        //        0 	1	2	3   4       5       6       7       8   9       10          11      12      13
        //        №№	№R  X	Y	Hфакт	D(1+4)	B=1,75	R пр	ПК	Смещ	HпрУГР  	Hпр 	Rфакт	Откл.

        if ((xOk == true) && (yOk == true)){
            zadacha.raschetPkSm(xkoor,
                                ykoor,
                                PKpoint,
                                SMpoint,
                                RL);

            if(PKpoint!="0"){
                zadacha.raschetPer(PKpoint,
                                   SMpoint,
                                   RL,
                                   SMpointZ,
                                   SMpointZQ);

                zadacha.raschetH(PKpoint,Hpoint);

                xring.nkolca = ui->table_Kolca->item(i,0)->text().toInt(&xOk);    //Номер кольца
                xring.nr     = ui->table_Kolca->item(i,1)->text().toInt(&xOk);    //Номер радиуса

                xring.rx     = ui->table_Kolca->item(i,2)->text().toDouble(&xOk); //x факт Радиуса
                xring.ry     = ui->table_Kolca->item(i,3)->text().toDouble(&xOk); //y факт Радиуса
                xring.rh     = ui->table_Kolca->item(i,4)->text().toDouble(&yOk); //h факт Радиуса

                switch (xring.nr) {
                case 1:
                    xring.D1_4   = ui->table_Kolca->item(i,5)->text().toDouble(&xOk); //Диаметр измеренный
                    break;
                case 2:
                    xring.D1_4   = ui->table_Kolca->item(i,5)->text().toDouble(&xOk); //Диаметр измеренный
                    break;
                case 3:
                    xring.D1_4   = ui->table_Kolca->item(i,5)->text().toDouble(&xOk); //Диаметр измеренный
                    break;
                case 4:
                    xring.D1_4   = ui->table_Kolca->item(i,5)->text().toDouble(&xOk); //Диаметр измеренный
                    break;
                default:
                    xring.D1_4   = 0;
                    break;
                }

                xring.B      = ui->table_Kolca->item(i,6)->text().toDouble(&xOk); //Домер от УГР до центра кольца (1,75)
                xring.R      = ui->table_Kolca->item(i,7)->text().toDouble(&xOk);  //R     проект

                xring.rPK     = PKpoint;                                                             // ПК
                if (SMpointZQ!=0) {xring.rsm_qz = SMpointZQ; }else{if (SMpointZ!=0){xring.rsm_qz = SMpointZ;} else {xring.rsm_qz = SMpoint;} }  //Смещение от оси тоннеля
                xring.rh_ugr  = Hpoint;                                                              // H УГР
                xring.rf      = sqrt(pow((xring.rh-(xring.rh_ugr+xring.B)),2)+pow(xring.rsm_qz,2));  // Rфакт
                xring.rdiff_r = (xring.rf - xring.R)*1000;                                           // Отклонение

                ui->table_Kolca->setItem(i,8 ,new QTableWidgetItem(PKpoint));
                ui->table_Kolca->setItem(i,9 ,new QTableWidgetItem(QString::number( xring.rsm_qz,              'f', 4 )));           //смещение от оси тоннеля
                ui->table_Kolca->setItem(i,10,new QTableWidgetItem(QString::number( xring.rh_ugr,              'f', 4 )));           // H УГР
                ui->table_Kolca->setItem(i,11,new QTableWidgetItem(QString::number( xring.rh_ugr+xring.B,      'f', 4 )));           // Hпроект
                ui->table_Kolca->setItem(i,12,new QTableWidgetItem(QString::number( xring.rf,                  'f', 4 )));           // Rфакт
                ui->table_Kolca->setItem(i,13,new QTableWidgetItem(QString::number( xring.rdiff_r,             'f', 0 )));           // Отклонение
                if (xring.rdiff_r<50&&xring.rdiff_r>-50){
                    ui->table_Kolca->item(i,13)->setTextColor(QColor(0,0,0));
                }else{
                    ui->table_Kolca->item(i,13)->setTextColor(Qt::red);
                    //ui->table_Kolca->item(i,13)->setFont(new QFont::~QFont());
                }
                ui->table_Kolca->item(i,8)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                ui->table_Kolca->item(i,9)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                ui->table_Kolca->item(i,10)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                ui->table_Kolca->item(i,11)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                ui->table_Kolca->item(i,12)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);
                ui->table_Kolca->item(i,13)->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEditable);

                rings.append(xring);
            }
        }
    }
    statusBar()->showMessage(tr("Расчет выполнен, создаем сводную ведомость"), 3000);

    QString dir = QDir::home().absolutePath()+QDir::separator()+QStandardPaths::displayName(QStandardPaths::DocumentsLocation)+QDir::separator();
    if(QDir(dir).exists())
    {
        qDebug()<<"Есть Папка";
    } else {
        dir = QDir::home().absolutePath()+QDir::separator();
        qDebug()<<"Нету Папки";
    }
    QString nameFile;
    nameFile   = QFileDialog::getSaveFileName(this,tr("Выберите файл экспорта в csv"),dir+tr("Сводная"),tr("Сводная (*.csv)") );

    QFileInfo inf(nameFile);

    if (inf.suffix().isEmpty()){
        nameFile.append(".csv");
    } else {
        qDebug()<<"Расширение есть"<<inf.suffix();
    }
    if(!nameFile.isEmpty())    {
        qDebug()<<nameFile;


        //    1 A
        //    2 B
        //    3 C
        //    4 D
        //    5 E
        //    6 F
        //    7 G
        //    8 H


        QString lineSave;
        double RA = 0;
        double RB = 0;
        double RC = 0;
        double RD = 0;
        double RE = 0;
        double RF = 0;
        double RG = 0;
        double RH = 0;

        double RA_Eism = 0;
        double RB_Fism = 0;
        double RC_Gism = 0;
        double RD_Hism = 0;
        double ddif = 0;
        QString tstroka;

        //#ifdef Q_OS_WIN
        //QFile fileSave( "D:/nesmit/Desktop/Svodnaya.csv");
        QFile fileSave( nameFile );
        //#else

        //            QFile fileSave(QCoreApplication::applicationDirPath() + "/OtchetFull.xml");
        //#endif

        if (!fileSave.open(QIODevice::WriteOnly | QIODevice::Text)) {
            qDebug()<<"Файл Svodnaya.csv не открывается...";
            return;
        } else {
            QTextStream out(&fileSave);
            out.setCodec("Windows-1251");
            out << QString("Номер кольца;ПК кольца;Центр кольца в плане;Центр кольца по высоте;"
                           "Rпроект;R1;dR1;R2;dR2;R3;dR3;R4;dR4;R5;dR5;R6;dR6;R7;dR7;R8;dR8;"
                           "D1-5;d;D2-6;d;D3-7;d;D4-8;d") << endl;
            lineSave = QString("nkolca;PKkolca;xydiff;zdiff;R_pr;radiusAf;radiusAdiff_r;radiusBf;radiusBdiff_r;radiusCf;radiusCdiff_r;radiusDf;radiusDdiff_r;radiusEf;radiusEdiff_r;radiusFf;radiusFdiff_r;radiusGf;radiusGdiff_r;radiusHf;radiusHdiff_r;"
                               "dA_Eizm;dA_Ediff;"
                               "dB_Fizm;dB_Fdiff;"
                               "dC_Gizm;dC_Gdiff;"
                               "dD_Hizm;dD_Hdiff;");
            for (int k = 0; k < rings.count();k++ ){
                if (k==0) {
                } else {
                    if (rings.value(k).nr==1) {
                        lineSave = QString("nkolca;PKkolca;xydiff;zdiff;R_pr;radiusAf;radiusAdiff_r;radiusBf;radiusBdiff_r;radiusCf;radiusCdiff_r;radiusDf;radiusDdiff_r;radiusEf;radiusEdiff_r;radiusFf;radiusFdiff_r;radiusGf;radiusGdiff_r;radiusHf;radiusHdiff_r;"
                                           "dA_Eizm;dA_Ediff;"
                                           "dB_Fizm;dB_Fdiff;"
                                           "dC_Gizm;dC_Gdiff;"
                                           "dD_Hizm;dD_Hdiff;");
                        RA=0;
                        RB=0;
                        RC=0;
                        RD=0;
                        RE=0;
                        RF=0;
                        RG=0;
                        RH=0;
                        RA_Eism=0;
                        RB_Fism=0;
                        RC_Gism=0;
                        RD_Hism=0;
                    }
                }

                lineSave = lineSave.replace("nkolca" ,QString::number(rings.value(k).nkolca));
                tstroka=rings.value(k).rPK.split("+").at(0)+"+"+QString::number(rings.value(k).rPK.split("+").at(1).toDouble(),'f',2);

                lineSave = lineSave.replace("PKkolca",tstroka);
                lineSave = lineSave.replace("DiamPr",QString::number(rings.value(k).R*2,'f',3));
                lineSave = lineSave.replace("R_pr",QString::number(rings.value(k).R,'f',3));

                switch (rings.value(k).nr) {
                case 1:{
                    //A
                    lineSave = lineSave.replace("radiusAx",QString::number(rings.value(k).rx,'f',3));
                    lineSave = lineSave.replace("radiusAy",QString::number(rings.value(k).ry,'f',3));
                    lineSave = lineSave.replace("radiusAh",QString::number(rings.value(k).rh,'f',3));
                    lineSave = lineSave.replace("radiusAf",QString::number(rings.value(k).rf,'f',3));
                    RA=rings.value(k).rf;
                    lineSave = lineSave.replace("radiusAdiff_r",QString::number(rings.value(k).rdiff_r,'f',0));

                    lineSave = lineSave.replace("dA_Eizm" ,QString::number(rings.value(k).D1_4,'f',3));
                    RA_Eism=rings.value(k).D1_4;
                    ddif = (rings.value(k).D1_4-rings.value(k).R*2)*1000;
                    lineSave = lineSave.replace("dA_Ediff",QString::number(ddif,'f',0));
                    if(ddif<50&&ddif>-50){
                        lineSave = lineSave.replace("dA_Eover","нет");
                    }else{
                        if (ddif>0){ddif-=50;}else{ddif+=50;}
                        lineSave = lineSave.replace("dA_Eover",QString::number(ddif,'f',0));
                    }
                    break;
                }
                case 2:{
                    //B
                    lineSave = lineSave.replace("radiusBx",QString::number(rings.value(k).rx,'f',3));
                    lineSave = lineSave.replace("radiusBy",QString::number(rings.value(k).ry,'f',3));
                    lineSave = lineSave.replace("radiusBh",QString::number(rings.value(k).rh,'f',3));
                    lineSave = lineSave.replace("radiusBf",QString::number(rings.value(k).rf,'f',3));
                    RB=rings.value(k).rf;
                    lineSave = lineSave.replace("radiusBdiff_r",QString::number(rings.value(k).rdiff_r,'f',0));

                    lineSave = lineSave.replace("dB_Fizm" ,QString::number(rings.value(k).D1_4,'f',3));
                    RB_Fism=rings.value(k).D1_4;
                    ddif = (rings.value(k).D1_4-rings.value(k).R*2)*1000;
                    lineSave = lineSave.replace("dB_Fdiff",QString::number(ddif,'f',0));
                    if(ddif<50&&ddif>-50){
                        lineSave = lineSave.replace("dB_Fover","нет");
                    }else{
                        if (ddif>0){ddif-=50;}else{ddif+=50;}
                        lineSave = lineSave.replace("dB_Fover",QString::number(ddif,'f',0));
                    }
                    break;
                }
                case 3:{
                    //C
                    lineSave = lineSave.replace("radiusCx",QString::number(rings.value(k).rx,'f',3));
                    lineSave = lineSave.replace("radiusCy",QString::number(rings.value(k).ry,'f',3));
                    lineSave = lineSave.replace("radiusCh",QString::number(rings.value(k).rh,'f',3));
                    lineSave = lineSave.replace("radiusCf",QString::number(rings.value(k).rf,'f',3));
                    RC=rings.value(k).rf;
                    lineSave = lineSave.replace("radiusCdiff_r",QString::number(rings.value(k).rdiff_r,'f',0));

                    lineSave = lineSave.replace("dC_Gizm" ,QString::number(rings.value(k).D1_4,'f',3));
                    RC_Gism=rings.value(k).D1_4;
                    ddif = (rings.value(k).D1_4-rings.value(k).R*2)*1000;
                    lineSave = lineSave.replace("dC_Gdiff",QString::number(ddif,'f',0));
                    if(ddif<50&&ddif>-50){
                        lineSave = lineSave.replace("dC_Gover","нет");
                    }else{
                        if (ddif>0){ddif-=50;}else{ddif+=50;}
                        lineSave = lineSave.replace("dC_Gover",QString::number(ddif,'f',0));
                    }
                    break;
                }
                case 4:{
                    //D
                    lineSave = lineSave.replace("radiusDx",QString::number(rings.value(k).rx,'f',3));
                    lineSave = lineSave.replace("radiusDy",QString::number(rings.value(k).ry,'f',3));
                    lineSave = lineSave.replace("radiusDh",QString::number(rings.value(k).rh,'f',3));
                    lineSave = lineSave.replace("radiusDf",QString::number(rings.value(k).rf,'f',3));
                    RD=rings.value(k).rf;
                    lineSave = lineSave.replace("radiusDdiff_r",QString::number(rings.value(k).rdiff_r,'f',0));

                    lineSave = lineSave.replace("dD_Hizm" ,QString::number(rings.value(k).D1_4,'f',3));
                    RD_Hism=rings.value(k).D1_4;
                    ddif = (rings.value(k).D1_4-rings.value(k).R*2)*1000;
                    lineSave = lineSave.replace("dD_Hdiff",QString::number(ddif,'f',0));
                    if(ddif<50&&ddif>-50){
                        lineSave = lineSave.replace("dD_Hover","нет");
                    }else{
                        if (ddif>0){ddif-=50;}else{ddif+=50;}
                        lineSave = lineSave.replace("dD_Hover",QString::number(ddif,'f',0));
                    }
                    break;
                }
                case 5:{
                    //E
                    lineSave = lineSave.replace("radiusEx",QString::number(rings.value(k).rx,'f',3));
                    lineSave = lineSave.replace("radiusEy",QString::number(rings.value(k).ry,'f',3));
                    lineSave = lineSave.replace("radiusEh",QString::number(rings.value(k).rh,'f',3));
                    lineSave = lineSave.replace("radiusEf",QString::number(rings.value(k).rf,'f',3));
                    lineSave = lineSave.replace("radiusEdiff_r",QString::number(rings.value(k).rdiff_r,'f',0));
                    RE=rings.value(k).rf;
                    lineSave = lineSave.replace("dA_Ec",QString::number((RA+RE),'f',3));
                    lineSave = lineSave.replace("dA_Eddiff",QString::number((RA_Eism-(RA+RE))*1000,'f',0));
                    break;
                }
                case 6:{
                    //F
                    lineSave = lineSave.replace("radiusFx",QString::number(rings.value(k).rx,'f',3));
                    lineSave = lineSave.replace("radiusFy",QString::number(rings.value(k).ry,'f',3));
                    lineSave = lineSave.replace("radiusFh",QString::number(rings.value(k).rh,'f',3));
                    lineSave = lineSave.replace("radiusFf",QString::number(rings.value(k).rf,'f',3));
                    lineSave = lineSave.replace("radiusFdiff_r",QString::number(rings.value(k).rdiff_r,'f',0));
                    RF=rings.value(k).rf;
                    lineSave = lineSave.replace("dB_Fc",QString::number((RB+RF),'f',3));
                    lineSave = lineSave.replace("dB_Fddiff",QString::number(((RB_Fism -(RB+RF))*1000),'f',0));
                    break;
                }
                case 7:{
                    //G
                    lineSave = lineSave.replace("radiusGx",QString::number(rings.value(k).rx,'f',3));
                    lineSave = lineSave.replace("radiusGy",QString::number(rings.value(k).ry,'f',3));
                    lineSave = lineSave.replace("radiusGh",QString::number(rings.value(k).rh,'f',3));
                    lineSave = lineSave.replace("radiusGf",QString::number(rings.value(k).rf,'f',3));
                    lineSave = lineSave.replace("radiusGdiff_r",QString::number(rings.value(k).rdiff_r,'f',0));
                    RG=rings.value(k).rf;
                    lineSave = lineSave.replace("dC_Gc",QString::number((RC+RG),'f',3));
                    lineSave = lineSave.replace("dC_Gddiff",QString::number((RC_Gism-(RC+RG))*1000,'f',0));
                    break;
                }
                case 8:{
                    //H
                    lineSave = lineSave.replace("radiusHx",QString::number(rings.value(k).rx,'f',3));
                    lineSave = lineSave.replace("radiusHy",QString::number(rings.value(k).ry,'f',3));
                    lineSave = lineSave.replace("radiusHh",QString::number(rings.value(k).rh,'f',3));
                    lineSave = lineSave.replace("radiusHf",QString::number(rings.value(k).rf,'f',3));
                    lineSave = lineSave.replace("radiusHdiff_r",QString::number(rings.value(k).rdiff_r,'f',0));
                    RH=rings.value(k).rf;
                    lineSave = lineSave.replace("dD_Hc",QString::number((RD+RH),'f',3));
                    lineSave = lineSave.replace("dD_Hddiff",QString::number((RD_Hism-(RD+RH))*1000,'f',0));

                    ddif=((RE-RA)/2)*1000;
                    lineSave = lineSave.replace("xydiff" ,QString::number(ddif ,'f',0));
                    if(ddif<50&&ddif>-50){
                        lineSave = lineSave.replace("xyoverdiff","нет");
                    }else{
                        if (ddif>0){ddif-=50;}else{ddif+=50;}
                        lineSave = lineSave.replace("xyoverdiff",QString::number(ddif,'f',0));
                    }
                    ddif=( (RC-RG)/2  )*1000;
                    lineSave = lineSave.replace("zdiff"  ,QString::number( ddif ,'f',0));
                    if(ddif<50&&ddif>-50){
                        lineSave = lineSave.replace("zoverdiff","нет");
                    }else{
                        if (ddif>0){ddif-=50;}else{ddif+=50;}
                        lineSave = lineSave.replace("zoverdiff",QString::number(ddif,'f',0));
                    }
                    break;
                }
                }  //switch

                if (rings.value(k).nr==8) {         //фиксируем во временном протоколе все изменения.
                    out << lineSave << endl;
                }
            }//массив колец
            //        out << QString("Конец всему венец");
            fileSave.close();
        }
    } else {
        qDebug() << "имя файла пустое для экспорта";
    }

}

void MainWindow::on_pushButtonZip_clicked()
{

}
