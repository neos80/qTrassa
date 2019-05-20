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

#include "calcq.h"
#include "ui_calcq.h"

#include <QDebug>


CalcQ::CalcQ(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CalcQ)
{
    ui->setupUi(this);
}

CalcQ::~CalcQ()
{
    delete ui;
}

void CalcQ::on_buttonBox_accepted()
{
    qDebug()<< "Ok";
    calc();
    close();
}

void CalcQ::on_buttonBox_rejected()
{
    qDebug()<< "Отмена";
    q=0;
    close();
}

void CalcQ::calc()
{
    double h;
    double A;
    double B;
    h = ui->doubleh->value();
    A = ui->doubleA->value();
    B = ui->doubleB->value();
    q = h*(B/A);
    ui->doubleq->setValue(q);
}


void CalcQ::on_doubleh_valueChanged(double arg1)
{
    qDebug()<<arg1;
    calc();
}

void CalcQ::on_doubleB_valueChanged(double arg1)
{
    qDebug()<<arg1;
    calc();
}

void CalcQ::on_doubleA_valueChanged(double arg1)
{
    qDebug()<<arg1;
    calc();
}
