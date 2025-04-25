#include "changeSceneObjectCommand.h"
#include <QDebug>

/* static */
void ChangeSceneObjectCommand::setAllRoles(SceneObjectModel* m,
                                                        const QUuid&      uid,
                                                        const SceneObject& obj,
                                                        const QColor&     col)
{
    int row = m->rowForUid(uid);
    if (row < 0) return;

    const QModelIndex idx = m->index(row, 0);
    m->setData(idx, obj.name,                             SceneObjectModel::NameRole);
    m->setData(idx, QVariant::fromValue(obj.shape),       SceneObjectModel::ShapeRole);
    m->setData(idx, QVariant::fromValue(obj.projection),  SceneObjectModel::ProjectionRole);
    m->setData(idx, QVariant::fromValue(obj.rotators),    SceneObjectModel::RotatorsRole);
    m->setData(idx, QVariant::fromValue(obj.scale),       SceneObjectModel::ScaleRole);
    m->setData(idx, QVariant::fromValue(obj.offset),      SceneObjectModel::OffsetRole);
    m->setData(idx, col,                                  SceneObjectModel::ColorRole);
}

ChangeSceneObjectCommand::ChangeSceneObjectCommand(
    std::shared_ptr<SceneObjectModel>  model,
    int                                row,
    const SceneObject&                 after,
    const QColor&                      colorAfter,
    std::function<void()>              updateCb,
    std::function<void()>              rebuildUiCb,
    QUndoCommand*                      parent)
    : QUndoCommand(QObject::tr("Edit object"), parent)
    , model_(std::move(model))
    , originalRow_(row)
    , after_(after)
    , colorAfter_(colorAfter)
    , updateCallback_(std::move(updateCb))
    , rebuildEditUiCallback_(std::move(rebuildUiCb))
{
    if (auto beforeObj = model_->getObjectByRow(row)) {
        before_ = beforeObj->clone();
        uid_    = before_.uid;

        QModelIndex idx = model_->index(row, 0);
        colorBefore_ = model_->data(idx, SceneObjectModel::ColorRole).value<QColor>();
    }
}

void ChangeSceneObjectCommand::redo()
{
    setAllRoles(model_.get(), uid_, after_.clone(), colorAfter_);
    updateCallback_();
    rebuildEditUiCallback_();
    qDebug() << "Redo change";  model_->debugPrintAll();
}

void ChangeSceneObjectCommand::undo()
{
    setAllRoles(model_.get(), uid_, before_.clone(), colorBefore_);
    updateCallback_();
    rebuildEditUiCallback_();
    qDebug() << "Undo change";  model_->debugPrintAll();
}
