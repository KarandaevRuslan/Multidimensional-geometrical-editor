#include "addSceneObjectCommand.h"
#include <QUndoCommand>
#include <QColor>
#include <memory>
#include "../dataModels/sceneObjectModel.h"

AddSceneObjectCommand::AddSceneObjectCommand(std::shared_ptr<SceneObjectModel> model,
                                             SceneObject object,
                                             QColor color,
                                             QUndoCommand* parent)
    : QUndoCommand(parent),
    model_(std::move(model)),
    object_(std::move(object)),
    color_(std::move(color))
{
    setText(QString("Add object '%1'").arg(object_.name));
}

AddSceneObjectCommand::AddSceneObjectCommand(std::shared_ptr<SceneObjectModel> model,
                                             SceneObject object,
                                             QUndoCommand* parent)
    : AddSceneObjectCommand(model, object, SceneColorificator::defaultColor, parent)
{

}

void AddSceneObjectCommand::undo()
{
    // We only know which row it was appended at if we already did redo() once.
    if (insertedRow_ >= 0) {
        model_->removeSceneObject(insertedRow_);
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
}
