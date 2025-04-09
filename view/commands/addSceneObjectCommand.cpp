#include "addSceneObjectCommand.h"
#include <QUndoCommand>
#include <QColor>
#include <memory>
#include "../dataModels/sceneObjectModel.h"

AddSceneObjectCommand::AddSceneObjectCommand(std::shared_ptr<SceneObjectModel> model,
                                             SceneObject object,
                                             QColor color,
                                             std::function<void()> updateCallback,
                                             QUndoCommand* parent)
    : QUndoCommand(parent),
    model_(std::move(model)),
    object_(std::move(object)),
    color_(std::move(color)),
    updateCallback_(std::move(updateCallback))
{
    setText(QString("Add object '%1'").arg(object_.name));
}

AddSceneObjectCommand::AddSceneObjectCommand(std::shared_ptr<SceneObjectModel> model,
                                             SceneObject object,
                                             std::function<void()> updateCallback,
                                             QUndoCommand* parent)
    : AddSceneObjectCommand(model, object, SceneColorificator::defaultColor, std::move(updateCallback), parent)
{}

void AddSceneObjectCommand::undo()
{
    // We only know which row it was appended at if we already did redo() once.
    if (insertedRow_ >= 0) {
        model_->removeSceneObject(insertedRow_);
        if (updateCallback_) {
            updateCallback_();
        }
    }
}

void AddSceneObjectCommand::redo()
{
    // If this is our first time running redo() (the command was just pushed),
    // we figure out the row at which the model will place the new object.
    if (insertedRow_ < 0) {
        insertedRow_ = model_->rowCount();
    }

    model_->addSceneObject(object_, color_);
    if (updateCallback_) {
        updateCallback_();
    }
}
