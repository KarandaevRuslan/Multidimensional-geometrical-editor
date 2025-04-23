#ifndef CHANGE_SCENE_OBJECT_COMMAND_H
#define CHANGE_SCENE_OBJECT_COMMAND_H
#include <QUndoCommand>
#include <QColor>
#include <memory>
#include <functional>
#include "../dataModels/sceneObjectModel.h"

/**
 * @brief Wraps a single logical edit of a SceneObject.
 *
 * The command stores a deep copy of the object _before_
 * and _after_ the change, plus the colour.  It re‑applies
 * those snapshots through SceneObjectModel::setData()
 * for all relevant roles.
 */
class ChangeSceneObjectCommand : public QUndoCommand
{
public:
    /**
     * @param model                     Model used to write the change.
     * @param row                       Row index of the edited object.
     * @param after                     Updated SceneObject snapshot.
     * @param colorAfter                Updated colour.
     * @param updateCallback            Callback (typically lambda that
     *                                  triggers a renderer refresh).
     * @param rebuildEditUiCallback     Callback (typically lambda that
     *                                  triggers a editSceneObjectWidget refresh).
     * @param parent                    Optional parent command.
     */
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
    static void setAllRoles(SceneObjectModel* m, int id,
                            const SceneObject& obj, const QColor& col);

    std::shared_ptr<SceneObjectModel> model_;
    int           originalRow_;
    SceneObject   before_;
    SceneObject   after_;
    QColor        colorBefore_;
    QColor        colorAfter_;
    std::function<void()> updateCallback_;
    std::function<void()> rebuildEditUiCallback_;
    int id_;
};
#endif // CHANGE_SCENE_OBJECT_COMMAND_H
