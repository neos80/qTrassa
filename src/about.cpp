#include "about.h"
#include "ui_about.h"
#include "qsettings.h"
#include <QDesktopServices>
#include <QString>
#include <QDebug>


About::About(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::About)
{    

    ui->setupUi(this);
}

About::~About()
{
    delete ui;
}

void About::on_buttonBox_accepted()
{
    close();
}


void About::on_label_7_linkActivated(const QString &link)
{
    QDesktopServices::openUrl(QUrl("https://github.com/neos80/qTrassa/"));
    qDebug() << link;
}
