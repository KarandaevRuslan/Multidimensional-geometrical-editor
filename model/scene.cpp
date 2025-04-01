#include "scene.h"
#include <algorithm>
#include <stdexcept>
#include <QString>
#include <QDebug>

Scene::~Scene() {
    objects_.clear();
}

void Scene::addObject(int id, std::shared_ptr<NDShape> shape,
                      std::shared_ptr<Projection> projection,
                      const std::vector<Rotator>& rotators,
                      const std::vector<double>& scale,
                      const std::vector<double>& offset)
{
    auto it = std::find_if(objects_.begin(), objects_.end(),
                           [id](const std::shared_ptr<SceneObject>& obj) { return obj->id == id; });
    if (it != objects_.end()) {
        QString msg = "An object with the given ID already exists.";
        qWarning() << msg;
        throw std::invalid_argument(msg.toStdString());
    }

    // Check that scale and offset dimensions match the scene dimension if provided.
    if (!scale.empty() && scale.size() != sceneDimension_) {
        QString msg = "Scale dimension does not match the scene dimension.";
        qWarning() << msg;
        throw std::invalid_argument(msg.toStdString());
    }
    if (!offset.empty() && offset.size() != sceneDimension_) {
        QString msg = "Offset dimension does not match the scene dimension.";
        qWarning() << msg;
        throw std::invalid_argument(msg.toStdString());
    }

    std::shared_ptr<SceneObject> obj = std::make_shared<SceneObject>();
    obj->id = id;
    obj->shape = shape;
    obj->projection = projection;
    obj->rotators = rotators;
    obj->scale = scale;
    obj->offset = offset;
    objects_.push_back(obj);
}

void Scene::removeObject(int id)
{
    auto it = std::remove_if(objects_.begin(), objects_.end(),
                             [id](const std::shared_ptr<SceneObject>& obj) { return obj->id == id; });
    if (it == objects_.end()) {
        QString msg = "No object found with the given ID.";
        qWarning() << msg;
        throw std::out_of_range(msg.toStdString());
    }
    objects_.erase(it, objects_.end());
}

std::weak_ptr<SceneObject> Scene::getObject(int id) const
{
    auto it = std::find_if(objects_.begin(), objects_.end(),
                           [id](const std::shared_ptr<SceneObject>& obj) { return obj->id == id; });
    if (it == objects_.end()) {
        QString msg = "No object found with the given ID.";
        qWarning() << msg;
        throw std::out_of_range(msg.toStdString());
    }
    return std::weak_ptr<SceneObject>(*it);
}

void Scene::setObject(int id, std::shared_ptr<NDShape> shape,
                      std::shared_ptr<Projection> projection,
                      const std::vector<Rotator>& rotators,
                      const std::vector<double>& scale,
                      const std::vector<double>& offset)
{
    auto it = std::find_if(objects_.begin(), objects_.end(),
                           [id](const std::shared_ptr<SceneObject>& obj) { return obj->id == id; });
    if (it == objects_.end()) {
        QString msg = "No object found with the given ID.";
        qWarning() << msg;
        throw std::out_of_range(msg.toStdString());
    }

    // Check that scale and offset dimensions match the scene dimension if provided.
    if (!scale.empty() && scale.size() != sceneDimension_) {
        QString msg = "Scale dimension does not match the scene dimension.";
        qWarning() << msg;
        throw std::invalid_argument(msg.toStdString());
    }
    if (!offset.empty() && offset.size() != sceneDimension_) {
        QString msg = "Offset dimension does not match the scene dimension.";
        qWarning() << msg;
        throw std::invalid_argument(msg.toStdString());
    }

    (*it)->shape = shape;
    (*it)->projection = projection;
    (*it)->rotators = rotators;
    (*it)->scale = scale;
    (*it)->offset = offset;
}

std::vector<std::weak_ptr<SceneObject>> Scene::getAllObjects() const
{
    std::vector<std::weak_ptr<SceneObject>> weakObjects;
    for (const auto& obj : objects_) {
        weakObjects.push_back(std::weak_ptr<SceneObject>(obj));
    }
    return weakObjects;
}

ConvertedData Scene::convertObject(int id) const
{
    auto sp = getObject(id).lock();
    if (!sp) {
        QString msg = "No object found with the given ID.";
        qWarning() << msg;
        throw std::out_of_range(msg.toStdString());
    }
    ConvertedData data;
    data.objectId = sp->id;

    // 1. Start with the original shape.
    NDShape transformed = *(sp->shape);

    // 2. Apply rotations in sequence.
    for (const auto &rotator : sp->rotators) {
        transformed = rotator.applyRotation(transformed);
    }

    // 3. Apply projection if needed.
    NDShape projected;
    if (sp->projection && transformed.getDimension() > sceneDimension_) {
        projected = sp->projection->projectShapeToDimension(transformed, sceneDimension_);
    } else {
        projected = transformed;
    }

    // Retrieve vertices and edges from the projected shape.
    data.vertices = projected.getAllVertices();
    data.edges = projected.getEdges();

    // 4. Apply the scale transformation in the scene dimension.
    if (!sp->scale.empty()) {
        for (auto &vertex : data.vertices) {
            if (vertex.second.size() != sp->scale.size()) {
                QString msg = "Vertex dimension and scale dimension mismatch.";
                qWarning() << msg;
                throw std::runtime_error(msg.toStdString());
            }
            for (std::size_t i = 0; i < vertex.second.size(); ++i) {
                vertex.second[i] *= sp->scale[i];
            }
        }
    }

    // 5. Apply the offset transformation in the scene dimension.
    if (!sp->offset.empty()) {
        for (auto &vertex : data.vertices) {
            if (vertex.second.size() != sp->offset.size()) {
                QString msg = "Vertex dimension and offset dimension mismatch.";
                qWarning() << msg;
                throw std::runtime_error(msg.toStdString());
            }
            for (std::size_t i = 0; i < vertex.second.size(); ++i) {
                vertex.second[i] += sp->offset[i];
            }
        }
    }

    return data;
}

std::vector<ConvertedData> Scene::convertAllObjects() const
{
    std::vector<ConvertedData> results;
    for (const auto& obj : objects_) {
        results.push_back(convertObject(obj->id));
    }
    return results;
}

void Scene::setSceneDimension(std::size_t dim)
{
    if (dim < 1) {
        QString msg = "Scene dimension must be at least 1.";
        qWarning() << msg;
        throw std::invalid_argument(msg.toStdString());
    }
    sceneDimension_ = dim;
}

std::size_t Scene::getSceneDimension() const
{
    return sceneDimension_;
}
