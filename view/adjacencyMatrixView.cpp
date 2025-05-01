#include "adjacencyMatrixView.h"

AdjacencyMatrixView::AdjacencyMatrixView(QWidget* parent)
    : QTableView(parent) {
}

void AdjacencyMatrixView::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    emit viewportResized();
}
