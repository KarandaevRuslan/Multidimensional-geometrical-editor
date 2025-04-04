#include "sceneInputHandler.h"
#include "../objectController/cameraController.h"
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QCursor>

SceneInputHandler::SceneInputHandler(QObject *parent)
    : QObject(parent)
    , freeLookMode_(false)
    , firstMouseMove_(true)
    , forwardPressed_(false)
    , backwardPressed_(false)
    , leftPressed_(false)
    , rightPressed_(false)
    , upPressed_(false)
    , downPressed_(false)
    , shiftPressed_(false)
    , moveSpeed_(0.5f)
    , zoomSpeed_(0.5f)
{
}

bool SceneInputHandler::freeLookEnabled() const
{
    return freeLookMode_;
}

void SceneInputHandler::setFreeLookEnabled(bool enabled)
{
    if (enabled == freeLookMode_) return;

    freeLookMode_ = enabled;
    firstMouseMove_ = true;  // next mouse move is ignored for large warp
    emit freeLookModeToggled(enabled);
}

void SceneInputHandler::keyPressEvent(QKeyEvent* event, CameraController& /*camera*/)
{
    // Alt+F toggles free-look
    if ((event->modifiers() & Qt::AltModifier) && event->key() == Qt::Key_F) {
        setFreeLookEnabled(!freeLookMode_);
        return;
    }

    if (!freeLookMode_) {
        return;
    }

    switch (event->key()) {
    case Qt::Key_W:       forwardPressed_  = true; break;
    case Qt::Key_S:       backwardPressed_ = true; break;
    case Qt::Key_A:       leftPressed_     = true; break;
    case Qt::Key_D:       rightPressed_    = true; break;
    case Qt::Key_Space:   upPressed_       = true; break;
    case Qt::Key_Control: downPressed_     = true; break;
    case Qt::Key_Shift:   shiftPressed_    = true; break;
    default:
        break;
    }
}

void SceneInputHandler::keyReleaseEvent(QKeyEvent* event, CameraController& /*camera*/)
{
    switch (event->key()) {
    case Qt::Key_W:       forwardPressed_  = false; break;
    case Qt::Key_S:       backwardPressed_ = false; break;
    case Qt::Key_A:       leftPressed_     = false; break;
    case Qt::Key_D:       rightPressed_    = false; break;
    case Qt::Key_Space:   upPressed_       = false; break;
    case Qt::Key_Control: downPressed_     = false; break;
    case Qt::Key_Shift:   shiftPressed_    = false; break;
    default:
        break;
    }
}

void SceneInputHandler::mouseMoveEvent(QMouseEvent* event, CameraController& camera)
{
    // if (!freeLookMode_) {
    //     return;
    // }

    if (firstMouseMove_) {
        // Skip first move (avoid large jump)
        QCursor::setPos(centerScreenPos_);
        firstMouseMove_ = false;
        return;
    }

    int dx = event->globalX() - centerScreenPos_.x();
    int dy = event->globalY() - centerScreenPos_.y();

    if (dx == 0 && dy == 0) {
        // Possibly a warp back to center
        return;
    }

    float newYaw   = camera.yaw()   - dx * kMouseSensitivity;
    float newPitch = camera.pitch() - dy * kMouseSensitivity;

    camera.setYaw(newYaw);
    camera.setPitch(newPitch);

    // Warp the mouse back to center
    QCursor::setPos(centerScreenPos_);
}

void SceneInputHandler::mouseDoubleClickEvent(QMouseEvent* event)
{
    // Double-click left button to enable free look
    if (event->button() == Qt::LeftButton && !freeLookMode_) {
        setFreeLookEnabled(true);
    }
}

void SceneInputHandler::wheelEvent(QWheelEvent* event, CameraController& camera)
{
    int delta = event->angleDelta().y();
    if (delta == 0) return;

    float steps = static_cast<float>(delta) / 120.0f;
    camera.zoom(zoomSpeed_ * steps);
}

void SceneInputHandler::updateCamera(CameraController &camera)
{
    // If no movement keys pressed, skip
    if (!forwardPressed_ && !backwardPressed_ &&
        !leftPressed_ && !rightPressed_ &&
        !upPressed_ && !downPressed_) {
        return;
    }

    // SHIFT => sprint
    float actualSpeed = (shiftPressed_) ? (moveSpeed_ * 2.0f) : moveSpeed_;

    // Forward/backward
    if (forwardPressed_)  camera.moveForward(actualSpeed);
    if (backwardPressed_) camera.moveForward(-actualSpeed);

    // Left/right
    if (leftPressed_)     camera.moveRight(-actualSpeed);
    if (rightPressed_)    camera.moveRight(actualSpeed);

    // Up/down
    if (upPressed_)       camera.moveUp(actualSpeed);
    if (downPressed_)     camera.moveUp(-actualSpeed);
}

void SceneInputHandler::setWidgetCenter(const QPoint &globalCenterPos)
{
    centerScreenPos_ = globalCenterPos;
}

void SceneInputHandler::setMoveSpeed(float speed)
{
    moveSpeed_ = speed;
}

void SceneInputHandler::setZoomSpeed(float speed)
{
    zoomSpeed_ = speed;
}
