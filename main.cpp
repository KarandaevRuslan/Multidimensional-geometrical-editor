#include <QApplication>
#include <QSurfaceFormat>
#include <QScreen>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include "tools/configManager.h"
#include "tools/logger.h"
#include "view/mainWindow.h"
#include "model/scene.h"
#include "model/NDShape.h"
#include "model/Projection.h"
#include "model/Rotator.h"
#include "presenterMain.h"

std::shared_ptr<Scene> getTestScene();

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setApplicationName("Multidimensional Geometrical Editor");
    QCoreApplication::setOrganizationName("Ruslan Karandaev");
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);
    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::applicationName());
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption multipleSampleOption("multisample", "Multisampling");
    parser.addOption(multipleSampleOption);
    QCommandLineOption coreProfileOption("coreprofile", "Use core profile");
    parser.addOption(coreProfileOption);
    QCommandLineOption transparentOption("transparent", "Transparent window");
    parser.addOption(transparentOption);

    parser.process(app);

    // Adjusting opengl
    QSurfaceFormat fmt;
    fmt.setDepthBufferSize(24);
    if (parser.isSet(multipleSampleOption))
        fmt.setSamples(4);
    if (parser.isSet(coreProfileOption)) {
        fmt.setVersion(3, 2);
        fmt.setProfile(QSurfaceFormat::CoreProfile);
    }
    QSurfaceFormat::setDefaultFormat(fmt);

    // Logger
    if (!Logger::instance().openLogFile("application.log")) {
        fprintf(stderr, "Could not open log file.\n");
    }
    qInstallMessageHandler(customMessageHandler);

    // Config
    ConfigManager &configManager = ConfigManager::instance();
    if (!configManager.loadConfig("config.json")) {
        qWarning() << "Failed to load configuration. Using defaults.";
    }

    std::shared_ptr<Scene> scene = getTestScene();
    std::shared_ptr<SceneColorificator> sceneColorificator = std::make_shared<SceneColorificator>();
    sceneColorificator->setColorForObject(2, QColor(160,60,61));
    sceneColorificator->setColorForObject(1, QColor(28,98,15));

    std::shared_ptr<SceneRenderer> sceneRenderer = std::make_shared<SceneRenderer>();
    std::shared_ptr<MainWindow> mainWindow = std::make_shared<MainWindow>(nullptr, sceneRenderer);
    PresenterMain* presenterMain = new PresenterMain(
        mainWindow, scene, sceneColorificator);
    return app.exec();
}



std::shared_ptr<Scene> getTestScene() {
    try {
        Scene scene;
        scene.setSceneDimension(3);

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
        scene.addObject(1, tesseract, perspectiveProj, {rotatorTesseract}, {2, 2, 2}, {});

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
        scene.addObject(2, simplex5D, perspectiveProj, {rotatorSimplex}, {3, 3, 3}, {5, 5, 5});

        return std::make_shared<Scene>(scene);
    } catch (const std::exception& ex) {
        qFatal() << "Exception occurred: " << ex.what();
    }
}


