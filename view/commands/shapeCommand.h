#ifndef SHAPECOMMAND_H
#define SHAPECOMMAND_H

#include <QUndoCommand>
#include "../../model/NDShape.h"


class ShapeCommand final : public QUndoCommand {
public:
    ShapeCommand(std::shared_ptr<NDShape> s,
                 NDShape b, NDShape a, const QString& t,
                 std::function<void()> reload);
    void undo() override;
    void redo() override;
private:
    std::shared_ptr<NDShape> shape_;
    NDShape before_, after_; std::function<void()> reload_;
};

#endif // SHAPECOMMAND_H
