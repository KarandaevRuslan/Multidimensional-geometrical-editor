#include "rotatorDelegate.h"

#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QtMath>

RotatorDelegate::RotatorDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{}

QWidget* RotatorDelegate::createEditor(QWidget* parent,
                                       const QStyleOptionViewItem&,
                                       const QModelIndex& index) const
{
    if (index.column() == 2)
    {
        auto* editor = new QDoubleSpinBox(parent);
        editor->setDecimals(4);
        editor->setSingleStep(0.1);
        editor->setMinimum(-M_PI);
        editor->setMaximum(M_PI);
        return editor;
    } else {
        auto* editor = new QSpinBox(parent);
        editor->setMinimum(0);
        editor->setMaximum(20);
        return editor;
    }
}

void RotatorDelegate::setEditorData(QWidget* editor,
                                    const QModelIndex& index) const
{
    QVariant value = index.model()->data(index, Qt::EditRole);
    if (auto* spin = qobject_cast<QSpinBox*>(editor)) {
        spin->setValue(value.toInt());
    } else if (auto* dbl = qobject_cast<QDoubleSpinBox*>(editor)) {
        dbl->setValue(value.toDouble());
    }
}

void RotatorDelegate::setModelData(QWidget* editor,
                                   QAbstractItemModel* model,
                                   const QModelIndex& index) const
{
    if (auto* spin = qobject_cast<QSpinBox*>(editor)) {
        model->setData(index, spin->value(), Qt::EditRole);
    } else if (auto* dbl = qobject_cast<QDoubleSpinBox*>(editor)) {
        model->setData(index, dbl->value(), Qt::EditRole);
    }
}
