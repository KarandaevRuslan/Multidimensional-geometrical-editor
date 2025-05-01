#include "shapeCommand.h"

ShapeCommand::ShapeCommand(std::shared_ptr<NDShape> s,
             NDShape b, NDShape a, const QString& t,
             std::function<void()> reload)
    : shape_(std::move(s)), before_(std::move(b)),
    after_(std::move(a)), reload_(std::move(reload))
{
    setText(t);
}
void ShapeCommand::undo() {
    *shape_=before_;
    reload_();
}
void ShapeCommand::redo() {
    *shape_=after_;
    reload_();
}

