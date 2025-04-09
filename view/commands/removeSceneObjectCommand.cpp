#include "removeSceneObjectCommand.h"

RemoveSceneObjectCommand::RemoveSceneObjectCommand(std::shared_ptr<SceneObjectModel> model,
                                                   int row,
                                                   std::function<void()> updateCallback,
                                                   QUndoCommand* parent)
    : QUndoCommand(parent),
    model_(std::move(model)),
    row_(row),
    updateCallback_(std::move(updateCallback))
{
    auto objPtr = model_->getObjectByRow(row_);
    if (objPtr) {
        objectSnapshot_ = *objPtr;
        QVariant colorVar = model_->data(model_->index(row_, 0), SceneObjectModel::ColorRole);
        colorSnapshot_ = colorVar.isValid() ? colorVar.value<QColor>() : QColor(SceneColorificator::defaultColor);
        valid_ = true;
        setText(QString("Remove object '%1'").arg(objectSnapshot_.name));
    } else {
        setText("Remove object (invalid index)");
    }
}

void RemoveSceneObjectCommand::undo()
{
    // If the snapshot is valid, we can add it back
    if (valid_) {
        model_->addSceneObject(objectSnapshot_, colorSnapshot_);
        if (updateCallback_) {
            updateCallback_();
        }
    }
}

void RemoveSceneObjectCommand::redo()
{
    if (valid_) {
        model_->removeSceneObject(row_);
        if (updateCallback_) {
            updateCallback_();
        }
    }
}
