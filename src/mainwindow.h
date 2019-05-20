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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDialog>
//#include <QtGui>

namespace Ui {
class MainWindow;
class CalcQ;
class About;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    QString pathDirTemplate;
    ~MainWindow();

public slots:    

private slots:

    void on_action_close_triggered();

    void on_action_Save_triggered();

    void on_action_open_triggered();

    void on_action_new_triggered();

    void on_Button_Run_clicked();

    void writeSettings();

    void readSettings();

    void on_EditXnach_returnPressed();

    void on_EditYnach_returnPressed();

    void on_EditPKnach_returnPressed();    

    void on_actionAbout_Qt_triggered();

    void on_pushButton_Raschet_run_clicked();

    void on_action_about_program_triggered();

    void on_tableRaschet_customContextMenuRequested(const QPoint &pos);    

    void on_tablePred_Raschet_customContextMenuRequested(const QPoint &pos);

    void on_Button_Primer_clicked();

    void addRowtableTrassa(int row);

    void addRowtableTrassa_2(int row);

    void on_tableTrassa_customContextMenuRequested(const QPoint &pos);

    void on_tableTrassa_2_customContextMenuRequested(const QPoint &pos);

    void on_tableProfil_customContextMenuRequested(const QPoint &pos);

    void on_tableNPK_customContextMenuRequested(const QPoint &pos);

    void on_table_Kolca_customContextMenuRequested(const QPoint &pos);

    void on_tableWidget_customContextMenuRequested(const QPoint &pos);

    void on_Button_Predraschet_clicked();

    void on_pushButton_Pred_Raschet_run_clicked();

    void on_pushButton_RaschetColca_clicked();

    void on_pushButton_Otchet_kolca_clicked();

    void saveFile(QString fileName);

    void fileOpen(QString fileName,bool ask);

    void on_action_demo_triggered();

    void on_actionSaveAs_triggered();

    void on_action_settings_triggered();

    void readListTemplete(QString dir);

    void on_pB_reload_template_clicked();

    void on_pB_edit_template_clicked();

    void on_pushButton_Razbivka_clicked();

    void on_pushButton_Revert_clicked();

    void on_pushButtonExport2DXF_clicked();

    void on_lineEdit_RazbivkaPKnach_textChanged(const QString &arg1);

    void on_lineEdit_RazbivkaPKkon_textChanged(const QString &arg1);

    void on_doubleSpinBox_RazbivkaL_valueChanged(const QString &arg1);    

    void saveDXF(QString fileSavePath);

    void on_TypeOsComboBox_3_currentIndexChanged(const QString &arg1);


    void on_pushButton_Svodnaya_kolca_clicked();

    void on_pushButtonZip_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
