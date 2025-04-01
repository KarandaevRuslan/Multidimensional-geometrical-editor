#ifndef CAMERA_CONTROLLER_H
#define CAMERA_CONTROLLER_H

#include <QVector3D>

/**
 * @brief Manages camera position, orientation, and 3D vector calculations
 *        (pitch, yaw, forward/right/up vectors, etc.).
 */
class CameraController
{
public:
    /**
     * @brief Constructs a camera controller with default position and angles.
     */
    CameraController();

    /**
     * @brief Sets the camera position in world coordinates.
     * @param position The new position.
     */
    void setPosition(const QVector3D& position);

    /**
     * @brief Retrieves the camera's current position.
     * @return A reference to the camera position.
     */
    const QVector3D& position() const;

    /**
     * @brief Sets the camera's pitch (rotation around X-axis).
     * @param pitchDegrees The pitch angle in degrees.
     */
    void setPitch(float pitchDegrees);

    /**
     * @brief Gets the camera's current pitch angle in degrees.
     * @return The pitch angle.
     */
    float pitch() const;

    /**
     * @brief Sets the camera's yaw (rotation around Y-axis).
     * @param yawDegrees The yaw angle in degrees.
     */
    void setYaw(float yawDegrees);

    /**
     * @brief Gets the camera's current yaw angle in degrees.
     * @return The yaw angle.
     */
    float yaw() const;

    /**
     * @brief Moves the camera forward by a certain amount along its forward vector.
     * @param amount Distance to move.
     */
    void moveForward(float amount);

    /**
     * @brief Moves the camera horizontally along its right vector.
     * @param amount Distance to move.
     */
    void moveRight(float amount);

    /**
     * @brief Moves the camera along the world-up direction (0, 1, 0).
     * @param amount Distance to move up.
     */
    void moveUp(float amount);

    /**
     * @brief Zooms by moving the camera along its forward vector.
     * @param amount Zoom distance.
     */
    void zoom(float amount);

    /**
     * @brief Retrieves the camera's forward (look) vector, based on pitch and yaw.
     * @return A normalized forward vector.
     */
    QVector3D forwardVector() const;

    /**
     * @brief Retrieves the camera's right vector, derived via cross with worldUp.
     * @return A normalized right vector.
     */
    QVector3D rightVector() const;

    /**
     * @brief Retrieves the camera's up vector, derived via cross from right x forward.
     * @return A normalized up vector.
     */
    QVector3D upVector() const;

private:
    /**
     * @brief Clamps the pitch angle to prevent flipping (gimbal lock).
     */
    void clampPitch();

private:
    QVector3D cameraPos_;  ///< The camera’s current position.
    float pitch_;          ///< Pitch angle in degrees.
    float yaw_;            ///< Yaw angle in degrees.

    static constexpr float kMaxPitch = 89.0f;   ///< Upper limit for pitch.
    static constexpr float kMinPitch = -89.0f;  ///< Lower limit for pitch.
};

#endif // CAMERA_CONTROLLER_H
