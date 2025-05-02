#ifndef ROTATOR_TABLE_MODEL_H
#define ROTATOR_TABLE_MODEL_H

#include <QAbstractTableModel>
#include <QList>
#include "../../model/rotator.h"
#include "../../model/scene.h"

/**
 * @brief Simple editable table‑model that stores a QList<Rotator>.
 *
 * Column 0 – first axis (uint)
 * Column 1 – second axis (uint)
 * Column 2 – angle (double)
 */
class RotatorTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit RotatorTableModel(std::function<void(SceneObject)> commit, QObject *parent = nullptr);

    // ---- model interface ----
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    // ---- helpers ----
    const std::vector<Rotator> *rotators() const
    {
        if (auto scene = sceneObject_.lock())
            if (scene && !scene->rotators.empty())
                return &scene->rotators;

        return nullptr;
    }

    Rotator* getRotator(int index) {
        if (auto scene = sceneObject_.lock()) {
            if (index >= 0 && index < static_cast<int>(scene->rotators.size())) {
                return &scene->rotators[index];
            }
        }
        return nullptr;
    }

    void reload();
    void setSceneObject(std::weak_ptr<SceneObject> sceneObject) {
        sceneObject_ = std::move(sceneObject);
    }

private:
    std::weak_ptr<SceneObject> sceneObject_{};
    std::function<void(SceneObject)> commit_;
};

#endif // ROTATOR_TABLE_MODEL_H
