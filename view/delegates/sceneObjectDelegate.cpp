#include "sceneObjectDelegate.h"
#include <QPainter>
#include <QApplication>
#include "../dataModels/sceneObjectModel.h"

SceneObjectDelegate::SceneObjectDelegate(QObject* parent)
    : QStyledItemDelegate(parent) {}

void SceneObjectDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option,
                                const QModelIndex& index) const {
    painter->save();

    QString name = index.data(SceneObjectModel::NameRole).toString();
    QColor color = index.data(SceneObjectModel::ColorRole).value<QColor>();

    // Draw selection background
    if (option.state & QStyle::State_Selected)
        painter->fillRect(option.rect, option.palette.highlight());

    // Circle geometry
    int radius = option.rect.height() / 3;
    QRect circleRect(option.rect.left() + 6, option.rect.center().y() - radius,
                     2 * radius, 2 * radius);

    // Fill color
    painter->setBrush(color);
    painter->setPen(outlineColor_);
    painter->drawEllipse(circleRect);

    // Draw name text
    QRect textRect = option.rect.adjusted(2 * radius + 12, 0, 0, 0);
    painter->setPen(option.palette.text().color());
    painter->drawText(textRect, Qt::AlignVCenter | Qt::AlignLeft, name);

    painter->restore();
}

QSize SceneObjectDelegate::sizeHint(const QStyleOptionViewItem& option,
                                    const QModelIndex& index) const {
    Q_UNUSED(index);
    return QSize(option.rect.width(), 28);
}
