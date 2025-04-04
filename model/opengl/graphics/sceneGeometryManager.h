#ifndef SCENE_GEOMETRY_MANAGER_H
#define SCENE_GEOMETRY_MANAGER_H

#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <memory>
#include <QVector3D>
#include <QPen>
#include <QFont>
#include "../../scene.h"
#include "../../sceneColorificator.h"
#include "../other/axisSystem.h"

/**
 * @class SceneGeometryManager
 * @brief Manages creation and rendering of scene geometry.
 */
class SceneGeometryManager : protected QOpenGLFunctions_3_3_Core
{
public:
    /**
     * @struct VertexData
     * @brief Holds position, normal, and color for a single vertex.
     */
    struct VertexData {
        QVector3D position; ///< Vertex position
        QVector3D normal;   ///< Vertex normal
        QVector3D color;    ///< Vertex color (RGB)
    };

    /**
     * @brief Constructor
     */
    SceneGeometryManager();

    /**
     * @brief Destructor
     */
    ~SceneGeometryManager();

    /**
     * @brief Initializes necessary OpenGL state and buffers.
     */
    void initialize();

    /**
     * @brief Sets the scene to manage.
     * @param scene A weak pointer to the scene.
     */
    void setScene(std::weak_ptr<Scene> scene);

    /**
     * @brief Sets the colorificator used to color geometry.
     * @param colorificator A weak pointer to the colorificator.
     */
    void setSceneColorificator(std::weak_ptr<SceneColorificator> colorificator);

    /**
     * @brief Updates all geometry buffers.
     * @param onlySceneObjects Only updates geometry of scene objects.
     */
    void updateGeometry(bool onlySceneObjects);

    /**
     * @brief Renders all geometry. Expects the shader to be bound.
     * @param program The shader program.
     */
    void renderAll(QOpenGLShaderProgram* program);

    /**
     * @brief Draws text overlay labels on top of the 3D scene.
     */
    void paintOverlayLabels(QOpenGLWidget *widget,
                            const QMatrix4x4 &mvp) const;

    /**
     * @brief Updates axis lengths and generates tick marks based on camera position.
     */
    void updateAxes(const QVector3D& cameraPos);

private:
    // Buffer creation helper
    /**
     * @brief Uploads data to a VAO/VBO; configures vertex attributes.
     */
    void createOrUpdateBuffer(GLuint &vao,
                              GLuint &vbo,
                              const VertexData* data,
                              size_t dataSize,
                              GLsizei &vertexCount);

    // Geometry update helpers
    void updateAxesData();
    void updateTicksData();
    void updatePointsData();
    void updateLinesData();

    // Overlay methods

    /**
     * @brief Projects a 3D point to 2D screen coordinates.
     */
    QPointF projectToScreen(const QOpenGLWidget *widget,
                            const QVector3D &point,
                            const QMatrix4x4 &mvp) const;

    // Internal geometry-building methods

    /**
     * @brief Builds a UV-sphere mesh of radius `radius` with the given rings & sectors.
     */
    std::vector<VertexData> buildSphere(float radius,
                                        int rings,
                                        int sectors,
                                        const QVector3D& center,
                                        const QVector3D& color);

    /**
     * @brief Builds a closed cylinder from `start` to `end` with radius `radius`.
     */
    std::vector<VertexData> buildCylinderWithCaps(const QVector3D& start,
                                                  const QVector3D& end,
                                                  float radius,
                                                  int segments,
                                                  const QVector3D& color);

    /**
     * @brief Builds a cone with a circular base.
     */
    std::vector<VertexData> buildConeWithBase(const QVector3D& tip,
                                              const QVector3D& baseCenter,
                                              float baseRadius,
                                              int segments,
                                              const QVector3D& color);

private:
    std::weak_ptr<Scene> scene_;
    std::weak_ptr<SceneColorificator> colorificator_;

    QList<Axis> axes_;

    // VAOs / VBOs
    GLuint vaoAxes_ = 0,   vboAxes_ = 0;
    GLuint vaoTicks_ = 0,  vboTicks_ = 0;
    GLuint vaoPoints_ = 0, vboPoints_ = 0;
    GLuint vaoLines_ = 0,  vboLines_ = 0;
    GLuint vaoArrowCone_ = 0, vboArrowCone_ = 0;

    // Vertex counts
    GLsizei axesVertexCount_ = 0;
    GLsizei ticksVertexCount_ = 0;
    GLsizei pointsVertexCount_ = 0;
    GLsizei linesVertexCount_ = 0;
    GLsizei arrowConeVertexCount_ = 0;

    // Configurable geometry parameters
    float lineWidthThin_ = 2.0f;
    float tickOffset_ = 0.1f;
    QVector3D ticksColor_ = {1.0f, 1.0f, 1.0f};
    float arrowSize_ = 2.0f;

    float coneRadius_;
    int   coneSegments_ = 20;

    float sphereRadius_ = 0.15f;
    int   sphereRings_ = 15;
    int   sphereSectors_ = 15;

    float tubeRadius_ = 0.055f;
    int   tubeSegments_ = 18;

    int tickBoxFactor_ = 50;
    QVector3D origin_ = {0.0f, 0.0f, 0.0f};

    // --- Overlay ---
    float kOffScreenCoord_ = -9999.0f;
    int   kOverlayFontSize_ = 10;

    // --- Overlay style ---
    QPen  overlayNumberPen_ = QPen(Qt::black);
    QFont overlayNumberFont_ = QFont("Arial", kOverlayFontSize_);
    QFont overlayAxisNameFont_ = QFont("Arial", kOverlayFontSize_ * 1.5);
};

#endif // SCENE_GEOMETRY_MANAGER_H
