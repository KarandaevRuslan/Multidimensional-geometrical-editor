#include "sceneObjectModel.h"

SceneObjectModel::SceneObjectModel(std::weak_ptr<Scene> scene,
                                   std::weak_ptr<SceneColorificator> colorificator,
                                   QObject* parent)
    : QAbstractListModel(parent), scene_(std::move(scene)), colorificator_(std::move(colorificator)) {
    refresh();
}

int SceneObjectModel::rowCount(const QModelIndex& parent) const {
    return parent.isValid() ? 0 : static_cast<int>(object_ids_.size());
}

QVariant SceneObjectModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= static_cast<int>(object_ids_.size()))
        return QVariant();

    auto obj = getObjectByRow(index.row());
    if (!obj) return QVariant();

    switch (role) {
    case IdRole: return obj->id;
    case NameRole: return obj->name;
    case ShapeRole: return QVariant::fromValue(obj->shape);
    case ProjectionRole: return QVariant::fromValue(obj->projection);
    case RotatorsRole: return QVariant::fromValue(obj->rotators);
    case ScaleRole: return QVariant::fromValue(obj->scale);
    case OffsetRole: return QVariant::fromValue(obj->offset);
    case ColorRole: {
        auto colorificator = colorificator_.lock();
        if (!colorificator) return QColor(SceneColorificator::defaultColor);
        return colorificator->getColorForObject(obj->id);
    }
    default: return QVariant();
    }
}

bool SceneObjectModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid() || index.row() >= static_cast<int>(object_ids_.size()))
        return false;

    auto scene = scene_.lock();
    if (!scene) return false;

    int id = object_ids_[index.row()];
    auto obj = scene->getObject(id).lock();
    if (!obj) return false;

    SceneObject updated = *obj;

    switch (role) {
    case NameRole: updated.name = value.toString(); break;
    case ShapeRole: updated.shape = value.value<std::shared_ptr<NDShape>>(); break;
    case ProjectionRole: updated.projection = value.value<std::shared_ptr<Projection>>(); break;
    case RotatorsRole: updated.rotators = value.value<std::vector<Rotator>>(); break;
    case ScaleRole: updated.scale = value.value<std::vector<double>>(); break;
    case OffsetRole: updated.offset = value.value<std::vector<double>>(); break;
    case ColorRole: {
        auto colorificator = colorificator_.lock();
        if (colorificator) colorificator->setColorForObject(id, value.value<QColor>());
        emit dataChanged(index, index, { ColorRole });
        return true;
    }
    default: return false;
    }

    scene->setObject(updated.id, updated.name, updated.shape,
                     updated.projection, updated.rotators,
                     updated.scale, updated.offset);

    emit dataChanged(index, index, { role });
    return true;
}

QHash<int, QByteArray> SceneObjectModel::roleNames() const {
    return {
        { IdRole, "id" },
        { NameRole, "name" },
        { ShapeRole, "shape" },
        { ProjectionRole, "projection" },
        { RotatorsRole, "rotators" },
        { ScaleRole, "scale" },
        { OffsetRole, "offset" },
        { ColorRole, "color" }
    };
}

Qt::ItemFlags SceneObjectModel::flags(const QModelIndex& index) const {
    return index.isValid()
    ? Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable
    : Qt::NoItemFlags;
}

void SceneObjectModel::refresh() {
    auto scene = scene_.lock();
    if (!scene) return;

    beginResetModel();
    object_ids_.clear();
    for (const auto& weak_obj : scene->getAllObjects()) {
        if (auto obj = weak_obj.lock()) {
            object_ids_.push_back(obj->id);
        }
    }
    endResetModel();
}

void SceneObjectModel::addSceneObject(const SceneObject& obj, const QColor& color) {
    auto scene = scene_.lock();
    auto colorificator = colorificator_.lock();
    if (!scene) return;

    int row = static_cast<int>(object_ids_.size());
    beginInsertRows(QModelIndex(), row, row);
    int id = scene->addObject(obj.id, obj.name, obj.shape, obj.projection,
                     obj.rotators, obj.scale, obj.offset);
    object_ids_.push_back(id);
    if (colorificator) {
        colorificator->setColorForObject(id, color);
    }
    endInsertRows();
}

void SceneObjectModel::addSceneObject(const SceneObject& obj){
    addSceneObject(obj, SceneColorificator::defaultColor);
}

void SceneObjectModel::removeSceneObject(int row) {
    auto scene = scene_.lock();
    auto colorificator = colorificator_.lock();
    if (!scene || row < 0 || row >= static_cast<int>(object_ids_.size())) return;

    int id = object_ids_[row];

    beginRemoveRows(QModelIndex(), row, row);
    scene->removeObject(id);
    object_ids_.erase(object_ids_.begin() + row);
    if (colorificator) {
        try {
            colorificator->removeColorForObject(id);
        } catch (...) {
            // Safe to ignore if color mapping doesn't exist
        }
    }
    endRemoveRows();
}

std::shared_ptr<SceneObject> SceneObjectModel::getObjectByRow(int row) const {
    auto scene = scene_.lock();
    if (!scene || row < 0 || row >= static_cast<int>(object_ids_.size())) return nullptr;
    return scene->getObject(object_ids_[row]).lock();
}

void SceneObjectModel::setScene(std::weak_ptr<Scene> scene) {
    scene_ = std::move(scene);
    refresh();
}

void SceneObjectModel::setSceneColorificator(std::weak_ptr<SceneColorificator> colorificator) {
    colorificator_ = std::move(colorificator);
    // Optionally, you can emit dataChanged for color updates if needed
    emit dataChanged(index(0), index(rowCount() - 1), { ColorRole });
}
