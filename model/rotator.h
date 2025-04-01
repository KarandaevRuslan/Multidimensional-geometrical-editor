#ifndef ROTATOR_H
#define ROTATOR_H

#include <cstddef>
#include "NDShape.h"

/**
 * @brief The Rotator class encapsulates a rotation transformation.
 *
 * This class stores a rotation angle (in radians) and two coordinate axes that define the plane of rotation.
 * It provides a member function to apply the stored rotation to an NDShape object.
 */
class Rotator {
public:
    /**
     * @brief Constructs a Rotator with the specified axes and rotation angle.
     *
     * @param axis1 Index of the first axis defining the rotation plane.
     * @param axis2 Index of the second axis defining the rotation plane.
     * @param angle Rotation angle in radians.
     */
    Rotator(std::size_t axis1, std::size_t axis2, double angle);

    /**
     * @brief Destructor for the Rotator class.
     */
    ~Rotator() = default;

    /**
     * @brief Applies the stored rotation transformation to the provided NDShape.
     *
     * @param shape The NDShape to be rotated.
     * @return A new NDShape instance reflecting the applied rotation.
     *
     * @throws std::invalid_argument If either axis index is out of range or if both axes are identical.
     */
    NDShape applyRotation(const NDShape& shape) const;

private:
    std::size_t axis1_;
    std::size_t axis2_;
    double angle_;
};

#endif // ROTATOR_H
