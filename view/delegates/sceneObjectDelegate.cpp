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

    // --- Circle setup ---
    int radius = option.rect.height() / 4;
    int circleDiameter = 2 * radius;
    int circleMargin = 10;

    QRect circleRect(option.rect.left() + circleMargin,
                     option.rect.center().y() - radius,
                     circleDiameter, circleDiameter);

    // Draw circle
    painter->setBrush(color);
    painter->setPen(outlineColor_);
    painter->drawEllipse(circleRect);

    // --- Text setup ---
    int textLeft = circleRect.right() + circleMargin;
    int textRight = option.rect.right() - circleMargin;
    int textWidth = textRight - textLeft;

    QFontMetrics fm(option.font);
    int nameTextWidth = fm.horizontalAdvance(name);
    int nameTextHeight = fm.height();

    // Center the text within the available space
    int textX = textLeft + (textWidth - nameTextWidth) / 2;
    int textY = option.rect.center().y() + nameTextHeight / 2 - fm.descent();

    painter->setPen(option.palette.text().color());
    painter->drawText(QPoint(textX, textY), name);

    painter->restore();
}

QSize SceneObjectDelegate::sizeHint(const QStyleOptionViewItem& option,
                                    const QModelIndex& index) const {
    Q_UNUSED(index);
    return QSize(-1, 28);
}
