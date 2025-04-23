#ifndef SCENE_OBJECT_EDITOR_WIDGET_H
#define SCENE_OBJECT_EDITOR_WIDGET_H

#include <QWidget>
#include <QColor>
#include <memory>

#include "../model/scene.h"
#include "../model/sceneColorificator.h"

#include "axesGroupBox.h"

class QLabel;
class QLineEdit;
class QPushButton;
class QTableWidget;
class QDoubleSpinBox;
class QComboBox;

/**
 * @brief Right‑hand panel inside MainWindowTabWidget.
 *
 * The widget never modifies Scene/Model directly; it emits
 * objectEdited(updatedObj, newColor, geometryChanged)
 * every time a user‑driven edit is completed.
 */
class SceneObjectEditorWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SceneObjectEditorWidget(QWidget *parent=nullptr);

    /**
     * @brief Display the given object in the editor.
     *
     * @param obj              Shared pointer (may be null).
     * @param colorGetter      Getter delegate for current color for that object.
     */
    void setObject(std::shared_ptr<SceneObject>        obj,
                   std::function<QColor()>             colorGetter);

    /// Clears UI and disables the panel.
    void clear();

    void rebuildUiFromCurrent();

signals:
    /**
     * @brief Emitted after *every* logical edit.
     *
     * @param updated           Complete SceneObject snapshot.
     * @param color             Updated colour.
     * @param geometryChanged   True when shape / projection /
     *                          rotators / scale / offset / colour
     *                          has changed (-> needs repaint).
     */
    void objectEdited(const SceneObject& updated,
                      const QColor&      color,
                      bool               geometryChanged);

private slots:
    void chooseColor();
    void projectionChanged();
    void nameEditingFinished();
    void rotCellChanged(int,int);
    void scaleChanged();
    void offsetChanged();
    void openShapeDialog();

    void addRotator();
    void copyRotator();
    void cutRotator();
    void pasteRotator();
    void deleteRotator();
    void moveRotatorUp();
    void moveRotatorDown();

    void showRotContextMenu(const QPoint &pos);

private:
    /* helpers */
    void commit(bool geometryChanged,
                SceneObject& obj, QColor& color);

    std::shared_ptr<SceneObject> cur_;
    std::function<QColor()>      curColorGetter_;

    /* ui widgets */
    QLabel*         idLabel_;
    QLineEdit*      nameEdit_;
    QPushButton*    colorBtn_;
    QComboBox*      projCombo_;
    QDoubleSpinBox* perspDist_;
    QPushButton*    shapeBtn_;
    AxesGroupBox*   scaleBox_;
    AxesGroupBox*   offsetBox_;

    QTableWidget*   rotTable_;
    QPushButton* addRotBtn_;
    QPushButton* copyRotBtn_;
    QPushButton* cutRotBtn_;
    QPushButton* pasteRotBtn_;
    QPushButton* deleteRotBtn_;
    QPushButton* upRotBtn_;
    QPushButton* downRotBtn_;
    // internal clipboard for cut/copy
    QList<Rotator> rotClipboard_;
    QLabel *distanceLabel_;

    QMap<QString, QAction*> rotatorActions_;
    QGroupBox *rotGroupBox_;
};

#endif // SCENE_OBJECT_EDITOR_WIDGET_H
