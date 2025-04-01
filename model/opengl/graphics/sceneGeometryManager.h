#ifndef SCENE_GEOMETRY_MANAGER_H
#define SCENE_GEOMETRY_MANAGER_H

#include <QOpenGLFunctions_3_3_Core>
#include <memory>
#include <QVector3D>
#include "../../scene.h"
#include "../../sceneColorificator.h"

/**
 * @brief Manages geometry buffers (VAOs/VBOs) for axes, ticks, points, and lines,
 *        updating them based on data from Scene and SceneColorificator.
 */
class SceneGeometryManager : protected QOpenGLFunctions_3_3_Core
{
public:
    /**
     * @brief VertexData holds position and color for a single vertex.
     */
    struct VertexData {
        QVector3D position; ///< 3D position of the vertex
        QVector3D color;    ///< RGB color of the vertex
    };

    SceneGeometryManager();
    ~SceneGeometryManager();

    /**
     * @brief Must be called once when a valid OpenGL context is current.
     *        Generates VAOs and VBOs for axes, ticks, points, lines, and arrow cones.
     */
    void initialize();

    /**
     * @brief Sets the scene from which geometry data is extracted.
     * @param scene Weak pointer to the Scene.
     */
    void setScene(std::weak_ptr<Scene> scene);

    /**
     * @brief Sets the colorificator that provides color data for scene points/edges.
     * @param colorificator Weak pointer to a SceneColorificator.
     */
    void setSceneColorificator(std::weak_ptr<SceneColorificator> colorificator);

    /**
     * @brief Updates all geometry data (axes, ticks, points, lines, arrow cones)
     *        based on current scene and colorificator content.
     */
    void updateGeometry();

    /**
     * @brief Renders all geometry (axes, ticks, points, lines, arrow cones).
     *        Note that we are NOT setting the MVP uniform here anymore.
     */
    void renderAll();

private:
    /**
     * @brief Utility function to build or update a VAO/VBO pair.
     */
    void createOrUpdateBuffer(GLuint &vao, GLuint &vbo,
                              const VertexData *data,
                              size_t dataSize,
                              GLsizei &vertexCount);

    // Sub-update functions
    void updateAxesData();
    void updateTicksData();
    void updatePointsData();
    void updateLinesData();

private:
    // Scene references
    std::weak_ptr<Scene> scene_;
    std::weak_ptr<SceneColorificator> colorificator_;

    // OpenGL buffer data for various primitives
    GLuint vaoAxes_, vboAxes_;
    GLsizei axesVertexCount_;

    GLuint vaoTicks_, vboTicks_;
    GLsizei ticksVertexCount_;

    GLuint vaoPoints_, vboPoints_;
    GLsizei pointsVertexCount_;

    GLuint vaoLines_, vboLines_;
    GLsizei linesVertexCount_;

    // OpenGL buffers for the arrow cone geometry
    GLuint vaoArrowCone_, vboArrowCone_;
    GLsizei arrowConeVertexCount_;

    // Constants for drawing
    static constexpr float kLineWidthThin  = 2.0f;   ///< Used for axes & ticks lines
    static constexpr float kLineWidthThick = 4.0f;   ///< Used for scene lines
    static constexpr float kPointSize      = 10.0f;  ///< Used for scene points
    static constexpr int   kTickRange      = 40;     ///< Ticks from -kTickRange..kTickRange
    static constexpr float kTickOffset     = 0.1f;   ///< Half-length of a tick line
    static constexpr float kAxisEnd        = 100.0f; ///< Half-length of each axis
    static constexpr float kArrowSize      = 2.0f;   ///< Length of axis arrow (cone height)

    // Constants for cone geometry
    static constexpr float kConeRadius   = kArrowSize * 0.3f;
    static constexpr int   kConeSegments = 20;
};

#endif // SCENE_GEOMETRY_MANAGER_H
