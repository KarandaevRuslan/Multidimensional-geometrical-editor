#include "removeSceneObjectCommand.h"
#include "../../model/scene.h"

RemoveSceneObjectCommand::RemoveSceneObjectCommand(std::shared_ptr<SceneObjectModel> model,
                                                   int row,
                                                   std::function<void()> updateCallback,
                                                   QUndoCommand* parent)
    : QUndoCommand(parent),
    model_(std::move(model)),
    orignalRow_(row),
    updateCallback_(std::move(updateCallback))
{
    auto objPtr = model_->getObjectByRow(orignalRow_);
    if (objPtr) {
        objectSnapshot_ = objPtr->clone();
        removedId_      = objectSnapshot_.id;
        QVariant colorVar = model_->data(model_->index(orignalRow_, 0), SceneObjectModel::ColorRole);
        colorSnapshot_ = colorVar.isValid() ? colorVar.value<QColor>() : QColor(SceneColorificator::defaultColor);
        valid_ = true;
        setText(QString("Remove object '%1'").arg(objectSnapshot_.name));
    } else {
        setText("Remove object (invalid index)");
    }
}

void RemoveSceneObjectCommand::redo()
{
    qDebug() << "Redo remove ";
    model_->debugPrintAll();

    int r = model_->rowForId(removedId_);
    if (valid_) {
        model_->removeSceneObject(r);
        if (updateCallback_) {
            updateCallback_();
        }
    }
}

void RemoveSceneObjectCommand::undo()
{
    qDebug() << "Undo remove ";
    model_->debugPrintAll();

    // If the snapshot is valid, we can add it back
    if (valid_) {
        model_->addSceneObject(objectSnapshot_.clone(), colorSnapshot_);
        if (updateCallback_) {
            updateCallback_();
        }
    }
}
