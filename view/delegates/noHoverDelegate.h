#ifndef NO_HOVER_DELEGATE_H
#define NO_HOVER_DELEGATE_H

#include <QStyledItemDelegate>

class NoHoverDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    void paint(QPainter* painter,
               const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
};

#endif // NO_HOVER_DELEGATE_H
