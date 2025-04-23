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
    void onAddVertex();
    void onRemoveSelectedVertex();
    void onVertexTableItemChanged(QTableWidgetItem* it);
    void onAdjItemChanged(QTableWidgetItem* it);

private:
    // ───────────────── helpers ─────────────────
    void rebuildVertexTable();
    void rebuildAdjacencyTable();
    void refreshAdjHeaders();
    void writeBackIntoShape();

    // ───────────────── state ─────────────────
    std::shared_ptr<NDShape> shape_;            ///< the working clone
    QVector<std::size_t>     rowToId_;          ///< row→vertex-ID map

    // ───────────────── ui ─────────────────
    QSpinBox*     dimSpin_      = nullptr;
    QTableWidget* vertTable_    = nullptr;
    QToolButton*  addVertexBtn_ = nullptr;
    QToolButton*  delVertexBtn_ = nullptr;
    QTableWidget* adjTable_     = nullptr;
};

#endif // NDSHAPE_EDITOR_DIALOG_H
