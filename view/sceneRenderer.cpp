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
    depthProgram_.reset();
    if (depthMapFbo_ != 0)
        glDeleteFramebuffers(1, &depthMapFbo_);
    if (depthMapTex_ != 0)
        glDeleteTextures(1, &depthMapTex_);
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
    glDisable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glClearColor(kClearColorR, kClearColorG, kClearColorB, kClearColorA);

    // 1) Create + link your main program (already in your code):
    program_ = std::make_unique<QOpenGLShaderProgram>();
    program_->addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/shaders/vertex_shader.glsl");
    program_->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragment_shader.glsl");
    program_->link();

    // Cache uniform locations from main program
    mvpMatrixLoc_        = program_->uniformLocation("uMvpMatrix");
    lightSpaceMatrixLoc_ = program_->uniformLocation("uLightSpaceMatrix");
    shadowMapLoc_        = program_->uniformLocation("uShadowMap");
    // … you already have e.g. uniform locs for camera, lighting, etc.

    // 2) Initialize your geometry
    geometryManager_->initialize();

    // 3) Prepare the depth‐only “shadow” program
    depthProgram_ = std::make_unique<QOpenGLShaderProgram>();
    depthProgram_->addShaderFromSourceFile(
        QOpenGLShader::Vertex, ":/shaders/shadow_vertex.glsl");
    depthProgram_->addShaderFromSourceFile(
        QOpenGLShader::Fragment, ":/shaders/shadow_fragment.glsl");
    depthProgram_->link();

    // Cache depth program uniform location
    depthMvpLoc_ = depthProgram_->uniformLocation("uLightSpaceMatrix");

    // 4) Create the shadow-map FBO + texture
    glGenFramebuffers(1, &depthMapFbo_);
    glGenTextures(1, &depthMapTex_);
    glBindTexture(GL_TEXTURE_2D, depthMapTex_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 kShadowMapSize, kShadowMapSize, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // If you want to reduce artifacts at edges:
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFbo_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                           GL_TEXTURE_2D, depthMapTex_, 0);
    // We don’t render color here—just depth
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    // Check completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        qWarning() << "Shadow-map FBO is not complete!";
    }
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        qWarning() << "Shadow-map FBO is not complete! Status:" << status;
    } else {
        qDebug() << "Shadow-map FBO is complete!";
    }

    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
}

void SceneRenderer::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);

    centerScreenPos_ = mapToGlobal(QPoint(w * devicePixelRatio() / 2, h * devicePixelRatio() / 2));
    inputHandler_->setWidgetCenter(centerScreenPos_);
}

void SceneRenderer::paintGL()
{
    // 1) Render the scene from the light’s perspective into the depthMapFbo_
    renderShadowPass();

    // 2) Render the scene from the camera’s perspective (your existing main pass),
    //    but now we have a shadow map to sample from in the fragment shader.
    renderScenePass();

    // That’s it. Overlays remain intact as well.
}

void SceneRenderer::renderShadowPass()
{
    // Prep
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFbo_);

    glViewport(0, 0, kShadowMapSize, kShadowMapSize);

    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // Bind the “depth only” program
    depthProgram_->bind();

    QMatrix4x4 model;
    model.setToIdentity();
    depthProgram_->setUniformValue("uModelMatrix", model);

    // Construct the light-space MVP
    QMatrix4x4 lightSpace = buildLightSpaceMatrix();
    depthProgram_->setUniformValue(depthMvpLoc_, lightSpace);

    // Render your geometry (same geometry calls, just no color nor normal usage).
    geometryManager_->updateGeometry();
    geometryManager_->renderAll(depthProgram_.get());

    depthProgram_->release();

    // // Restore
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
    glEnable(GL_CULL_FACE);
}

void SceneRenderer::renderScenePass()
{
    glViewport(0, 0, width() * devicePixelRatio(), height() * devicePixelRatio());

    // Restore full window size
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    // 1) Overlays are still drawn in 2D after this pass, so we can keep or reorder.
    //    But if you want them on top, do them after geometry.


    QMatrix4x4 mvp = buildMvpMatrix();
    // paintOverlayLabels(mvp);
    // glEnable(GL_DEPTH_TEST);

    // 2) Prepare your main program (with lighting + shadow sampling).
    program_->bind();

    // a) Build your regular MVP from camera

    program_->setUniformValue(mvpMatrixLoc_, mvp);

    QMatrix4x4 model;
    model.setToIdentity();
    program_->setUniformValue("uModelMatrix", model);

    // b) Build the same light-space matrix used in shadow pass
    QMatrix4x4 lightSpace = buildLightSpaceMatrix();
    QMatrix4x4 biasMatrix;
    biasMatrix.setToIdentity();
    biasMatrix.translate(0.5f, 0.5f, 0.5f);
    biasMatrix.scale(0.5f, 0.5f, 0.5f);
    lightSpace = biasMatrix * lightSpace;

    program_->setUniformValue(lightSpaceMatrixLoc_, lightSpace);

    // c) Bind the shadow map to some texture unit, say GL_TEXTURE0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthMapTex_);
    program_->setUniformValue(shadowMapLoc_, 0); // “uShadowMap = texture unit 0”

    // d) Set other existing lighting uniforms

    program_->setUniformValue("uShadowDir", QVector3D(20, 20, 20));
    program_->setUniformValue("uCameraForward", cameraController_->forwardVector());
    program_->setUniformValue("uViewPos",       cameraController_->position());
    program_->setUniformValue("uLightColor",      QVector3D(1.0f, 1.0f, 1.0f));
    program_->setUniformValue("uAmbientColor",    QVector3D(1.0f, 1.0f, 1.0f));
    program_->setUniformValue("uAmbientStrength", 0.2f);
    program_->setUniformValue("uDirectionalStrength", 1.0f);
    program_->setUniformValue("uSpecularStrength", 0.5f);
    program_->setUniformValue("uShininess", 32.0f);

    // e) Render the scene
    geometryManager_->updateGeometry();
    geometryManager_->renderAll(program_.get());

    program_->release();

    // 3) Now draw your textual overlays (the axis labels, etc.)

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

QMatrix4x4 SceneRenderer::buildLightSpaceMatrix() const
{

    QVector3D lightPos(-20.0f, -20.0f, -20.0f);
    QVector3D target(0.0f, 0.0f, 0.0f);
    QMatrix4x4 lightView;
    lightView.lookAt(lightPos, target, QVector3D(0, 1, 0));

    // 4) Build an orthographic projection that encloses your scene.
    //    (For small scenes, ±some bounding region)
    QMatrix4x4 lightProj;
    float halfSize = 50.0f; // must be big enough to contain geometry
    lightProj.ortho(-halfSize, halfSize, -halfSize, halfSize, -halfSize, halfSize);


    QMatrix4x4 lightModel;
    lightModel.setToIdentity();


    // Комбинируем матрицы для получения light-space матрицы
    QMatrix4x4 lightSpace = lightProj * lightView * lightModel;

    return lightSpace;
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
