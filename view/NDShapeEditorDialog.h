#ifndef NDSHAPE_EDITOR_DIALOG_H
#define NDSHAPE_EDITOR_DIALOG_H

#include <QDialog>
#include <QTableWidgetItem>
#include <QVector>
#include <memory>

#include "../model/NDShape.h"

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
    void onVertexTableItemChanged(QTableWidgetItem* it);
    void onAdjCellClicked(int row, int col);

    void showVertContextMenu(const QPoint& pos);
    void copyVertices();
    void cutVertices();
    void pasteVertices();

private:
    QHash<QString,QAction*> vertActions_;
    QList<std::vector<double>> vertClipboard_;

    // ───────────────── helpers ─────────────────
    void refreshVertexVerticalHeader();
    void rebuildVertexTable();
    void rebuildAdjacencyTable();
    void refreshAdjHeaders();

    // ───────────────── state ─────────────────
    std::shared_ptr<NDShape> shape_;            ///< the working clone
    QVector<std::size_t>     rowToId_;          ///< row→vertex-ID map

    // ───────────────── ui ─────────────────
    QSpinBox*     dimSpin_      = nullptr;
    QTableWidget* vertTable_    = nullptr;
    QTableWidget* adjTable_     = nullptr;

    QColor colorUndefined_ = Qt::black;
    QColor colorTrue_ = Qt::darkGreen;
    QColor colorFalse_ = Qt::darkRed;
};

#endif // NDSHAPE_EDITOR_DIALOG_H
