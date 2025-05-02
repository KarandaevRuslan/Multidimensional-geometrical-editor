#include "wheelShiftTableView.h"
#include <QTableView>
#include <QWheelEvent>
#include <QScrollBar>

void WheelShiftTableView::wheelEvent(QWheelEvent *ev)
{
    const bool shift = ev->modifiers() & Qt::ShiftModifier;
    const bool onlyY = ev->angleDelta().y() != 0 && ev->angleDelta().x() == 0;

    if (shift && onlyY) {
        ensurePerPixel();
        const int k = 2;
        rest_ += -ev->angleDelta().y();
        const int step = rest_ / k;
        rest_ %= k;
        horizontalScrollBar()->setValue(
            horizontalScrollBar()->value() + step);
        ev->accept();
        return;
    }

    QTableView::wheelEvent(ev);
}

void WheelShiftTableView::ensurePerPixel()
{
    if (horizontalScrollMode() != QAbstractItemView::ScrollPerPixel) {
        setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
        setVerticalScrollMode(ScrollPerPixel);
    }
}
