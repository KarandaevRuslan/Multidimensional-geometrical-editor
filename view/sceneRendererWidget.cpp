#include "sceneRendererWidget.h"
#include "sceneRenderer.h"
#include "../model/opengl/input/sceneInputHandler.h"

#include <QVBoxLayout>
#include <QWindow>

SceneRendererWidget::SceneRendererWidget(QWidget* parent)
    : QWidget(parent)
{
    // Create the QOpenGLWindow
    glWindow_ = new SceneRenderer;

    // Wrap it in a QWidget container
    container_ = QWidget::createWindowContainer(glWindow_, this);

    // container_->setMinimumSize(640, 480);

    // Put container into layout
    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(container_);
    setLayout(layout);
}

SceneRendererWidget::~SceneRendererWidget()
{
}

void SceneRendererWidget::setScene(std::weak_ptr<Scene> scene)
{
    if (glWindow_) {
        glWindow_->setScene(scene);
    }
}

void SceneRendererWidget::setSceneColorificator(std::weak_ptr<SceneColorificator> colorificator)
{
    if (glWindow_) {
        glWindow_->setSceneColorificator(colorificator);
    }
}

void SceneRendererWidget::updateAll()
{
    if (glWindow_) {
        glWindow_->updateAll();
    }
}

std::shared_ptr<SceneInputHandler> SceneRendererWidget::inputHandler() const {
    return glWindow_->inputHandler();
}
std::shared_ptr<CameraController> SceneRendererWidget::cameraController() const {
    return glWindow_->cameraController();
}
