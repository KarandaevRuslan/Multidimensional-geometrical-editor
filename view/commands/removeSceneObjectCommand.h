#ifndef REMOVE_SCENE_OBJECT_COMMAND_H
#define REMOVE_SCENE_OBJECT_COMMAND_H

#include <QUndoCommand>
#include <QColor>
#include <memory>
#include "../dataModels/sceneObjectModel.h"

/**
 * @brief Command that deletes a SceneObject and supports undo.
 */
class RemoveSceneObjectCommand : public QUndoCommand
{
public:
    RemoveSceneObjectCommand(std::shared_ptr<SceneObjectModel> model,
                             int                 row,
                             std::function<void()> updateCallback,
                             QUndoCommand*       parent = nullptr);

    void undo() override;
    void redo() override;

private:
    std::shared_ptr<SceneObjectModel> model_;
    SceneObject   snapshot_;
    QColor        colorSnapshot_;
    std::function<void()> updateCallback_;

    QUuid         removedUid_;
};

#endif // REMOVE_SCENE_OBJECT_COMMAND_H
