#include "removeSceneObjectCommand.h"
#include <QDebug>

RemoveSceneObjectCommand::RemoveSceneObjectCommand(std::shared_ptr<SceneObjectModel> model,
                                                   int                  row,
                                                   std::function<void()> updateCb,
                                                   QUndoCommand*        parent)
    : QUndoCommand(parent)
    , model_(std::move(model))
    , updateCallback_(std::move(updateCb))
{
    if (auto obj = model_->getObjectByRow(row)) {
        snapshot_   = obj->clone();
        removedUid_ = snapshot_.uid;

        QVariant clrVar = model_->data(model_->index(row,0), SceneObjectModel::ColorRole);
        colorSnapshot_  = clrVar.value<QColor>();

        setText(QString("Remove object '%1'").arg(snapshot_.name));
    } else {
        setText("Remove object (invalid index)");
    }
}

void RemoveSceneObjectCommand::redo()
{
    int r = model_->rowForUid(removedUid_);
    if (r >= 0) model_->removeSceneObject(r);

    if (updateCallback_) updateCallback_();

    qDebug() << "Redo remove";
    model_->debugPrintAll();
}

void RemoveSceneObjectCommand::undo()
{
    SceneObject objClone = snapshot_.clone();
    objClone.uid = removedUid_;
    model_->addSceneObject(objClone, colorSnapshot_);

    if (updateCallback_) updateCallback_();

    qDebug() << "Undo remove";
    model_->debugPrintAll();
}
