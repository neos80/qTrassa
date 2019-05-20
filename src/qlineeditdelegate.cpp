#include "qlineeditdelegate.h"
#include <QtGui>
#include <QRegExp>
#include <QString>

QLineEditDelegate::QLineEditDelegate(QObject *parent)
    : QItemDelegate(parent)
{
    pk = false;
    celoe = false;
}

QWidget *QLineEditDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &/* option */,
    const QModelIndex &/* index */) const
{
    QLineEdit *editor = new QLineEdit(parent);
    return editor;
}

void QLineEditDelegate::setPK(bool bpk){
    pk = bpk;
}

void QLineEditDelegate::setCeloe(bool bCeloe){
    celoe = bCeloe;
}

void QLineEditDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
    QString ds = QString::number(0.1).at(1);
    QString value = index.model()->data(index, Qt::EditRole).toString();

    QLineEdit *lineedit = static_cast<QLineEdit*>(editor);
    if (pk==false)
    {
        QString s="";
        if (celoe==false) s = "[?-]{0,1}[0-9]{1,6}[?"+ ds + "]{0,1}[0-9]{0,4}"; else s = "[0-9]{1,6}";
        QRegExp regXY(s);
        lineedit->setValidator(new QRegExpValidator(regXY));
        lineedit->setText(value);
    } else {
            QString PK = "[0-9]{1,4}[+]{0,1}[0-9]{1,3}[" + ds +"]{0,1}[0-9]{0,4}";
            QRegExp regPK(PK);
            lineedit->setValidator(new QRegExpValidator(regPK));
            lineedit->setText(value);
    }
}


void QLineEditDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
    QLineEdit *lineedit = static_cast<QLineEdit*>(editor);
    QString value;
    if (pk==false)
    {
        if (celoe==false) value = QString::number( lineedit->text().toDouble(), 'f', 4 ); else value = QString::number( lineedit->text().toDouble(), 'f', 0 );
    } else {

        int         PKt      = lineedit->text().split("+").value(0).toInt();
        double      PKtPlus  = lineedit->text().split("+").value(1).toDouble();
        value = QString::number(PKt)+ "+" + QString::number(PKtPlus,'f',4);
    }
    model->setData(index, value, Qt::EditRole);
}


void QLineEditDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}

