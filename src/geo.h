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

#ifndef GEO_H
#define GEO_H

#include <QString>
#include <QVector>

class geo
{
public:
    geo();

    double stringGradToDouble (QString ugol);     //переводит 0 00'00" в double
    QString DegToGradstring   (double ugol);      //переводит double в 0 00'00"
    QString GradstringToGradstring(QString ugol); //переводит 0 00 00 в 0 00'00"

    void   Pgz (double x1is,double y1is,double x2,double y2,double Ugol,double L, double &xp, double &yp);
    double OgzDir      (double x1,double y1,double x2,double y2);
    double OgzLine     (double x1,double y1,double x2,double y2);
    bool   FindUchLine (double Xnach,double Ynach,double Xkon,double Ykon,double Xpoint,double Ypoint);
    bool   FindUchLine2(double Xnach,double Ynach,double Xkon,double Ykon,double Xpoint,double Ypoint); // не работает, хз почему.
    double PKPlusLine  (double Xnach,double Ynach,double Xkon,double Ykon,double Xpoint,double Ypoint);
    double SmeshLine   (double Xnach,double Ynach,double Xkon,double Ykon,double Xpoint,double Ypoint);

    bool   FindUchKr   (double Xnach,double Ynach,double Xkon,double Ykon,double Xck,double Yck,double Xpoint,double Ypoint);
    bool   Povorot     (double Xnach,double Ynach,double Xkon,double Ykon,double Xck,double Yck);
    double PKPlusKr    (double Xnach,double Ynach,double Xck,double Yck,double Xpoint,double Ypoint);
    double SmeshKr     (bool Povorot,double Xnach,double Ynach,double Xck,double Yck,double Xpoint,double Ypoint);
    double ugol        (double Xnach,double Ynach,double Xpoint,double Ypoint,double Xck,double Yck);
    double linekr      (double Xnach,double Ynach,double Xck,double Yck,double Xkon,double Ykon,double r);


    QString GetPK2L    (QString StartPK, double L,        double StndPK, QVector<QString> Npk, QVector<QString> NpL); //Пикет + Расстояние = Новый Пикет
    double  GetLpk2pk  (QString StartPK, QString PKpoint, double StndPK, QVector<QString> Npk, QVector<QString> NpL );//Пикет + Пикет = Расстояние между ними

    double Yi (double li, double C);
    double Qi (double li, double L, double Q);
    double Zi (double li, double lii, double C, double R);

    //Таблица неправильных пикетов
    double PkStnd;
    QVector<QString> Npk;
    QVector<QString> NpL;

    //Расчет, читаем таблицу Трассы
    QVector<QString> TrassaKrLine;
    QVector<QString> TrassaXnach;
    QVector<QString> TrassaYnach;
    QVector<QString> TrassaXkon;
    QVector<QString> TrassaYkon;
    QVector<QString> TrassaL;
    QVector<QString> TrassaLfull;
    QVector<QString> TrassaRadius;
    QVector<QString> TrassaXck;
    QVector<QString> TrassaYck;
    QVector<QString> TrassaPKnach;
    //Таблица переходных кривых    
    QVector<QString> z;     //8
    QVector<QString> q;     //9
    QVector<QString> PKin;  //10   ПК начала входящей  переходной кривой
    QVector<QString> Lin;   //11
    QVector<QString> Cin;   //12
    QVector<QString> Rin;     //4
    QVector<QString> PKout; //13   ПК начала выходящей переходной кривой
    QVector<QString> Lout;  //14
    QVector<QString> Cout;  //15
    QVector<QString> h;  //16
    QVector<QString> CenterLR;
    QVector<QString> PKkrin; //7   ПК конца входящей  переходной кривой
    QVector<QString> PKkrout;//7+L ПК конца выходящей переходной кривой
    QVector<QString> Rout;     //4
    QVector<QString> PKnkr;  //7   ПК начала круговой кривой
    QVector<QString> PKkkr;  //    ПК конца  круговой кривой
    //Таблица Профиль
    QVector<QString> ProfilPKnach; //0
    QVector<QString> ProfilH;      //1
    QVector<QString> ProfilR;      //2
    QVector<QString> ProfilT;      //3
    QVector<QString> Profili;      //4
    QVector<QString> ProfilL;      //5
    //Кольца

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
    QVector<ring> rings;

    void appendNpk(QString pk , QString l);
    void appendTrassa(QString KrLine,QString PKn,QString Xn,QString Yn,QString Xk,QString Yk,QString Xck,QString Yck);
    void appendPerehod(QString pPKin,QString pLin,QString pCin,QString pQ,QString pZ,QString pPKout,QString pLout,QString pCout);
    void appendPerehod(QString pPKin,QString pLin,QString pCin,QString pQ,QString pZ,QString pPKout,QString pLout,QString pCout,QString ph);
    void appendProfil(QString PKnach,QString H,QString R);
    void predRaschet();

//-----------Расчетная часть---------------
    void raschetPkSm(double xp,
                     double yp,
                     QString &PKpoint,
                     double  &SMpoint,
                     QString &RL);

    void raschetPer(QString PKpoint,
                    double  SMpoint,
                    QString RL,
                    double  &SMpointZ,
                    double  &SMpointZQ);

    void raschetPer(QString PKpoint,
                    double  SMpoint,
                    QString RL,
                    double  &SMpointZ,
                    double  &SMpointZQ,
                    double  &hg,
                    double &qg,
                    double &zg);

    void raschetH(QString PK,double  &Hpoint);    

    //Определяем фактическое q и qz смещение
    void GetSmRazbivOs(QString PKpoint,
                       double  SMpoint,    //реальное смещение, нужно только для определения с какой стороны оси.
                       QString RL,         // Попал на линию или кривую L/R
                       double  &SMZ,   //100(+/-)Z
                       double  &SMZQ); //100(+/-)ZQ

    void GetSmRazbivOs(QString PKpoint,
                       double  SMpoint,    //реальное смещение, нужно только для определения с какой стороны оси.
                       QString RL,         // Попал на линию или кривую L/R
                       double  &SMZ,   //100(+/-)Z
                       double  &SMZQ, //100(+/-)ZQ
                       double  &hg,
                       double &qg,
                       double &zg);


    void raschetXY(
            QString PKpoint,
            double  SMpoint,
            double  SMpointZ,
            double  SMpointZQ,
            double &xp,
            double &yp);

    void raschetXY(
            QString PKpoint,
            double  SMpoint,
            double  SMpointZ,
            double  SMpointZQ,
            double &xp,
            double &yp,
            double &hg,
            double &qg,
            double &zg);
//-----------Расчетная часть---------------

private:
    double getLpk      (int pk, double StndPk, QVector<QString> Npk, QVector<QString> NpL);

};


#endif // GEO_H
