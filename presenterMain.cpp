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
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonDocument>
#include "tools/sceneSerialization.h"

static void setupScene(std::shared_ptr<Scene> scene,
                       std::shared_ptr<SceneColorificator> sceneColorificator);

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
    mainWindow_->resize(QSize(936, 624));
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

bool PresenterMain::hasDirtyTabs() const {
    for (auto& p : tabPresenters_) {
        if (p->isDirty()) return true;
    }
    return false;
}

void PresenterMain::createNewTab() {
    // Create a new MainWindowTabWidget.
    auto tabWidget = new MainWindowTabWidget();

    // Pass the common scene and colorificator to the tab widget.
    std::shared_ptr<Scene> scene = std::make_shared<Scene>();
    std::shared_ptr<SceneColorificator> sceneColorificator = std::make_shared<SceneColorificator>();
    ::setupScene(scene, sceneColorificator);

    // Pass the shared delegate.
    tabWidget->setDelegate(sharedDelegate_);

    // Create the sub-presenter for this tab.
    auto tabPresenter = std::make_shared<PresenterMainTab>(tabWidget, scene, sceneColorificator, this);
    tabWidget->setPresenterMainTab(tabPresenter);
    tabWidget->setScene(scene);
    tabWidget->setSceneColorificator(sceneColorificator);

    // Store the sub-presenter.
    tabPresenters_.push_back(tabPresenter);

    QString tabLabel = QString(QObject::tr("Untitled"));
    // Add the tab widget to the main window’s tab container.
    mainWindow_->getTabWidget()->addTab(tabWidget, tabLabel);
    mainWindow_->getTabWidget()->setCurrentWidget(tabWidget);
}

void PresenterMain::removeTab(int index) {
    QTabWidget* tabs = mainWindow_->getTabWidget();
    QWidget* widgetToRemove = tabs->widget(index);

    if (auto p = presenterFor(widgetToRemove); p && p->isDirty()) {
        QString name = tabs->tabText(index);
        auto btn = QMessageBox::warning(
            mainWindow_,
            QObject::tr("Unsaved Changes"),
            QObject::tr("The tab \"%1\" has unsaved changes.\n"
                "Do you want to save your changes before closing?").arg(name),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (btn == QMessageBox::Save && !p->save(false, mainWindow_))
            return;
        else if (btn == QMessageBox::Cancel) {
            return;
        }
   }

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

// --------------------- PresenterMain helpers ---------------------------
void PresenterMain::saveCurrentTab(bool saveAs)
{
    QWidget* current = mainWindow_->getTabWidget()->currentWidget();
    if (auto p = presenterFor(current))
        p->save(saveAs, mainWindow_);
}

void PresenterMain::openSceneInNewTab()
{
    QString fn = QFileDialog::getOpenFileName(mainWindow_, QObject::tr("Open scene"), QString(),
                                              QObject::tr("JSON files (*.json)"));
    if (fn.isEmpty()) return;

    std::shared_ptr<Scene>             scene   = std::make_shared<Scene>();
    std::shared_ptr<SceneColorificator> colors = std::make_shared<SceneColorificator>();

    // attempt to load file ------------------------------------------------
    if (!loadFromFile(fn, scene, colors)) {
        return;
    }
    auto tabWidget = new MainWindowTabWidget();
    auto tabPresenter = std::make_shared<PresenterMainTab>(tabWidget, scene, colors, this);
    tabWidget->setPresenterMainTab(tabPresenter);
    tabPresenter->markSaved(fn);

    // Delegate & UI wiring ----------------------------------------------
    tabWidget->setScene(scene);
    tabWidget->setSceneColorificator(colors);
    tabWidget->setDelegate(sharedDelegate_);

    tabPresenters_.push_back(tabPresenter);
    QString base = QFileInfo(fn).completeBaseName();
    mainWindow_->getTabWidget()->addTab(tabWidget, base);
    mainWindow_->getTabWidget()->setCurrentWidget(tabWidget);
}

bool PresenterMain::loadFromFile(
    const QString& fn,
    std::shared_ptr<Scene> scene,
    std::shared_ptr<SceneColorificator> sceneColorificator)
{
    QFile f(fn);
    if (!f.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(mainWindow_, QObject::tr("Open scene"),
                              QObject::tr("Cannot open %1").arg(fn));
        return false;
    }

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) {
        QMessageBox::critical(mainWindow_, QObject::tr("Open scene"),
                              QObject::tr("%1 is not a valid scene").arg(fn));
        return false;
    }
    f.close();

    SceneSerializer::fromJson(doc, *scene, *sceneColorificator);
    return true;
}

std::shared_ptr<PresenterMainTab> PresenterMain::presenterFor(QWidget* w) const {
    auto it = std::find_if(tabPresenters_.begin(), tabPresenters_.end(),
                           [w](auto& p){ return p->getTabWidget() == w; });
    return it == tabPresenters_.end() ? nullptr : *it;
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

bool PresenterMainTab::isDirty() const
{
    return isDirty_;
}

void PresenterMainTab::updateLabel()
{
    int idx = parent_->getMainWindow()
    ->getTabWidget()->indexOf(tabWidget_);
    if (idx != -1) {
        QString t = baseName_;
        if (isDirty_) t += " *";
        parent_->getMainWindow()->getTabWidget()->setTabText(idx, t);
    }
}

void PresenterMainTab::markDirty()
{
    if (!isDirty_) {
        isDirty_ = true;
        updateLabel();
    }
}
void PresenterMainTab::markSaved(QString filePath)
{
    if (!filePath.isEmpty()) {
        baseName_ = QFileInfo(filePath).completeBaseName();
        filePath_ = filePath;
    }
    else {
        qWarning() << "We can not mark \"saved\" empty file.";
        return;
    }
    isDirty_ = false;
    updateLabel();
}

// ----------------- PresenterMainTab save / load ------------------------
bool PresenterMainTab::save(bool saveAs, QWidget* parentWindow)
{
    QString fn = filePath_;
    if (saveAs || filePath_.isEmpty()) {
        fn = QFileDialog::getSaveFileName(
            parentWindow, QObject::tr("Save scene"),
            filePath_.isEmpty()? baseName_ + ".json" : filePath_,
            QObject::tr("JSON files (*.json)"));
        if (fn.isEmpty()) return false;
    }

    QFile f(fn);
    if (!f.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(parentWindow, QObject::tr("Save scene"),
                              QObject::tr("Cannot write to %1").arg(fn));
        return false;
    }
    QJsonDocument doc = SceneSerializer::toJson(*scene_, *sceneColorificator_);
    f.write(doc.toJson(QJsonDocument::Indented)); f.close();
    markSaved(fn);
    return true;
}


static void setupScene(std::shared_ptr<Scene> scene,
                       std::shared_ptr<SceneColorificator> sceneColorificator) {
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
        auto uid1 = scene->addObject(QUuid::createUuid(), 1, "Tesseract", tesseract, perspectiveProj, {rotatorTesseract}, {2, 2, 2}, {});
        sceneColorificator->setColorForObject(uid1, QColor(160,60,61));

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
        auto uid2 = scene->addObject(QUuid::createUuid(), 2, "Simplex5D", simplex5D, perspectiveProj, {rotatorSimplex}, {3, 3, 3}, {5, 5, 5});
        sceneColorificator->setColorForObject(uid2, QColor(28,98,15));
    } catch (const std::exception& ex) {
        qFatal() << "Exception occurred: " << ex.what();
    }
}
