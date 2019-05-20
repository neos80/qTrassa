#ifndef QLINEEDITDELEGATE_H
#define QLINEEDITDELEGATE_H
#include <QItemDelegate>
#include <QModelIndex>
#include <QObject>
#include <QSize>
#include <QLineEdit>


class QLineEditDelegate : public QItemDelegate
{
    Q_OBJECT

public:

    bool pk;
    bool celoe;

    QLineEditDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const;


    void setPK(bool bpk);
    void setCeloe(bool bCeloe);
};

#endif // QLINEEDITDELEGATE_H
