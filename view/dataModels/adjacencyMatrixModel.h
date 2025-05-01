#ifndef ADJACENCY_MATRIX_MODEL_H
#define ADJACENCY_MATRIX_MODEL_H

#include <QAbstractTableModel>
#include <memory>
#include "../../model/NDShape.h"
#include <QColor>

class QUndoStack;

/*! Square N×N model showing NDShape edges as coloured cells. */
class AdjacencyMatrixModel final : public QAbstractTableModel
{
    Q_OBJECT
public:
    AdjacencyMatrixModel(std::shared_ptr<NDShape> shape,
                     QUndoStack* undoStack,
                     std::shared_ptr<std::vector<std::size_t>> rowToId,
                     std::function<void()> structuralReload,
                     QObject* parent = nullptr);

    int            rowCount   (const QModelIndex& = {}) const override;
    int            columnCount(const QModelIndex& = {}) const override;
    QVariant       headerData (int, Qt::Orientation, int role) const override;
    QVariant       data       (const QModelIndex&, int role) const override;
    Qt::ItemFlags  flags      (const QModelIndex&) const override;

    /* user interaction: toggle via view->clicked */
    void toggleEdge(int row, int col);
    void setEdge(int row, int col, bool isEdge);

    void reload();

private:
    struct PairHash {
        size_t operator()(const std::pair<std::size_t,std::size_t>& p) const noexcept
        {
            return std::hash<std::size_t>()((p.first<<32) ^ p.second);
        }
    };

    bool edgeExists(std::size_t a, std::size_t b) const;

    void buildEdgeLookup();
    void rebuildCache();

    // ────── state ────────────────────────────────────────────────────────
    const QColor colorUndefined_ = Qt::black;
    const QColor colorTrue_      = Qt::darkGreen;
    const QColor colorFalse_     = Qt::darkRed;

    std::shared_ptr<NDShape> shape_;
    QUndoStack*              undo_{};

    // keeps mapping for full graph
    std::shared_ptr<std::vector<std::size_t>> rowToId_;

    // fast O(1) edge lookup
    std::unordered_set<std::pair<std::size_t,std::size_t>, PairHash> edges_;

    std::function<void()> structuralReload_;
};

#endif // ADJACENCY_MATRIX_MODEL_H
