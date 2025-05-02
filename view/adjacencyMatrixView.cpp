#include "adjacencyMatrixView.h"

AdjacencyMatrixView::AdjacencyMatrixView(QWidget* parent)
    : WheelShiftTableView(parent) {
}

void AdjacencyMatrixView::resizeEvent(QResizeEvent *ev) {
    const QSize oldSz = ev->oldSize();
    const QSize newSz = ev->size();

    WheelShiftTableView::resizeEvent(ev);

    if (newSz.width() > oldSz.width()) {
        const QRect r(oldSz.width(), 0,
                      newSz.width() - oldSz.width(),
                      newSz.height());
        viewport()->update(r);
    } else if (newSz.width() < oldSz.width()) {
        const QRect r(newSz.width(), 0,
                      oldSz.width() - newSz.width(),
                      newSz.height());
        viewport()->update(r);
    }

    if (newSz.height() > oldSz.height()) {
        const QRect r(0, oldSz.height(),
                      newSz.width(),
                      newSz.height() - oldSz.height());
        viewport()->update(r);
    } else if (newSz.height() < oldSz.height()) {
        const QRect r(0, newSz.height(),
                      newSz.width(),
                      oldSz.height() - newSz.height());
        viewport()->update(r);
    }

    emit viewportResized();
}
