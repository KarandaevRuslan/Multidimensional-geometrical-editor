#include "sceneRenderer.h"
#include <QOpenGLShaderProgram>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPainter>
#include <QCursor>
#include <QtMath>

#include "../model/opengl/objectController/cameraController.h"
#include "../model/opengl/graphics/sceneGeometryManager.h"
#include "../model/opengl/input/sceneInputHandler.h"

SceneRenderer::SceneRenderer(QWidget* parent)
    : QOpenGLWidget(parent)
    , program_(nullptr)
    , cameraController_(std::make_unique<CameraController>())
    , geometryManager_(std::make_unique<SceneGeometryManager>())
    , inputHandler_(std::make_unique<SceneInputHandler>())
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
    setCursor(Qt::BlankCursor);

    movementTimer_.setInterval(kCameraUpdateIntervalMs);
    connect(&movementTimer_, &QTimer::timeout, this, &SceneRenderer::updateCamera);
    movementTimer_.start();

    connect(inputHandler_.get(), &SceneInputHandler::freeLookModeToggled,
            this, [this](bool enabled) {
                setCursor(enabled ? Qt::BlankCursor : Qt::ArrowCursor);
            });
}

SceneRenderer::~SceneRenderer()
{
    makeCurrent();
    program_.reset();
    doneCurrent();
}

void SceneRenderer::setScene(std::weak_ptr<Scene> scene)
{
    geometryManager_->setScene(scene);
    update();
}

void SceneRenderer::setSceneColorificator(std::weak_ptr<SceneColorificator> colorificator)
{
    geometryManager_->setSceneColorificator(colorificator);
    update();
}

void SceneRenderer::initializeGL()
{
    initializeOpenGLFunctions();

    glEnable(GL_DEPTH_TEST);
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glClearColor(kClearColorR, kClearColorG, kClearColorB, kClearColorA);

    // Create + link a shader program
    program_ = std::make_unique<QOpenGLShaderProgram>();
    program_->addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/shaders/vertex_shader.glsl");
    program_->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragment_shader.glsl");
    program_->link();

    // Cache uniform locations
    mvpMatrixLoc_        = program_->uniformLocation("uMvpMatrix");
    // Additional lighting uniforms (you can store them in class members if you like)
    // e.g.: lightDirLoc_ = program_->uniformLocation("uLightDirection");
    //       etc.

    geometryManager_->initialize();
}

void SceneRenderer::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    centerScreenPos_ = mapToGlobal(QPoint(width()/2, height()/2));
    inputHandler_->setWidgetCenter(centerScreenPos_);
}

void SceneRenderer::paintGL()
{
    glEnable(GL_DEPTH_TEST);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    // 1) Update geometry buffers from scene
    geometryManager_->updateGeometry();

    // 2) Build MVP
    QMatrix4x4 mvp = buildMvpMatrix();

    paintOverlayLabels(mvp);

    glEnable(GL_DEPTH_TEST);

    // 3) Bind the shader program
    program_->bind();

    // 4) Set the necessary uniforms:
    program_->setUniformValue(mvpMatrixLoc_, mvp);

    QMatrix4x4 model;
    model.setToIdentity();
    program_->setUniformValue("uModelMatrix", model);

    // Pass the camera forward vector; the shader will use its negative for lighting.
    program_->setUniformValue("uCameraForward", cameraController_->forwardVector());

    // Set the camera position for specular lighting calculations.
    program_->setUniformValue("uViewPos", cameraController_->position());

    // Set other lighting uniforms.
    program_->setUniformValue("uLightColor",      QVector3D(1.0f, 1.0f, 1.0f));
    program_->setUniformValue("uAmbientColor",    QVector3D(1.0f, 1.0f, 1.0f));
    program_->setUniformValue("uAmbientStrength", 0.2f);
    program_->setUniformValue("uDirectionalStrength", 1.0f);
    program_->setUniformValue("uSpecularStrength", 0.5f);
    program_->setUniformValue("uShininess", 32.0f);

    // 5) Render all geometry
    geometryManager_->renderAll(program_.get());

    program_->release();
}

void SceneRenderer::keyPressEvent(QKeyEvent* event)
{
    inputHandler_->keyPressEvent(event, *cameraController_);
    QOpenGLWidget::keyPressEvent(event);
}

void SceneRenderer::keyReleaseEvent(QKeyEvent* event)
{
    inputHandler_->keyReleaseEvent(event, *cameraController_);
    QOpenGLWidget::keyReleaseEvent(event);
}

void SceneRenderer::mouseMoveEvent(QMouseEvent* event)
{
    inputHandler_->mouseMoveEvent(event, *cameraController_);
    QOpenGLWidget::mouseMoveEvent(event);
}

void SceneRenderer::mouseDoubleClickEvent(QMouseEvent* event)
{
    inputHandler_->mouseDoubleClickEvent(event);
    QOpenGLWidget::mouseDoubleClickEvent(event);
}

void SceneRenderer::wheelEvent(QWheelEvent* event)
{
    inputHandler_->wheelEvent(event, *cameraController_);
    QOpenGLWidget::wheelEvent(event);
}

void SceneRenderer::updateCamera()
{
    inputHandler_->updateCamera(*cameraController_);
    update();
}

QMatrix4x4 SceneRenderer::buildMvpMatrix() const
{
    QMatrix4x4 projection;
    projection.perspective(kDefaultFovY, float(width())/float(height()),
                           kDefaultNearPlane, kDefaultFarPlane);

    const QVector3D camPos  = cameraController_->position();
    const QVector3D forward = cameraController_->forwardVector();
    const QVector3D up      = cameraController_->upVector();

    QMatrix4x4 view;
    view.lookAt(camPos, camPos + forward, up);

    QMatrix4x4 model;
    model.setToIdentity(); // or any per-object transform

    return projection * view * model;
}

void SceneRenderer::paintOverlayLabels(const QMatrix4x4 &mvp)
{
    QPainter painter(this);
    painter.setPen(Qt::black);
    painter.setFont(QFont("Arial", kOverlayFontSize));

    painter.beginNativePainting();
    painter.endNativePainting();

    // Just an example labeling: axis ticks from -20..+20
    auto labelIfVisible = [&](int i, const QVector3D &worldPos)
    {
        QPointF sp = projectToScreen(worldPos, mvp);
        if (sp.x() < 0.0f) return; // means off-screen
        painter.drawText(sp, QString::number(i));
    };

    // "0" at origin
    {
        QPointF sp = projectToScreen(QVector3D(0,0,0), mvp);
        if (sp.x() >= 0.0f) {
            painter.drawText(sp, "0");
        }
    }

    // X axis
    for (int i = -kAxisLabelRange; i <= kAxisLabelRange; ++i) {
        if (i == 0) continue;
        labelIfVisible(i, QVector3D(float(i), 0.0f, 0.0f));
    }
    // Y axis
    for (int i = -kAxisLabelRange; i <= kAxisLabelRange; ++i) {
        if (i == 0) continue;
        labelIfVisible(i, QVector3D(0.0f, float(i), 0.0f));
    }
    // Z axis
    for (int i = -kAxisLabelRange; i <= kAxisLabelRange; ++i) {
        if (i == 0) continue;
        labelIfVisible(i, QVector3D(0.0f, 0.0f, float(i)));
    }
}

QPointF SceneRenderer::projectToScreen(const QVector3D &point, const QMatrix4x4 &mvp) const
{
    QVector4D clip = mvp * QVector4D(point, 1.0f);

    if (clip.w() <= 0.0f) {
        return QPointF(kOffScreenCoord, kOffScreenCoord);
    }

    float ndcX = clip.x() / clip.w();
    float ndcY = clip.y() / clip.w();
    float ndcZ = clip.z() / clip.w();

    if (ndcX < -1.0f || ndcX > 1.0f ||
        ndcY < -1.0f || ndcY > 1.0f ||
        ndcZ < -1.0f || ndcZ > 1.0f)
    {
        return QPointF(kOffScreenCoord, kOffScreenCoord);
    }

    float sx = (ndcX * 0.5f + 0.5f) * float(width());
    float sy = (1.0f - (ndcY * 0.5f + 0.5f)) * float(height());
    return QPointF(sx, sy);
}
