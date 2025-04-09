#ifndef SCENE_OBJECT_DELEGATE_H
#define SCENE_OBJECT_DELEGATE_H

#include <QStyledItemDelegate>
#include <QColor>

/**
 * @brief Delegate for custom rendering of scene objects.
 *
 * SceneObjectDelegate handles custom painting of scene objects in views that use the SceneObjectModel.
 * It also provides item size hints for consistent layout.
 */
class SceneObjectDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    /**
     * @brief Constructs a SceneObjectDelegate.
     *
     * @param parent The parent QObject (default is nullptr).
     */
    SceneObjectDelegate(QObject* parent = nullptr);

    /**
     * @brief Renders a scene object item.
     *
     * Draws the scene object's color (as a circle), its selection background, and its name.
     *
     * @param painter The QPainter used for drawing.
     * @param option Style options for the item.
     * @param index The index of the item in the model.
     */
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    /**
     * @brief Provides a size hint for a scene object item.
     *
     * @param option Style options for the item.
     * @param index The index of the item in the model.
     * @return The recommended size for the item.
     */
    QSize sizeHint(const QStyleOptionViewItem& option,
                   const QModelIndex& index) const override;

private:
    QColor outlineColor_ = Qt::black; ///< Outline color for the drawn scene object.
};

#endif // SCENE_OBJECT_DELEGATE_H
