#include "presenterMain.h"
#include "view/mainwindow.h"
#include "model/scene.h"
#include "model/sceneColorificator.h"
#include <QApplication>
#include <QSurfaceFormat>
#include <QScreen>
#include <QCommandLineParser>
#include <QCommandLineOption>

PresenterMain::PresenterMain(MainWindow* mainWindow,
              Scene* scene,
              SceneColorificator* sceneColorificator)
    : mainWindow_(mainWindow), scene_(scene), sceneColorificator_(sceneColorificator)
{
    mainWindow_->sceneRenderer_->setScene(scene_);
    mainWindow_->sceneRenderer_->setSceneColorificator(sceneColorificator_);

    mainWindow_->resize(QSize(720,480));
    int desktopArea = QGuiApplication::primaryScreen()->size().width() *
                      QGuiApplication::primaryScreen()->size().height();
    int widgetArea = mainWindow_->width() * mainWindow_->height();
    if (((float)widgetArea / (float)desktopArea) < 0.75f)
        mainWindow_->show();
    else
        mainWindow_->showMaximized();
}

PresenterMain::~PresenterMain(){
    qDebug() << "Presenter main destroyed";
    qDebug() << mainWindow_.use_count();
}
