#include "sceneColorificator.h"
#include <stdexcept>
#include <QString>
#include <QDebug>

/* ===== ColoredVertexIterator =========================================== */
ColoredVertexIterator::ColoredVertexIterator(const Scene* scene,
                                             const SceneColorificator* colorificator,
                                             std::size_t objIndex,
                                             std::size_t vertexIndex)
    : scene_(scene)
    , colorificator_(colorificator)
    , objIndex_(objIndex)
    , vertexIndex_(vertexIndex)
{
    if (scene_ && objIndex_ < scene_->getAllObjects().size()) {
        loadCurrentConversion();
        if (currentConv_.vertices.empty()) {
            advanceToNext();
        }
    }
}

void ColoredVertexIterator::loadCurrentConversion()
{
    const auto& objs = scene_->getAllObjects();
    if (objIndex_ >= objs.size()) return;

    auto sp = objs[objIndex_].lock();
    if (!sp) throw std::runtime_error("Expired SceneObject pointer");

    currentConv_ = scene_->convertObject(sp->uid);
}

void ColoredVertexIterator::advanceToNext()
{
    const auto& objs = scene_->getAllObjects();
    ++vertexIndex_;
    while (objIndex_ < objs.size() && vertexIndex_ >= currentConv_.vertices.size()) {
        ++objIndex_;
        vertexIndex_ = 0;
        if (objIndex_ < objs.size()) loadCurrentConversion();
    }
}

ColoredVertexIterator::value_type ColoredVertexIterator::operator*() const
{
    const auto& objs = scene_->getAllObjects();
    if (objIndex_ >= objs.size() || vertexIndex_ >= currentConv_.vertices.size())
        throw std::out_of_range("ColoredVertexIterator dereference out of range");

    ColoredVertex cv;
    cv.coords = currentConv_.vertices[vertexIndex_].second;
    cv.color  = colorificator_->getColorForObject(currentConv_.objectUid);
    return cv;
}

ColoredVertexIterator& ColoredVertexIterator::operator++() { advanceToNext(); return *this; }

bool ColoredVertexIterator::operator==(const ColoredVertexIterator& other) const
{
    return scene_ == other.scene_ &&
           objIndex_ == other.objIndex_ &&
           vertexIndex_ == other.vertexIndex_;
}
bool ColoredVertexIterator::operator!=(const ColoredVertexIterator& other) const { return !(*this == other); }

/* ===== ColoredEdgeIterator ============================================= */
ColoredEdgeIterator::ColoredEdgeIterator(const Scene* scene,
                                         const SceneColorificator* colorificator,
                                         std::size_t objIndex,
                                         std::size_t edgeIndex)
    : scene_(scene)
    , colorificator_(colorificator)
    , objIndex_(objIndex)
    , edgeIndex_(edgeIndex)
{
    if (scene_ && objIndex_ < scene_->getAllObjects().size()){
        loadCurrentConversion();
        if (currentConv_.edges.empty()) {
            advanceToNext();
        }
    }
}

void ColoredEdgeIterator::loadCurrentConversion()
{
    const auto& objs = scene_->getAllObjects();
    if (objIndex_ >= objs.size()) return;

    auto sp = objs[objIndex_].lock();
    if (!sp) throw std::runtime_error("Expired SceneObject pointer");

    currentConv_ = scene_->convertObject(sp->uid);
}

void ColoredEdgeIterator::advanceToNext()
{
    const auto& objs = scene_->getAllObjects();
    ++edgeIndex_;
    while (objIndex_ < objs.size() && edgeIndex_ >= currentConv_.edges.size()) {
        ++objIndex_;
        edgeIndex_ = 0;
        if (objIndex_ < objs.size()) loadCurrentConversion();
    }
}

ColoredEdgeIterator::value_type ColoredEdgeIterator::operator*() const
{
    const auto& objs = scene_->getAllObjects();
    if (objIndex_ >= objs.size() || edgeIndex_ >= currentConv_.edges.size())
        throw std::out_of_range("ColoredEdgeIterator dereference out of range");

    ColoredLine cl;
    auto id1 = currentConv_.edges[edgeIndex_].first;
    auto id2 = currentConv_.edges[edgeIndex_].second;

    for (const auto& v : currentConv_.vertices) {
        if (v.first == id1) cl.start = v.second;
        if (v.first == id2) cl.end   = v.second;
    }
    cl.color = colorificator_->getColorForObject(currentConv_.objectUid);
    return cl;
}

ColoredEdgeIterator& ColoredEdgeIterator::operator++() { advanceToNext(); return *this; }

bool ColoredEdgeIterator::operator==(const ColoredEdgeIterator& other) const
{
    return scene_ == other.scene_ &&
           objIndex_ == other.objIndex_ &&
           edgeIndex_ == other.edgeIndex_;
}
bool ColoredEdgeIterator::operator!=(const ColoredEdgeIterator& other) const { return !(*this == other); }

/* ===== SceneColorificator ============================================== */
void SceneColorificator::setColorForObject(const QUuid& uid, const QColor& color)
{
    colorMapping_[uid] = color;
}

void SceneColorificator::removeColorForObject(const QUuid& uid)
{
    auto it = colorMapping_.find(uid);
    if (it == colorMapping_.end())
        throw std::out_of_range("No color mapping for given uid");
    colorMapping_.erase(it);
}

QColor SceneColorificator::getColorForObject(const QUuid& uid) const
{
    auto it = colorMapping_.find(uid);
    return (it == colorMapping_.end()) ? QColor(SceneColorificator::defaultColor) : it->second;
}

SceneColorificator::VertexIterator SceneColorificator::beginVertices(const Scene& scene) const
{
    return { &scene, this, 0, 0 };
}
SceneColorificator::VertexIterator SceneColorificator::endVertices(const Scene& scene) const
{
    return { &scene, this, scene.getAllObjects().size(), 0 };
}

SceneColorificator::EdgeIterator SceneColorificator::beginEdges(const Scene& scene) const
{
    return { &scene, this, 0, 0 };
}
SceneColorificator::EdgeIterator SceneColorificator::endEdges(const Scene& scene) const
{
    return { &scene, this, scene.getAllObjects().size(), 0 };
}

QColor SceneColorificator::defaultColor = QColor(Qt::white);
