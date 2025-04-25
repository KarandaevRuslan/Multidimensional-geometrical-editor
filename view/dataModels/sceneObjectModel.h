#ifndef SCENE_OBJECT_MODEL_H
#define SCENE_OBJECT_MODEL_H

#include <QAbstractListModel>
#include <memory>
#include <vector>
#include <QUuid>
#include "../../model/Scene.h"
#include "../../model/sceneColorificator.h"

/**
 * @brief List-model that exposes SceneObject items for QML / Qt Views.
 *
 *  *Internal key* — `uid` (`QUuid`), immutable and unique.
 *  Visual integer `id` remaining only for visual, but is not used
 *  for searching, deleting, changing.
 */
class SceneObjectModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum SceneRoles {
        UidRole         = Qt::UserRole + 1,   ///< immutable QUuid
        VisualIdRole,                         ///< visual-only integer id (read-only)
        NameRole,
        ShapeRole,
        ProjectionRole,
        RotatorsRole,
        ScaleRole,
        OffsetRole,
        ColorRole
    };

    SceneObjectModel(std::weak_ptr<Scene>               scene,
                     std::weak_ptr<SceneColorificator>  colorificator,
                     QObject* parent = nullptr);

    int               rowCount(const QModelIndex& parent = {}) const override;
    QVariant          data     (const QModelIndex& index, int role) const override;
    bool              setData  (const QModelIndex& index, const QVariant& value, int role) override;
    QHash<int,QByteArray> roleNames() const override;
    Qt::ItemFlags     flags    (const QModelIndex& index) const override;

    void refresh();                                         ///< repopulate list
    void addSceneObject   (const SceneObject& obj, const QColor& color);
    void addSceneObject   (const SceneObject& obj);         ///< default color
    void removeSceneObject(int row);

    std::shared_ptr<SceneObject> getObjectByRow(int row) const;

    void setScene             (std::weak_ptr<Scene> scene);
    void setSceneColorificator(std::weak_ptr<SceneColorificator> colorificator);

    /// return row index for given uid or -1
    int rowForUid(const QUuid& uid) const;

    Q_INVOKABLE void debugPrintAll() const;

private:
    std::weak_ptr<Scene>              scene_;
    std::weak_ptr<SceneColorificator> colorificator_;
    std::vector<QUuid>                object_uids_;   ///< primary keys
};

#endif // SCENE_OBJECT_MODEL_H
