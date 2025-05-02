#include "rotatorTableModel.h"
#include "../../model/scene.h"
#include <QString>

RotatorTableModel::RotatorTableModel(std::function<void(SceneObject)> commit,
                                     QObject *p)
    : QAbstractTableModel(p), commit_(commit)
{
    reload();
}

int RotatorTableModel::rowCount(const QModelIndex& p) const {
    if (!p.isValid())
        if (const auto* r = rotators())
            return static_cast<int>(r->size());
    return 0;
}
int RotatorTableModel::columnCount(const QModelIndex& p) const {
    return p.isValid() ? 0 : 3;
}

QVariant RotatorTableModel::data(const QModelIndex &index, int role) const
{
    if (!sceneObject_.lock() || !index.isValid() || index.row() >= rotators()->size())
        return {};

    const Rotator &r = rotators()->at(index.row());
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
        case 0: return static_cast<qulonglong>(r.axis1());
        case 1: return static_cast<qulonglong>(r.axis2());
        case 2: return r.angle();
        }
    }
    return {};
}

bool RotatorTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!sceneObject_.lock() || role != Qt::EditRole ||
        !index.isValid() || index.row() >= rotators()->size())
        return false;


    SceneObject upd = sceneObject_.lock()->clone();
    Rotator* r = &upd.rotators[index.row()];
    bool changed = false;
    switch (index.column()) {
    case 0:
        if (value.toUInt() != r->axis1()) {
            r->setAxis1(value.toUInt());
            changed = true;
        }
        break;
    case 1:
        if (value.toUInt() != r->axis2()) {
            r->setAxis2(value.toUInt());
            changed = true;
        }
        break;
    case 2:
        if (std::abs(r->angle() - value.toDouble()) > 1e-9) {
            r->setAngle(value.toDouble());
            changed = true;
        }
        break;
    }
    if (changed) {
        commit_(upd);
        // emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});
    }
    return changed;
}

Qt::ItemFlags RotatorTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags f = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    if (index.isValid())
        f |= Qt::ItemIsEditable;
    return f;
}

QVariant RotatorTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return {};

    if (orientation == Qt::Horizontal) {
        switch (section) {
        case 0: return QObject::tr("Axis i");
        case 1: return QObject::tr("Axis j");
        case 2: return QObject::tr("Angle");
        }
    } else if (orientation == Qt::Vertical) {
        return QString::number(section);
    }

    return {};
}

void RotatorTableModel::reload()
{
    beginResetModel();
    endResetModel();
}
