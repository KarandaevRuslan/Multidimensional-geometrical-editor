#include "addSceneObjectCommand.h"
#include <QDebug>

AddSceneObjectCommand::AddSceneObjectCommand(std::shared_ptr<SceneObjectModel> model,
                                             SceneObject       object, // should be deep copied before passing here
                                             QColor            color,
                                             std::function<void()> updateCb,
                                             QUndoCommand*     parent)
    : QUndoCommand(parent)
    , model_(std::move(model))
    , object_(std::move(object))
    , color_(std::move(color))
    , updateCallback_(std::move(updateCb))
{
    setText(QString("Add object '%1'").arg(object_.name));
}

AddSceneObjectCommand::AddSceneObjectCommand(std::shared_ptr<SceneObjectModel> model,
                                             SceneObject        object,
                                             std::function<void()> updateCb,
                                             QUndoCommand*      parent)
    : AddSceneObjectCommand(std::move(model), std::move(object),
                            SceneColorificator::defaultColor,
                            std::move(updateCb), parent)
{}

void AddSceneObjectCommand::redo()
{
    if (insertedUid_.isNull()){
        insertedUid_ = QUuid::createUuid(); // Create as new object
    }
    SceneObject objClone = object_.clone();
    objClone.uid = insertedUid_;
    model_->addSceneObject(objClone, color_);

    if (updateCallback_) updateCallback_();

    qDebug() << "Redo add";
    model_->debugPrintAll();
}

void AddSceneObjectCommand::undo()
{
    if (!insertedUid_.isNull()) {
        int r = model_->rowForUid(insertedUid_);
        if (r >= 0) {
            model_->removeSceneObject(r);
            if (updateCallback_) updateCallback_();
        }
    }

    qDebug() << "Undo add";
    model_->debugPrintAll();
}
