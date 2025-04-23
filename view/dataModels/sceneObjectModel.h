#ifndef SCENE_OBJECT_MODEL_H
#define SCENE_OBJECT_MODEL_H

#include <QAbstractListModel>
#include <memory>
#include <vector>
#include "../../model/Scene.h"
#include "../../model/sceneColorificator.h"

/**
 * @brief Model representing a collection of scene objects.
 *
 * SceneObjectModel maintains a list of scene objects within a scene and serves as the data source
 * for views using the Model-View-Controller (MVC) paradigm. It supports operations for adding,
 * removing, updating, and refreshing the scene objects.
 */
class SceneObjectModel : public QAbstractListModel {
    Q_OBJECT

public:
    /**
     * @brief Custom roles for scene object properties.
     */
    enum SceneRoles {
        IdRole = Qt::UserRole + 1,  ///< Unique identifier for the scene object.
        NameRole,                   ///< Name of the scene object.
        ShapeRole,                  ///< Shape (geometry) information.
        ProjectionRole,             ///< Projection properties.
        RotatorsRole,               ///< Rotation parameters.
        ScaleRole,                  ///< Scale values.
        OffsetRole,                 ///< Positional offset values.
        ColorRole                   ///< Color used for display.
    };

    /**
     * @brief Constructs a SceneObjectModel.
     *
     * @param scene A weak pointer to the Scene containing the objects.
     * @param colorificator A weak pointer to the SceneColorificator for managing object colors.
     * @param parent The parent QObject (default is nullptr).
     */
    SceneObjectModel(std::weak_ptr<Scene> scene,
                     std::weak_ptr<SceneColorificator> colorificator,
                     QObject* parent = nullptr);

    /**
     * @brief Returns the number of scene objects in the model.
     *
     * @param parent The parent index (default is an invalid QModelIndex).
     * @return Number of rows corresponding to scene objects.
     */
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    /**
     * @brief Retrieves data for a given index and role.
     *
     * @param index The index of the scene object.
     * @param role The role that defines the data to return.
     * @return A QVariant containing the requested data.
     */
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    /**
     * @brief Sets the data for a given index and role.
     *
     * @param index The index of the scene object.
     * @param value The new value to set.
     * @param role The role for which to set the data.
     * @return True if the data is successfully set; otherwise, false.
     */
    bool setData(const QModelIndex& index, const QVariant& value, int role) override;

    /**
     * @brief Returns a mapping of custom role names.
     *
     * @return A QHash mapping role numbers to their respective names.
     */
    QHash<int, QByteArray> roleNames() const override;

    /**
     * @brief Returns the item flags for the specified index.
     *
     * @param index The index of the item.
     * @return Flags indicating properties like editable, selectable, etc.
     */
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    /**
     * @brief Refreshes the list of scene objects.
     *
     * Clears the current object identifiers and repopulates the list from the Scene.
     */
    void refresh();

    /**
     * @brief Adds a new scene object to the model.
     *
     * @param obj The SceneObject to add.
     * @param color The display color for the object.
     */
    void addSceneObject(const SceneObject& obj, const QColor& color);

    void addSceneObject(const SceneObject& obj);

    /**
     * @brief Removes a scene object from the model.
     *
     * @param row The row index of the object to remove.
     */
    void removeSceneObject(int row);

    /**
     * @brief Retrieves the scene object corresponding to the given row.
     *
     * @param row The row index.
     * @return A shared pointer to the SceneObject, or nullptr if the index is invalid.
     */
    std::shared_ptr<SceneObject> getObjectByRow(int row) const;

    /**
     * @brief Sets the Scene instance for the model.
     * @param scene A weak pointer to the new Scene.
     */
    void setScene(std::weak_ptr<Scene> scene);

    /**
     * @brief Sets the SceneColorificator instance for the model.
     * @param colorificator A weak pointer to the new SceneColorificator.
     */
    void setSceneColorificator(std::weak_ptr<SceneColorificator> colorificator);

    /// return the row index for the given object id, or -1 if not found
    int rowForId(int objectId) const;

    Q_INVOKABLE void debugPrintAll() const;
private:
    std::weak_ptr<Scene> scene_;                           ///< Weak pointer to the Scene.
    std::weak_ptr<SceneColorificator> colorificator_;        ///< Weak pointer to the color manager.
    std::vector<int> object_ids_;                          ///< List of scene object identifiers.
};

#endif // SCENE_OBJECT_MODEL_H
