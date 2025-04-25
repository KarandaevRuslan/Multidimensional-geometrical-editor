#include "axisSystem.h"
#include <QtMath>
#include <limits>
#include <QDebug>

namespace {

// === Configurable Limits ===
constexpr float AXIS_MIN_LENGTH = 50.0f;
constexpr float AXIS_MAX_LENGTH = 1000.0f;
constexpr float AXIS_MIN_TICK_SPACING = 0.5f;
constexpr float AXIS_MAX_TICK_SPACING = 100.0f;
const float AXIS_SIZE_FACTOR = std::sqrt(2.0);
const float BASE_TICK_SPACING = 1.0f;

float logBase2(float value) {
    return qLn(qMax(value, 0.001f)) / qLn(2.0f);
}

float distanceToAxis(const QVector3D& point, const QVector3D& axisDir) {
    QVector3D crossProd = QVector3D::crossProduct(axisDir.normalized(), point);
    return crossProd.length();
}

QVector3D projectOntoAxis(const QVector3D& point, const QVector3D& axisDir) {
    return axisDir.normalized() * QVector3D::dotProduct(point, axisDir.normalized());
}

bool isInsideBox(const QVector3D& point, const QVector3D& center, float halfSize) {
    return (qAbs(point.x() - center.x()) <= halfSize ||
            qFuzzyCompare(qAbs(point.x() - center.x()), halfSize)) &&
           (qAbs(point.y() - center.y()) <= halfSize ||
            qFuzzyCompare(qAbs(point.y() - center.y()), halfSize)) &&
           (qAbs(point.z() - center.z()) <= halfSize ||
            qFuzzyCompare(qAbs(point.z() - center.z()), halfSize));
}

Axis findClosestAxis(QList<Axis>& axes, const QVector3D& point) {
    float minDistance = std::numeric_limits<float>::max();
    Axis closest;

    for (Axis& axis : axes) {
        QVector3D unitDir = axis.direction.normalized();
        float dist = distanceToAxis(point, unitDir);
        if (dist < minDistance) {
            minDistance = dist;
            closest = axis;
        }
    }
    closest.direction = closest.direction.normalized();
    return closest;
}

} // namespace

void updateAxes(QList<Axis>& axes, const QVector3D& cameraPos,
                int tickBoxFactor, float arrowOffset,
                const QVector3D& origin) {
    QVector3D cameraLocPos = cameraPos - origin;
    float maxComponent = qMax(
        qAbs(cameraLocPos.x()),
        qMax(qAbs(cameraLocPos.y()),
             qAbs(cameraLocPos.z())));

    float scaledLength = maxComponent * 2.0 * AXIS_SIZE_FACTOR;
    float length = qBound(AXIS_MIN_LENGTH, scaledLength, AXIS_MAX_LENGTH);
    float halfLength = length * 0.5f;

    Axis closestAxis = findClosestAxis(axes, cameraLocPos);
    QVector3D closestDir = closestAxis.direction;
    QVector3D projectionCenter = projectOntoAxis(cameraLocPos, closestDir);

    float distToClosestAxis = distanceToAxis(cameraLocPos, closestDir);
    int tickLevel = qCeil(logBase2(2.0f * distToClosestAxis / (3.0f * BASE_TICK_SPACING))) - 2;
    float scaledTickSpacing = qPow(2.0f, tickLevel) * BASE_TICK_SPACING;
    float tickSpacing = qBound(AXIS_MIN_TICK_SPACING, scaledTickSpacing, AXIS_MAX_TICK_SPACING);

    for (Axis& axis : axes) {
        const QVector3D unitDir = axis.direction.normalized();
        axis.length = length;
        axis.tickSpacing = tickSpacing;

        float cameraLocCoord = QVector3D::dotProduct(cameraLocPos, unitDir);
        cameraLocCoord = qRound(cameraLocCoord / tickSpacing) * tickSpacing;

        // Generate tick positions
        axis.tickPositions.clear();
        const float halfBox = axis.tickSpacing * tickBoxFactor;

        for (int t = 0; t <= tickBoxFactor; ++t) {

            QVector3D tickPos1 = unitDir * (cameraLocCoord + t * tickSpacing);
            QVector3D tickPos2 = unitDir * (cameraLocCoord - t * tickSpacing);

            float axisProjection1 = QVector3D::dotProduct(tickPos1, unitDir);
            float axisProjection2 = QVector3D::dotProduct(tickPos2, unitDir);

            constexpr double epsilon = 1e-8;

            if (std::abs(tickPos1.length()) >= epsilon &&
                isInsideBox(tickPos1, projectionCenter, halfBox) &&
                qAbs(axisProjection1) <= qAbs(halfLength - arrowOffset))
                axis.tickPositions.append(tickPos1 + origin);

            if (std::abs(tickPos2.length()) >= epsilon &&
                isInsideBox(tickPos2, projectionCenter, halfBox) &&
                qAbs(axisProjection2) <= qAbs(halfLength - arrowOffset))
            {
                axis.tickPositions.append(tickPos2 + origin);
            }
        }
    }
}
