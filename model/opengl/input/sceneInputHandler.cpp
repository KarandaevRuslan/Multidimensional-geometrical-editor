#include "sceneInputHandler.h"
#include "../objectController/cameraController.h"
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QCursor>

SceneInputHandler::SceneInputHandler(QObject *parent)
    : QObject(parent)
    , freeLookMode_(false)
    , forwardPressed_(false)
    , backwardPressed_(false)
    , mouseButtonPressed_(false)
    , leftPressed_(false)
    , rightPressed_(false)
    , upPressed_(false)
    , downPressed_(false)
    , shiftPressed_(false)
    , turnLeftPressed_(false)
    , turnRightPressed_(false)
    , turnUpPressed_(false)
    , turnDownPressed_(false)
    , rotationSpeed_(1.0f)
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
    if (enabled == freeLookMode_)
        return;

    freeLookMode_ = enabled;
    emit freeLookModeToggled(enabled);
}

void SceneInputHandler::keyPressEvent(QKeyEvent* event, CameraController& /*camera*/)
{
    // Shift+F toggles free-look
#ifdef Q_OS_WIN
    quint32 vk = event->nativeVirtualKey();
    if ((event->modifiers() & Qt::ShiftModifier) && vk == 0x46) { // VK = 'F'
        setFreeLookEnabled(!freeLookMode_);
        return;
    }
#elif defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    quint32 sc = event->nativeScanCode();
    if ((event->modifiers() & Qt::ShiftModifier) && sc == 33) {     // ScanCode F = 33
        setFreeLookEnabled(!freeLookMode_);
        return;
    }
#endif

    switch (event->key()){
        case Qt::Key_Left:    turnLeftPressed_  = true; break;
        case Qt::Key_Right:   turnRightPressed_ = true; break;
        case Qt::Key_Up:      turnUpPressed_    = true; break;
        case Qt::Key_Down:    turnDownPressed_  = true; break;
    }

    if (!freeLookMode_) {
        return;
    }

// W/A/S/D by hardware
#ifdef Q_OS_WIN
    if      (vk == 0x57)      forwardPressed_  = true; // 'W'
    else if (vk == 0x53)      backwardPressed_ = true; // 'S'
    else if (vk == 0x41)      leftPressed_     = true; // 'A'
    else if (vk == 0x44)      rightPressed_    = true; // 'D'
#elif defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    if      (sc == 17)        forwardPressed_  = true; // ScanCode W
    else if (sc == 31)        backwardPressed_ = true; // ScanCode S
    else if (sc == 30)        leftPressed_     = true; // ScanCode A
    else if (sc == 32)        rightPressed_    = true; // ScanCode D
#endif

    // Remaining keys
    if (event->key() == Qt::Key_Space)   upPressed_   = true;
    if (event->key() == Qt::Key_Control) downPressed_ = true;
    if (event->key() == Qt::Key_Shift)   shiftPressed_= true;
}

void SceneInputHandler::keyReleaseEvent(QKeyEvent* event, CameraController& /*camera*/)
{
    // W/A/S/D releasing
#ifdef Q_OS_WIN
    quint32 vk = event->nativeVirtualKey();
    if      (vk == 0x57)      forwardPressed_  = false;
    else if (vk == 0x53)      backwardPressed_ = false;
    else if (vk == 0x41)      leftPressed_     = false;
    else if (vk == 0x44)      rightPressed_    = false;
#elif defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    quint32 sc = event->nativeScanCode();
    if      (sc == 17)        forwardPressed_  = false;
    else if (sc == 31)        backwardPressed_ = false;
    else if (sc == 30)        leftPressed_     = false;
    else if (sc == 32)        rightPressed_    = false;
#endif

    // Arrow buttons and other
    switch (event->key()) {
    case Qt::Key_Space:   upPressed_       = false; break;
    case Qt::Key_Control: downPressed_     = false; break;
    case Qt::Key_Shift:   shiftPressed_    = false; break;
    case Qt::Key_Left:    turnLeftPressed_ = false; break;
    case Qt::Key_Right:   turnRightPressed_= false; break;
    case Qt::Key_Up:      turnUpPressed_   = false; break;
    case Qt::Key_Down:    turnDownPressed_ = false; break;
    default: break;
    }
}

bool SceneInputHandler::mouseMoveEvent(QMouseEvent* event, CameraController& camera)
{
    if (!isWindows || !freeLookMode_ && !mouseButtonPressed_)
        return false;

    const QPoint  globalPos = event->globalPosition().toPoint();

    int dx = globalPos.x() - centerScreenPos_.x();
    int dy = globalPos.y() - centerScreenPos_.y();
    if (dx == 0 && dy == 0)
        return false;

    camera.setYaw  (camera.yaw()   - dx * kMouseSensitivity);
    camera.setPitch(camera.pitch() - dy * kMouseSensitivity);

    QCursor::setPos(centerScreenPos_);

    emit cameraMoved();

    return true;
}

void SceneInputHandler::mousePressEvent(QMouseEvent* event)
{
    if (freeLookMode_) {
        return;
    }

    if (event->button() == Qt::LeftButton){
        mouseButtonPressed_ = true;
    }
}

void SceneInputHandler::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton){
        mouseButtonPressed_ = false;
    }
    QCursor::setPos(centerScreenPos_);
}

void SceneInputHandler::mouseDoubleClickEvent(QMouseEvent* event)
{
    // Double-click left button to enable free look
    if (event->button() == Qt::LeftButton && !freeLookMode_) {
        setFreeLookEnabled(true);
        mouseButtonPressed_ = false;
    }
}

void SceneInputHandler::wheelEvent(QWheelEvent* event, CameraController& camera)
{
    int delta = event->angleDelta().y();
    if (delta == 0) return;

    float steps = static_cast<float>(delta) / 120.0f;
    camera.zoom(zoomSpeed_ * steps);

    emit cameraMoved();
}

bool SceneInputHandler::updateCamera(CameraController &camera)
{
    // If no movement keys pressed, skip
    if (!forwardPressed_ && !backwardPressed_ &&
        !leftPressed_ && !rightPressed_ &&
        !upPressed_ && !downPressed_ &&
        !turnLeftPressed_ && !turnRightPressed_ &&
        !turnUpPressed_ && !turnDownPressed_)
        return false;

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

    if (turnLeftPressed_)  camera.setYaw  (camera.yaw()   + rotationSpeed_);
    if (turnRightPressed_) camera.setYaw  (camera.yaw()   - rotationSpeed_);
    if (turnUpPressed_)    camera.setPitch(camera.pitch() + rotationSpeed_);
    if (turnDownPressed_)  camera.setPitch(camera.pitch() - rotationSpeed_);

    emit cameraMoved();

    return true;
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
