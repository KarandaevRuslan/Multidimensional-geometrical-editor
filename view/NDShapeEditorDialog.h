#ifndef NDSHAPE_EDITOR_DIALOG_H
#define NDSHAPE_EDITOR_DIALOG_H

#include <QDialog>
#include <QTableWidgetItem>
#include <QTableView>
#include <QVector>
#include <memory>
#include <QUndoStack>
#include "../model/NDShape.h"
#include "dataModels/adjacencyMatrixModel.h"
#include "dataModels/vertexTableModel.h"

class QSpinBox;
class QTableWidget;
class QToolButton;

/*! Dialog that lets the user edit dimension, vertices and the adjacency
 *  matrix of an NDShape.  On construction it *clones* the supplied shape,
 *  so cancelling never mutates the original.  When the user clicks **OK**
 *  call `takeShape()` to obtain the edited shape.
 */
class NDShapeEditorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit NDShapeEditorDialog(const NDShape& startShape,
                                 QWidget* parent = nullptr);

    /// Transfers ownership of the edited shape to the caller.
    std::shared_ptr<NDShape> takeShape() { return std::move(shape_); }

private slots:
    void onDimensionChanged(int d);
    void addVertex();
    void removeVertices();

    void showVertContextMenu(const QPoint& pos);
    void copyVertices();
    void cutVertices();
    void pasteVertices();

    void onAdjCellClicked(const QModelIndex& idx);
    void onAdjCellSelected(const QItemSelection &selected, const QItemSelection &deselected);
private:
    void structuralReload();

    std::shared_ptr<NDShape> shape_;
    QUndoStack*              undo_;

    QSpinBox*               dimSpin_{};
    QTableView*             vertView_{};
    QTableView*             adjView_{};
    VertexTableModel*       vertModel_{};
    AdjacencyMatrixModel*   adjModel_{};

    QVector<std::vector<double>>    vertClipboard_;
    QHash<QString,QAction*>         vertActs_;
    std::shared_ptr<std::vector<std::size_t>> rowToId_;

    int dimMin_ = 3;
    int   dimMax_ = 20;
};

#endif // NDSHAPE_EDITOR_DIALOG_H
