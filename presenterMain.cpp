#include "presenterMain.h"
#include "view/mainwindow.h"
#include "view/mainWindowTabWidget.h"
#include "model/scene.h"
#include "model/sceneColorificator.h"
#include <QApplication>
#include <QSurfaceFormat>
#include <QScreen>
#include <QCommandLineParser>
#include <QCommandLineOption>

static void setupScene(std::shared_ptr<Scene> scene);

PresenterMain::PresenterMain(MainWindow* mainWindow)
    : mainWindow_(mainWindow)
{
    // Initialize the shared clipboard variables.
    copyBuffer_ = std::make_shared<SceneObject>();
    copyColor_ = std::make_shared<QColor>(SceneColorificator::defaultColor);

    // Create the shared delegate using the main window as its parent.
    sharedDelegate_ = std::make_shared<SceneObjectDelegate>(mainWindow_);

    // Inform the main window of the presenter.
    mainWindow_->setPresenterMain(this);

    // Basic window sizing logic remains.
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
    qDebug() << "Presenter main died";
}

void PresenterMain::createNewTab() {
    // Create a new MainWindowTabWidget.
    auto tabWidget = new MainWindowTabWidget();

    // Pass the common scene and colorificator to the tab widget.
    std::shared_ptr<Scene> scene = std::make_shared<Scene>();
    ::setupScene(scene);
    tabWidget->setScene(scene);

    std::shared_ptr<SceneColorificator> sceneColorificator = std::make_shared<SceneColorificator>();
    sceneColorificator->setColorForObject(2, QColor(160,60,61));
    sceneColorificator->setColorForObject(1, QColor(28,98,15));
    tabWidget->setSceneColorificator(sceneColorificator);

    // Pass the shared delegate.
    tabWidget->setDelegate(sharedDelegate_);

    // Create the sub-presenter for this tab.
    auto tabPresenter = std::make_shared<PresenterMainTab>(tabWidget, scene, sceneColorificator, this);
    tabWidget->setPresenterMainTab(tabPresenter);

    // Store the sub-presenter.
    tabPresenters_.push_back(tabPresenter);

    // Add the tab widget to the main window’s tab container.
    QString tabLabel = QString(QObject::tr("Untitled") + " - %1").arg(mainWindow_->getTabWidget()->count() + 1);
    mainWindow_->getTabWidget()->addTab(tabWidget, tabLabel);
}

void PresenterMain::removeTab(int index) {
    QTabWidget* tabs = mainWindow_->getTabWidget();
    QWidget* widgetToRemove = tabs->widget(index);

    // First: Remove the PresenterMainTab that owns this widget
    tabPresenters_.erase(
        std::remove_if(tabPresenters_.begin(), tabPresenters_.end(),
                       [widgetToRemove](const std::shared_ptr<PresenterMainTab>& presenter) {
                           return presenter->getTabWidget() == widgetToRemove;
                       }),
        tabPresenters_.end()
        );

    // Second: Remove tab from UI
    tabs->removeTab(index);

    // Third: Delete the actual widget (we own it via shared_ptr, but QTabWidget doesn't delete it)
    delete widgetToRemove;
}


// --- Implementation of PresenterMainTab ---

PresenterMainTab::PresenterMainTab(MainWindowTabWidget* tabWidget,
                                   std::shared_ptr<Scene> scene,
                                   std::shared_ptr<SceneColorificator> sceneColorificator,
                                   PresenterMain* parent)
    : tabWidget_(tabWidget),
    scene_(scene),
    sceneColorificator_(sceneColorificator),
    parent_(parent)
{
}

PresenterMainTab::~PresenterMainTab() {
    qDebug() << "PresenterMainTab destroyed";
}

PresenterMain* PresenterMainTab::getParentPresenter() const {
    if (!parent_){
        qDebug() << "Parent main presenter is nullptr";
    }

    return parent_;
}

QWidget* PresenterMainTab::getTabWidget() const {
    return tabWidget_;
}

static void setupScene(std::shared_ptr<Scene> scene) {
    try {
        scene->setSceneDimension(3);

        std::shared_ptr<NDShape> tesseract = std::make_shared<NDShape>(4);
        std::vector<std::size_t> tesseractVertices;
        for (int i = 0; i < 16; ++i) {
            double coord0 = (i & 1) ? 1.0 : -1.0;
            double coord1 = (i & 2) ? 1.0 : -1.0;
            double coord2 = (i & 4) ? 1.0 : -1.0;
            double coord3 = (i & 8) ? 1.0 : -1.0;
            tesseractVertices.push_back(tesseract->addVertex({coord0, coord1, coord2, coord3}));
        }
        for (int i = 0; i < 16; ++i) {
            for (int j = 0; j < 4; ++j) {
                int neighbor = i ^ (1 << j);
                if (i < neighbor) {
                    tesseract->addEdge(tesseractVertices[i], tesseractVertices[neighbor]);
                }
            }
        }
        std::shared_ptr<Projection> perspectiveProj = std::make_shared<PerspectiveProjection>(15.0);
        Rotator rotatorTesseract(0, 1, 0.5);
        scene->addObject(1, "Tesseract", tesseract, perspectiveProj, {rotatorTesseract}, {2, 2, 2}, {});

        std::shared_ptr<NDShape> simplex5D = std::make_shared<NDShape>(5);
        std::vector<std::size_t> simplexVertices;
        std::vector<std::vector<double>> coords = {
            { 1.0,  0.0,  0.0,  0.0,  0.0},
            { 0.0,  1.0,  0.0,  0.0,  0.0},
            { 0.0,  0.0,  1.0,  0.0,  0.0},
            { 0.0,  0.0,  0.0,  1.0,  0.0},
            { 0.0,  0.0,  0.0,  0.0,  1.0},
            {-1.0, -1.0, -1.0, -1.0, -1.0}
        };
        for (const auto& vertex : coords) {
            simplexVertices.push_back(simplex5D->addVertex(vertex));
        }
        for (size_t i = 0; i < simplexVertices.size(); ++i) {
            for (size_t j = i + 1; j < simplexVertices.size(); ++j) {
                simplex5D->addEdge(simplexVertices[i], simplexVertices[j]);
            }
        }
        Rotator rotatorSimplex(1, 2, 0.3);
        scene->addObject(2, "Simplex5D", simplex5D, perspectiveProj, {rotatorSimplex}, {3, 3, 3}, {5, 5, 5});
    } catch (const std::exception& ex) {
        qFatal() << "Exception occurred: " << ex.what();
    }
}
