#ifndef SCENE_COLORIFICATOR_H
#define SCENE_COLORIFICATOR_H

#include <unordered_map>
#include <QString>
#include <QColor>
#include <QDebug>
#include <QUuid>
#include "scene.h"

/* ---------- helpers ------------------------------------------------------ */
struct UidHash {
    std::size_t operator()(const QUuid& uid) const noexcept { return qHash(uid); }
};
/* ------------------------------------------------------------------------- */

/* ---------- coloured primitives ----------------------------------------- */
struct ColoredVertex {
    std::vector<double> coords;
    QColor              color;
};

struct ColoredLine {
    std::vector<double> start;
    std::vector<double> end;
    QColor              color;
};
/* ------------------------------------------------------------------------- */

/* ---------- forward declarations ---------------------------------------- */
class SceneColorificator;

class ColoredVertexIterator {
public:
    using value_type        = ColoredVertex;
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;

    ColoredVertexIterator(const Scene*                       scene,
                          const SceneColorificator*          colorificator,
                          std::size_t                        objIndex,
                          std::size_t                        vertexIndex);

    value_type                 operator*()  const;
    ColoredVertexIterator&     operator++();
    bool                       operator==(const ColoredVertexIterator& other) const;
    bool                       operator!=(const ColoredVertexIterator& other) const;

private:
    const Scene*                scene_;
    const SceneColorificator*   colorificator_;
    std::size_t                 objIndex_;
    std::size_t                 vertexIndex_;
    ConvertedData               currentConv_;

    void loadCurrentConversion();
    void advanceToNext();
};

class ColoredEdgeIterator {
public:
    using value_type        = ColoredLine;
    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;

    ColoredEdgeIterator(const Scene*                      scene,
                        const SceneColorificator*         colorificator,
                        std::size_t                       objIndex,
                        std::size_t                       edgeIndex);

    value_type              operator*()  const;
    ColoredEdgeIterator&    operator++();
    bool                    operator==(const ColoredEdgeIterator& other) const;
    bool                    operator!=(const ColoredEdgeIterator& other) const;

private:
    const Scene*               scene_;
    const SceneColorificator*  colorificator_;
    std::size_t                objIndex_;
    std::size_t                edgeIndex_;
    ConvertedData              currentConv_;

    void loadCurrentConversion();
    void advanceToNext();
};
/* ------------------------------------------------------------------------- */

/* ---------- SceneColorificator ------------------------------------------ */
class SceneColorificator {
public:
    SceneColorificator()  = default;
    ~SceneColorificator() = default;

    void   setColorForObject(const QUuid& uid, const QColor& color);
    void   removeColorForObject(const QUuid& uid);
    QColor getColorForObject(const QUuid& uid) const;

    using VertexIterator = ColoredVertexIterator;
    using EdgeIterator   = ColoredEdgeIterator;

    VertexIterator beginVertices(const Scene& scene) const;
    VertexIterator endVertices  (const Scene& scene) const;

    EdgeIterator   beginEdges   (const Scene& scene) const;
    EdgeIterator   endEdges     (const Scene& scene) const;

    static QColor  defaultColor;

private:
    std::unordered_map<QUuid, QColor, UidHash> colorMapping_;
};
/* ------------------------------------------------------------------------- */

#endif // SCENE_COLORIFICATOR_H
