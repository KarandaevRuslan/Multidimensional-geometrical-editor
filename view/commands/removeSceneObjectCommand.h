#ifndef REMOVE_SCENE_OBJECT_COMMAND_H
#define REMOVE_SCENE_OBJECT_COMMAND_H

#include <QUndoCommand>
#include <QColor>
#include <memory>
#include "../dataModels/sceneObjectModel.h"

/**
 * @brief Command class to remove a scene object.
 *
 * This class encapsulates the removal of a scene object from a SceneObjectModel. It stores a snapshot
 * of the object and its associated color before removal, making it possible to undo the removal.
 */
class RemoveSceneObjectCommand : public QUndoCommand
{
public:
    /**
     * @brief Constructs a RemoveSceneObjectCommand.
     *
     * @param model A shared pointer to the SceneObjectModel from which the object will be removed.
     * @param row The row index of the object to remove.
     * @param updateCallback A delegate to be called after undo/redo for UI updates.
     * @param parent Optional parent command to group commands.
     */
    RemoveSceneObjectCommand(std::shared_ptr<SceneObjectModel> model,
                             int row,
                             std::function<void()> updateCallback,
                             QUndoCommand* parent = nullptr);

    /**
     * @brief Undo the removal of the scene object.
     *
     * Restores the removed scene object into the model using the stored snapshot.
     */
    void undo() override;

    /**
     * @brief Redo the removal of the scene object.
     *
     * Removes the scene object from the model if the object snapshot is valid.
     */
    void redo() override;

private:
    std::shared_ptr<SceneObjectModel> model_;  ///< Pointer to the scene object model.
    int orignalRow_;                                  ///< The row index of the object to be removed.
    SceneObject objectSnapshot_;               ///< Snapshot of the scene object (captured before removal).
    QColor colorSnapshot_;                     ///< Snapshot of the associated color.
    bool valid_ = false;                       ///< Flag indicating if the snapshot is valid.

    std::function<void()> updateCallback_;
    int removedId_;
};

#endif // REMOVE_SCENE_OBJECT_COMMAND_H
