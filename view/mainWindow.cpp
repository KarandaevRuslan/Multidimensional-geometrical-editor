#include "mainWindow.h"
#include "../view/sceneRenderer.h"
#include <QFile>
#include <QUiLoader>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent,
                       SceneRenderer* sceneRenderer)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    sceneRenderer_(sceneRenderer)
{
    // ui->setupUi(this);
    if (sceneRenderer_) {
        setCentralWidget(sceneRenderer_.get());
    }
}

MainWindow::~MainWindow()
{
    delete ui;
    qDebug() << "Main window destroyed";
}
