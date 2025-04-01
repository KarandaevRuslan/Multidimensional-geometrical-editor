#include "cameraController.h"
#include <QtMath>

CameraController::CameraController()
    : cameraPos_(8.0f, 8.0f, 8.0f)
    , pitch_(-30.0f)
    , yaw_(225.0f)
{
}

void CameraController::setPosition(const QVector3D &position)
{
    cameraPos_ = position;
}

const QVector3D& CameraController::position() const
{
    return cameraPos_;
}

void CameraController::setPitch(float pitchDegrees)
{
    pitch_ = pitchDegrees;
    clampPitch();
}

float CameraController::pitch() const
{
    return pitch_;
}

void CameraController::setYaw(float yawDegrees)
{
    yaw_ = yawDegrees;
}

float CameraController::yaw() const
{
    return yaw_;
}

void CameraController::moveForward(float amount)
{
    cameraPos_ += forwardVector() * amount;
}

void CameraController::moveRight(float amount)
{
    cameraPos_ += rightVector() * amount;
}

void CameraController::moveUp(float amount)
{
    // Move along world up
    cameraPos_ += QVector3D(0.0f, 1.0f, 0.0f) * amount;
}

void CameraController::zoom(float amount)
{
    moveForward(amount);
}

QVector3D CameraController::forwardVector() const
{
    float pitchRad = qDegreesToRadians(pitch_);
    float yawRad   = qDegreesToRadians(yaw_);

    float cosP = std::cos(pitchRad);
    float sinP = std::sin(pitchRad);
    float cosY = std::cos(yawRad);
    float sinY = std::sin(yawRad);

    QVector3D forward(cosP * sinY, sinP, cosP * cosY);
    return forward.normalized();
}

QVector3D CameraController::rightVector() const
{
    // Cross with worldUp
    const QVector3D worldUp(0.0f, 1.0f, 0.0f);
    QVector3D right = QVector3D::crossProduct(forwardVector(), worldUp).normalized();
    return right;
}

QVector3D CameraController::upVector() const
{
    // up = right x forward
    return QVector3D::crossProduct(rightVector(), forwardVector()).normalized();
}

void CameraController::clampPitch()
{
    if (pitch_ > kMaxPitch)  pitch_ = kMaxPitch;
    if (pitch_ < kMinPitch)  pitch_ = kMinPitch;
}
