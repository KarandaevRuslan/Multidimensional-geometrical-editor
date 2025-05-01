#include "noHoverDelegate.h"

#include <QPainter>

void NoHoverDelegate::paint(QPainter* painter,
           const QStyleOptionViewItem& option,
           const QModelIndex& index) const
{
    QStyleOptionViewItem opt(option);
    opt.state &= ~QStyle::State_MouseOver;
    opt.state &= ~QStyle::State_HasFocus;
    opt.state &= ~QStyle::State_Selected;
    QStyledItemDelegate::paint(painter, opt, index);
}
