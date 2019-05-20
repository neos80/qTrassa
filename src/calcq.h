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

#ifndef CALCQ_H
#define CALCQ_H

#include <QDialog>

namespace Ui {
class CalcQ;
}

class CalcQ : public QDialog
{
    Q_OBJECT

public:
    explicit CalcQ(QWidget *parent = nullptr);
    ~CalcQ();
    double q;

public slots:
    void calc();


private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    void on_doubleh_valueChanged(double arg1);

    void on_doubleB_valueChanged(double arg1);

    void on_doubleA_valueChanged(double arg1);

private:
    Ui::CalcQ *ui;
};

#endif // CALCQ_H
