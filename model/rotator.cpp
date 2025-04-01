#include "rotator.h"
#include <cmath>
#include <QString>
#include <QDebug>

Rotator::Rotator(std::size_t axis1, std::size_t axis2, double angle)
    : axis1_(axis1), axis2_(axis2), angle_(angle)
{
}

NDShape Rotator::applyRotation(const NDShape& shape) const {
    std::size_t dim = shape.getDimension();

    // Validate that the provided axes are within the dimension range and distinct.
    if (axis1_ >= dim || axis2_ >= dim) {
        QString msg = QString("Axis index out of range: axis1=%1, axis2=%2, dimension=%3")
        .arg(axis1_).arg(axis2_).arg(dim);
        qWarning() << msg;
        throw std::invalid_argument(msg.toStdString());
    }
    if (axis1_ == axis2_) {
        QString msg = QString("Cannot rotate in a plane with identical axes: %1 and %2")
        .arg(axis1_).arg(axis2_);
        qWarning() << msg;
        throw std::invalid_argument(msg.toStdString());
    }

    NDShape rotatedShape = shape;
    double cosA = std::cos(angle_);
    double sinA = std::sin(angle_);

    // Retrieve all vertices from the shape and apply the rotation.
    auto allVertices = shape.getAllVertices();
    for (const auto& [vertexId, coords] : allVertices) {
        double x = coords[axis1_];
        double y = coords[axis2_];

        std::vector<double> newCoords = coords;
        newCoords[axis1_] = x * cosA - y * sinA;
        newCoords[axis2_] = x * sinA + y * cosA;

        rotatedShape.setVertexCoords(vertexId, newCoords);
    }

    return rotatedShape;
}
