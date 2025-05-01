#include "vertexTableModel.h"
#include <QUndoStack>
#include <cmath>
#include "../../tools/numTools.h"
#include "../commands/shapeCommand.h"

/*──────────── ctor ───────────────────────────────────────────────*/
VertexTableModel::VertexTableModel(std::shared_ptr<NDShape> s,
                                   QUndoStack* st,
                                   std::shared_ptr<std::vector<std::size_t>> rti,
                                   std::function<void()> sr,
                                   QObject* p)
    : QAbstractTableModel(p), shape_(std::move(s)),
    rowToId_(std::move(rti)), undo_(st), structuralReload_(std::move(sr))
{
    reload();
}

/*──────────── Qt essentials ──────────────────────────────────────*/
int VertexTableModel::rowCount(const QModelIndex& p) const
{
    return p.isValid() ? 0 : int(rowToId_->size());
}

int VertexTableModel::columnCount(const QModelIndex& p) const
{
    return p.isValid() ? 0 : int(shape_->getDimension());
}

QVariant VertexTableModel::headerData(int sec, Qt::Orientation o, int role) const
{
    if (role != Qt::DisplayRole) return {};
    return (o == Qt::Horizontal) ? tr("x%1").arg(sec + 1)
                                 : QVariant(sec + 1);
}

QVariant VertexTableModel::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid()) return {};
    const auto& c = shape_->getVertex(rowToId_->at(idx.row()));
    if (role == Qt::DisplayRole || role == Qt::EditRole)
        return QString::number(c[idx.column()], 'g', 6);
    return {};
}

Qt::ItemFlags VertexTableModel::flags(const QModelIndex& idx) const
{
    return idx.isValid() ? (Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable)
                         : Qt::NoItemFlags;
}

/*──────────── Editing ────────────────────────────────────────────*/
bool VertexTableModel::setData(const QModelIndex& idx,
                               const QVariant&    v,
                               int                role)
{
    if (!idx.isValid() || role != Qt::EditRole) return false;
    bool ok = false;
    double val = v.toDouble(&ok);
    if(!ok) return false;

    auto coords = shape_->getVertex(rowToId_->at(idx.row()));
    if (std::abs(coords[idx.column()] - val) < 1e-9) return true;

    NDShape before = *shape_;
    coords[idx.column()] = val;
    shape_->setVertexCoords(rowToId_->at(idx.row()), coords);
    NDShape after = *shape_;

    undo_->push(new ShapeCommand(shape_, before, after,
                                 tr("Edit vertex"),
                                 [this]{ structuralReload_(); }));

    emit dataChanged(idx, idx, {Qt::DisplayRole, Qt::EditRole});
    return true;
}

/*──────────── reload (row↔id cache) ──────────────────────────────*/
void VertexTableModel::reload()
{
    beginResetModel();
    *rowToId_ = sortedIds(*shape_);
    endResetModel();
}
