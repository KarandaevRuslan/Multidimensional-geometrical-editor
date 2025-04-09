#ifndef SCENE_RENDERER_WIDGET_H
#define SCENE_RENDERER_WIDGET_H

#include <QWidget>
#include <memory>
#include "../model/scene.h"
#include "../model/sceneColorificator.h"

class CameraController;
class SceneInputHandler;
class SceneRenderer;

/**
 * @brief A QWidget wrapper that embeds a SceneRenderer (QOpenGLWindow)
 *        in a normal layout. Also sets focus/mouse policy on the container
 *        and toggles free-look mode via the same lambda you had before.
 */
class SceneRendererWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SceneRendererWidget(QWidget* parent = nullptr);
    ~SceneRendererWidget();

    void setScene(std::weak_ptr<Scene> scene);
    void setSceneColorificator(std::weak_ptr<SceneColorificator> colorificator);

    /**
     * @brief Update shadows and geometry
     */
    void updateAll();

private:
    SceneRenderer* glWindow_ = nullptr;  ///< The actual QOpenGLWindow
    QWidget*       container_ = nullptr; ///< The QWidget container around glWindow_
};

#endif // SCENE_RENDERER_WIDGET_H
