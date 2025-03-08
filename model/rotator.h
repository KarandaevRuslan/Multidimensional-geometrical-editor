#ifndef ROTATOR_H
#define ROTATOR_H

#include <cstddef>
#include "NDShape.h"

/**
 * @brief Rotator is a utility class that provides non-mutating rotation of NDShape objects
 *        in planes determined by any pair of coordinate axes.
 */
class Rotator {
public:
    /**
     * @brief Rotates the given NDShape in the specified plane by a given angle, returning a new shape.
     *
     * @param shape The original NDShape to be rotated.
     * @param axis1 Index of one axis of the rotation plane.
     * @param axis2 Index of the other axis of the rotation plane.
     * @param angle The rotation angle in radians.
     * @return A new NDShape instance reflecting the rotation in the specified plane.
     *
     * @throws std::invalid_argument If axis1 or axis2 are out of range, or if axis1 == axis2.
     */
    NDShape rotateInPlane(const NDShape& shape, std::size_t axis1, std::size_t axis2, double angle);
};

#endif // ROTATOR_H
