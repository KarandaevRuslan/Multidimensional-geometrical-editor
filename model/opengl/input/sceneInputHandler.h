#ifndef SCENE_INPUT_HANDLER_H
#define SCENE_INPUT_HANDLER_H

#include <QObject>
#include <QPoint>

class QKeyEvent;
class QMouseEvent;
class QWheelEvent;
class CameraController;

/**
 * @brief Encapsulates all keyboard and mouse input. Applies camera movements and toggles free-look mode.
 */
class SceneInputHandler : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Constructs an input handler with default settings.
     * @param parent Optional parent QObject.
     */
    explicit SceneInputHandler(QObject* parent = nullptr);

    /**
     * @brief Indicates if free-look mode (FPS-style camera) is currently enabled.
     * @return True if free-look is enabled; otherwise false.
     */
    bool freeLookEnabled() const;

    /**
     * @brief Enables or disables free-look mode.
     * @param enabled True to enable, false to disable.
     */
    void setFreeLookEnabled(bool enabled);

    /**
     * @brief Handles key press events and updates camera movement states.
     * @param event   A pointer to the QKeyEvent.
     * @param camera  Reference to a CameraController to be manipulated.
     */
    void keyPressEvent(QKeyEvent* event, CameraController& camera);

    /**
     * @brief Handles key release events and updates camera movement states.
     * @param event   A pointer to the QKeyEvent.
     * @param camera  Reference to a CameraController to be manipulated.
     */
    void keyReleaseEvent(QKeyEvent* event, CameraController& camera);

    /**
     * @brief Handles mouse move events for free-look rotation.
     * @param event   A pointer to the QMouseEvent.
     * @param camera  Reference to a CameraController to be manipulated.
     */
    void mouseMoveEvent(QMouseEvent* event, CameraController& camera);

    /**
     * @brief Handles mouse press events and updates.
     * @param event   A pointer to the QMouseEvent.
     */
    void mousePressEvent(QMouseEvent* event);

    /**
     * @brief Handles mouse release events.
     * @param event   A pointer to the QMouseEvent.
     */
    void mouseReleaseEvent(QMouseEvent* event);

    /**
     * @brief Handles mouse double-click events (e.g., toggling free-look on).
     * @param event   A pointer to the QMouseEvent.
     */
    void mouseDoubleClickEvent(QMouseEvent* event);

    /**
     * @brief Handles mouse wheel events for camera zoom.
     * @param event   A pointer to the QWheelEvent.
     * @param camera  Reference to a CameraController to be manipulated.
     */
    void wheelEvent(QWheelEvent* event, CameraController& camera);

    /**
     * @brief Called periodically (e.g., via timer) to update camera movement smoothly.
     * @param camera  Reference to a CameraController to be manipulated.
     */
    void updateCamera(CameraController& camera);

    /**
     * @brief Sets the widget center (in global coordinates) for recentering the mouse in free-look.
     * @param globalCenterPos Center point in global coordinates.
     */
    void setWidgetCenter(const QPoint& globalCenterPos);

    /**
     * @brief Sets the camera movement speed.
     * @param speed Movement speed factor.
     */
    void setMoveSpeed(float speed);

    /**
     * @brief Sets the camera zoom speed.
     * @param speed Zoom speed factor.
     */
    void setZoomSpeed(float speed);

signals:
    /**
     * @brief Emitted when free-look mode is toggled on or off.
     * @param enabled True if free-look is enabled.
     */
    void freeLookModeToggled(bool enabled);

    void cameraMoved();

private:
    bool freeLookMode_;       ///< Indicates whether free-look mode is active
    bool ignoreNextMouseMove_;
    QPoint centerScreenPos_;  ///< Stores the center position of the widget for mouse warping

    // Key states
    bool forwardPressed_;
    bool backwardPressed_;
    bool mouseButtonPressed_;
    bool leftPressed_;
    bool rightPressed_;
    bool upPressed_;
    bool downPressed_;
    bool shiftPressed_;

    float moveSpeed_;         ///< Movement speed for WASD
    float zoomSpeed_;         ///< Movement speed for zoom

    // For mouse look
    static constexpr float kMouseSensitivity = 0.3f; ///< Mouse sensitivity for free-look
};

#endif // SCENE_INPUT_HANDLER_H
