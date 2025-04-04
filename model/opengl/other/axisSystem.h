#ifndef AXIS_SYSTEM_H
#define AXIS_SYSTEM_H

#include <QVector3D>
#include <QList>

/**
 * @brief Represents a coordinate axis with scalable length and tick marks.
 */
struct Axis {
    QString name;                   ///< Name of axis (X, Y, Z)
    QVector3D color;                ///< Color of axis
    QVector3D direction;            ///< Unit vector in the axis direction
    float length;                   ///< Scaled length
    float tickSpacing;              ///< Scaled tick spacing
    QList<QVector3D> tickPositions; ///< Tick mark world-space positions
};

/**
 * @brief Updates axis lengths and generates tick marks based on camera position.
 *
 * @param axes List of 3 axes (X, Y, Z).
 * @param cameraPos Current camera world position.
 * @param tickBoxFactor Changes size (in the tick spaces) of the cube where ticks are generated (centered on camera projection).
 * @param arrowOffset Offset to not draw ticks near arrow.
 * @param origin Origin point from which axes extend (default is (0, 0, 0)).
 */
void updateAxes(QList<Axis>& axes, const QVector3D& cameraPos,
                int tickBoxFactor, float arrowOffset,
                const QVector3D& origin = QVector3D(0.0f, 0.0f, 0.0f));

#endif // AXIS_SYSTEM_H
