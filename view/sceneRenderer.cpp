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
    , cameraController_(std::make_unique<CameraController>())
    , geometryManager_(std::make_unique<SceneGeometryManager>())
    , inputHandler_(std::make_unique<SceneInputHandler>())
{
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(false);
    setCursor(Qt::BlankCursor);

    movementTimer_.setInterval(kCameraUpdateIntervalMs_);
    connect(&movementTimer_, &QTimer::timeout, this, &SceneRenderer::updateCamera);
    movementTimer_.start();

    connect(inputHandler_.get(), &SceneInputHandler::freeLookModeToggled, this,
            [this](bool enabled)
            {
                // setCursor(enabled ? Qt::BlankCursor : Qt::ArrowCursor);
                setMouseTracking(enabled);

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
    glClearColor(kClearColorR_, kClearColorG_, kClearColorB_, kClearColorA_);

    setupMainProgram();
    setupDepthProgram();
    initShadowFBO();

    geometryManager_->initialize();

    // Prerender scene shadow map
    renderShadowPass();

}

void SceneRenderer::setupMainProgram()
{
    program_ = std::make_unique<QOpenGLShaderProgram>();
    program_->addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/shaders/vertex_shader.glsl");
    program_->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/fragment_shader.glsl");
    program_->link();

    mvpMatrixLoc_        = program_->uniformLocation("uMvpMatrix");
    lightSpaceMatrixLoc_ = program_->uniformLocation("uLightSpaceMatrix");
    shadowMapLoc_        = program_->uniformLocation("uShadowMap");
}

void SceneRenderer::setupDepthProgram()
{
    depthProgram_ = std::make_unique<QOpenGLShaderProgram>();
    depthProgram_->addShaderFromSourceFile(QOpenGLShader::Vertex,   ":/shaders/shadow_vertex.glsl");
    depthProgram_->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/shadow_fragment.glsl");
    depthProgram_->link();

    depthMvpLoc_ = depthProgram_->uniformLocation("uLightSpaceMatrix");
}

void SceneRenderer::initShadowFBO()
{
    glGenFramebuffers(1, &depthMapFbo_);
    glGenTextures(1, &depthMapTex_);

    glBindTexture(GL_TEXTURE_2D, depthMapTex_);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 shadowMapSize_, shadowMapSize_, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFbo_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTex_, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
        qWarning() << "Shadow-map FBO is not complete! Status:" << status;

    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
}

void SceneRenderer::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
    centerScreenPos_ = mapToGlobal(QPoint(int(w * devicePixelRatio() / 2),
                                          int(h * devicePixelRatio() / 2)));
    inputHandler_->setWidgetCenter(centerScreenPos_);
}

void SceneRenderer::paintGL()
{
    // renderShadowPass();
    renderScenePass();
}

void SceneRenderer::renderShadowPass()
{
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFbo_);
    glViewport(0, 0, shadowMapSize_, shadowMapSize_);
    glClear(GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    depthProgram_->bind();

    QMatrix4x4 lightSpace = buildLightSpaceMatrix();
    depthProgram_->setUniformValue(depthMvpLoc_, lightSpace);

    geometryManager_->updateGeometry(true);
    geometryManager_->renderAll(depthProgram_.get());

    depthProgram_->release();

    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
    glEnable(GL_CULL_FACE);
}

void SceneRenderer::renderScenePass()
{
    glViewport(0, 0,
               static_cast<GLsizei>(width() * devicePixelRatio()),
               static_cast<GLsizei>(height() * devicePixelRatio()));
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    // Draw ovelay numbers
    QMatrix4x4 mvp = buildMvpMatrix();
    geometryManager_->updateAxes(cameraController_->position());
    geometryManager_->paintOverlayLabels(this, mvp);
    glEnable(GL_DEPTH_TEST);

    program_->bind();

    // ---------------------------
    // Vertex shader
    // ---------------------------

    program_->setUniformValue(mvpMatrixLoc_, mvp);

    QMatrix4x4 model;
    model.setToIdentity();
    program_->setUniformValue("uModelMatrix", model);

    // Build the same light-space matrix used in shadow pass, then apply bias:
    QMatrix4x4 lightSpace = buildLightSpaceMatrix();
    QMatrix4x4 biasMatrix;
    biasMatrix.translate(0.5f, 0.5f, 0.5f);
    biasMatrix.scale(0.5f, 0.5f, 0.5f);
    lightSpace = biasMatrix * lightSpace;
    program_->setUniformValue(lightSpaceMatrixLoc_, lightSpace);


    // ---------------------------
    // Fragment shader
    // ---------------------------

    // Lighting control uniforms
    program_->setUniformValue("uShininess",           shininess_);
    program_->setUniformValue("uAmbientStrength",     ambientStrength_);
    program_->setUniformValue("uSpecularStrength",    specularStrength_);
    program_->setUniformValue("uDirectionalStrength", directionalStrength_);
    program_->setUniformValue("uShadowLightStrength", shadowLightStrength_);
    program_->setUniformValue("uColorBlendFactor",    colorBlendFactor_);

    // Lighting properties uniforms
    program_->setUniformValue("uAmbientColor",        ambientColor_);
    program_->setUniformValue("uLightColor",          lightColor_);
    program_->setUniformValue("uShadowLightColor",    shadowLightColor_);

    // Light directions & positions uniforms
    program_->setUniformValue("uCameraForward",       cameraController_->forwardVector());
    program_->setUniformValue("uShadowDir",           shadowLightTarget_ - shadowLightPos_);
    program_->setUniformValue("uViewPos",             cameraController_->position());
    program_->setUniformValue("uShadowViewPos",       shadowLightPos_);

    // Shadow mapping uniforms
    program_->setUniformValue("uPcfKernelDim",        pcfKernelDim_);
    program_->setUniformValue("uShadowBiasScale",     shadowBiasScale_);
    program_->setUniformValue("uShadowBiasMin",       shadowBiasMin_);
    // Bind shadow map texture (unit 0)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthMapTex_);
    program_->setUniformValue(shadowMapLoc_, 0);

    geometryManager_->updateGeometry(false);
    geometryManager_->renderAll(program_.get());

    program_->release();
}


QMatrix4x4 SceneRenderer::buildLightSpaceMatrix() const
{
    QMatrix4x4 lightView;
    lightView.lookAt(shadowLightPos_, shadowLightTarget_, shadowLightUpDir_);

    QMatrix4x4 lightProj;
    lightProj.ortho(-shadowOrthographicSize_, shadowOrthographicSize_,
                    -shadowOrthographicSize_, shadowOrthographicSize_,
                    -shadowOrthographicSize_, shadowOrthographicSize_);


    return lightProj * lightView;
}

QMatrix4x4 SceneRenderer::buildMvpMatrix() const
{
    QMatrix4x4 projection;
    projection.perspective(kDefaultFovY_, float(width()) / float(height()),
                           kDefaultNearPlane_, kDefaultFarPlane_);

    const QVector3D camPos  = cameraController_->position();
    const QVector3D forward = cameraController_->forwardVector();
    const QVector3D up      = cameraController_->upVector();

    QMatrix4x4 view;
    view.lookAt(camPos, camPos + forward, up);

    return projection * view;
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
