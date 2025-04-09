#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <memory>
#include <QString>
#include "NDShape.h"
#include "Projection.h"
#include "Rotator.h"

/**
 * @brief Structure representing a scene object.
 *
 * Each scene object includes:
 *  - A mandatory unique integer identifier.
 *  - A name of object.
 *  - A shared pointer to an NDShape representing the object's geometry.
 *  - An optional Projection strategy (if dimension reduction is needed).
 *  - A list of Rotator objects applied sequentially.
 *  - A scale vector (in scene dimension) applied after projection.
 *  - An offset vector (in scene dimension) applied after scaling.
 */
struct SceneObject {
    int id;
    QString name;
    std::shared_ptr<NDShape> shape;
    std::shared_ptr<Projection> projection;
    std::vector<Rotator> rotators;
    std::vector<double> scale;
    std::vector<double> offset;
};

/**
 * @brief Structure representing converted data.
 *
 * Conversion extracts:
 *  - vertices: a list of pairs where the first element is the vertex ID and the second is the vector of coordinates.
 *  - edges: a list of pairs of vertex IDs representing the shape's edges.
 */
struct ConvertedData {
    int objectId;
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
     * @param id Unique identifier for the object.
     * @param A name of an object.
     * @param shape A shared pointer to the NDShape associated with the object.
     *              The Scene will share ownership of this shape with the caller.
     * @param projection An optional shared pointer to the Projection strategy.
     * @param rotators A vector of Rotator transformations.
     * @param scale The scale vector to be applied after projection.
     * @param offset The offset vector to be applied after scaling.
     *
     * @throws std::invalid_argument If an object with the given ID already exists or if the scale/offset dimensions do not match the scene dimension.
     */
    void addObject(int id, QString name,
                   std::shared_ptr<NDShape> shape,
                   std::shared_ptr<Projection> projection,
                   const std::vector<Rotator>& rotators,
                   const std::vector<double>& scale,
                   const std::vector<double>& offset);

    /**
     * @brief Removes the scene object with the specified ID.
     *
     * @param id The unique identifier of the object to remove.
     * @throws std::out_of_range If no object with the given ID exists.
     */
    void removeObject(int id);

    /**
     * @brief Retrieves the scene object with the given ID.
     *
     * @param id The unique identifier of the desired object.
     * @return A weak pointer to the SceneObject.
     * @throws std::out_of_range If no object with the given ID exists.
     */
    std::weak_ptr<SceneObject> getObject(int id) const;

    /**
     * @brief Updates the scene object with the specified ID.
     *
     * Replaces the object's NDShape, Projection strategy, list of Rotator objects, scale, and offset.
     *
     * @param id The unique identifier of the object to update.
     * @param A name of an object.
     * @param shape A shared pointer to the new NDShape. Ownership is shared with the Scene.
     * @param projection A new (optional) shared pointer to a Projection strategy.
     * @param rotators The new list of Rotator transformations.
     * @param scale The new scale vector to be applied after projection.
     * @param offset The new offset vector to be applied after scaling.
     *
     * @throws std::out_of_range If no object with the given ID exists.
     */
    void setObject(int id, QString name,
                   std::shared_ptr<NDShape> shape,
                   std::shared_ptr<Projection> projection,
                   const std::vector<Rotator>& rotators,
                   const std::vector<double>& scale,
                   const std::vector<double>& offset);

    /**
     * @brief Retrieves a list of all scene objects.
     *
     * @return A vector containing weak pointers to all SceneObject instances.
     */
    std::vector<std::weak_ptr<SceneObject>> getAllObjects() const;

    /**
     * @brief Converts the NDShape for the scene object identified by the given id.
     *
     * This method:
     * 1. Looks up the object by its id.
     * 2. Applies each stored rotation transformation in sequence.
     * 3. If a projection strategy is provided and the resulting shape's dimension is greater than
     *    the scene target dimension, applies projectShapeToDimension.
     * 4. Applies the scale transformation.
     * 5. Applies the offset transformation.
     * 6. Returns the resulting ConvertedData.
     *
     * @param id The mandatory scene object identifier.
     * @return The ConvertedData for that object.
     * @throws std::out_of_range If no object with the given id exists.
     */
    ConvertedData convertObject(int id) const;

    /**
     * @brief Converts all stored NDShapes.
     *
     * For each scene object, conversion is performed as in convertObject(id).
     *
     * @return A vector of ConvertedData for each scene object.
     */
    std::vector<ConvertedData> convertAllObjects() const;

    /**
     * @brief Sets the target dimension for projection.
     * @param dim The desired dimension (must be >= 1).
     * @throws std::invalid_argument If dim is less than 1.
     */
    void setSceneDimension(std::size_t dim);

    /**
     * @brief Retrieves the current target scene dimension.
     * @return The current scene dimension.
     */
    std::size_t getSceneDimension() const;

private:
    std::vector<std::shared_ptr<SceneObject>> objects_;
    std::size_t sceneDimension_ = 3;
};

#endif // SCENE_H
