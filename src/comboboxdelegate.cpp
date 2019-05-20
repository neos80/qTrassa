#include "comboboxdelegate.h"
#include <QtGui>

ComboBoxDelegate::ComboBoxDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}

QWidget *ComboBoxDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &/* option */,
    const QModelIndex &/* index */) const
{
    QComboBox *editor = new QComboBox(parent);

    return editor;
}


void ComboBoxDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
    QString value = index.model()->data(index, Qt::EditRole).toString();

    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    comboBox->addItem("Прямая");
    comboBox->addItem("Кривая");
    comboBox->setCurrentText(value);
}


void ComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
    QComboBox *comboBox = static_cast<QComboBox*>(editor);
    QString value = comboBox->itemText(comboBox->currentIndex());
    model->setData(index, value, Qt::EditRole);
}


void ComboBoxDelegate::updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}
