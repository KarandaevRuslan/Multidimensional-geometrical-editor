#ifndef VERTEX_TABLE_MODEL_H
#define VERTEX_TABLE_MODEL_H

#include <QAbstractTableModel>
#include <memory>
#include "../../model/NDShape.h"

class QUndoStack;

/*! Table-model for NDShape vertices (row == vertex). */
class VertexTableModel final : public QAbstractTableModel
{
    Q_OBJECT
public:
    VertexTableModel(std::shared_ptr<NDShape> shape,
                     QUndoStack* undoStack,
                     std::shared_ptr<std::vector<std::size_t>> rowToId,
                     QObject* parent = nullptr);

    /* Qt interface */
    int            rowCount   (const QModelIndex& = {}) const override;
    int            columnCount(const QModelIndex& = {}) const override;
    QVariant       headerData (int, Qt::Orientation, int role) const override;
    QVariant       data       (const QModelIndex&, int role) const override;
    Qt::ItemFlags  flags      (const QModelIndex&) const override;
    bool           setData    (const QModelIndex&, const QVariant&, int role) override;

    /* stable mapping helpers */
    void        reload();                   //!< call after structural changes

private:
    std::shared_ptr<NDShape> shape_;
    QUndoStack*              undo_;         // not owned
    std::shared_ptr<std::vector<std::size_t>> rowToId_;
};

#endif // VERTEX_TABLE_MODEL_H
