#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <memory>
#include <QString>
#include <QUuid>
#include "NDShape.h"
#include "projection.h"
#include "rotator.h"

/**
 * @brief Structure representing a scene object.
 *
 * Each scene object includes:
 *  - A mandatory unique QUuid identifier (uid).
 *  - A visual-only integer identifier (id).
 *  - A name of object.
 *  - A shared pointer to an NDShape representing the object's geometry.
 *  - An optional Projection strategy (if dimension reduction is needed).
 *  - A list of Rotator objects applied sequentially.
 *  - A scale vector (in scene dimension) applied after projection.
 *  - An offset vector (in scene dimension) applied after scaling.
 */
struct SceneObject {
    QUuid                     uid;
    int                       id;
    QString                   name;
    std::shared_ptr<NDShape>  shape;
    std::shared_ptr<Projection> projection;
    std::vector<Rotator>      rotators;
    std::vector<double>       scale;
    std::vector<double>       offset;

    /// Deep copy (keeps the same uid and id).
    SceneObject clone();
};

/**
 * @brief Structure representing converted data.
 *
 * Conversion extracts:
 *  - vertices: a list of pairs where the first element is the vertex ID and the second is the vector of coordinates.
 *  - edges: a list of pairs of vertex IDs representing the shape's edges.
 */
struct ConvertedData {
    QUuid objectUid;
    std::vector<std::pair<std::size_t, std::vector<double>>> vertices;
    std::vector<std::pair<std::size_t, std::size_t>> edges;
};

/**
 * @brief The Scene class manages a collection of scene objects.
 *
 * It supports adding, removing, updating, and converting objects.
 * The conversion applies stored rotations, projection, scaling, and offset transformations.
 *
 * The target dimension (default 3) can be set and retrieved.
 */
class Scene {
public:
    Scene() = default;
    ~Scene();

    /**
     * @brief Adds a new scene object to the collection.
     *
     * @return QUuid of the added object.
     */
    QUuid addObject(QUuid uid, int id, QString name,
                    std::shared_ptr<NDShape>      shape,
                    std::shared_ptr<Projection>   projection,
                    const std::vector<Rotator>&   rotators,
                    const std::vector<double>&    scale,
                    const std::vector<double>&    offset);

    /// Removes the scene object with the specified uid.
    void removeObject(const QUuid& uid);

    /// Retrieves the scene object with the given uid.
    std::weak_ptr<SceneObject> getObject(const QUuid& uid) const;

    /// Updates the scene object with the specified uid.
    void setObject(const QUuid&               uid,
                   QString                    name,
                   std::shared_ptr<NDShape>   shape,
                   std::shared_ptr<Projection> projection,
                   const std::vector<Rotator>& rotators,
                   const std::vector<double>&  scale,
                   const std::vector<double>&  offset);

    /// Retrieves a list of all scene objects.
    std::vector<std::weak_ptr<SceneObject>> getAllObjects() const;

    /// Returns the number of objects currently in the scene.
    std::size_t objectCount() const;

    /// Performs full conversion on the given object.
    static ConvertedData convertObject(const SceneObject& obj, int sceneDimension);

    /// Converts the NDShape for the scene object identified by the given uid.
    ConvertedData convertObject(const QUuid& uid) const;

    /// Converts all stored NDShapes.
    std::vector<ConvertedData> convertAllObjects() const;

    void            setSceneDimension(std::size_t dim);
    std::size_t     getSceneDimension() const;

private:
    std::vector<std::shared_ptr<SceneObject>> objects_;
    std::size_t                               sceneDimension_ = 3;
};

#endif // SCENE_H
