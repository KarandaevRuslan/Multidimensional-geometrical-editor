#ifndef ADJACENCY_MATRIX_VIEW_H
#define ADJACENCY_MATRIX_VIEW_H

#include <QTableView>
#include <QResizeEvent>


class AdjacencyMatrixView : public QTableView {
    Q_OBJECT
public:
    explicit AdjacencyMatrixView(QWidget* parent = nullptr);
signals:
    void viewportResized();

protected:
    void resizeEvent(QResizeEvent *event) override;
};

#endif // ADJACENCY_MATRIX_VIEW_H
