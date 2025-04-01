#include "sceneGeometryManager.h"
#include <QtMath>
#include <QMatrix4x4>
#include <vector>
#include <cstddef>

SceneGeometryManager::SceneGeometryManager()
    : vaoAxes_(0)
    , vboAxes_(0)
    , axesVertexCount_(0)
    , vaoTicks_(0)
    , vboTicks_(0)
    , ticksVertexCount_(0)
    , vaoPoints_(0)
    , vboPoints_(0)
    , pointsVertexCount_(0)
    , vaoLines_(0)
    , vboLines_(0)
    , linesVertexCount_(0)
    , vaoArrowCone_(0)
    , vboArrowCone_(0)
    , arrowConeVertexCount_(0)
{
}

SceneGeometryManager::~SceneGeometryManager()
{
    // Clean up OpenGL buffers. A current context must be active.
    glDeleteBuffers(1, &vboAxes_);
    glDeleteVertexArrays(1, &vaoAxes_);

    glDeleteBuffers(1, &vboTicks_);
    glDeleteVertexArrays(1, &vaoTicks_);

    glDeleteBuffers(1, &vboPoints_);
    glDeleteVertexArrays(1, &vaoPoints_);

    glDeleteBuffers(1, &vboLines_);
    glDeleteVertexArrays(1, &vaoLines_);

    glDeleteBuffers(1, &vboArrowCone_);
    glDeleteVertexArrays(1, &vaoArrowCone_);
}

void SceneGeometryManager::initialize()
{
    initializeOpenGLFunctions();

    glGenVertexArrays(1, &vaoAxes_);
    glGenBuffers(1, &vboAxes_);

    glGenVertexArrays(1, &vaoTicks_);
    glGenBuffers(1, &vboTicks_);

    glGenVertexArrays(1, &vaoPoints_);
    glGenBuffers(1, &vboPoints_);

    glGenVertexArrays(1, &vaoLines_);
    glGenBuffers(1, &vboLines_);

    // Generate buffers for arrow cone geometry
    glGenVertexArrays(1, &vaoArrowCone_);
    glGenBuffers(1, &vboArrowCone_);
}

void SceneGeometryManager::setScene(std::weak_ptr<Scene> scene)
{
    scene_ = scene;
}

void SceneGeometryManager::setSceneColorificator(std::weak_ptr<SceneColorificator> colorificator)
{
    colorificator_ = colorificator;
}

void SceneGeometryManager::updateGeometry()
{
    updateAxesData();
    updateTicksData();
    updatePointsData();
    updateLinesData();
}

void SceneGeometryManager::renderAll()
{
    // We do NOT set the MVP uniform here anymore.
    // We assume the current shader program is already bound
    // and the MVP has been set by the caller (SceneRenderer).

    // Render axis lines
    if (axesVertexCount_ > 0) {
        glLineWidth(kLineWidthThin);
        glBindVertexArray(vaoAxes_);
        glDrawArrays(GL_LINES, 0, axesVertexCount_);
        glBindVertexArray(0);
    }

    // Render arrow cones
    if (arrowConeVertexCount_ > 0) {
        glBindVertexArray(vaoArrowCone_);
        glDrawArrays(GL_TRIANGLES, 0, arrowConeVertexCount_);
        glBindVertexArray(0);
    }

    // Render ticks
    if (ticksVertexCount_ > 0) {
        glLineWidth(kLineWidthThin);
        glBindVertexArray(vaoTicks_);
        glDrawArrays(GL_LINES, 0, ticksVertexCount_);
        glBindVertexArray(0);
    }

    // Render points
    if (pointsVertexCount_ > 0) {
        glPointSize(kPointSize);
        glBindVertexArray(vaoPoints_);
        glDrawArrays(GL_POINTS, 0, pointsVertexCount_);
        glBindVertexArray(0);
    }

    // Render scene lines
    if (linesVertexCount_ > 0) {
        glLineWidth(kLineWidthThick);
        glBindVertexArray(vaoLines_);
        glDrawArrays(GL_LINES, 0, linesVertexCount_);
        glBindVertexArray(0);
    }
}

void SceneGeometryManager::updateAxesData()
{
    std::vector<VertexData> axisLines;
    axisLines.reserve(6); // 3 axes * 2 endpoints

    std::vector<VertexData> arrowConeVertices;
    arrowConeVertices.reserve(6 * kConeSegments * 3);

    const QVector3D red(1, 0, 0);
    const QVector3D green(0, 1, 0);
    const QVector3D blue(0, 0, 1);

    auto addAxis = [&](const QVector3D &dir, const QVector3D &color) {
        QVector3D negPoint  = -dir * kAxisEnd;
        QVector3D arrowBase =  dir * (kAxisEnd - kArrowSize);

        // Axis line
        axisLines.push_back({ negPoint,  color });
        axisLines.push_back({ arrowBase, color });

        // Tip of the cone
        QVector3D tip        = dir * kAxisEnd;
        // Center of the cone's circular base
        QVector3D baseCenter = arrowBase;

        // Build cone by generating circle around baseCenter
        QVector3D upCandidate = qFuzzyCompare(dir.y(), 1.f) ?
                                    QVector3D(1, 0, 0) : QVector3D(0, 1, 0);

        QVector3D perp      = QVector3D::crossProduct(dir, upCandidate).normalized();
        QVector3D bitangent = QVector3D::crossProduct(dir, perp).normalized();

        std::vector<QVector3D> baseCircle;
        baseCircle.reserve(kConeSegments);

        for (int i = 0; i < kConeSegments; ++i) {
            float theta1 = (2.0f * float(M_PI) * i) / float(kConeSegments);
            float theta2 = (2.0f * float(M_PI) * (i + 1)) / float(kConeSegments);

            // Two consecutive vertices
            QVector3D baseVertex1 = baseCenter + (std::cos(theta1) * perp + std::sin(theta1) * bitangent) * kConeRadius;
            QVector3D baseVertex2 = baseCenter + (std::cos(theta2) * perp + std::sin(theta2) * bitangent) * kConeRadius;

            // Lateral surface
            arrowConeVertices.push_back({ tip,         color });
            arrowConeVertices.push_back({ baseVertex1, color });
            arrowConeVertices.push_back({ baseVertex2, color });

            baseCircle.push_back(baseVertex1);
        }

        // Base cap
        for (int i = 0; i < kConeSegments; ++i) {
            int next = (i + 1) % kConeSegments;
            arrowConeVertices.push_back({ baseCenter,       color });
            arrowConeVertices.push_back({ baseCircle[i],    color });
            arrowConeVertices.push_back({ baseCircle[next], color });
        }
    };

    // X, Y, Z axes
    addAxis(QVector3D(1, 0, 0), red);
    addAxis(QVector3D(0, 1, 0), green);
    addAxis(QVector3D(0, 0, 1), blue);

    // Update buffers
    createOrUpdateBuffer(vaoAxes_, vboAxes_,
                         axisLines.data(),
                         axisLines.size() * sizeof(VertexData),
                         axesVertexCount_);

    createOrUpdateBuffer(vaoArrowCone_, vboArrowCone_,
                         arrowConeVertices.data(),
                         arrowConeVertices.size() * sizeof(VertexData),
                         arrowConeVertexCount_);
}

void SceneGeometryManager::updateTicksData()
{
    std::vector<VertexData> ticks;
    ticks.reserve(kTickRange * 2 * 3 * 2);

    const QVector3D white(1, 1, 1);
    for (int i = -kTickRange; i <= kTickRange; ++i) {
        if (i == 0) continue;
        // X
        ticks.push_back({ QVector3D(float(i),  kTickOffset, 0), white });
        ticks.push_back({ QVector3D(float(i), -kTickOffset, 0), white });
        // Y
        ticks.push_back({ QVector3D( kTickOffset, float(i), 0), white });
        ticks.push_back({ QVector3D(-kTickOffset, float(i), 0), white });
        // Z
        ticks.push_back({ QVector3D(0,  kTickOffset, float(i)), white });
        ticks.push_back({ QVector3D(0, -kTickOffset, float(i)), white });
    }

    createOrUpdateBuffer(vaoTicks_, vboTicks_,
                         ticks.data(),
                         ticks.size() * sizeof(VertexData),
                         ticksVertexCount_);
}

void SceneGeometryManager::updatePointsData()
{
    auto scenePtr = scene_.lock();
    auto colorPtr = colorificator_.lock();
    if (!scenePtr || !colorPtr) {
        pointsVertexCount_ = 0;
        return;
    }

    auto vBegin = colorPtr->beginVertices(*scenePtr);
    auto vEnd   = colorPtr->endVertices(*scenePtr);
    std::vector<VertexData> points;
    points.reserve(std::distance(vBegin, vEnd));

    for (auto it = vBegin; it != vEnd; ++it) {
        auto cv = *it;
        QVector3D pos(0, 0, 0);
        if (!cv.coords.empty())   pos.setX(cv.coords[0]);
        if (cv.coords.size() > 1) pos.setY(cv.coords[1]);
        if (cv.coords.size() > 2) pos.setZ(cv.coords[2]);

        QVector3D col(cv.color.redF(), cv.color.greenF(), cv.color.blueF());
        points.push_back({ pos, col });
    }

    createOrUpdateBuffer(vaoPoints_, vboPoints_,
                         points.data(),
                         points.size() * sizeof(VertexData),
                         pointsVertexCount_);
}

void SceneGeometryManager::updateLinesData()
{
    auto scenePtr = scene_.lock();
    auto colorPtr = colorificator_.lock();
    if (!scenePtr || !colorPtr) {
        linesVertexCount_ = 0;
        return;
    }

    auto eBegin = colorPtr->beginEdges(*scenePtr);
    auto eEnd   = colorPtr->endEdges(*scenePtr);
    std::vector<VertexData> lines;
    lines.reserve(std::distance(eBegin, eEnd) * 2);

    for (auto it = eBegin; it != eEnd; ++it) {
        auto cl = *it;
        QVector3D start(0, 0, 0), end(0, 0, 0);

        if (!cl.start.empty()) {
            start.setX(cl.start[0]);
            if (cl.start.size() > 1) start.setY(cl.start[1]);
            if (cl.start.size() > 2) start.setZ(cl.start[2]);
        }

        if (!cl.end.empty()) {
            end.setX(cl.end[0]);
            if (cl.end.size() > 1) end.setY(cl.end[1]);
            if (cl.end.size() > 2) end.setZ(cl.end[2]);
        }

        QVector3D col(cl.color.redF(), cl.color.greenF(), cl.color.blueF());
        lines.push_back({ start, col });
        lines.push_back({ end,   col });
    }

    createOrUpdateBuffer(vaoLines_, vboLines_,
                         lines.data(),
                         lines.size() * sizeof(VertexData),
                         linesVertexCount_);
}

void SceneGeometryManager::createOrUpdateBuffer(GLuint &vao, GLuint &vbo,
                                                const VertexData *data,
                                                size_t dataSize,
                                                GLsizei &vertexCount)
{
    if (!data || dataSize == 0) {
        vertexCount = 0;
        return;
    }

    vertexCount = static_cast<GLsizei>(dataSize / sizeof(VertexData));
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, dataSize, data, GL_STATIC_DRAW);

    // Position attribute at location = 0
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(VertexData),
                          reinterpret_cast<void*>(offsetof(VertexData, position)));

    // Color attribute at location = 1
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          sizeof(VertexData),
                          reinterpret_cast<void*>(offsetof(VertexData, color)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
