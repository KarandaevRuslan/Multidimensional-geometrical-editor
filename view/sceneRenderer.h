#ifndef SCENERENDERER_H
#define SCENERENDERER_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QMatrix4x4>
#include <QTimer>
#include <memory>
#include <QPen>

#include "../model/scene.h"
#include "../model/sceneColorificator.h"

class QOpenGLShaderProgram;
class CameraController;
class SceneGeometryManager;
class SceneInputHandler;

/**
 * @brief High-level widget class that orchestrates rendering, camera, geometry,
 *        and user input for a 3D scene with basic shadow mapping.
 */
class SceneRenderer : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT
public:
    /**
     * @brief Constructs the scene renderer.
     * @param parent Parent widget.
     */
    explicit SceneRenderer(QWidget* parent = nullptr);

    /**
     * @brief Destructor for cleaning up OpenGL resources.
     */
    ~SceneRenderer();

    /**
     * @brief Assigns the scene that will be rendered.
     * @param scene Weak pointer to a Scene instance.
     */
    void setScene(std::weak_ptr<Scene> scene);

    /**
     * @brief Assigns a colorificator that can modify the scene’s colors.
     * @param colorificator Weak pointer to a SceneColorificator instance.
     */
    void setSceneColorificator(std::weak_ptr<SceneColorificator> colorificator);

protected:
    /**
     * @brief Initializes the OpenGL state, programs, and GPU resources.
     */
    void initializeGL() override;

    /**
     * @brief Called when the widget is resized.
     * @param w New width in pixels.
     * @param h New height in pixels.
     */
    void resizeGL(int w, int h) override;

    /**
     * @brief Main per-frame rendering entry point.
     */
    void paintGL() override;

    /**
     * @brief Handles keyboard press events.
     */
    void keyPressEvent(QKeyEvent* event) override;

    /**
     * @brief Handles keyboard release events.
     */
    void keyReleaseEvent(QKeyEvent* event) override;

    /**
     * @brief Handles mouse movement events.
     */
    void mouseMoveEvent(QMouseEvent* event) override;

    /**
     * @brief Handles mouse double-click events.
     */
    void mouseDoubleClickEvent(QMouseEvent* event) override;

    /**
     * @brief Handles mouse wheel (scroll) events.
     */
    void wheelEvent(QWheelEvent* event) override;

private slots:
    /**
     * @brief Updates camera movement each frame (called by a timer).
     */
    void updateCamera();

private:
    /**
     * @brief Sets up the primary rendering shader program.
     */
    void setupMainProgram();

    /**
     * @brief Sets up the depth-only (shadow map) shader program.
     */
    void setupDepthProgram();

    /**
     * @brief Creates the shadow-map FBO and configures its texture attachment.
     */
    void initShadowFBO();

    /**
     * @brief Renders the scene from the light’s perspective into the shadow map.
     */
    void renderShadowPass();

    /**
     * @brief Renders the scene from the camera’s perspective using the shadow map.
     */
    void renderScenePass();

    /**
     * @brief Builds a light-space matrix that transforms world coordinates
     *        into the light’s orthographic space.
     */
    QMatrix4x4 buildLightSpaceMatrix() const;

    /**
     * @brief Builds a standard MVP matrix from the camera’s perspective.
     */
    QMatrix4x4 buildMvpMatrix() const;

    /**
     * @brief Projects a 3D point to 2D screen coordinates.
     */
    QPointF projectToScreen(const QVector3D &point, const QMatrix4x4 &mvp) const;

    /**
     * @brief Draws text overlay labels (e.g. axis ticks) on top of the 3D scene.
     */
    void paintOverlayLabels(const QMatrix4x4 &mvp);

private:
    // --- Shader programs ---
    std::unique_ptr<QOpenGLShaderProgram> program_;
    std::unique_ptr<QOpenGLShaderProgram> depthProgram_;

    // --- Camera, geometry, input helpers ---
    std::unique_ptr<CameraController> cameraController_;
    std::unique_ptr<SceneGeometryManager> geometryManager_;
    std::unique_ptr<SceneInputHandler> inputHandler_;

    // --- Shadow map resources ---
    GLuint depthMapFbo_ = 0;
    GLuint depthMapTex_ = 0;

    // --- Uniforms (main program) ---
    GLint mvpMatrixLoc_        = -1;
    GLint lightSpaceMatrixLoc_ = -1;
    GLint shadowMapLoc_        = -1;

    // --- Uniforms (depth program) ---
    GLint depthMvpLoc_         = -1;

    // --- Configurable parameters ---
    int   shadowMapSize_          = 16384;        ///< Resolution of the shadow map.
    float shadowOrthographicSize_ = 100.0f;       ///< Half-width for orthographic projection.
    QVector3D shadowLightPos_     = { 30.0f, 25.0f, 35.0f };
    QVector3D shadowLightTarget_  = { 0.0f, 0.0f,  0.0f };
    QVector3D shadowLightUpDir_   = { 0.0f, 1.0f,  0.0f };

    // --- Overlay style ---
    QPen  overlayPen_              = QPen(Qt::black);
    QFont overlayFont_             = QFont("Arial", kOverlayFontSize);

    // --- Lighting parameters ---
    float     shininess_           = 32.0f;
    float     ambientStrength_     = 0.2f;
    float     specularStrength_    = 0.5f;
    float     directionalStrength_ = 1.0f;
    float     shadowLightStrength_ = 1.0f;
    float     colorBlendFactor_    = 0.5f;

    QVector3D ambientColor_        = { 1.0f, 1.0f, 1.0f };
    QVector3D lightColor_          = { 1.0f, 1.0f, 1.0f };
    QVector3D shadowLightColor_    = { 1.0f, 1.0f, 1.0f };

    // --- Shadow sampling/PCF parameters ---
    int       pcfKernelDim_        = 3;
    float     shadowBiasScale_     = 0.001f;
    float     shadowBiasMin_       = 0.0008f;

    // --- Timer for camera movement ---
    QTimer movementTimer_;
    QPoint centerScreenPos_;

    // --- Camera configuration constants ---
    static constexpr int   kCameraUpdateIntervalMs = 16;   // ~60fps
    static constexpr float kDefaultFovY            = 45.0f;
    static constexpr float kDefaultNearPlane       = 0.1f;
    static constexpr float kDefaultFarPlane        = 1000.0f;

    // --- Clear color constants ---
    static constexpr float kClearColorR = 0.55f;
    static constexpr float kClearColorG = 0.55f;
    static constexpr float kClearColorB = 0.55f;
    static constexpr float kClearColorA = 1.0f;

    // --- Overlay constants ---
    static constexpr float kOffScreenCoord = -9999.0f;
    static constexpr int   kAxisLabelRange = 20;
    static constexpr int   kOverlayFontSize = 10;
};

#endif // SCENERENDERER_H
