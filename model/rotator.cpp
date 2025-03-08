#include "rotator.h"

#include <cmath>
#include <QString>
#include <QDebug>

NDShape Rotator::rotateInPlane(const NDShape& shape, std::size_t axis1, std::size_t axis2, double angle)
{

    std::size_t dim = shape.getDimension();
    if (axis1 >= dim || axis2 >= dim) {
        QString msg = QString("Axis index out of range: axis1=%1, axis2=%2, dimension=%3")
        .arg(axis1).arg(axis2).arg(dim);
        qWarning() << msg;
        throw std::invalid_argument(msg.toStdString());
    }

    if (axis1 == axis2) {
        QString msg = QString("Cannot rotate in a plane with identical axes: %1 and %2")
        .arg(axis1).arg(axis2);
        qWarning() << msg;
        throw std::invalid_argument(msg.toStdString());
    }

    NDShape rotatedShape = shape;

    double cosA = std::cos(angle);
    double sinA = std::sin(angle);

    auto allVertices = shape.getAllVertices();
    for (const auto& [vertexId, coords] : allVertices) {
        double x = coords[axis1];
        double y = coords[axis2];

        std::vector<double> newCoords = coords;
        newCoords[axis1] = x * cosA - y * sinA;
        newCoords[axis2] = x * sinA + y * cosA;

        rotatedShape.setVertexCoords(vertexId, newCoords);
    }

    return rotatedShape;
}
