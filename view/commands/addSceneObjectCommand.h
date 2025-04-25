#ifndef ADD_SCENEOBJECT_COMMAND_H
#define ADD_SCENEOBJECT_COMMAND_H

#include <QUndoCommand>
#include <QColor>
#include <memory>
#include "../../model/sceneColorificator.h"
#include "../dataModels/sceneObjectModel.h"

/**
 * @brief Command that appends a SceneObject to SceneObjectModel.
 *
 * Works with QUuid (uid) as the primary key.
 */
class AddSceneObjectCommand : public QUndoCommand
{
public:
    AddSceneObjectCommand(std::shared_ptr<SceneObjectModel> model,
                          SceneObject        object,
                          QColor             color,
                          std::function<void()> updateCallback,
                          QUndoCommand*      parent = nullptr);

    AddSceneObjectCommand(std::shared_ptr<SceneObjectModel> model,
                          SceneObject        object,
                          std::function<void()> updateCallback,
                          QUndoCommand*      parent = nullptr);

    void undo() override;
    void redo() override;

private:
    std::shared_ptr<SceneObjectModel> model_;
    SceneObject      object_;
    QColor           color_;
    QUuid            insertedUid_{};

    std::function<void()> updateCallback_;
};

#endif // ADD_SCENEOBJECT_COMMAND_H
