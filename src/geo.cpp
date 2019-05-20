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

#include "geo.h"
#include <stdio.h>
#include <QtMath>
#include <QString>
#include <QDebug>

//#define M_PI       3.14159265358979323846
//#define M_PI_2     1.57079632679489661923

geo::geo(){
    //вывод в консоль
//    printf("ты попал Lime \n");
    PkStnd = 100;

    Npk.clear();
    NpL.clear();

}

//{Функция Обработки координыты "Х"}
//Function PGZXKoord(XisXod,Adir,L:Extended):Extended;
//begin {Начало}           //RadToDeg DegToRad
//PGZXKoord:=XisXod+(L*cos((Adir)*Pi/180));
//end;  {Конец функции Обработки координыты "Х"}

//{Функция Обработки координыты "Y"}
//Function PGZYKoord(YisXod,Adir,L:Extended):Extended;
//begin {Начало}
//PGZYKoord:=YisXod+(L*sin((Adir)*Pi/180));
//end;  {Конец функции Обработки координыты "Y"}


//PGZ
void geo::Pgz (double x1is,double y1is,double x2,double y2,double Ugol,double L, double &xp, double &yp){
    double newDir = OgzDir(x1is,y1is,x2,y2)+Ugol;
    if (newDir >360) newDir=newDir - 360;
    if (newDir <  0) newDir=newDir + 360;
    xp = x1is+(L*cos((newDir)*M_PI/180));
    yp = y1is+(L*sin((newDir)*M_PI/180));
}


//OgzDir - Вычисления дирекционного угла
double geo::OgzDir (double x1,double y1,double x2,double y2){
    if ((x2 - x1 != 0) || (y2 - y1 != 0 )) {
        double s = (x2-x1) / sqrt( pow((x2 - x1),2) + (pow((y2 - y1),2)));
        double c = qRadiansToDegrees(M_PI_2 - atan(s/sqrt(1-pow(s,2) ) ) );
        if ((y2 - y1) < 0) return 360-c; else return c;
    } else return 0;
}

//Обратная геодезическая, расстояние
double geo::OgzLine (double x1,double y1,double x2,double y2){
    if ((x2 - x1 != 0) || (y2 - y1 != 0 )) return sqrt( pow((x2 - x1),2) + (pow((y2 - y1),2))); else return 0;
}

//FindUchLine - Функция поискаучастка на прямой
bool geo::FindUchLine(double Xnach,double Ynach,double Xkon,double Ykon,double Xpoint,double Ypoint) {
    double Adir = 0; //
    if ((Xkon - Xnach != 0) || (Ykon - Ynach != 0 )) {
        double dist = sqrt( pow((Xkon - Xnach),2) + (pow((Ykon - Ynach),2)));
        double s = (Xkon - Xnach) / sqrt( pow((Xkon - Xnach),2) + (pow((Ykon - Ynach),2)));
        double c = qRadiansToDegrees(M_PI_2 - atan(s/sqrt(1-pow(s,2) ) ) );
        if ((Ykon - Ynach) < 0) Adir = 360-c; else Adir = c;
        // посчитали ОГЗ и проверяем попали на участок или нет
        if    ( ( (Ypoint-Ynach)*sin((Adir)*M_PI/180) + (Xpoint-Xnach)*cos((Adir)*M_PI/180) > 0)
             && ( (Ypoint-Ynach)*sin((Adir)*M_PI/180) + (Xpoint-Xnach)*cos((Adir)*M_PI/180) <= dist)
             && ( (Ypoint-Ynach)*cos((Adir)*M_PI/180) - (Xpoint-Xnach)*sin((Adir)*M_PI/180) < 200)
             && ( (Ypoint-Ynach)*cos((Adir)*M_PI/180) - (Xpoint-Xnach)*sin((Adir)*M_PI/180) > -200) ) {
            return true;
        }
    }
return false;
} //Требуется доработка, уход от сложных условий

//не правильно отрабатывает
bool geo::FindUchLine2(double Xnach,double Ynach,double Xkon,double Ykon,double Xpoint,double Ypoint) {
    double  dist = OgzLine(Xnach,Ynach,Xkon,Ykon);
    double     a = PKPlusLine(Xnach,Ynach,Xkon,Ykon,Xpoint,Ypoint);
    double     b = SmeshLine(Xnach,Ynach,Xkon,Ykon,Xpoint,Ypoint);
    if (b < 0) {
//        qDebug() << "Пк со знаком минус" << b;
        return false;}
    if (b > dist) {
//        qDebug() << "Пк больше чем расстояние НК и КК" << b << dist;
        return false;}
    if ((a>600)&&(a<-600)){
//        qDebug() << "Смещение больше 600м и -600м" << a;
        return false;}
    return true;
}

//Определяем расстояние по ПК
double geo::PKPlusLine(double Xnach,double Ynach,double Xkon,double Ykon,double Xpoint,double Ypoint){
    double Adir = 0;
    double s = (Xkon - Xnach) / sqrt( pow((Xkon - Xnach),2) + (pow((Ykon - Ynach),2)));
    double c = qRadiansToDegrees(M_PI_2 - atan(s/sqrt(1-pow(s,2) ) ) );
    if ((Ykon - Ynach) < 0) Adir = 360-c; else Adir = c;
    return (Ypoint-Ynach)*sin((Adir)*M_PI/180) + (Xpoint-Xnach)*cos((Adir)*M_PI/180);
}

//Определяем расстояние от оси, смещение
double geo::SmeshLine(double Xnach,double Ynach,double Xkon,double Ykon,double Xpoint,double Ypoint){
    double Adir = 0;
    double    s = (Xkon - Xnach) / sqrt( pow((Xkon - Xnach),2) + (pow((Ykon - Ynach),2)));
    double    c = qRadiansToDegrees(M_PI_2 - atan(s/sqrt(1-pow(s,2) ) ) );
    if ((Ykon - Ynach) < 0) Adir = 360-c; else Adir = c;
    return (Ypoint-Ynach)*cos((Adir)*M_PI/180) - (Xpoint-Xnach)*sin((Adir)*M_PI/180);
}

//FindUchKr - Функция поискаучастка на кривой
bool geo::FindUchKr(double Xnach,double Ynach,double Xkon,double Ykon,double Xck,double Yck,double Xpoint,double Ypoint) {
    bool       p = Povorot(Xnach,Ynach,Xkon,Ykon,Xck,Yck); //false центр с лева и true центр с права
    double rkr=OgzLine(Xck,Yck,Xnach,Ynach);
    double rpi=OgzLine(Xck,Yck,Xpoint,Ypoint);
    qDebug() << "центр кривой" << p;
    if (p==false) {
        qDebug() << "если кривая с лева то..";
        if (Povorot(Xck,Yck,Xnach,Ynach,Xpoint,Ypoint)==false) { //Если точка с лева, но по ходу пикета
            double ugolKk=ugol(Xnach,Ynach,Xkon,  Ykon,  Xck,Yck);
            double ugolPi=ugol(Xnach,Ynach,Xpoint,Ypoint,Xck,Yck);
            qDebug() << "центр с лева KK=" << ugolKk <<" Pi=" << ugolPi << Xkon << Ykon << Xpoint << Ypoint;
            if (ugolPi<ugolKk&&rpi<rkr*2){qDebug() <<"точка на кривой"; return true;}else{qDebug() <<"точка НЕ на кривой";return false;}
        }else{qDebug() << "точка против хода пикета..";}
    }else{
        qDebug() << "если кривая с права то..";
        if (Povorot(Xck,Yck,Xnach,Ynach,Xpoint,Ypoint)==true) { //Если точка с права, но по ходу пикета
            double ugolKk=ugol(Xnach,Ynach,Xkon,Ykon,Xck,Yck);
            double ugolPi=ugol(Xnach,Ynach,Xpoint,Ypoint,Xck,Yck);
            qDebug() << "центр с права KK=" << ugolKk <<" Pi=" << ugolPi << Xkon << Ykon << Xpoint << Ypoint;
            if (ugolPi<ugolKk&&rpi<rkr*2){qDebug() <<"точка на кривой";return true;}else{qDebug() <<"точка НЕ на кривой";return false;}
        }else{qDebug() << "точка против хода пикета..";}

    }
  return false;
}


//Povorot Расположение центра кривой false с лева / true справа
bool geo::Povorot(double Xnach,double Ynach,double Xkon,double Ykon,double Xck,double Yck) {
    if (SmeshLine(Xnach,Ynach,Xkon,Ykon,Xck,Yck) < 0 ) {
//        qDebug() << "Центр с лева" << SmeshLine(Xnach,Ynach,Xkon,Ykon,Xck,Yck);
        return false;
    } else {
//        qDebug() << "Центр с права" << SmeshLine(Xnach,Ynach,Xkon,Ykon,Xck,Yck);
        return true;
    }
}

double geo::ugol(double Xnach,double Ynach,double Xpoint,double Ypoint,double Xck,double Yck) {
    double A =  sqrt( pow((Xpoint - Xnach),2) + pow((Ypoint - Ynach),2) );
    double B =  sqrt( pow((Xck    - Xnach),2) + pow((Yck    - Ynach),2) );
    double C =  sqrt( pow((Xpoint - Xck  ),2) + pow((Ypoint - Yck  ),2) );
    return    qRadiansToDegrees(acos((pow(A,2) - pow(B,2) - pow(C,2)) / (-2*B*C)));
}


double geo::linekr(double Xnach,double Ynach,double Xck,double Yck,double Xkon,double Ykon,double r){
    double u = ugol(Xnach,Ynach,Xkon,Ykon,Xck,Yck);
    return (M_PI*r*u) / 180;
}


//PKPlusKr - Вычисления ПК на кривой
//если со знаком + то вперед, со знаком - назад
double geo::PKPlusKr(double Xnach,double Ynach,double Xck,double Yck,double Xpoint,double Ypoint) {
    double      R = OgzLine(Xck,Yck,Xnach, Ynach);
    return qDegreesToRadians(ugol(Xnach,Ynach,Xpoint,Ypoint,Xck,Yck)*R);
}

//SmeshKr - Вычисления Смещение на кривой
//если со знаком - слева, + справа
double geo::SmeshKr(bool Povorot,double Xnach,double Ynach,double Xck,double Yck,double Xpoint,double Ypoint) {
    double R =OgzLine(Xck,Yck,Xnach, Ynach);
    double Pi=OgzLine(Xck,Yck,Xpoint, Ypoint);
//    qDebug() << "R=" << R << "Pi="<<Pi;
    if(Povorot==false) {//цк с лева
        if (Pi>R){return Pi-R;}else{return (R-Pi)*(-1);}
    }else{
        if (Pi>R){return (Pi-R)*(-1);}else{return R-Pi;}
    }//else
}


//Вычисляет ПК точки, известно: расстояние, пикет начала участка, стандартная длинна пикета и массив неправильных пикетов
QString geo::GetPK2L (QString StartPK, double L, double StndPK, QVector<QString> Npk, QVector<QString> NpL){
    //StartPk ПК начала участка
    //L       Расстояние от начала участка до пикетной точки
    //StndPk  Длинна стандартного пикета
    //Npk     Номер неправильного пикета
    //NpL     Его длинна
    int         nachPK     = StartPK.split("+").value(0).toInt();
    double      nachPKPlus = StartPK.split("+").value(1).toDouble();
    //первая проверка, если L меньше чем длинна пикета
    if ( L < (getLpk(nachPK,StndPK,Npk,NpL)-nachPKPlus) ){
        return QString::number(nachPK)+ "+" + QString::number(L+nachPKPlus,'f',4);
    }
    //Если больше длинны пикета.
    double Lt  = L+nachPKPlus;      //временное расстояние
    int    pkt = nachPK; //временный Пк
    while (Lt > getLpk(pkt,StndPK,Npk,NpL)) {
        Lt=Lt-getLpk(pkt,StndPK,Npk,NpL);
        pkt++; // ПК=ПК+1
    }
    //qDebug()<<"хуйня какая то"<<Lt<<(getLpk(pkt,StndPK,Npk,NpL)-0.0015)<<pkt<<Lt;
    if (Lt==StndPK){
        pkt++;
        Lt=0;
        return QString::number(pkt)+ "+" + QString::number(Lt,'f',4);
    }
    return QString::number(pkt)+ "+" + QString::number(Lt,'f',4);
}

//Вычисляет расстояние между двумя пикетами, известно: два пикета, стандартная длинна пикета  и  массив неправильных пикетов
double geo::GetLpk2pk (QString StartPK, QString PKpoint, double StndPK, QVector<QString> Npk, QVector<QString> NpL ){
    //StartPK ПК начала переходной кривой
    //PKpoint ПК точки
    //StndPk  Длинна стандартного пикета
    //Npk     Номер неправильного пикета
    //NpL     Его длинна
    int         PKnach      = StartPK.split("+").value(0).toInt();
    double      PKPlusNach  = StartPK.split("+").value(1).toDouble();
    int         PKpnt       = PKpoint.split("+").value(0).toInt();
    double      PKPlusPnt   = PKpoint.split("+").value(1).toDouble();

    //Если ПК точки находится До ПК начала переходной
    if (PKpnt<PKnach) {
        //        qDebug() << "Пикет точки меньше чем пикет начала переходной кривой"  ;
        return -1;
    }
    if (PKnach==PKpnt) {
        //        qDebug() << "Пикет точки равен пикету начала переходной кривой";
        if (PKPlusPnt>=PKPlusNach) return PKPlusPnt-PKPlusNach; else {
            //            qDebug() << "Дробная часть точки меньше чем у начала кривой";
            return -1;
        }
    }
    //    qDebug() << "Пикет точки больше пикета начала переходной кривой";
    double Lt  = getLpk(PKnach,StndPK,Npk,NpL)-PKPlusNach;
    for (int i = PKnach+1; i < PKpnt;i++ ){
        //        qDebug() << "count=" << i << "PKnach=" << PKnach << "PKpnt=" << PKpnt << "Lt=" << Lt;
        Lt=Lt+getLpk(i,StndPK,Npk,NpL);
    }
    return Lt+PKPlusPnt;
}

//Проверяем пикет правильный или нет, если нет то возращает длинну стандартного
double geo::getLpk(int pk, double StndPk, QVector<QString> Npk, QVector<QString> NpL){
    for (int i=0;i<(Npk.count());i++){
        if (pk==Npk.value(i).toInt()) return NpL.value(i).toDouble();
    }
    return StndPk;
}


//Вычисляем Yi для переходки
double geo::Yi (double li, double C){
    //li разность пикетажа полигонометрического знака и начала переходной кривой
    //C  параметр переходной кривой
    qDebug() << (pow(li,3)/(6*C)) << (pow(li,7)/(336*(pow(C,3))) );
    return ( (pow(li,3)/(6*C)) - (pow(li,7)/(336*(pow(C,3)))) );
}

//Вычисляем Qi для переходки
double geo::Qi (double li, double L, double Q){
    //li разность пикетажа полигонометрического знака и начала переходной кривой
    //L  длинна переходной кривой
    //Q  смещение от оси тоннеля до оси пути
    return  Q* (li/L);
}

//Вычисляем Qi для переходки
double geo::Zi(double li, double lii, double C, double R){
    //li  разность пикетажа полигонометрического знака и начала переходной кривой
    //lii расстояние от начала круговой кривой
    //C   параметр переходной кривой
    //R   радиус кривой
//    qDebug() << li << lii << C << R;
//    qDebug() << pow(li,3)/(6*C)
//             << R - sqrt( pow(R,2)-pow(lii,2) );
    return ( pow(li,3)/(6*C) )-( R-sqrt( pow(R,2)-pow(lii,2) ) );
}


//переводим строковый угол в числовой градусной меры.
double geo::stringGradToDouble (QString ugol){
    int g = ugol.split(" ").value(0).toInt();
    double m = ugol.split(" ").value(1).toDouble();
    double s = ugol.split(" ").value(2).toDouble();
    if (ugol.split(" ").value(0).at(0).isDigit() == true){
        return (g + m /60 + s /3600);
    } else {
        return (g - m /60 - s /3600);
    }
}

//Функция перевода градусов в " ° ' " "
QString geo::DegToGradstring(double ugol){
    QString sminus ="";
    if (ugol<0){
        sminus ="-";
        ugol=ugol*(-1);
    }
    int g = ugol;
    int m = (ugol-g)*60;
    double dm = (ugol-g)*60;
    double ss = (dm-m)*60;
    QString mul = (m<10)?("0"):("");
    QString sul = (ss<10)?("0"):("");
    return sminus+QString::number(g)+" "+ mul + QString::number(m)+" "+sul+ QString::number(ss,'f',1);
}

QString geo::GradstringToGradstring(QString ugol){
    return ugol.split(" ").value(0)+"°"+ugol.split(" ").value(1)+"'"+ugol.split(" ").value(2)+"\"";
}

void geo::appendNpk(QString pk , QString l)
{
    Npk.append(pk);   //Номер  неправильного пикета
    NpL.append(l);    //Длинна неправильного пикета
}

void geo::appendTrassa(QString KrLine,QString PKn,QString Xn,QString Yn,QString Xk,QString Yk,QString Xck,QString Yck)
{
    TrassaKrLine.append(KrLine);
    if (TrassaPKnach.count() < 1 )  {
        TrassaPKnach.append(PKn);
        TrassaXnach.append(Xn);
        TrassaYnach.append(Yn);
        qDebug() << "раз" << TrassaPKnach.count() << TrassaKrLine.count();
    } else {
        TrassaPKnach.append("0");
        TrassaXnach.append("0");
        TrassaYnach.append("0");
        qDebug() << TrassaPKnach.count() << TrassaKrLine.count() ;
    }
    TrassaXkon.append(Xk);
    TrassaYkon.append(Yk);
    TrassaXck.append(Xck);
    TrassaYck.append(Yck);
    TrassaL.append("0");
    TrassaLfull.append("0");
    TrassaRadius.append("0");
}

void geo::appendPerehod(QString pPKin,QString pLin,QString pCin,
                        QString pQ,QString pZ,
                        QString pPKout,QString pLout,QString pCout)
{
    PKin.append(pPKin);
    Lin.append(pLin);
    Cin.append(pCin);

    q.append(pQ);
    z.append(pZ);

    PKout.append(pPKout);
    Lout.append(pLout);
    Cout.append(pCout);

    //вычисляется
    Rin.append("Netu");
    Rout.append("Netu");
    CenterLR.append("Netu");

    PKkrin.append("Netu");
    PKkrout.append("Netu");

    PKnkr.append("Netu");
    PKkkr.append("Netu");
}

void geo::appendPerehod(QString pPKin,QString pLin,QString pCin,
                        QString pQ,QString pZ,
                        QString pPKout,QString pLout,QString pCout,
                        QString ph)
{
    PKin.append(pPKin);
    Lin.append(pLin);
    Cin.append(pCin);

    q.append(pQ);
    z.append(pZ);

    PKout.append(pPKout);
    Lout.append(pLout);
    Cout.append(pCout);

    h.append(ph);

    //вычисляется
    Rin.append("Netu");
    Rout.append("Netu");
    CenterLR.append("Netu");

    PKkrin.append("Netu");
    PKkrout.append("Netu");

    PKnkr.append("Netu");
    PKkkr.append("Netu");
}

void geo::appendProfil(QString PKnach,QString H,QString R)
{
    ProfilPKnach.append(PKnach);
    ProfilH.append(H);
    ProfilR.append(R);
    ProfilT.append("0");
    Profili.append("0");
    ProfilL.append("0");
}

void geo::predRaschet()
{
    double LTrfull  = 0; //общая длинна трассы
    double tl       = 0; //длинна временная
    double r1       = 0; //Радиус 1
    double r2       = 0; //Радиус 2
    double vT       = 0; //Радиус 1


    //Предрасчет трассы
    for (int i = 1; i < TrassaKrLine.count();i++ ){
        TrassaXnach.replace(i,TrassaXkon.value(i-1));
        TrassaYnach.replace(i,TrassaYkon.value(i-1));
    }

    for (int i = 0; i < TrassaKrLine.count();i++ ){
        if (TrassaKrLine.value(i)=="Прямая"){
            tl = OgzLine(TrassaXnach.value(i).toDouble(),TrassaYnach.value(i).toDouble(),TrassaXkon.value(i).toDouble(),TrassaYkon.value(i).toDouble());
            TrassaL.replace(i,QString::number(tl,'f',4));
        }
        if (TrassaKrLine.value(i)=="Кривая"){
            r1 = OgzLine(TrassaXnach.value(i).toDouble(),TrassaYnach.value(i).toDouble(),TrassaXck.value(i).toDouble(),TrassaYck.value(i).toDouble());
            r2 = OgzLine(TrassaXkon.value(i).toDouble(),TrassaYkon.value(i).toDouble(),TrassaXck.value(i).toDouble(),TrassaYck.value(i).toDouble());
            TrassaRadius.replace(i,QString::number((r1+r2)/2,'f',4));
            tl = linekr(TrassaXnach.value(i).toDouble(),TrassaYnach.value(i).toDouble(),TrassaXck.value(i).toDouble(),TrassaYck.value(i).toDouble(),TrassaXkon.value(i).toDouble(),TrassaYkon.value(i).toDouble(),TrassaRadius.value(i).toDouble());
            TrassaL.replace(i,QString::number(tl,'f',4));
        }        
    }

    for (int i = 0; i < TrassaKrLine.count();i++ ){
        LTrfull += TrassaL.value(i).toDouble();
        TrassaLfull.replace(i,QString::number(LTrfull,'f',4));
    }

    for (int i = 1; i < TrassaKrLine.count();i++ ){
        TrassaPKnach.replace(i,GetPK2L(TrassaPKnach.value(i-1),TrassaL.value(i-1).toDouble(),PkStnd,Npk,NpL));
    }

    //Предрасчет переходки
    for (int i = 0; i < PKin.count();i++ ){
        PKkrin.replace( i,GetPK2L( PKin.value(i), Lin.value(i).toDouble(), PkStnd,Npk,NpL)); // Пк конца ВХОД переходной кривой
        PKkrout.replace(i,GetPK2L( PKout.value(i),Lout.value(i).toDouble(),PkStnd,Npk,NpL)); // Пк конца ВЫХОД. переходной кривой
    }
    //qDebug() << "!!!!!!!!!ПЕРЕХОДОК!!!!!!" << PKin.count();
    for (int i = 0; i < PKin.count();i++ ){    
        for (int t = 0; t < TrassaKrLine.count();t++ ){
            if (TrassaKrLine.value(t) == "Кривая") {
                if (GetLpk2pk( TrassaPKnach.value(t),PKkrin.value(i),PkStnd,Npk,NpL) < TrassaL.value(t).toDouble()){
                    if (GetLpk2pk( TrassaPKnach.value(t),PKkrin.value(i),PkStnd,Npk,NpL)>0){
                        Rin.replace(i,TrassaRadius.value(t));
                        if (Povorot(TrassaXnach.value(t).toDouble(),
                                    TrassaYnach.value(t).toDouble(),
                                    TrassaXkon.value(t).toDouble(),
                                    TrassaYkon.value(t).toDouble(),
                                    TrassaXck.value(t).toDouble(),
                                    TrassaYck.value(t).toDouble()) == true) {CenterLR.replace(i,"R");} else {CenterLR.replace(i,"L");}
                        PKnkr.replace(i,TrassaPKnach.value(t));

                        qDebug() << " Пк КОНЦА Входящей переходки попала на кривую "
                                 << t << "TrassaPKnach="  <<TrassaPKnach.value(t)
                                 << "PKkrin=" <<PKkrin.value(i)<<GetLpk2pk( TrassaPKnach.value(t),PKkrin.value(i),PkStnd,Npk,NpL)
                                 << "TrassaL="<<TrassaL.value(t)
                                 << "PKkrin="<<PKkrin.value(i);
                    }
                }

                if (GetLpk2pk( TrassaPKnach.value(t),PKout.value(i),PkStnd,Npk,NpL) < TrassaL.value(t).toDouble()){
                    if (GetLpk2pk(TrassaPKnach.value(t),PKout.value(i),PkStnd,Npk,NpL)>0){
                        Rout.replace(i,TrassaRadius.value(t));
                        if (Povorot(TrassaXnach.value(t).toDouble(),
                                    TrassaYnach.value(t).toDouble(),
                                    TrassaXkon.value(t).toDouble(),
                                    TrassaYkon.value(t).toDouble(),
                                    TrassaXck.value(t).toDouble(),
                                    TrassaYck.value(t).toDouble()) == true) {CenterLR.replace(i,"R");} else {CenterLR.replace(i,"L");}
                        PKkkr.replace(i,GetPK2L( TrassaPKnach.value(t), TrassaL.value(t).toDouble(), PkStnd,Npk,NpL));

                        qDebug() << " Пк НАЧАЛА Выходящей переходки попала на кривую " << t<< "TrassaPKnach="  << TrassaPKnach.value(t)
                                 << "PKout =" << PKout.value(i)<<GetLpk2pk( TrassaPKnach.value(t),PKout.value(i),PkStnd,Npk,NpL)
                                 << "TrassaL ="<<TrassaL.value(t)
                                 << "PKkkr ="<<PKkkr.value(i)   ;

                    }
                }
            }
        }
    }

    //Предрасчет Профиля

    double lfull;
    double luch;
    double it;

    for (int i = 0; i < ProfilPKnach.count()-1;i++ ){
        ProfilL.replace(i,QString::number(GetLpk2pk( ProfilPKnach.value(i),ProfilPKnach.value(i+1),PkStnd,Npk,NpL),'f',4));
        if (ProfilH.value(i).toDouble()==0){ //если отметка ноль
            if ((i+1<ProfilPKnach.count()-1)&&(ProfilH.value(i+1).toDouble()==0)){ //если +1 отметка ноль
                //                qDebug()<<"DO i+1!=0"<<"  i=" <<ProfilH.value(i)<<"  i+1=" <<ProfilH.value(i+1);
                lfull = GetLpk2pk( ProfilPKnach.value(i-1),ProfilPKnach.value(i+2),PkStnd,Npk,NpL);
                it = (ProfilH.value(i+2).toDouble()-ProfilH.value(i-1).toDouble())/lfull;

                luch  = GetLpk2pk( ProfilPKnach.value(i-1),ProfilPKnach.value(i  ),PkStnd,Npk,NpL);
                ProfilH.replace(i,QString::number(ProfilH.value(i-1).toDouble()+(it*luch),'f',4));

                luch  = GetLpk2pk( ProfilPKnach.value(i-1),ProfilPKnach.value(i+1 ),PkStnd,Npk,NpL);
                ProfilH.replace(i+1,QString::number(ProfilH.value(i-1).toDouble()+(it*luch),'f',4));

                //                qDebug()<<"POSLE i+1!=0"<<"  i=" <<ProfilH.value(i)<<"  i+1=" <<ProfilH.value(i+1);

            }else{//если +1 отметка не равна 0
                //                qDebug()<<"DO i+1!=0"<<"  i=" <<ProfilH.value(i)<<"  i+1=" <<ProfilH.value(i+1);
                lfull = GetLpk2pk( ProfilPKnach.value(i-1),ProfilPKnach.value(i+1),PkStnd,Npk,NpL);
                it = (ProfilH.value(i+1).toDouble()-ProfilH.value(i-1).toDouble())/lfull;

                luch  = GetLpk2pk( ProfilPKnach.value(i-1),ProfilPKnach.value(i  ),PkStnd,Npk,NpL);
                ProfilH.replace(i,QString::number(ProfilH.value(i-1).toDouble()+(it*luch),'f',4));
                //                qDebug()<<"POSLE i+1!=0"<<"  i=" <<ProfilH.value(i)<<"  i+1=" <<ProfilH.value(i+1);
            }
        } //Проверка ProfilH==0
    }
    for (int i = 0; i < ProfilPKnach.count()-1;i++ ){

        Profili.replace(i,QString::number(( (ProfilH.value(i+1).toDouble()-ProfilH.value(i).toDouble())/ProfilL.value(i).toDouble()),'f',3));
        if (ProfilR.value(i).toDouble()!=0){
            vT = (ProfilR.value(i).toDouble()/2)*((Profili.value(i).toDouble()-Profili.value(i-1).toDouble()));
            if (vT<0) vT*=-1;
            ProfilT.replace(i,QString::number( vT,'f',2) );
        }
//        qDebug() << "ПК="  << ProfilPKnach.value(i)
//                 << "H="   << ProfilH.value(i)
//                 << "R="   << ProfilR.value(i)
//                 << "i="   << Profili.value(i)
//                 << "Т="   << ProfilT.value(i)
//                 << "L="   << ProfilL.value(i);
    }
}


void geo::raschetPkSm(double xp,
                      double yp,
                      QString &PKpoint,
                      double  &SMpoint,
                      QString &RL){
// Временные переменные для поиска участка
       double xn  = 0;
       double yn  = 0;
       double xk  = 0;
       double yk  = 0;
       double xck = 0;
       double yck = 0;
       QString pk="";

       qDebug() << "-----------------------РАСЧЕТНАЯ ЧАСТЬ Пикета и смещение от разбивочной оси ------------------------------";
       // Поиск участка
       for (int i = 0; i < TrassaKrLine.count();i++ ){
           pk = TrassaPKnach.value(i);
           xn = TrassaXnach.value(i).toDouble();
           yn = TrassaYnach.value(i).toDouble();
           xk = TrassaXkon.value(i).toDouble();
           yk = TrassaYkon.value(i).toDouble();
           xck =TrassaXck.value(i).toDouble();
           yck =TrassaYck.value(i).toDouble();

           if (TrassaKrLine.value(i) == "Прямая") {
               if (FindUchLine(xn,yn,xk,yk,xp,yp) == true) {
                   PKpoint = GetPK2L(pk,PKPlusLine(xn,yn,xk,yk,xp,yp),PkStnd,Npk,NpL);
                   SMpoint = SmeshLine(xn,yn,xk,yk,xp,yp);
                   RL="L";
                   break;
               }else{qDebug() << "ненашли на прямой";}
               qDebug() << "-----------------------------------------------------";
           }
           if (TrassaKrLine.value(i) == "Кривая") {
               qDebug()<< "count=" << i;

               if (FindUchKr(xn,yn,xk,yk,xck,yck,xp,yp) == true) {
                   PKpoint = GetPK2L(pk,PKPlusKr(xn,yn,xck,yck,xp,yp),PkStnd,Npk,NpL);
                   SMpoint = SmeshKr(Povorot(xn,yn,xk,yk,xck,yck),xn,yn,xck,yck,xp,yp);
                   RL = "R";
               break;
               }else{qDebug() << "ненашли на кривой";}
               qDebug() << "-----------------------------------------------------";
           }
       }
       qDebug() << "-----------------------РАСЧЕТНАЯ ЧАСТЬ Пикета и смещение от разбивочной оси ------------------------------";
}


void geo::raschetPer(QString PKpoint,
                     double  SMpoint,
                     QString RL,
                     double  &SMpointZ,
                     double  &SMpointZQ){
    qDebug() << "--------------------------------Переходная кривая -------------------------------------------";

    QString PKr      = "";   //Пикет начала по радиусу
    double  Lr     = 0;      //Расстояние между ПК по радиусу
    double  Lpk2pk = 0;      //Расстояние между ПК начала переходной и ПК точки
    double  zi  = 0;    
    double  qi  = 0;
    double  li  = 0;
    double  lii = 0;
    qDebug() << "!!!!!!!!!ПЕРЕХОДОК!!!!!!" << PKin.count();
    for (int i = 0; i < PKin.count();i++ ){
        qDebug()<<RL<<"=RL="<<CenterLR.value(i);
        Lpk2pk = GetLpk2pk(PKin.value(i),PKpoint,PkStnd,Npk,NpL); //Расстояние от начала переходки до точки
        //начало, входящая переходка
        qDebug() << "in" << i << "|" << Lpk2pk << "ПКнач" << PKin.value(i);
        if ( (Lpk2pk != -1)&& (Lpk2pk <= Lin.value(i).toDouble()) ) {
            qDebug() << "попал!in! ";
            if (RL=="L"){
                zi = Yi(Lpk2pk,Cin.value(i).toDouble());
                qi = Qi(Lpk2pk,Lin.value(i).toDouble(),q.value(i).toDouble());
                qDebug() << "от тангенса= "<< "Lпер.="<< Lin.value(i) << "Li="<< Lpk2pk << "Yi=" << zi << "Qi=" << qi;
            }//Переходка на прямом участке
            if (RL=="R"){
                lii = GetLpk2pk(PKnkr.value(i),PKpoint,PkStnd,Npk,NpL);
                zi = Zi(Lpk2pk,lii,Cin.value(i).toDouble(),Rin.value(i).toDouble());
                qi = Qi(Lpk2pk,Lin.value(i).toDouble(),q.value(i).toDouble());
                qDebug() << "переходка от кривой"<< "ПкНачкр.=" << PKnkr.value(i) << "PKpoint=" << PKpoint << "li=" << Lpk2pk << "lii=" << lii << "C=" << Cin.value(i) << "R=" << Rin.value(i) << "Zi=" << zi << "Qi=" << qi;
            }//Переходка на кривой
        }
        if (PKpoint==PKin.value(i)) {
            zi =0;
            qi =0;
            qDebug()<<"Уникальный случай"<< PKpoint << "=" <<PKin.value(i);
        }
        //середина, между переходками
        PKr    = GetPK2L(  PKin.value(i),Lin.value(i).toDouble(),PkStnd,Npk,NpL); //ПК начала кривой между переходками
        Lr     = GetLpk2pk(PKr,PKout.value(i),PkStnd,Npk,NpL);                    //Расстояние между началом и концом кривой
        Lpk2pk = GetLpk2pk(PKr,PKpoint,PkStnd,Npk,NpL);                           //Расстояние от начала до точки на кривой
        qDebug() << "R"<< i << "|"<<  Lpk2pk  << "ПКнач" << PKr;

        if ( (Lpk2pk != -1)&& (Lpk2pk <= Lr) ) { //Обработка смещения между переходными кривыми q+z
            qDebug() << "попал!"<< CenterLR.value(i);
            zi=z.value(i).toDouble();
            qi=q.value(i).toDouble();
        }

        //конец, выходящая
        Lpk2pk = GetLpk2pk(PKout.value(i),PKpoint,PkStnd,Npk,NpL);
        qDebug() << "out"<< i << "|"<< Lpk2pk << "ПКнач" << PKin.value(i);
        if ( (Lpk2pk != -1) && (Lpk2pk < Lout.value(i).toDouble()) ) {
            qDebug() << "попал!out! ";
            li  = Lout.value(i).toDouble()-Lpk2pk;
            qDebug() << "li=" << li;
            if (RL=="L"){
                zi = Yi(li,Cout.value(i).toDouble());
                qi = Qi(li,Lout.value(i).toDouble(),q.value(i).toDouble());
                qDebug() << "от тангенса= " << "Yi=" << zi << "Qi=" << qi;
            }//Переходка на прямом участке
            if (RL=="R"){
            lii = GetLpk2pk(PKpoint,PKkkr.value(i),PkStnd,Npk,NpL); ;
            qDebug() << "lii=" << lii;
            zi = Zi(li,lii,Cout.value(i).toDouble(),Rout.value(i).toDouble());
            qi = Qi(li,Lout.value(i).toDouble(),q.value(i).toDouble());
            qDebug() << "переходка от кривой"<< "ПкКОНкр.=" << PKkkr.value(i) << "PKpoint=" << PKpoint << "li=" << Lpk2pk << "lii=" << lii << "C=" << Cin.value(i) << "R=" << Rout.value(i) << "Zi=" << zi << "Qi=" << qi;
            }//Переходка на кривой
        }

        if (PKpoint==PKout.value(i)) {
            zi=z.value(i).toDouble();
            qi=q.value(i).toDouble();
            qDebug()<<"Уникальный случай"<< PKpoint << "=" <<PKout.value(i);
        }

        if ((zi!=0)||(qi!=0)){
            //Вычисляем смецения от оси пути и оси тоннеля из q и z
            if (CenterLR.value(i)=="L") {
                if (SMpoint>0){
                    SMpointZ  = SMpoint +  zi;
                    SMpointZQ = SMpoint + (zi+qi);
                } else {
                    SMpointZ  = SMpoint +  zi;
                    SMpointZQ = SMpoint + (zi+qi);
                }
            }//ЦК кривой слева
            if (CenterLR.value(i)=="R") {
                if (SMpoint>0){
                    SMpointZ  = SMpoint -  zi;
                    SMpointZQ = SMpoint - (zi+qi);
                } else {
                    SMpointZ  = SMpoint -  zi;
                    SMpointZQ = SMpoint - (zi+qi);
                }
            }//ЦК кривой справа
            qDebug() << "Смещение от разбивочной оси = " << QString::number(SMpoint,  'f',4);
            qDebug() << "Смещение от оси пути        = " << QString::number(SMpointZ, 'f',4) <<"zi= " << zi <<"               Центр кривой с ="<<CenterLR.value(i);
            qDebug() << "Смещение от оси тоннеля     = " << QString::number(SMpointZQ,'f',4) <<"zi= " << zi <<"qi= " << qi <<"Центр кривой с ="<<CenterLR.value(i);
            qDebug() << "Примечание: Если центр кривой с права, знак минус для zi qi.";
            break;
        }

    }
}

void geo::raschetPer(QString PKpoint,
                     double  SMpoint,
                     QString RL,
                     double  &SMpointZ,
                     double  &SMpointZQ,
                     double  &hg,
                     double &zg,
                     double &qg){
    qDebug() << "--------------------------------Переходная кривая -------------------------------------------";

    QString PKr      = "";   //Пикет начала по радиусу
    double  Lr     = 0;      //Расстояние между ПК по радиусу
    double  Lpk2pk = 0;      //Расстояние между ПК начала переходной и ПК точки
    double  zi  = 0;
    double  qi  = 0;
    double  li  = 0;
    double  lii = 0;    
    qDebug() << "!!!!!!!!!ПЕРЕХОДОК!!!!!!" << PKin.count();
    for (int i = 0; i < PKin.count();i++ ){
        qDebug()<<RL<<"=RL="<<CenterLR.value(i);
        Lpk2pk = GetLpk2pk(PKin.value(i),PKpoint,PkStnd,Npk,NpL); //Расстояние от начала переходки до точки
        //начало, входящая переходка
        qDebug() << "in" << i << "|" << Lpk2pk << "ПКнач" << PKin.value(i);
        if ( (Lpk2pk != -1)&& (Lpk2pk <= Lin.value(i).toDouble()) ) {
            qDebug() << "попал!in! ";
            if (RL=="L"){
                zi = Yi(Lpk2pk,Cin.value(i).toDouble());
                qi = Qi(Lpk2pk,Lin.value(i).toDouble(),q.value(i).toDouble());
                qDebug() << "от тангенса= "<< "Lпер.="<< Lin.value(i) << "Li="<< Lpk2pk << "Yi=" << zi << "Qi=" << qi;
            }//Переходка на прямом участке
            if (RL=="R"){
                lii = GetLpk2pk(PKnkr.value(i),PKpoint,PkStnd,Npk,NpL);
                zi = Zi(Lpk2pk,lii,Cin.value(i).toDouble(),Rin.value(i).toDouble());
                qi = Qi(Lpk2pk,Lin.value(i).toDouble(),q.value(i).toDouble());
                qDebug() << "переходка от кривой"<< "ПкНачкр.=" << PKnkr.value(i) << "PKpoint=" << PKpoint << "li=" << Lpk2pk << "lii=" << lii << "C=" << Cin.value(i) << "R=" << Rin.value(i) << "Zi=" << zi << "Qi=" << qi;
            }//Переходка на кривой
            //расчет h
            //hg = h.value(i).toDouble();
            hg = h.value(i).toDouble()*Lpk2pk/Lin.value(i).toDouble();
            qDebug() << "Возвышение: H" << h.value(i) << " L от начала пер. до точки" << Lpk2pk << " L всей пер." <<Lin.value(i);
        }
        if (PKpoint==PKin.value(i)) {
            zi = 0;
            qi = 0;
            hg = 0;
            qg = 0;
            zg = 0;
            qDebug()<<"Уникальный случай"<< PKpoint << "=" <<PKin.value(i);
        }
        //середина, между переходками
        PKr    = GetPK2L(  PKin.value(i),Lin.value(i).toDouble(),PkStnd,Npk,NpL); //ПК начала кривой между переходками
        Lr     = GetLpk2pk(PKr,PKout.value(i),PkStnd,Npk,NpL);                    //Расстояние между началом и концом кривой
        Lpk2pk = GetLpk2pk(PKr,PKpoint,PkStnd,Npk,NpL);                           //Расстояние от начала до точки на кривой        
        qDebug() << "R"<< i << "|"<<  Lpk2pk  << "ПКнач" << PKr;

        if ( (Lpk2pk != -1)&& (Lpk2pk <= Lr) ) { //Обработка смещения между переходными кривыми q+z
            qDebug() << "попал!"<< CenterLR.value(i);
            zi=z.value(i).toDouble();
            qi=q.value(i).toDouble();
            hg = h.value(i).toDouble();
            qDebug() << "Возвышение: H" << h.value(i) << "h=" << hg;
        }

        //конец, выходящая
        Lpk2pk = GetLpk2pk(PKout.value(i),PKpoint,PkStnd,Npk,NpL);
        qDebug() << "out"<< i << "|"<< Lpk2pk << "ПКнач" << PKin.value(i);
        if ( (Lpk2pk != -1) && (Lpk2pk < Lout.value(i).toDouble()) ) {
            qDebug() << "попал!out! ";
            li  = Lout.value(i).toDouble()-Lpk2pk;
            qDebug() << "li=" << li;
            if (RL=="L"){
                zi = Yi(li,Cout.value(i).toDouble());
                qi = Qi(li,Lout.value(i).toDouble(),q.value(i).toDouble());
                qDebug() << "от тангенса= " << "Yi=" << zi << "Qi=" << qi;
            }//Переходка на прямом участке
            if (RL=="R"){
            lii = GetLpk2pk(PKpoint,PKkkr.value(i),PkStnd,Npk,NpL); ;
            qDebug() << "lii=" << lii;
            zi = Zi(li,lii,Cout.value(i).toDouble(),Rout.value(i).toDouble());
            qi = Qi(li,Lout.value(i).toDouble(),q.value(i).toDouble());
            qDebug() << "переходка от кривой"<< "ПкКОНкр.=" << PKkkr.value(i) << "PKpoint=" << PKpoint << "li=" << Lpk2pk << "lii=" << lii << "C=" << Cin.value(i) << "R=" << Rout.value(i) << "Zi=" << zi << "Qi=" << qi;
            }//Переходка на кривой
            //расчет h
//            hg = h.value(i).toDouble();
            hg = h.value(i).toDouble()*(Lout.value(i).toDouble()-Lpk2pk)/Lout.value(i).toDouble();
            qDebug() << "Возвышение: H" << h.value(i) << " L от начала пер. до точки" << Lpk2pk << " L всей пер." <<Lin.value(i);
        }

        if (PKpoint==PKout.value(i)) {
            zi=z.value(i).toDouble();
            qi=q.value(i).toDouble();
            hg = h.value(i).toDouble();
            qDebug()<<"Уникальный случай"<< PKpoint << "=" <<PKout.value(i);
        }

        if ((zi!=0)||(qi!=0)){
            //Вычисляем смецения от оси пути и оси тоннеля из q и z
            if (CenterLR.value(i)=="L") {
                if (SMpoint>0){
                    SMpointZ  = SMpoint +  zi;
                    SMpointZQ = SMpoint + (zi+qi);
                } else {
                    SMpointZ  = SMpoint +  zi;
                    SMpointZQ = SMpoint + (zi+qi);
                }
            }//ЦК кривой слева
            if (CenterLR.value(i)=="R") {
                if (SMpoint>0){
                    SMpointZ  = SMpoint -  zi;
                    SMpointZQ = SMpoint - (zi+qi);
                } else {
                    SMpointZ  = SMpoint -  zi;
                    SMpointZQ = SMpoint - (zi+qi);
                }
            }//ЦК кривой справа
            qDebug() << "Смещение от разбивочной оси = " << QString::number(SMpoint,  'f',4);
            qDebug() << "Смещение от оси пути        = " << QString::number(SMpointZ, 'f',4) <<"zi= " << zi <<"               Центр кривой с ="<<CenterLR.value(i);
            qDebug() << "Смещение от оси тоннеля     = " << QString::number(SMpointZQ,'f',4) <<"zi= " << zi <<"qi= " << qi <<"Центр кривой с ="<<CenterLR.value(i);
            qDebug() << "Примечание: Если центр кривой с права, знак минус для zi qi.";
            qg = qi;
            zg = zi;
            break;
        }

    }
}


void geo::raschetH(QString PK,double  &Hpoint){
    double Lr     = 0;
    double iukl   = 0;
    double Lpk2pk = 0;
    qDebug() << "----------------Профиль Предрасчет----------------";
    for (int i = 0; i < ProfilPKnach.count()-1;i++ ){
        Lr     = GetLpk2pk(ProfilPKnach.value(i),ProfilPKnach.value(i+1),PkStnd,Npk,NpL); //Расстояние между началом и концом участка
        Lpk2pk = GetLpk2pk(ProfilPKnach.value(i),PK,PkStnd,Npk,NpL);                      //Расстояние от начала до точки
        qDebug() << ProfilPKnach.value(i)<<Lr<<Lpk2pk;
        if (ProfilPKnach.value(i)==PK){
            Hpoint=ProfilH.value(i).toDouble();
            break;
        } else {
            if ( (Lpk2pk != -1) && (Lpk2pk <= Lr) ) {
                qDebug() << "попали на участок"<< ProfilPKnach.value(i)<<ProfilPKnach.value(i+1)<<ProfilR.value(i);
                iukl   =(ProfilH.value(i+1).toDouble()-ProfilH.value(i).toDouble())/Lr;
                Hpoint = ProfilH.value(i).toDouble()+(iukl*Lpk2pk);
                qDebug() << "Отметка" << Hpoint;
                // На кривом тангенс 1
                if (ProfilR.value(i+1).toDouble()!=0){
                    Hpoint=Hpoint+(pow(Lpk2pk,2)/(2*ProfilR.value(i+1).toDouble()));
                    qDebug() << "тангенс 1"<< Hpoint;
                }
                // На кривом тангенс 2
                if (ProfilR.value(i).toDouble()!=0){
                    Hpoint=Hpoint+(pow((Lr-Lpk2pk),2)/(2*ProfilR.value(i).toDouble()));
                    qDebug() << "тангенс 2"<< Hpoint;
                }                
                break;
            } else {
                Hpoint=0; //если не нашли то обнуляем
            }
        }
    }
}

//Определяем фактическое q и qz смещение
void geo::GetSmRazbivOs(QString PKpoint,
                        double  SMpoint,   //реальное смещение, нужно только для определения с какой стороны оси.
                        QString RL,        // Попал на линию или кривую L/R
                        double  &SMZ,
                        double  &SMZQ){

    double tz=0;
    double tzq=0;
    if (SMpoint>0) {
        raschetPer(PKpoint, 100,RL,tz,tzq);
        if (tz!=0){
        SMZ  = (tz -100)*-1;
        SMZQ = (tzq-100)*-1;
        } else {
            SMZ  = 0;
            SMZQ = 0;
        }
    } //Если с права возвращаем сколько q и qz
    if (SMpoint<0) {
        raschetPer(PKpoint,-100,RL,tz,tzq);
        if (tz!=0){
        SMZ  = (tz +100)*-1;
        SMZQ = (tzq+100)*-1;
        }else {
            SMZ  = 0;
            SMZQ = 0;
        }
    } //Если с права возвращаем сколько q и qz
}

void geo::GetSmRazbivOs(QString PKpoint,
                        double  SMpoint,   //реальное смещение, нужно только для определения с какой стороны оси.
                        QString RL,        // Попал на линию или кривую L/R
                        double  &SMZ,
                        double  &SMZQ,
                        double  &hg,
                        double  &qg,
                        double  &zg){

    double tz=0;
    double tzq=0;
    hg = 0;
    qg = 0;
    zg = 0;
    if (SMpoint>0) {
        raschetPer(PKpoint, 100,RL,tz,tzq,hg,qg,zg);
        if (tz!=0){
        SMZ  = (tz -100)*-1;
        SMZQ = (tzq-100)*-1;
        } else {
            SMZ  = 0;
            SMZQ = 0;
        }
    } //Если с права возвращаем сколько q и qz
    if (SMpoint<0) {
        raschetPer(PKpoint,-100,RL,tz,tzq,hg,qg,zg);
        if (tz!=0){
        SMZ  = (tz +100)*-1;
        SMZQ = (tzq+100)*-1;
        }else {
            SMZ  = 0;
            SMZQ = 0;
        }
    } //Если с права возвращаем сколько q и qz
}


void geo::raschetXY(
        QString PKpoint,
        double  SMpoint,
        double  SMpointZ,
        double  SMpointZQ,
        double &xp,
        double &yp){
//    QVector<QString> TrassaKrLine;
//    QVector<QString> TrassaXnach;
//    QVector<QString> TrassaYnach;
//    QVector<QString> TrassaXkon;
//    QVector<QString> TrassaYkon;
//    QVector<QString> TrassaL;
//    QVector<QString> TrassaLfull;
//    QVector<QString> TrassaRadius;
//    QVector<QString> TrassaXck;
//    QVector<QString> TrassaYck;
//    QVector<QString> TrassaPKnach;
//    double tSM    = 0;    //временная переменная
    double SMZ    = 0;    //для z
    double SMZQ   = 0;    //для q z
    double Lpk2pk = 0;    //Расстояние от начала участка до точки по оси, без смещения.
    double Rugol  = 0;    //Угол между началом кривого участка до точки по оси, без смещения.
    QString cLR   = " ";
    qDebug() << "----------------План Предрасчет----------------";
    for (int i = 0; i < TrassaKrLine.count();i++ ){
        Lpk2pk = GetLpk2pk(TrassaPKnach.value(i),PKpoint,PkStnd,Npk,NpL);                   //Расстояние от начала до точки по оси
        if ( (Lpk2pk != -1) && (Lpk2pk <= TrassaL.value(i).toDouble()) ) {
            qDebug() << "попали на участок Трассы:"<< TrassaPKnach.value(i)<<Lpk2pk<<TrassaKrLine.value(i);
            qDebug() << "Данные на входе:" <<PKpoint<<"SMpoint="<<SMpoint<<"SMpointZ="<<SMpointZ<<"SMpointZQ="<<SMpointZQ;

            if (SMpoint != 0){
                qDebug()<<SMpoint;
            } else {
                if (SMpointZ !=0) {
                    if (TrassaKrLine.value(i)=="Кривая") {
                        GetSmRazbivOs(PKpoint,SMpointZ,"R",SMZ,SMZQ);
                    }
                    if (TrassaKrLine.value(i)=="Прямая") {
                        GetSmRazbivOs(PKpoint,SMpointZ,"L",SMZ,SMZQ);
                    }
                    SMpoint=SMpointZ+SMZ;
                } //Z
                if (SMpointZQ!=0) {
                    if (TrassaKrLine.value(i)=="Кривая") {
                        GetSmRazbivOs(PKpoint,SMpointZQ,"R",SMZ,SMZQ);
                    }
                    if (TrassaKrLine.value(i)=="Прямая") {
                        GetSmRazbivOs(PKpoint,SMpointZQ,"L",SMZ,SMZQ);
                    }
                    SMpoint=SMpointZQ+SMZQ;
                }//ZQ
            }//определили смещение от разбивочной оси
            qDebug() <<"Подали на вход:"<<"Pk="<<PKpoint<<"Sm="<<SMpoint<<"SmZ="<<SMpointZ<<"SmZQ="<<SMpointZQ;
            qDebug() <<"Получили на выходе:"<<"SMZ="<<SMZ<<"SMZQ="<<SMZQ;
            qDebug() << "Переходку просчитывать не надо"<<TrassaKrLine.value(i);
            if (TrassaKrLine.value(i)=="Кривая") {
                Rugol = (Lpk2pk*180)/(M_PI*TrassaRadius.value(i).toDouble());
                qDebug() << TrassaKrLine.value(i)<<Lpk2pk<<TrassaRadius.value(i)<<M_PI<<Rugol;

                //Определяем с какой стороны центр кривой
                if (SmeshLine(TrassaXck.value(i).toDouble(),  TrassaYck.value(i).toDouble(),
                               TrassaXnach.value(i).toDouble(),TrassaYnach.value(i).toDouble(),
                               TrassaXkon.value(i).toDouble(), TrassaYkon.value(i).toDouble() )
                        > 0 ) {cLR = "R";}  else {cLR = "L";}
                if (cLR=="L") {
                    qDebug() << "Кривая цк слева";
                    Pgz(TrassaXck.value(i).toDouble(),  TrassaYck.value(i).toDouble(),
                        TrassaXnach.value(i).toDouble(),TrassaYnach.value(i).toDouble(),
                        Rugol*-1,
                        TrassaRadius.value(i).toDouble()+SMpoint, //Расстояние от начала участка до точки.
                        xp,yp);
                }//Кривая с лева
                if (cLR=="R") {
                    qDebug() << "Кривая цк справа";
                    Pgz(TrassaXck.value(i).toDouble(),  TrassaYck.value(i).toDouble(),
                        TrassaXnach.value(i).toDouble(),TrassaYnach.value(i).toDouble(),
                        Rugol,
                        TrassaRadius.value(i).toDouble()-SMpoint, //Расстояние от начала участка до точки.
                        xp,yp);

                }//Кривая с права
            }//Кривая

            if (TrassaKrLine.value(i)=="Прямая") {

                Rugol = qRadiansToDegrees(atan(SMpoint/Lpk2pk));

                Pgz(TrassaXnach.value(i).toDouble(),TrassaYnach.value(i).toDouble(),
                    TrassaXkon.value(i).toDouble(),TrassaYkon.value(i).toDouble(),
                    Rugol,
                    sqrt(pow(Lpk2pk,2)+pow(SMpoint,2)), //Расстояние от начала участка до точки.
                    xp,yp);
                qDebug() << TrassaKrLine.value(i)<<Lpk2pk<<SMpoint<<Rugol;
            } //Прямая
            break;
        }
    }

}

void geo::raschetXY(
        QString PKpoint,
        double  SMpoint,
        double  SMpointZ,
        double  SMpointZQ,
        double &xp,
        double &yp,
        double &hg,
        double &qg,
        double &zg){
//    QVector<QString> TrassaKrLine;
//    QVector<QString> TrassaXnach;
//    QVector<QString> TrassaYnach;
//    QVector<QString> TrassaXkon;
//    QVector<QString> TrassaYkon;
//    QVector<QString> TrassaL;
//    QVector<QString> TrassaLfull;
//    QVector<QString> TrassaRadius;
//    QVector<QString> TrassaXck;
//    QVector<QString> TrassaYck;
//    QVector<QString> TrassaPKnach;
//    double tSM    = 0;    //временная переменная
    double SMZ    = 0;    //для z
    double SMZQ   = 0;    //для q z
    double Lpk2pk = 0;    //Расстояние от начала участка до точки по оси, без смещения.
    double Rugol  = 0;    //Угол между началом кривого участка до точки по оси, без смещения.
    QString cLR   = " ";
    qDebug() << "----------------План Предрасчет----------------";
    for (int i = 0; i < TrassaKrLine.count();i++ ){
        Lpk2pk = GetLpk2pk(TrassaPKnach.value(i),PKpoint,PkStnd,Npk,NpL);                   //Расстояние от начала до точки по оси
        if ( (Lpk2pk != -1) && (Lpk2pk <= TrassaL.value(i).toDouble()) ) {
            qDebug() << "попали на участок Трассы:"<< TrassaPKnach.value(i)<<Lpk2pk<<TrassaKrLine.value(i);
            qDebug() << "Данные на входе:" <<PKpoint<<"SMpoint="<<SMpoint<<"SMpointZ="<<SMpointZ<<"SMpointZQ="<<SMpointZQ;

            if (SMpoint != 0){
                qDebug()<<SMpoint;
            } else {
                if (SMpointZ !=0) {
                    if (TrassaKrLine.value(i)=="Кривая") {
                        GetSmRazbivOs(PKpoint,SMpointZ,"R",SMZ,SMZQ,hg,qg,zg);
                    }
                    if (TrassaKrLine.value(i)=="Прямая") {
                        GetSmRazbivOs(PKpoint,SMpointZ,"L",SMZ,SMZQ,hg,qg,zg);
                    }
                    SMpoint=SMpointZ+SMZ;
                } //Z
                if (SMpointZQ!=0) {
                    if (TrassaKrLine.value(i)=="Кривая") {
                        GetSmRazbivOs(PKpoint,SMpointZQ,"R",SMZ,SMZQ,hg,qg,zg);
                    }
                    if (TrassaKrLine.value(i)=="Прямая") {
                        GetSmRazbivOs(PKpoint,SMpointZQ,"L",SMZ,SMZQ,hg,qg,zg);
                    }
                    SMpoint=SMpointZQ+SMZQ;
                }//ZQ
            }//определили смещение от разбивочной оси
            qDebug() <<"Подали на вход:"<<"Pk="<<PKpoint<<"Sm="<<SMpoint<<"SmZ="<<SMpointZ<<"SmZQ="<<SMpointZQ;
            qDebug() <<"Получили на выходе:"<<"SMZ="<<SMZ<<"SMZQ="<<SMZQ;
            qDebug() << "Переходку просчитывать не надо"<<TrassaKrLine.value(i);
            if (TrassaKrLine.value(i)=="Кривая") {
                Rugol = (Lpk2pk*180)/(M_PI*TrassaRadius.value(i).toDouble());
                qDebug() << TrassaKrLine.value(i)<<Lpk2pk<<TrassaRadius.value(i)<<M_PI<<Rugol;

                //Определяем с какой стороны центр кривой
                if (SmeshLine(TrassaXck.value(i).toDouble(),  TrassaYck.value(i).toDouble(),
                               TrassaXnach.value(i).toDouble(),TrassaYnach.value(i).toDouble(),
                               TrassaXkon.value(i).toDouble(), TrassaYkon.value(i).toDouble() )
                        > 0 ) {cLR = "R";}  else {cLR = "L";}
                if (cLR=="L") {
                    qDebug() << "Кривая цк слева";
                    Pgz(TrassaXck.value(i).toDouble(),  TrassaYck.value(i).toDouble(),
                        TrassaXnach.value(i).toDouble(),TrassaYnach.value(i).toDouble(),
                        Rugol*-1,
                        TrassaRadius.value(i).toDouble()+SMpoint, //Расстояние от начала участка до точки.
                        xp,yp);
                }//Кривая с лева
                if (cLR=="R") {
                    qDebug() << "Кривая цк справа";
                    Pgz(TrassaXck.value(i).toDouble(),  TrassaYck.value(i).toDouble(),
                        TrassaXnach.value(i).toDouble(),TrassaYnach.value(i).toDouble(),
                        Rugol,
                        TrassaRadius.value(i).toDouble()-SMpoint, //Расстояние от начала участка до точки.
                        xp,yp);

                }//Кривая с права
            }//Кривая

            if (TrassaKrLine.value(i)=="Прямая") {

                Rugol = qRadiansToDegrees(atan(SMpoint/Lpk2pk));

                Pgz(TrassaXnach.value(i).toDouble(),TrassaYnach.value(i).toDouble(),
                    TrassaXkon.value(i).toDouble(),TrassaYkon.value(i).toDouble(),
                    Rugol,
                    sqrt(pow(Lpk2pk,2)+pow(SMpoint,2)), //Расстояние от начала участка до точки.
                    xp,yp);
                qDebug() << TrassaKrLine.value(i)<<Lpk2pk<<SMpoint<<Rugol;
            } //Прямая
            break;
        }
    }

}


