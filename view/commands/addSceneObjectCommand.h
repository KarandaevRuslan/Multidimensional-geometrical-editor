#ifndef ADD_SCENEOBJECT_COMMAND_H
#define ADD_SCENEOBJECT_COMMAND_H

#include <QUndoCommand>
#include <QColor>
#include <memory>
#include "../../model/sceneColorificator.h"
#include "../dataModels/sceneObjectModel.h"

/**
 * @brief Command class to add a scene object.
 *
 * This class encapsulates the operation of adding a scene object to a SceneObjectModel.
 * It supports undo and redo operations by recording the state necessary to add and later remove
 * the object.
 */
class AddSceneObjectCommand : public QUndoCommand
{
public:
    /**
     * @brief Constructs an AddSceneObjectCommand.
     *
     * @param model A shared pointer to the SceneObjectModel where the object will be added.
     * @param object The SceneObject to be added.
     * @param color The color to associate with the object.
     * @param updateCallback A delegate to be called after undo/redo for UI updates.
     * @param parent Optional parent command to allow for command grouping.
     */
    AddSceneObjectCommand(std::shared_ptr<SceneObjectModel> model,
                          SceneObject object,
                          QColor color,
                          std::function<void()> updateCallback,
                          QUndoCommand* parent = nullptr);

    AddSceneObjectCommand(std::shared_ptr<SceneObjectModel> model,
                          SceneObject object,
                          std::function<void()> updateCallback,
                          QUndoCommand* parent = nullptr);


    /**
     * @brief Undo the addition of the scene object.
     *
     * Removes the previously added scene object from the model if it has been added.
     */
    void undo() override;

    /**
     * @brief Redo the addition of the scene object.
     *
     * Adds the scene object to the model. On the first call, determines the row where the
     * object will be inserted.
     */
    void redo() override;

private:
    std::shared_ptr<SceneObjectModel> model_; ///< Pointer to the scene object model.
    SceneObject object_;                        ///< The scene object to add.
    QColor color_;                              ///< The color assigned to the object.
    int insertedRow_ = -1;                      ///< The row index at which the object is inserted.
    int insertedId_ = -1;

    std::function<void()> updateCallback_;
};

#endif // ADD_SCENEOBJECT_COMMAND_H
