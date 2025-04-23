#include "changeSceneObjectCommand.h"

/* static */
void ChangeSceneObjectCommand::setAllRoles(SceneObjectModel* m, int id,
                                           const SceneObject& obj, const QColor& col)
{
    int row = m->rowForId(id);
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
    std::function<void()>              updateCallback,
    std::function<void()>              rebuildEditUiCallback,
    QUndoCommand*                      parent)
    : QUndoCommand(QObject::tr("Edit object"), parent),
    model_(std::move(model)), originalRow_(row),
    after_(after), colorAfter_(colorAfter),
    updateCallback_(std::move(updateCallback)),
    rebuildEditUiCallback_(std::move(rebuildEditUiCallback))
{
    auto beforeObj = model_->getObjectByRow(row);
    if (beforeObj) {
        before_ = beforeObj->clone();
        id_     = before_.id;

        QModelIndex idx = model_->index(row, 0);
        colorBefore_ = model_->data(idx, SceneObjectModel::ColorRole).value<QColor>();
    }
}

void ChangeSceneObjectCommand::redo() {
    qDebug() << "Redo change ";
    model_->debugPrintAll();

    setAllRoles(model_.get(), id_, after_.clone(), colorAfter_);
    updateCallback_();
    rebuildEditUiCallback_();
}

void ChangeSceneObjectCommand::undo() {
    qDebug() << "Undo change ";
    model_->debugPrintAll();

    setAllRoles(model_.get(), id_, before_.clone(), colorBefore_);
    updateCallback_();
    rebuildEditUiCallback_();
}
