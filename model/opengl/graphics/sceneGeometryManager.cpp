#include "sceneGeometryManager.h"
#include <QtMath>
#include <QOpenGLWidget>
#include <QMatrix4x4>
#include <vector>
#include <cstddef>
#include <QPainter>
#include <QOpenGLWindow>
#include "../other/axisSystem.h"
#include "../../../tools/numTools.h"

SceneGeometryManager::SceneGeometryManager()
    : coneRadius_(arrowSize_ * 0.3f)
{
    axes_.clear();

    Axis xAxis;
    xAxis.name = "X";
    xAxis.color = QVector3D(1, 0, 0);
    xAxis.direction = QVector3D(1.0f, 0.0f, 0.0f);
    axes_.append(xAxis);

    Axis yAxis;
    yAxis.name = "Y";
    yAxis.color = QVector3D(0, 1, 0);
    yAxis.direction = QVector3D(0.0f, 1.0f, 0.0f);
    axes_.append(yAxis);

    Axis zAxis;
    zAxis.name = "Z";
    zAxis.color = QVector3D(0, 0, 1);
    zAxis.direction = QVector3D(0.0f, 0.0f, 1.0f);
    axes_.append(zAxis);
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
    if (!geometryDirty_) {
        return;
    }

    updatePointsData();
    updateLinesData();

    geometryDirty_ = false;
}

void SceneGeometryManager::renderAll(QOpenGLShaderProgram* program)
{
    // Render ticks (lines, no lighting)
    if (ticksVertexCount_ > 0) {
        program->setUniformValue("uApplyLighting", false);
        program->setUniformValue("uApplyShadow", false);
        glLineWidth(lineWidthThin_);
        glBindVertexArray(vaoTicks_);
        glDrawArrays(GL_LINES, 0, ticksVertexCount_);
        glBindVertexArray(0);
    }

    // Render scene lines (triangles, with lighting)
    if (linesVertexCount_ > 0) {
        program->setUniformValue("uApplyLighting", true);
        program->setUniformValue("uApplyShadow", true);
        glBindVertexArray(vaoLines_);
        glDrawArrays(GL_TRIANGLES, 0, linesVertexCount_);
        glBindVertexArray(0);
    }

    // Render points (small spheres, with lighting)
    if (pointsVertexCount_ > 0) {
        program->setUniformValue("uApplyLighting", true);
        program->setUniformValue("uApplyShadow", true);
        glBindVertexArray(vaoPoints_);
        glDrawArrays(GL_TRIANGLES, 0, pointsVertexCount_);
        glBindVertexArray(0);
    }

    // Render axes (lines, no lighting)
    if (axesVertexCount_ > 0) {
        program->setUniformValue("uApplyLighting", false);
        program->setUniformValue("uApplyShadow", false);
        glLineWidth(lineWidthThin_);
        glBindVertexArray(vaoAxes_);
        glDrawArrays(GL_LINES, 0, axesVertexCount_);
        glBindVertexArray(0);
    }

    // Render arrow cones (triangles, with lighting)
    if (arrowConeVertexCount_ > 0) {
        program->setUniformValue("uApplyLighting", true);
        program->setUniformValue("uApplyShadow", false);
        glBindVertexArray(vaoArrowCone_);
        glDrawArrays(GL_TRIANGLES, 0, arrowConeVertexCount_);
        glBindVertexArray(0);
    }
}

void SceneGeometryManager::updateAxesData()
{
    std::vector<VertexData> axisLines;
    axisLines.reserve(6); // 3 axes * 2 endpoints each

    std::vector<VertexData> arrowConeVertices;
    arrowConeVertices.reserve(6 * coneSegments_ * 3);

    // Helper to add an axis line + cone
    auto addAxis = [&](const Axis &axis)
    {
        QVector3D dir = axis.direction.normalized();
        float halfLength = axis.length / 2.0f;

        QVector3D negPt = origin_ - dir * halfLength;
        QVector3D arrowBase = origin_ + dir * (halfLength - arrowSize_);

        // Axis line
        axisLines.push_back({negPt, QVector3D(), axis.color});
        axisLines.push_back({arrowBase, QVector3D(), axis.color});

        // Cone
        QVector3D tip = origin_ + dir * halfLength;
        auto cone = buildConeWithBase(tip,
                                      arrowBase,
                                      coneRadius_,
                                      coneSegments_,
                                      axis.color);
        arrowConeVertices.insert(arrowConeVertices.end(),
                                 cone.begin(),
                                 cone.end());
    };

    // X, Y, Z
    addAxis(axes_[0]);
    addAxis(axes_[1]);
    addAxis(axes_[2]);

    // Upload lines
    createOrUpdateBuffer(vaoAxes_, vboAxes_,
                         axisLines.data(),
                         axisLines.size() * sizeof(VertexData),
                         axesVertexCount_);

    // Upload cones
    createOrUpdateBuffer(vaoArrowCone_, vboArrowCone_,
                         arrowConeVertices.data(),
                         arrowConeVertices.size() * sizeof(VertexData),
                         arrowConeVertexCount_);
}

void SceneGeometryManager::updateTicksData()
{
    int ticks_total = std::accumulate(axes_.begin(), axes_.end(), 0,
        [](int total, const Axis& axis) {
            return total + axis.tickPositions.size();
        });
    std::vector<VertexData> ticks;
    ticks.reserve(ticks_total * 2);

    for (int axisIndex = 0; axisIndex < axes_.size(); ++axisIndex) {
        const Axis& axis = axes_[axisIndex];
        for (const QVector3D& tick : axis.tickPositions) {

            switch (axisIndex) {
                // X
                case 0:
                    ticks.push_back({tick + QVector3D(0,  tickOffset_, 0), {}, ticksColor_});
                    ticks.push_back({tick + QVector3D(0,  -tickOffset_, 0), {}, ticksColor_});
                    break;
                // Y
                case 1:
                    ticks.push_back({tick + QVector3D(tickOffset_, 0, 0), {}, ticksColor_});
                    ticks.push_back({tick + QVector3D(-tickOffset_, 0, 0), {}, ticksColor_});
                    break;
                // Z
                case 2:
                    ticks.push_back({tick + QVector3D(0,  tickOffset_, 0), {}, ticksColor_});
                    ticks.push_back({tick + QVector3D(0,  -tickOffset_, 0), {}, ticksColor_});
                    break;
                default:
                    qFatal() << "Unknown axis index: " << axisIndex;
                    break;
            }
        }
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

    std::vector<VertexData> sphereTriangles;

    for (auto it = vBegin; it != vEnd; ++it) {
        auto cv = *it;
        QVector3D pos(0,0,0);

        if (!cv.coords.empty())   pos.setX(cv.coords[0]);
        if (cv.coords.size() > 1) pos.setY(cv.coords[1]);
        if (cv.coords.size() > 2) pos.setZ(cv.coords[2]);

        QVector3D col(cv.color.redF(),
                      cv.color.greenF(),
                      cv.color.blueF());

        // Build a small sphere
        auto sphereVerts = buildSphere(sphereRadius_,
                                       sphereRings_,
                                       sphereSectors_,
                                       pos,
                                       col);
        sphereTriangles.insert(sphereTriangles.end(),
                               sphereVerts.begin(),
                               sphereVerts.end());
    }

    // Upload
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

        QVector3D col(cl.color.redF(),
                      cl.color.greenF(),
                      cl.color.blueF());

        // Build a cylinder
        auto cylVerts = buildCylinderWithCaps(start,
                                              end,
                                              tubeRadius_,
                                              tubeSegments_,
                                              col);
        allCylinders.insert(allCylinders.end(),
                            cylVerts.begin(),
                            cylVerts.end());
    }

    createOrUpdateBuffer(vaoLines_, vboLines_,
                         allCylinders.data(),
                         allCylinders.size() * sizeof(VertexData),
                         linesVertexCount_);
}

void SceneGeometryManager::createOrUpdateBuffer(GLuint &vao,
                                                GLuint &vbo,
                                                const VertexData* data,
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

QPointF SceneGeometryManager::projectToScreen(const QOpenGLWindow *widget,
                        const QVector3D &point,
                        const QMatrix4x4 &mvp) const
{
    QVector4D clip = mvp * QVector4D(point, 1.0f);

    if (clip.w() <= 0.0f)
        return QPointF(kOffScreenCoord_, kOffScreenCoord_);

    float ndcX = clip.x() / clip.w();
    float ndcY = clip.y() / clip.w();
    float ndcZ = clip.z() / clip.w();

    if (ndcX < -1.0f || ndcX > 1.0f ||
        ndcY < -1.0f || ndcY > 1.0f ||
        ndcZ < -1.0f || ndcZ > 1.0f)
    {
        return QPointF(kOffScreenCoord_, kOffScreenCoord_);
    }

    float sx = (ndcX * 0.5f + 0.5f) * float(widget->width());
    float sy = (1.0f - (ndcY * 0.5f + 0.5f)) * float(widget->height());

    return QPointF(sx, sy);
}

void SceneGeometryManager::paintOverlayLabels(QOpenGLWindow *widget,
                        const QMatrix4x4 &mvp) const
{
    QPainter painterNumbers(widget);
    painterNumbers.beginNativePainting();
    painterNumbers.endNativePainting();


    auto labelIfVisible = [&](float i, const QVector3D &worldPos)
    {
        painterNumbers.setPen(overlayNumberPen_);
        painterNumbers.setFont(overlayNumberFont_);

        QPointF sp = projectToScreen(widget, worldPos, mvp);
        if (sp.x() < 0.0f) return;
        if (::isInteger(i))
            painterNumbers.drawText(sp, QString::number(std::round(i)));
        else
            painterNumbers.drawText(sp, QString::number(i, 'f', 1));
    };

    // Zero at origin
    labelIfVisible(0, origin_);


    for (const Axis& axis : axes_) {
        for (const QVector3D& tick : axis.tickPositions) {
            labelIfVisible(
                QVector3D::dotProduct(
                    tick, axis.direction.normalized()),
                tick);
        }
        QColor color(
            static_cast<int>(axis.color.x() * 255),
            static_cast<int>(axis.color.y() * 255),
            static_cast<int>(axis.color.z() * 255));
        painterNumbers.setPen(QPen(color));
        painterNumbers.setFont(overlayAxisNameFont_);

        QPointF sp = projectToScreen(
            widget,
            (axis.length * 0.5) * axis.direction.normalized() + arrowSize_ * 0.5 * QVector3D(0, 1, 0),
            mvp);
        if (sp.x() >= 0.0f) painterNumbers.drawText(sp, axis.name);
    }
}

void SceneGeometryManager::updateAxes(const QVector3D& cameraPos)
{
    ::updateAxes(axes_, cameraPos, tickBoxFactor_,  arrowSize_ * 3, origin_);
    updateAxesData();
    updateTicksData();
}

void SceneGeometryManager::markGeometryDirty()
{
    geometryDirty_ = true;
}

bool SceneGeometryManager::isGeometryDirty(){
    return geometryDirty_;
}

std::vector<SceneGeometryManager::VertexData>
SceneGeometryManager::buildSphere(float radius,
                                  int rings,
                                  int sectors,
                                  const QVector3D& center,
                                  const QVector3D& color)
{
    std::vector<VertexData> vertices;
    vertices.reserve(rings * sectors * 6);

    for (int r = 0; r < rings; ++r) {
        float theta1 = float(M_PI) * float(r)       / rings;
        float theta2 = float(M_PI) * float(r + 1)   / rings;

        for (int s = 0; s < sectors; ++s) {
            float phi1 = 2.0f * float(M_PI) * float(s)       / sectors;
            float phi2 = 2.0f * float(M_PI) * float(s + 1)   / sectors;

            QVector3D p1(std::sin(theta1) * std::cos(phi1),
                         std::cos(theta1),
                         std::sin(theta1) * std::sin(phi1));
            QVector3D p2(std::sin(theta1) * std::cos(phi2),
                         std::cos(theta1),
                         std::sin(theta1) * std::sin(phi2));
            QVector3D p3(std::sin(theta2) * std::cos(phi1),
                         std::cos(theta2),
                         std::sin(theta2) * std::sin(phi1));
            QVector3D p4(std::sin(theta2) * std::cos(phi2),
                         std::cos(theta2),
                         std::sin(theta2) * std::sin(phi2));

            p1 = p1 * radius + center;
            p2 = p2 * radius + center;
            p3 = p3 * radius + center;
            p4 = p4 * radius + center;

            QVector3D n1 = (p1 - center).normalized();
            QVector3D n2 = (p2 - center).normalized();
            QVector3D n3 = (p3 - center).normalized();
            QVector3D n4 = (p4 - center).normalized();

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

std::vector<SceneGeometryManager::VertexData>
SceneGeometryManager::buildCylinderWithCaps(const QVector3D& start,
                                            const QVector3D& end,
                                            float radius,
                                            int segments,
                                            const QVector3D& color)
{
    std::vector<VertexData> verts;
    verts.reserve(segments * 12); // sides + caps

    QVector3D axis = end - start;
    float height = axis.length();
    if (height < 1e-6f) {
        return verts; // degenerate
    }

    QVector3D axisDir = axis.normalized();
    QVector3D up(0,1,0);
    if (std::fabs(QVector3D::dotProduct(axisDir, up)) > 0.999f) {
        up = QVector3D(1,0,0);
    }
    QVector3D perpX = QVector3D::crossProduct(axisDir, up).normalized();
    QVector3D perpY = QVector3D::crossProduct(axisDir, perpX).normalized();

    std::vector<QVector3D> ringStart(segments), ringEnd(segments);
    for (int i = 0; i < segments; ++i) {
        float theta = 2.0f * float(M_PI) * float(i) / float(segments);
        float x = radius * std::cos(theta);
        float y = radius * std::sin(theta);
        ringStart[i] = start + x * perpX + y * perpY;
        ringEnd[i]   = end   + x * perpX + y * perpY;
    }

    // Sides
    for (int i = 0; i < segments; ++i) {
        int iNext = (i + 1) % segments;
        const QVector3D& s1 = ringStart[i];
        const QVector3D& s2 = ringStart[iNext];
        const QVector3D& e1 = ringEnd[i];
        const QVector3D& e2 = ringEnd[iNext];

        QVector3D n1 = QVector3D::crossProduct(s2 - s1, e1 - s1).normalized();
        QVector3D n2 = QVector3D::crossProduct(e2 - s2, e1 - s2).normalized();

        // Tri1
        verts.push_back({ s1, n1, color });
        verts.push_back({ s2, n1, color });
        verts.push_back({ e1, n1, color });

        // Tri2
        verts.push_back({ e1, n2, color });
        verts.push_back({ s2, n2, color });
        verts.push_back({ e2, n2, color });
    }

    // Bottom cap
    QVector3D bottomNormal = -axisDir;
    for (int i = 0; i < segments; ++i) {
        int iNext = (i + 1) % segments;
        const QVector3D &p1 = ringStart[i];
        const QVector3D &p2 = ringStart[iNext];

        // Ensure CCW
        QVector3D c = QVector3D::crossProduct(p2 - p1, start - p1);
        if (QVector3D::dotProduct(c, bottomNormal) < 0.0f) {
            verts.push_back({ p2, bottomNormal, color });
            verts.push_back({ p1, bottomNormal, color });
            verts.push_back({ start, bottomNormal, color });
        } else {
            verts.push_back({ p1, bottomNormal, color });
            verts.push_back({ p2, bottomNormal, color });
            verts.push_back({ start, bottomNormal, color });
        }
    }

    // Top cap
    QVector3D topNormal = axisDir;
    for (int i = 0; i < segments; ++i) {
        int iNext = (i + 1) % segments;
        const QVector3D &p1 = ringEnd[i];
        const QVector3D &p2 = ringEnd[iNext];

        QVector3D c = QVector3D::crossProduct(p2 - p1, end - p1);
        if (QVector3D::dotProduct(c, topNormal) < 0.0f) {
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

std::vector<SceneGeometryManager::VertexData>
SceneGeometryManager::buildConeWithBase(const QVector3D& tip,
                                        const QVector3D& baseCenter,
                                        float baseRadius,
                                        int segments,
                                        const QVector3D& color)
{
    std::vector<VertexData> verts;
    verts.reserve(segments * 6); // side + base

    QVector3D axis = baseCenter - tip;
    float height = axis.length();
    if (height < 1e-6f) {
        return verts; // degenerate
    }
    QVector3D axisDir = axis.normalized();

    QVector3D up(0,1,0);
    if (std::fabs(QVector3D::dotProduct(axisDir, up)) > 0.999f) {
        up = QVector3D(1,0,0);
    }
    QVector3D perpX = QVector3D::crossProduct(axisDir, up).normalized();
    QVector3D perpY = QVector3D::crossProduct(axisDir, perpX).normalized();

    std::vector<QVector3D> circlePts(segments);
    for (int i = 0; i < segments; ++i) {
        float theta = 2.0f * float(M_PI) * float(i) / float(segments);
        float x = baseRadius * std::cos(theta);
        float y = baseRadius * std::sin(theta);
        circlePts[i] = baseCenter + x * perpX + y * perpY;
    }

    // Side
    for (int i = 0; i < segments; ++i) {
        int iNext = (i + 1) % segments;
        const QVector3D &p1 = circlePts[i];
        const QVector3D &p2 = circlePts[iNext];

        QVector3D sideN = QVector3D::crossProduct(p2 - p1, tip - p1).normalized();
        sideN = -sideN;  // invert

        verts.push_back({ tip, sideN, color });
        verts.push_back({ p2, sideN, color });
        verts.push_back({ p1, sideN, color });
    }

    // Base
    QVector3D baseNormal = -axisDir;
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
