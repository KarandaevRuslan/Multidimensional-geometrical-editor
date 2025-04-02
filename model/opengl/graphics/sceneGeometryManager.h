#ifndef SCENE_GEOMETRY_MANAGER_H
#define SCENE_GEOMETRY_MANAGER_H

#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <memory>
#include <QVector3D>
#include "../../scene.h"
#include "../../sceneColorificator.h"

class SceneGeometryManager : protected QOpenGLFunctions_3_3_Core
{
public:
    /**
     * @brief VertexData now holds position, normal, and color.
     */
    struct VertexData {
        QVector3D position; ///< 3D position of the vertex
        QVector3D normal;   ///< Normal (for lighting); can be zero if not used
        QVector3D color;    ///< RGB color
    };

    // A helper function to build a UV-sphere mesh of radius `r` and `rings` × `sectors`
    static std::vector<VertexData> buildSphere(
        float radius,
        int rings,
        int sectors,
        const QVector3D& center,
        const QVector3D& color)
    {
        std::vector<VertexData> vertices;
        vertices.reserve(rings * sectors * 6);
        // each "quad" on the sphere => 2 triangles => 6 vertices

        for (int r = 0; r < rings; ++r) {
            float theta1 = float(M_PI) * (float(r) / rings);
            float theta2 = float(M_PI) * (float(r + 1) / rings);

            for (int s = 0; s < sectors; ++s) {
                float phi1 = 2.0f * float(M_PI) * (float(s) / sectors);
                float phi2 = 2.0f * float(M_PI) * (float(s + 1) / sectors);

                // 4 points on sphere “quad” in spherical coordinates:
                QVector3D p1(
                    std::sin(theta1) * std::cos(phi1),
                    std::cos(theta1),
                    std::sin(theta1) * std::sin(phi1));
                QVector3D p2(
                    std::sin(theta1) * std::cos(phi2),
                    std::cos(theta1),
                    std::sin(theta1) * std::sin(phi2));
                QVector3D p3(
                    std::sin(theta2) * std::cos(phi1),
                    std::cos(theta2),
                    std::sin(theta2) * std::sin(phi1));
                QVector3D p4(
                    std::sin(theta2) * std::cos(phi2),
                    std::cos(theta2),
                    std::sin(theta2) * std::sin(phi2));

                // Multiply by radius and offset by center
                p1 = p1 * radius + center;
                p2 = p2 * radius + center;
                p3 = p3 * radius + center;
                p4 = p4 * radius + center;

                // Calculate normals as (position - center).normalized()
                auto n1 = (p1 - center).normalized();
                auto n2 = (p2 - center).normalized();
                auto n3 = (p3 - center).normalized();
                auto n4 = (p4 - center).normalized();

                vertices.push_back({ p1, n1, color });
                vertices.push_back({ p2, n2, color });
                vertices.push_back({ p3, n3, color });

                vertices.push_back({ p2, n2, color });
                vertices.push_back({ p4, n4, color });
                vertices.push_back({ p3, n3, color });
            }
        }
        return vertices;
    }


    /**
     * Build a closed cylinder from "start" to "end."
     *
     * segments: how many slices around the axis
     * color: color for the entire cylinder
     *
     * Returns a vector of VertexData with CCW winding from the outside.
     */
    std::vector<VertexData> buildCylinderWithCaps(
        const QVector3D& start,
        const QVector3D& end,
        float radius,
        int segments,
        const QVector3D& color)
    {
        std::vector<VertexData> verts;
        verts.reserve(segments * 12); // side + 2 caps

        QVector3D axis    = end - start;
        float height       = axis.length();
        if (height < 1e-6f) {
            return verts; // degenerate
        }
        QVector3D axisDir = axis.normalized();

        // Build an orthonormal basis
        QVector3D up(0,1,0);
        if (fabsf(QVector3D::dotProduct(axisDir, up)) > 0.999f) {
            up = QVector3D(1,0,0);
        }
        QVector3D perpX = QVector3D::crossProduct(axisDir, up).normalized();
        QVector3D perpY = QVector3D::crossProduct(axisDir, perpX).normalized();

        // Precompute ring around start and end
        std::vector<QVector3D> ringStart(segments), ringEnd(segments);
        for (int i = 0; i < segments; ++i) {
            float theta = 2.0f * float(M_PI) * float(i) / float(segments);
            float x = radius * std::cos(theta);
            float y = radius * std::sin(theta);

            ringStart[i] = start + x * perpX + y * perpY;
            ringEnd[i]   = end   + x * perpX + y * perpY;
        }

        // ----- 1) Build side quads (CCW from outside) -----
        // For each sector: (s1->s2->e1) and (e1->s2->e2)
        // We want outward normals. For the side, approximate normal is direction from axis to ring.
        for (int i = 0; i < segments; ++i) {
            int iNext = (i + 1) % segments;
            const QVector3D& s1 = ringStart[i];
            const QVector3D& s2 = ringStart[iNext];
            const QVector3D& e1 = ringEnd[i];
            const QVector3D& e2 = ringEnd[iNext];

            // approximate side normals
            // Use the midpoint from axis, or simply the direction of s1-start but ignoring the vertical
            // We'll do it more correct: for s1 -> s2 -> e1, normal = cross((s2-s1),(e1-s1))
            QVector3D n1 = QVector3D::crossProduct((s2 - s1),(e1 - s1)).normalized();
            QVector3D n2 = QVector3D::crossProduct((e2 - s2),(e1 - s2)).normalized();

            // Tri1: s1->s2->e1
            verts.push_back({ s1, n1, color });
            verts.push_back({ s2, n1, color });
            verts.push_back({ e1, n1, color });

            // Tri2: e1->s2->e2
            verts.push_back({ e1, n2, color });
            verts.push_back({ s2, n2, color });
            verts.push_back({ e2, n2, color });
        }

        // ----- 2) Bottom cap at `start` -----
        // normal = -axisDir for the bottom (pointing outward if the outside is "above")
        QVector3D bottomNormal = -axisDir;
        for (int i = 0; i < segments; ++i) {
            int iNext = (i+1) % segments;
            const QVector3D& p1 = ringStart[i];
            const QVector3D& p2 = ringStart[iNext];

            // We want the tri to be CCW from the outside (looking from outside "above"?).
            // Typically that means (p1->p2->start) might or might not align with bottomNormal.
            // Let's do a cross check:

            QVector3D c = QVector3D::crossProduct((p2 - p1), (start - p1));
            if (QVector3D::dotProduct(c, bottomNormal) < 0.0f) {
                // flip
                verts.push_back({ p2, bottomNormal, color });
                verts.push_back({ p1, bottomNormal, color });
                verts.push_back({ start, bottomNormal, color });
            } else {
                verts.push_back({ p1, bottomNormal, color });
                verts.push_back({ p2, bottomNormal, color });
                verts.push_back({ start, bottomNormal, color });
            }
        }

        // ----- 3) Top cap at `end` -----
        // normal = +axisDir
        QVector3D topNormal = axisDir;
        for (int i = 0; i < segments; ++i) {
            int iNext = (i+1) % segments;
            const QVector3D& p1 = ringEnd[i];
            const QVector3D& p2 = ringEnd[iNext];

            QVector3D c = QVector3D::crossProduct((p2 - p1), (end - p1));
            if (QVector3D::dotProduct(c, topNormal) < 0.0f) {
                // flip
                verts.push_back({ p2, topNormal, color });
                verts.push_back({ p1, topNormal, color });
                verts.push_back({ end, topNormal, color });
            } else {
                verts.push_back({ p1, topNormal, color });
                verts.push_back({ p2, topNormal, color });
                verts.push_back({ end, topNormal, color });
            }
        }

        return verts;
    }

    // This code expects glFrontFace(GL_CCW).
    // If you see it inverted, just swap base triangles or call glFrontFace(GL_CW).

    static std::vector<SceneGeometryManager::VertexData> buildConeWithBase(
        const QVector3D& tip,
        const QVector3D& baseCenter,
        float baseRadius,
        int segments,
        const QVector3D& color)
    {
        std::vector<SceneGeometryManager::VertexData> verts;
        verts.reserve(segments * 6); // side triangles + base triangles

        // Define the axis from tip to baseCenter.
        QVector3D axis = baseCenter - tip;
        float height = axis.length();
        if (height < 1e-6f) {
            return verts; // degenerate cone (tip == baseCenter)
        }
        QVector3D axisDir = axis.normalized();

        // Build a perpendicular frame for the base circle.
        QVector3D up(0, 1, 0);
        if (fabsf(QVector3D::dotProduct(axisDir, up)) > 0.999f) {
            up = QVector3D(1, 0, 0);
        }
        QVector3D perpX = QVector3D::crossProduct(axisDir, up).normalized();
        QVector3D perpY = QVector3D::crossProduct(axisDir, perpX).normalized();

        // Generate circle points around the base.
        std::vector<QVector3D> circlePts(segments);
        for (int i = 0; i < segments; ++i) {
            float theta = 2.0f * float(M_PI) * float(i) / float(segments);
            float x = baseRadius * cosf(theta);
            float y = baseRadius * sinf(theta);
            circlePts[i] = baseCenter + x * perpX + y * perpY;
        }

        // --- 1) Side Triangles ---
        // Original order was (p1, p2, tip). We now reverse it to (tip, p2, p1)
        // and invert the computed normal.
        for (int i = 0; i < segments; ++i) {
            int iNext = (i + 1) % segments;
            const QVector3D &p1 = circlePts[i];
            const QVector3D &p2 = circlePts[iNext];

            QVector3D sideNormal = QVector3D::crossProduct((p2 - p1), (tip - p1)).normalized();
            sideNormal = -sideNormal;  // Invert the normal

            verts.push_back({ tip, sideNormal, color });
            verts.push_back({ p2, sideNormal, color });
            verts.push_back({ p1, sideNormal, color });
        }

        // --- 2) Base Triangles ---
        // Original order was (p2, p1, baseCenter). We now reverse it to (baseCenter, p1, p2)
        // and invert the base normal.
        QVector3D baseNormal = axisDir;
        baseNormal = -baseNormal;  // Invert the normal

        for (int i = 0; i < segments; ++i) {
            int iNext = (i + 1) % segments;
            const QVector3D &p1 = circlePts[i];
            const QVector3D &p2 = circlePts[iNext];

            verts.push_back({ baseCenter, baseNormal, color });
            verts.push_back({ p1, baseNormal, color });
            verts.push_back({ p2, baseNormal, color });
        }

        return verts;
    }


    SceneGeometryManager();
    ~SceneGeometryManager();

    void initialize();
    void setScene(std::weak_ptr<Scene> scene);
    void setSceneColorificator(std::weak_ptr<SceneColorificator> colorificator);

    /**
     * @brief Updates buffers (axes, ticks, points, lines, arrow cones).
     */
    void updateGeometry();

    /**
     * @brief Renders all geometry. Expects the shader to be bound and uniforms set.
     * @param program The shader program so we can set `uPrimitiveType` uniform
     *        before each geometry draw call.
     */
    void renderAll(QOpenGLShaderProgram* program);

private:
    void createOrUpdateBuffer(GLuint &vao, GLuint &vbo,
                              const VertexData *data,
                              size_t dataSize,
                              GLsizei &vertexCount);

    void updateAxesData();
    void updateTicksData();
    void updatePointsData();
    void updateLinesData();

private:
    std::weak_ptr<Scene> scene_;
    std::weak_ptr<SceneColorificator> colorificator_;

    GLuint vaoAxes_, vboAxes_;
    GLsizei axesVertexCount_;

    GLuint vaoTicks_, vboTicks_;
    GLsizei ticksVertexCount_;

    GLuint vaoPoints_, vboPoints_;
    GLsizei pointsVertexCount_;

    GLuint vaoLines_, vboLines_;
    GLsizei linesVertexCount_;

    // Arrow cone geometry
    GLuint vaoArrowCone_, vboArrowCone_;
    GLsizei arrowConeVertexCount_;

    static constexpr float kLineWidthThin  = 2.0f;
    static constexpr float kLineWidthThick = 4.0f;
    static constexpr float kPointSize      = 10.0f;
    static constexpr int   kTickRange      = 40;
    static constexpr float kTickOffset     = 0.1f;
    static constexpr float kAxisEnd        = 100.0f;
    static constexpr float kArrowSize      = 2.0f;

    // Cone geometry
    static constexpr float kConeRadius   = kArrowSize * 0.3f;
    static constexpr int   kConeSegments = 20;
};

#endif // SCENE_GEOMETRY_MANAGER_H
