#ifndef ROTATOR_DELEGATE_H
#define ROTATOR_DELEGATE_H

#include <QStyledItemDelegate>

class RotatorDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit RotatorDelegate(QObject* parent = nullptr);

    QWidget* createEditor(QWidget* parent,
                          const QStyleOptionViewItem& option,
                          const QModelIndex& index) const override;

    void setEditorData(QWidget* editor,
                       const QModelIndex& index) const override;

    void setModelData(QWidget* editor,
                      QAbstractItemModel* model,
                      const QModelIndex& index) const override;
};


#endif // ROTATOR_DELEGATE_H
