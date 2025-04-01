#include "sceneColorificator.h"
#include <stdexcept>
#include <QString>
#include <QDebug>

// ColoredVertexIterator implementation
ColoredVertexIterator::ColoredVertexIterator(const Scene* scene,
                                             const SceneColorificator* colorificator,
                                             std::size_t objIndex,
                                             std::size_t vertexIndex)
    : scene_(scene), colorificator_(colorificator), objIndex_(objIndex), vertexIndex_(vertexIndex)
{
    const auto& objs = scene_->getAllObjects();
    if (scene_ && objIndex_ < objs.size())
        loadCurrentConversion();
}

void ColoredVertexIterator::loadCurrentConversion() {
    const auto& objs = scene_->getAllObjects();
    if (objIndex_ < objs.size()) {
        auto sp = objs[objIndex_].lock();
        if (!sp) {
            QString msg = "Scene object at index " + QString::number(objIndex_) + " has expired.";
            qWarning() << msg;
            throw std::runtime_error(msg.toStdString());
        }
        currentConv_ = scene_->convertObject(sp->id);
    }
}

void ColoredVertexIterator::advanceToNext() {
    const auto& objs = scene_->getAllObjects();
    ++vertexIndex_;
    while (objIndex_ < objs.size() && vertexIndex_ >= currentConv_.vertices.size()) {
        ++objIndex_;
        vertexIndex_ = 0;
        if (objIndex_ < objs.size())
            loadCurrentConversion();
    }
}

ColoredVertex ColoredVertexIterator::operator*() const {
    const auto& objs = scene_->getAllObjects();
    if (!scene_ || objIndex_ >= objs.size() ||
        vertexIndex_ >= currentConv_.vertices.size()){
        QString msg = "Iterator out of range.";
        qWarning() << msg;
        throw std::out_of_range(msg.toStdString());
    }
    ColoredVertex cv;
    cv.coords = currentConv_.vertices[vertexIndex_].second;
    cv.color = colorificator_->getColorForObject(currentConv_.objectId);
    return cv;
}

ColoredVertexIterator& ColoredVertexIterator::operator++() {
    advanceToNext();
    return *this;
}

bool ColoredVertexIterator::operator==(const ColoredVertexIterator& other) const {
    return scene_ == other.scene_ &&
           objIndex_ == other.objIndex_ &&
           vertexIndex_ == other.vertexIndex_;
}

bool ColoredVertexIterator::operator!=(const ColoredVertexIterator& other) const {
    return !(*this == other);
}

// ColoredEdgeIterator implementation
ColoredEdgeIterator::ColoredEdgeIterator(const Scene* scene,
                                         const SceneColorificator* colorificator,
                                         std::size_t objIndex,
                                         std::size_t edgeIndex)
    : scene_(scene), colorificator_(colorificator), objIndex_(objIndex), edgeIndex_(edgeIndex)
{
    const auto& objs = scene_->getAllObjects();
    if (scene_ && objIndex_ < objs.size())
        loadCurrentConversion();
}

void ColoredEdgeIterator::loadCurrentConversion() {
    const auto& objs = scene_->getAllObjects();
    if (objIndex_ < objs.size()) {
        auto sp = objs[objIndex_].lock();
        if (!sp) {
            QString msg = "Scene object at index " + QString::number(objIndex_) + " has expired.";
            qWarning() << msg;
            throw std::runtime_error(msg.toStdString());
        }
        currentConv_ = scene_->convertObject(sp->id);
    }
}

void ColoredEdgeIterator::advanceToNext() {
    const auto& objs = scene_->getAllObjects();
    ++edgeIndex_;
    while (objIndex_ < objs.size() && edgeIndex_ >= currentConv_.edges.size()) {
        ++objIndex_;
        edgeIndex_ = 0;
        if (objIndex_ < objs.size())
            loadCurrentConversion();
    }
}

ColoredLine ColoredEdgeIterator::operator*() const {
    const auto& objs = scene_->getAllObjects();
    if (!scene_ || objIndex_ >= objs.size() ||
        edgeIndex_ >= currentConv_.edges.size()){
        QString msg = "Iterator out of range.";
        qWarning() << msg;
        throw std::out_of_range(msg.toStdString());
    }
    ColoredLine cl;
    auto v1Id = currentConv_.edges[edgeIndex_].first;
    auto v2Id = currentConv_.edges[edgeIndex_].second;
    std::vector<double> startCoords, endCoords;
    for (const auto& vp : currentConv_.vertices) {
        if (vp.first == v1Id)
            startCoords = vp.second;
        if (vp.first == v2Id)
            endCoords = vp.second;
    }
    cl.start = startCoords;
    cl.end = endCoords;
    cl.color = colorificator_->getColorForObject(currentConv_.objectId);
    return cl;
}

ColoredEdgeIterator& ColoredEdgeIterator::operator++() {
    advanceToNext();
    return *this;
}

bool ColoredEdgeIterator::operator==(const ColoredEdgeIterator& other) const {
    return scene_ == other.scene_ &&
           objIndex_ == other.objIndex_ &&
           edgeIndex_ == other.edgeIndex_;
}

bool ColoredEdgeIterator::operator!=(const ColoredEdgeIterator& other) const {
    return !(*this == other);
}

// SceneColorificator implementation
void SceneColorificator::setColorForObject(int objectId, const QColor &color) {
    colorMapping_[objectId] = color;
}

void SceneColorificator::removeColorForObject(int objectId) {
    auto it = colorMapping_.find(objectId);
    if (it != colorMapping_.end())
        colorMapping_.erase(it);
    else {
        QString msg = "No color mapping found for the given object ID.";
        qWarning() << msg;
        throw std::out_of_range(msg.toStdString());
    }
}

QColor SceneColorificator::getColorForObject(int objectId) const {
    auto it = colorMapping_.find(objectId);
    if (it == colorMapping_.end()) {
        return QColor(Qt::white);
    }
    return it->second;
}


SceneColorificator::VertexIterator SceneColorificator::beginVertices(const Scene &scene) const {
    return ColoredVertexIterator(&scene, this, 0, 0);
}

SceneColorificator::VertexIterator SceneColorificator::endVertices(const Scene &scene) const {
    return ColoredVertexIterator(&scene, this, scene.getAllObjects().size(), 0);
}

SceneColorificator::EdgeIterator SceneColorificator::beginEdges(const Scene &scene) const {
    return ColoredEdgeIterator(&scene, this, 0, 0);
}

SceneColorificator::EdgeIterator SceneColorificator::endEdges(const Scene &scene) const {
    return ColoredEdgeIterator(&scene, this, scene.getAllObjects().size(), 0);
}
