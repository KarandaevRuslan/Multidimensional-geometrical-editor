#ifndef SCENERENDERER_H
#define SCENERENDERER_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QMatrix4x4>
#include <QTimer>
#include <memory>

#include "../model/scene.h"
#include "../model/sceneColorificator.h"

class QOpenGLShaderProgram;
class CameraController;
class SceneGeometryManager;
class SceneInputHandler;

/**
 * @brief High-level widget class that orchestrates rendering, camera, geometry,
 *        and user input for a 3D scene. Single-pass version (no extra shadow pass).
 */
class SceneRenderer : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT
public:
    explicit SceneRenderer(QWidget* parent = nullptr);
    ~SceneRenderer();

    void setScene(std::weak_ptr<Scene> scene);
    void setSceneColorificator(std::weak_ptr<SceneColorificator> colorificator);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private slots:
    void updateCamera();

private:
    QMatrix4x4 buildMvpMatrix() const;

    // Overlays
    void paintOverlayLabels(const QMatrix4x4 &mvp);
    QPointF projectToScreen(const QVector3D &point, const QMatrix4x4 &mvp) const;

    // Sub-components
    std::unique_ptr<QOpenGLShaderProgram> program_;
    std::unique_ptr<CameraController> cameraController_;
    std::unique_ptr<SceneGeometryManager> geometryManager_;
    std::unique_ptr<SceneInputHandler> inputHandler_;

    // Timer for camera movement
    QTimer movementTimer_;
    QPoint centerScreenPos_;

    // Uniform location
    GLint mvpMatrixLoc_ = -1;

    // Constants
    static constexpr int   kCameraUpdateIntervalMs = 16;   // ~60fps
    static constexpr float kDefaultFovY            = 45.0f;
    static constexpr float kDefaultNearPlane       = 0.1f;
    static constexpr float kDefaultFarPlane        = 1000.0f;

    static constexpr float kClearColorR = 0.55f;
    static constexpr float kClearColorG = 0.55f;
    static constexpr float kClearColorB = 0.55f;
    static constexpr float kClearColorA = 1.0f;

    static constexpr float kOffScreenCoord = -9999.0f;
    static constexpr int   kAxisLabelRange = 20;
    static constexpr int   kOverlayFontSize = 10;
};

#endif // SCENERENDERER_H
