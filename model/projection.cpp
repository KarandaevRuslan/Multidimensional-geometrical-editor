#include "projection.h"
#include <stdexcept>
#include <cmath>
#include <QDebug>

NDShape Projection::projectShape(const NDShape& shape) const {
    std::size_t oldDim = shape.getDimension();
    if (oldDim <= 1) {
        QString msg = "Cannot project: NDShape dimension is 1 or less.";
        qWarning() << msg;
        throw std::invalid_argument(msg.toStdString());
    }

    NDShape newShape = shape.clone(oldDim - 1);

    auto allVertices = shape.getAllVertices();
    for (const auto& [vertexId, coords] : allVertices) {
        std::vector<double> projectedCoords = projectPoint(coords);
        newShape.setVertexCoords(vertexId, projectedCoords);
    }
    return newShape;
}

NDShape Projection::projectShapeToDimension(const NDShape& shape, std::size_t targetDim) const {
    if (targetDim == 0) {
        QString msg = "Target dimension cannot be zero.";
        qWarning() << msg;
        throw std::invalid_argument(msg.toStdString());
    }

    std::size_t currentDim = shape.getDimension();
    if (currentDim <= targetDim) {
        return shape;
    }

    NDShape tmp = shape;
    while (tmp.getDimension() > targetDim) {
        tmp = projectShape(tmp);
    }
    return tmp;
}

// PerspectiveProjection
PerspectiveProjection::PerspectiveProjection(double distance)
    : d_(distance)
{
}

std::vector<double> PerspectiveProjection::projectPoint(const std::vector<double>& point) const {
    std::size_t n = point.size();
    if (n <= 1) {
        QString msg = "Point dimension must be > 1 for PerspectiveProjection.";
        qWarning() << msg;
        throw std::invalid_argument(msg.toStdString());
    }

    double denominator = point[n - 1] + d_;
    if (std::fabs(denominator) < 1e-12) {
        QString msg = "Division by zero in PerspectiveProjection.";
        qWarning() << msg;
        throw std::runtime_error(msg.toStdString());
    }
    std::vector<double> result(n - 1);
    for (std::size_t i = 0; i < n - 1; ++i) {
        result[i] = (d_ * point[i]) / denominator;
    }
    return result;
}

std::shared_ptr<Projection> PerspectiveProjection::clone() const {
    return std::make_shared<PerspectiveProjection>(*this);
}

// OrthographicProjection
std::vector<double> OrthographicProjection::projectPoint(const std::vector<double>& point) const {
    std::size_t n = point.size();
    if (n <= 1) {
        QString msg = "Point dimension must be > 1 for OrthographicProjection.";
        qWarning() << msg;
        throw std::invalid_argument(msg.toStdString());
    }

    return std::vector<double>(point.begin(), point.end() - 1);
}

std::shared_ptr<Projection> OrthographicProjection::clone() const {
    return std::make_shared<OrthographicProjection>(*this);
}

// StereographicProjection
std::vector<double> StereographicProjection::projectPoint(const std::vector<double>& point) const {
    std::size_t n = point.size();
    if (n <= 1) {
        QString msg = "Point dimension must be > 1 for StereographicProjection.";
        qWarning() << msg;
        throw std::invalid_argument(msg.toStdString());
    }

    double denominator = 1.0 - point[n - 1];
    if (std::fabs(denominator) < 1e-12) {
        QString msg = "Division by zero in StereographicProjection.";
        qWarning() << msg;
        throw std::runtime_error(msg.toStdString());
    }
    std::vector<double> result(n - 1);
    for (std::size_t i = 0; i < n - 1; ++i) {
        result[i] = point[i] / denominator;
    }
    return result;
}


std::shared_ptr<Projection> StereographicProjection::clone() const {
    return std::make_shared<StereographicProjection>(*this);
}
