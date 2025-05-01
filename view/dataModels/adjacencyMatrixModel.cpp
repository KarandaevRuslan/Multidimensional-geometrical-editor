#include "adjacencyMatrixModel.h"
#include <QBrush>
#include <QUndoStack>
#include "../../tools/numTools.h"
#include "../commands/shapeCommand.h"

/*──────────── ctor ───────────────────────────────────────────────*/
AdjacencyMatrixModel::AdjacencyMatrixModel(std::shared_ptr<NDShape> s,
                                   QUndoStack* st,
                                   std::shared_ptr<std::vector<std::size_t>> rti,
                                   QObject* p)
    : QAbstractTableModel(p), shape_(std::move(s)),
    rowToId_(std::move(rti)), undo_(st)
{
    reload();
}

/*──────────── basic info ─────────────────────────────────────────*/
int AdjacencyMatrixModel::rowCount(const QModelIndex& p) const
{
    return p.isValid()?0:int(rowToId_->size());
}
int AdjacencyMatrixModel::columnCount(const QModelIndex& p) const
{
    return rowCount(p);
}

QVariant AdjacencyMatrixModel::headerData(int s, Qt::Orientation, int role) const
{
    return (role == Qt::DisplayRole)?QVariant(s+1):QVariant();
}

Qt::ItemFlags AdjacencyMatrixModel::flags(const QModelIndex& i) const
{
    return i.isValid() ? Qt::ItemIsEnabled : Qt::NoItemFlags;
}

// ────── fast edge check ──────────────────────────────────────────────────────
void AdjacencyMatrixModel::buildEdgeLookup()
{
    edges_.clear();
    edges_.reserve(shape_->edgesSize() * 2);
    for (auto&& e : shape_->getEdges()) {
        auto p = std::minmax(e.first, e.second);
        edges_.emplace(p.first, p.second);
    }
}

bool AdjacencyMatrixModel::edgeExists(std::size_t a, std::size_t b) const
{
    if (a == b) return false;
    auto p = std::minmax(a, b);
    return edges_.find({p.first, p.second}) != edges_.end();
}

QVariant AdjacencyMatrixModel::data(const QModelIndex& i, int role) const
{
    if(!i.isValid()) return {};
    if(i.row() == i.column())
        return (role == Qt::BackgroundRole) ? QBrush(colorUndefined_) : QVariant();

    bool has = edgeExists(rowToId_->at(i.row()), rowToId_->at(i.column()));
    if(role == Qt::BackgroundRole) return QBrush(has ? colorTrue_ : colorFalse_);
    return {};
}

/*──────────── user click handler ─────────────────────────────────*/
void AdjacencyMatrixModel::toggleEdge(int row,int col)
{
    if(row == col) return;
    std::size_t a = rowToId_->at(row), b = rowToId_->at(col);
    bool nowOn = !edgeExists(a,b);

    NDShape before=*shape_;
    nowOn ? shape_->addEdge(a,b) : shape_->removeEdge(a,b);
    NDShape after=*shape_;

    undo_->push(new ShapeCommand(shape_,before,after,
                                 tr("Toggle edge"),
                                 [this]{ reload(); }));

    QModelIndex tl=index(row,col), br=index(col,row);
    emit dataChanged(tl, tl, {Qt::BackgroundRole});
    emit dataChanged(br, br, {Qt::BackgroundRole});
}

/*──────────── reload cache ───────────────────────────────────────*/
void AdjacencyMatrixModel::reload()
{
    beginResetModel();
    *rowToId_ = sortedIds(*shape_);
    buildEdgeLookup();
    endResetModel();
}
