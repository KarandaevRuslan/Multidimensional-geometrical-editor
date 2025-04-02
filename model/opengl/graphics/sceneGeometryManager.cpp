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

// In SceneGeometryManager::renderAll(QOpenGLShaderProgram* program)

void SceneGeometryManager::renderAll(QOpenGLShaderProgram* program)
{
    // TICKS: no lighting
    if (ticksVertexCount_ > 0) {
        program->setUniformValue("uApplyLighting", false); // <--
        glLineWidth(kLineWidthThin);
        glBindVertexArray(vaoTicks_);
        glDrawArrays(GL_LINES, 0, ticksVertexCount_);
        glBindVertexArray(0);
    }

    // SCENE LINES: apply lighting
    if (linesVertexCount_ > 0) {
        program->setUniformValue("uApplyLighting", true);
        glBindVertexArray(vaoLines_);
        glDrawArrays(GL_TRIANGLES, 0, linesVertexCount_);
        glBindVertexArray(0);
    }

    // POINTS: apply lighting
    if (pointsVertexCount_ > 0) {
        program->setUniformValue("uApplyLighting", true);
        glBindVertexArray(vaoPoints_);
        glDrawArrays(GL_TRIANGLES, 0, pointsVertexCount_);
        glBindVertexArray(0);
    }

    // AXES: no lighting
    if (axesVertexCount_ > 0) {
        program->setUniformValue("uApplyLighting", false); // <--
        glLineWidth(kLineWidthThin);
        glBindVertexArray(vaoAxes_);
        glDrawArrays(GL_LINES, 0, axesVertexCount_);
        glBindVertexArray(0);
    }

    // ARROW CONES: use lighting (triangles)
    if (arrowConeVertexCount_ > 0) {
        program->setUniformValue("uApplyLighting", true); // <--
        glBindVertexArray(vaoArrowCone_);
        glDrawArrays(GL_TRIANGLES, 0, arrowConeVertexCount_);
        glBindVertexArray(0);
    }
}

void SceneGeometryManager::updateAxesData()
{
    // Lines for axes
    std::vector<VertexData> axisLines;
    axisLines.reserve(6); // 3 axes * 2 endpoints

    // Triangles for arrow cones
    std::vector<VertexData> arrowConeVertices;
    arrowConeVertices.reserve(6 * kConeSegments * 3);

    const QVector3D red(1, 0, 0);
    const QVector3D green(0, 1, 0);
    const QVector3D blue(0, 0, 1);

    auto addAxis = [&](const QVector3D &dir, const QVector3D &color)
    {
        // Negative endpoint
        QVector3D negPoint  = -dir * kAxisEnd;
        // The base of the arrow (where the cone starts)
        QVector3D arrowBase =  dir * (kAxisEnd - kArrowSize);

        // Draw a simple line for the axis
        axisLines.push_back({ negPoint, QVector3D(0,0,0), color });
        axisLines.push_back({ arrowBase, QVector3D(0,0,0), color });

        // Build a cone:
        //   tip = dir * kAxisEnd
        //   baseCenter = arrowBase
        //   baseRadius = kConeRadius
        //   segments = kConeSegments
        //   color = same axis color
        QVector3D tip        = dir * kAxisEnd;
        QVector3D baseCenter = arrowBase;

        auto cone = buildConeWithBase(
            tip,
            baseCenter,
            kConeRadius,
            kConeSegments,
            color);

        // Append to arrowConeVertices
        arrowConeVertices.insert(
            arrowConeVertices.end(),
            cone.begin(),
            cone.end());
    };

    // X, Y, Z axes
    addAxis(QVector3D(1, 0, 0), red);
    addAxis(QVector3D(0, 1, 0), green);
    addAxis(QVector3D(0, 0, 1), blue);

    // Upload axis lines to GPU
    createOrUpdateBuffer(vaoAxes_, vboAxes_,
                         axisLines.data(),
                         axisLines.size() * sizeof(VertexData),
                         axesVertexCount_);

    // Upload arrow cones (triangles) to GPU
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
        ticks.push_back({ QVector3D(float(i),  kTickOffset, 0), QVector3D(0,0,0), white });
        ticks.push_back({ QVector3D(float(i), -kTickOffset, 0), QVector3D(0,0,0), white });
        // Y
        ticks.push_back({ QVector3D(kTickOffset, float(i), 0), QVector3D(0,0,0), white });
        ticks.push_back({ QVector3D(-kTickOffset, float(i),0), QVector3D(0,0,0), white });
        // Z
        ticks.push_back({ QVector3D(0, kTickOffset, float(i)), QVector3D(0,0,0), white });
        ticks.push_back({ QVector3D(0,-kTickOffset, float(i)), QVector3D(0,0,0), white });
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
    std::vector<VertexData> sphereTriangles;  // big container

    // pick sphere detail
    const int rings   = 15;
    const int sectors = 15;
    const float sphereRadius = 0.15f;

    for (auto it = vBegin; it != vEnd; ++it) {
        auto cv = *it;
        QVector3D pos(0,0,0);
        if (!cv.coords.empty())   pos.setX(cv.coords[0]);
        if (cv.coords.size() > 1) pos.setY(cv.coords[1]);
        if (cv.coords.size() > 2) pos.setZ(cv.coords[2]);

        QVector3D col(cv.color.redF(), cv.color.greenF(), cv.color.blueF());

        // build a small sphere at this point
        auto sphereVerts = buildSphere(sphereRadius, rings, sectors, pos, col);
        // append them into the big container
        sphereTriangles.insert(sphereTriangles.end(), sphereVerts.begin(), sphereVerts.end());
    }

    // Now upload to the GPU as triangles
    createOrUpdateBuffer(vaoPoints_, vboPoints_,
                         sphereTriangles.data(),
                         sphereTriangles.size() * sizeof(VertexData),
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

    std::vector<VertexData> allCylinders;
    // tube radius for lines
    float tubeRadius = 0.055f;
    int tubeSegments = 18;

    for (auto it = eBegin; it != eEnd; ++it) {
        auto cl = *it;
        QVector3D start(0,0,0), end(0,0,0);

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

        // build a cylinder from start to end
        auto cylinderVerts = buildCylinderWithCaps(start, end, tubeRadius, tubeSegments, col);
        allCylinders.insert(allCylinders.end(), cylinderVerts.begin(), cylinderVerts.end());
    }

    createOrUpdateBuffer(vaoLines_, vboLines_,
                         allCylinders.data(),
                         allCylinders.size() * sizeof(VertexData),
                         linesVertexCount_);
}

//
// ----------------- createOrUpdateBuffer --------------------
//

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

    // Position => location 0
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(VertexData),
                          reinterpret_cast<void*>(offsetof(VertexData, position)));

    // Normal => location 1
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          sizeof(VertexData),
                          reinterpret_cast<void*>(offsetof(VertexData, normal)));

    // Color => location 2
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE,
                          sizeof(VertexData),
                          reinterpret_cast<void*>(offsetof(VertexData, color)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}
