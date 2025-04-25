#ifndef CHANGE_SCENE_OBJECT_COMMAND_H
#define CHANGE_SCENE_OBJECT_COMMAND_H

#include <QUndoCommand>
#include <QColor>
#include <memory>
#include <functional>
#include "../dataModels/sceneObjectModel.h"

/**
 * @brief Encapsulates one logical edit of a SceneObject.
 */
class ChangeSceneObjectCommand : public QUndoCommand
{
public:
    ChangeSceneObjectCommand(std::shared_ptr<SceneObjectModel>  model,
                             int                                row,
                             const SceneObject&                 after,
                             const QColor&                      colorAfter,
                             std::function<void()>              updateCallback,
                             std::function<void()>              rebuildEditUiCallback,
                             QUndoCommand*                      parent = nullptr);

    void redo() override;
    void undo() override;

private:
    static void setAllRoles(SceneObjectModel*          m,
                            const QUuid&               uid,
                            const SceneObject&         obj,
                            const QColor&              col);

    std::shared_ptr<SceneObjectModel> model_;
    int            originalRow_;
    SceneObject    before_;
    SceneObject    after_;
    QColor         colorBefore_;
    QColor         colorAfter_;
    std::function<void()> updateCallback_;
    std::function<void()> rebuildEditUiCallback_;
    QUuid          uid_;
};

#endif // CHANGE_SCENE_OBJECT_COMMAND_H
