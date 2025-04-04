#ifndef SCENE_COLORIFICATOR_H
#define SCENE_COLORIFICATOR_H

#include <unordered_map>
#include <QString>
#include <QColor>
#include <QDebug>
#include "scene.h"

/**
 * @brief Structure representing a colored vertex.
 *
 * A ColoredVertex consists of the vertex coordinates and the associated color.
 */
struct ColoredVertex {
    std::vector<double> coords;
    QColor color;
};

/**
 * @brief Structure representing a colored line.
 *
 * A ColoredLine consists of a start coordinate, an end coordinate, and the associated color.
 */
struct ColoredLine {
    std::vector<double> start;
    std::vector<double> end;
    QColor color;
};

/**
 * @brief Iterator for colored vertices in a Scene.
 *
 * This iterator provides a forward iterator interface over all colored vertices in a Scene.
 * It yields one ColoredVertex at a time, converting each SceneObject's geometry on demand.
 */
class ColoredVertexIterator {
public:
    /// Iterator type definitions.
    using value_type = ColoredVertex;
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type;

    /**
     * @brief Constructs a ColoredVertexIterator.
     * @param scene Pointer to the Scene to iterate over.
     * @param colorificator Pointer to the SceneColorificator for color lookup.
     * @param objIndex Index of the current SceneObject within the Scene.
     * @param vertexIndex Index of the current vertex within the converted vertices of the object.
     */
    ColoredVertexIterator(const Scene* scene,
                          const class SceneColorificator* colorificator,
                          std::size_t objIndex,
                          std::size_t vertexIndex);

    /**
     * @brief Dereference operator.
     * @return The current ColoredVertex.
     * @throws std::out_of_range If the iterator is out of range.
     */
    value_type operator*() const;

    /**
     * @brief Pre-increment operator.
     * @return Reference to the incremented iterator.
     */
    ColoredVertexIterator& operator++();

    /**
     * @brief Equality comparison operator.
     * @param other The iterator to compare with.
     * @return True if both iterators are equal.
     */
    bool operator==(const ColoredVertexIterator& other) const;

    /**
     * @brief Inequality comparison operator.
     * @param other The iterator to compare with.
     * @return True if the iterators are not equal.
     */
    bool operator!=(const ColoredVertexIterator& other) const;

private:
    const Scene* scene_;
    const class SceneColorificator* colorificator_;
    std::size_t objIndex_;
    std::size_t vertexIndex_;
    ConvertedData currentConv_;

    /**
     * @brief Loads the conversion data for the current SceneObject.
     *
     * Uses Scene::convertObject() with the current object's mandatory ID.
     */
    void loadCurrentConversion();

    /**
     * @brief Advances the iterator to the next valid vertex.
     *
     * If the end of the current object's vertex list is reached, moves to the next SceneObject.
     */
    void advanceToNext();
};

/**
 * @brief Iterator for colored edges in a Scene.
 *
 * This iterator provides a forward iterator interface over all colored edges in a Scene.
 * It yields one ColoredLine at a time by converting each SceneObject's geometry on demand.
 */
class ColoredEdgeIterator {
public:
    /// Iterator type definitions.
    using value_type = ColoredLine;
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using pointer = value_type*;
    using reference = value_type;

    /**
     * @brief Constructs a ColoredEdgeIterator.
     * @param scene Pointer to the Scene to iterate over.
     * @param colorificator Pointer to the SceneColorificator for color lookup.
     * @param objIndex Index of the current SceneObject within the Scene.
     * @param edgeIndex Index of the current edge within the converted edges of the object.
     */
    ColoredEdgeIterator(const Scene* scene,
                        const class SceneColorificator* colorificator,
                        std::size_t objIndex,
                        std::size_t edgeIndex);

    /**
     * @brief Dereference operator.
     * @return The current ColoredLine.
     * @throws std::out_of_range If the iterator is out of range.
     */
    value_type operator*() const;

    /**
     * @brief Pre-increment operator.
     * @return Reference to the incremented iterator.
     */
    ColoredEdgeIterator& operator++();

    /**
     * @brief Equality comparison operator.
     * @param other The iterator to compare with.
     * @return True if both iterators are equal.
     */
    bool operator==(const ColoredEdgeIterator& other) const;

    /**
     * @brief Inequality comparison operator.
     * @param other The iterator to compare with.
     * @return True if the iterators are not equal.
     */
    bool operator!=(const ColoredEdgeIterator& other) const;

private:
    const Scene* scene_;
    const class SceneColorificator* colorificator_;
    std::size_t objIndex_;
    std::size_t edgeIndex_;
    ConvertedData currentConv_;

    /**
     * @brief Loads the conversion data for the current SceneObject.
     *
     * Uses Scene::convertObject() with the current object's mandatory ID.
     */
    void loadCurrentConversion();

    /**
     * @brief Advances the iterator to the next valid edge.
     *
     * If the end of the current object's edge list is reached, moves to the next SceneObject.
     */
    void advanceToNext();
};

/**
 * @brief The SceneColorificator class associates scene object IDs with colors.
 *
 * It maintains a dictionary mapping each scene object's mandatory ID to a QColor.
 * Additionally, it provides iterator interfaces for traversing colored vertices and colored edges
 * in a Scene without requiring the whole geometry to be built at once.
 */
class SceneColorificator {
public:
    /**
     * @brief Default constructor.
     */
    SceneColorificator() = default;

    /**
     * @brief Default destructor.
     */
    ~SceneColorificator() = default;

    /**
     * @brief Adds or updates the color mapping for a given scene object ID.
     * @param objectId The scene object ID.
     * @param color The QColor to associate with the object.
     */
    void setColorForObject(int objectId, const QColor& color);

    /**
     * @brief Removes the color mapping for a given scene object ID.
     * @param objectId The scene object ID.
     * @throws std::out_of_range If no mapping exists for the given object ID.
     */
    void removeColorForObject(int objectId);

    /**
     * @brief Retrieves the color associated with a given scene object ID.
     *        If the ID is not found, a default color (white) is returned.
     * @param objectId The scene object ID.
     * @return The associated QColor, or white if no mapping is found.
     */
    QColor getColorForObject(int objectId) const;

    /// Type alias for the colored vertex iterator.
    using VertexIterator = ColoredVertexIterator;

    /// Type alias for the colored edge iterator.
    using EdgeIterator   = ColoredEdgeIterator;

    /**
     * @brief Returns an iterator to the first colored vertex in the specified Scene.
     * @param scene The Scene from which to extract colored vertices.
     * @return A VertexIterator positioned at the first colored vertex.
     */
    VertexIterator beginVertices(const Scene& scene) const;

    /**
     * @brief Returns an iterator representing the end of colored vertices in the specified Scene.
     * @param scene The Scene from which colored vertices are extracted.
     * @return A VertexIterator representing the end of the sequence.
     */
    VertexIterator endVertices(const Scene& scene) const;

    /**
     * @brief Returns an iterator to the first colored edge in the specified Scene.
     * @param scene The Scene from which to extract colored edges.
     * @return An EdgeIterator positioned at the first colored edge.
     */
    EdgeIterator beginEdges(const Scene& scene) const;

    /**
     * @brief Returns an iterator representing the end of colored edges in the specified Scene.
     * @param scene The Scene from which colored edges are extracted.
     * @return An EdgeIterator representing the end of the sequence.
     */
    EdgeIterator endEdges(const Scene& scene) const;

private:
    std::unordered_map<int, QColor> colorMapping_;
};

#endif // SCENE_COLORIFICATOR_H
