#include "sceneObjectModel.h"
#include <QListView>
#include <algorithm>

/* ===== ctor / helpers =================================================== */
SceneObjectModel::SceneObjectModel(std::weak_ptr<Scene>              scene,
                                   std::weak_ptr<SceneColorificator> colorificator,
                                   QObject* parent)
    : QAbstractListModel(parent)
    , scene_(std::move(scene))
    , colorificator_(std::move(colorificator))
{
    refresh();
}

/* ===== basic overrides ================================================== */
int SceneObjectModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : static_cast<int>(object_uids_.size());
}

QVariant SceneObjectModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= rowCount()) return {};

    auto obj = getObjectByRow(index.row());
    if (!obj) return {};

    switch (role) {
    case UidRole:        return obj->uid;
    case VisualIdRole:   return obj->id;
    case NameRole:       return obj->name;
    case ShapeRole:      return QVariant::fromValue(obj->shape);
    case ProjectionRole: return QVariant::fromValue(obj->projection);
    case RotatorsRole:   return QVariant::fromValue(obj->rotators);
    case ScaleRole:      return QVariant::fromValue(obj->scale);
    case OffsetRole:     return QVariant::fromValue(obj->offset);
    case ColorRole: {
        auto colMan = colorificator_.lock();
        return colMan ? colMan->getColorForObject(obj->uid)
                      : QColor(SceneColorificator::defaultColor);
    }
    default: return {};
    }
}

bool SceneObjectModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (!index.isValid() || index.row() >= rowCount()) return false;

    auto scene = scene_.lock();
    if (!scene) return false;

    QUuid uid = object_uids_[index.row()];
    auto  sp  = scene->getObject(uid).lock();
    if (!sp)  return false;

    SceneObject updated = *sp;   // local copy to modify

    switch (role) {
    case NameRole:       updated.name       = value.toString(); break;
    case ShapeRole:      updated.shape      = value.value<std::shared_ptr<NDShape>>(); break;
    case ProjectionRole: updated.projection = value.value<std::shared_ptr<Projection>>(); break;
    case RotatorsRole:   updated.rotators   = value.value<std::vector<Rotator>>(); break;
    case ScaleRole:      updated.scale      = value.value<std::vector<double>>();  break;
    case OffsetRole:     updated.offset     = value.value<std::vector<double>>();  break;
    case ColorRole: {
        if (auto colMan = colorificator_.lock())
            colMan->setColorForObject(uid, value.value<QColor>());
        emit dataChanged(index, index, { ColorRole });
        return true;
    }
    default: return false;
    }

    scene->setObject(uid, updated.name, updated.shape, updated.projection,
                     updated.rotators, updated.scale, updated.offset);

    emit dataChanged(index, index, { role });
    return true;
}

QHash<int, QByteArray> SceneObjectModel::roleNames() const
{
    return {
        { UidRole,        "uid"      },
        { VisualIdRole,   "id"       },
        { NameRole,       "name"     },
        { ShapeRole,      "shape"    },
        { ProjectionRole, "projection"},
        { RotatorsRole,   "rotators" },
        { ScaleRole,      "scale"    },
        { OffsetRole,     "offset"   },
        { ColorRole,      "color"    }
    };
}

Qt::ItemFlags SceneObjectModel::flags(const QModelIndex& index) const
{
    return index.isValid()
         ? Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable
         : Qt::NoItemFlags;
}

/* ===== public API ======================================================= */
void SceneObjectModel::refresh()
{
    auto scene = scene_.lock();
    if (!scene) return;

    beginResetModel();
    object_uids_.clear();
    for (const auto& w : scene->getAllObjects())
        if (auto sp = w.lock()) object_uids_.push_back(sp->uid);
    endResetModel();

    // if (auto lv = qobject_cast<QListView*>(parent()); lv && rowCount() > 0)
    //     lv->setCurrentIndex(index(0,0));
}

void SceneObjectModel::addSceneObject(const SceneObject& obj, const QColor& color)
{
    auto scene = scene_.lock();
    auto colMan= colorificator_.lock();
    if (!scene) return;

    int newRow = rowCount();
    beginInsertRows({}, newRow, newRow);

    QUuid uid = scene->addObject(obj.uid, obj.id, obj.name, obj.shape, obj.projection,
                                 obj.rotators, obj.scale, obj.offset);
    object_uids_.push_back(uid);
    if (colMan) colMan->setColorForObject(uid, color);

    endInsertRows();
}

void SceneObjectModel::addSceneObject(const SceneObject& obj)
{
    addSceneObject(obj, SceneColorificator::defaultColor);
}

void SceneObjectModel::removeSceneObject(int row)
{
    auto scene = scene_.lock();
    auto colMan= colorificator_.lock();
    if (!scene || row < 0 || row >= rowCount()) return;

    QUuid uid = object_uids_[row];

    beginRemoveRows({}, row, row);
    scene->removeObject(uid);
    object_uids_.erase(object_uids_.begin() + row);
    if (colMan) {
        try   { colMan->removeColorForObject(uid); }
        catch (...) { /* ignore missing mapping */ }
    }
    endRemoveRows();
}

std::shared_ptr<SceneObject> SceneObjectModel::getObjectByRow(int row) const
{
    auto scene = scene_.lock();
    if (!scene || row < 0 || row >= rowCount()) return {};
    return scene->getObject(object_uids_[row]).lock();
}

void SceneObjectModel::setScene(std::weak_ptr<Scene> scene)
{
    scene_ = std::move(scene);
    refresh();
}

void SceneObjectModel::setSceneColorificator(std::weak_ptr<SceneColorificator> c)
{
    colorificator_ = std::move(c);
    emit dataChanged(index(0), index(rowCount()-1), { ColorRole });
}

int SceneObjectModel::rowForUid(const QUuid& uid) const
{
    auto it = std::find(object_uids_.begin(), object_uids_.end(), uid);
    return it == object_uids_.end() ? -1
                                    : static_cast<int>(std::distance(object_uids_.begin(), it));
}

void SceneObjectModel::debugPrintAll() const
{
    auto colMan = colorificator_.lock();
    qDebug() << "---- SceneObjectModel (uids + colors) ----";
    for (const QUuid& uid : object_uids_) {
        QColor col = colMan ? colMan->getColorForObject(uid)
                            : QColor(SceneColorificator::defaultColor);
        qDebug() << "  uid =" << uid.toString(QUuid::WithoutBraces)
                 << ", color =" << col.name();
    }
    qDebug() << "-----------------------------------------";
}
