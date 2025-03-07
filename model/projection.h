#ifndef PROJECTION_H
#define PROJECTION_H

#include "NDShape.h"
#include <vector>
#include <stdexcept>

/**
 * @brief Base interface for projecting an n-dimensional point to (n-1) dimensions
 *        and thereby reducing an NDShape by one dimension at a time.
 */
class Projection {
public:
    virtual ~Projection() = default;

    /**
     * @brief Projects a single n-dimensional point to (n-1) dimensions.
     * @param point The n-dimensional point.
     * @return The projected (n-1)-dimensional point.
     * @throws std::invalid_argument If point.size() <= 1.
     */
    virtual std::vector<double> projectPoint(const std::vector<double>& point) const = 0;

    /**
     * @brief Projects the entire NDShape from dimension n to (n-1) using projectPoint().
     *
     * @param shape The NDShape to project.
     * @return A new NDShape with dimension = shape.getDimension() - 1.
     * @throws std::invalid_argument If shape.getDimension() <= 1.
     */
    NDShape projectShape(const NDShape& shape) const;

    /**
     * @brief Iteratively projects the given NDShape until it reaches the specified target dimension.
     *
     * @param shape The NDShape to reduce.
     * @param targetDim The dimension to stop at. Must be >= 1.
     * @return A new NDShape whose dimension is the minimum of shape.getDimension() and targetDim.
     * @throws std::invalid_argument If targetDim = 0.
     */
    NDShape projectShapeToDimension(const NDShape& shape, std::size_t targetDim) const;
};

/**
 * @brief PerspectiveProjection reduces dimension by applying a perspective formula.
 *
 * The parameter distance (d_) is used in the formula:
 *   pᵢ = (d * xᵢ) / (xₙ + d)
 * for i = 1..(n-1).
 */
class PerspectiveProjection : public Projection {
public:
    /**
     * @brief Constructs a PerspectiveProjection with the given distance.
     * @param distance The distance parameter in the perspective formula.
     */
    explicit PerspectiveProjection(double distance);

    /**
     * @brief Implements projectPoint for perspective projection.
     * @param point The n-dimensional point.
     * @return The (n-1)-dimensional result.
     * @throws std::runtime_error If the denominator (xₙ + d) is zero or extremely close to zero.
     */
    std::vector<double> projectPoint(const std::vector<double>& point) const override;

private:
    double d_;
};

/**
 * @brief OrthographicProjection drops the last coordinate to reduce dimension by 1.
 */
class OrthographicProjection : public Projection {
public:
    OrthographicProjection() = default;
    std::vector<double> projectPoint(const std::vector<double>& point) const override;
};

/**
 * @brief StereographicProjection uses (x₁, …, xₙ) -> (xᵢ / (1 - xₙ)), i=1..(n-1).
 */
class StereographicProjection : public Projection {
public:
    StereographicProjection() = default;
    std::vector<double> projectPoint(const std::vector<double>& point) const override;
};

#endif // PROJECTION_H
