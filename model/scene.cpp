#include "scene.h"
#include <algorithm>
#include <set>
#include <stdexcept>
#include <QString>
#include <QDebug>
#include "projection.h"

SceneObject SceneObject::clone()
{
    SceneObject copy = *this;
    if (shape)      copy.shape      = std::make_shared<NDShape>(*shape);
    if (projection) copy.projection = projection->clone();
    return copy;
}

Scene::~Scene() { qDebug() << "Scene cleared"; }

QUuid Scene::addObject(QUuid uid, int id, QString name,
                       std::shared_ptr<NDShape>     shape,
                       std::shared_ptr<Projection>  projection,
                       const std::vector<Rotator>&  rotators,
                       const std::vector<double>&   scale,
                       const std::vector<double>&   offset)
{
    // Ensure id uniqueness (visual only).
    std::set<int> used;
    for (const auto& o : objects_) used.insert(o->id);
    while (used.count(id)) ++id;

    // Dimension checks.
    if (!scale.empty()  && scale.size()  != sceneDimension_)
        throw std::invalid_argument("Scale dimension mismatch");
    if (!offset.empty() && offset.size() != sceneDimension_)
        throw std::invalid_argument("Offset dimension mismatch");

    auto obj       = std::make_shared<SceneObject>();
    obj->uid       = uid;
    obj->id        = id;
    obj->name      = std::move(name);
    obj->shape     = std::move(shape);
    obj->projection= std::move(projection);
    obj->rotators  = rotators;
    obj->scale     = scale;
    obj->offset    = offset;

    objects_.push_back(obj);
    return obj->uid;
}

void Scene::removeObject(const QUuid& uid)
{
    auto it = std::remove_if(objects_.begin(), objects_.end(),
                             [&uid](const std::shared_ptr<SceneObject>& o){ return o->uid == uid; });
    if (it == objects_.end())
        throw std::out_of_range("No object with given uid");
    objects_.erase(it, objects_.end());
}

std::weak_ptr<SceneObject> Scene::getObject(const QUuid& uid) const
{
    auto it = std::find_if(objects_.cbegin(), objects_.cend(),
                           [&uid](const std::shared_ptr<SceneObject>& o){ return o->uid == uid; });
    if (it == objects_.cend())
        throw std::out_of_range("No object with given uid");
    return *it;
}

void Scene::setObject(const QUuid& uid,
                      QString name,
                      std::shared_ptr<NDShape>   shape,
                      std::shared_ptr<Projection> projection,
                      const std::vector<Rotator>& rotators,
                      const std::vector<double>&  scale,
                      const std::vector<double>&  offset)
{
    auto sp = getObject(uid).lock();
    if (!sp) throw std::out_of_range("Stale pointer for uid");

    if (!scale.empty()  && scale.size()  != sceneDimension_)
        throw std::invalid_argument("Scale dimension mismatch");
    if (!offset.empty() && offset.size() != sceneDimension_)
        throw std::invalid_argument("Offset dimension mismatch");

    sp->name       = std::move(name);
    sp->shape      = std::move(shape);
    sp->projection = std::move(projection);
    sp->rotators   = rotators;
    sp->scale      = scale;
    sp->offset     = offset;
}

std::vector<std::weak_ptr<SceneObject>> Scene::getAllObjects() const
{
    std::vector<std::weak_ptr<SceneObject>> out;
    for (auto& o : objects_) out.emplace_back(o);
    return out;
}

std::size_t Scene::objectCount() const {
    return objects_.size();
}

ConvertedData Scene::convertObject(const SceneObject& obj, int sceneDimension)
{
    ConvertedData res;
    res.objectUid = obj.uid;

    NDShape transformed = *obj.shape;
    for (const Rotator& r : obj.rotators)
        transformed = r.applyRotation(transformed);

    if(!obj.projection && transformed.getDimension() > sceneDimension)
        throw std::invalid_argument("Projection \"None\" is not allowed for this object.");

    NDShape projected = (transformed.getDimension() > sceneDimension)
                            ? obj.projection->projectShapeToDimension(transformed, sceneDimension)
                            : transformed;

    res.vertices = projected.getAllVertices();
    res.edges    = projected.getEdges();

    if (!obj.scale.empty()) {
        for (auto& v : res.vertices)
            for (std::size_t i = 0; i < v.second.size(); ++i)
                v.second[i] *= obj.scale[i];
    }
    if (!obj.offset.empty()) {
        for (auto& v : res.vertices)
            for (std::size_t i = 0; i < v.second.size(); ++i)
                v.second[i] += obj.offset[i];
    }
    return res;
}

ConvertedData Scene::convertObject(const QUuid& uid) const
{
    auto sp = getObject(uid).lock();
    if (!sp) throw std::out_of_range("Stale pointer for uid");
    return convertObject(*sp, sceneDimension_);
}

std::vector<ConvertedData> Scene::convertAllObjects() const
{
    std::vector<ConvertedData> v;
    for (auto& o : objects_) v.push_back(convertObject(o->uid));
    return v;
}

void Scene::setSceneDimension(std::size_t d)
{
    if (d < 1) throw std::invalid_argument("Scene dimension must be ≥ 1");
    sceneDimension_ = d;
}
std::size_t Scene::getSceneDimension() const { return sceneDimension_; }
