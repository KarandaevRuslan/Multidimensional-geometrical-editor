#include <QApplication>
#include <QSurfaceFormat>
#include <QScreen>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QOpenGLWindow>

#include "tools/configManager.h"
#include "tools/logger.h"
#include "view/mainWindow.h"
#include "model/sceneColorificator.h"
#include "presenterMain.h"

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
    QCommandLineOption coreProfileOption("coreprofile", "Use core profile");
    parser.addOption(coreProfileOption);

    parser.process(app);

    // Adjusting opengl
    QSurfaceFormat fmt;
    fmt.setDepthBufferSize(24);
    fmt.setStencilBufferSize(8);
    fmt.setSamples(16);
    if (parser.isSet(coreProfileOption)) {
        fmt.setVersion(3, 3);
        fmt.setProfile(QSurfaceFormat::CoreProfile);
    }
    QSurfaceFormat::setDefaultFormat(fmt);

    // Logger
    if (!Logger::instance().openLogFile("application.log")) {
        fprintf(stderr, "Could not open log file.\n");
    }
    qInstallMessageHandler(customMessageHandler);

    {
        // Config
        ConfigManager &configManager = ConfigManager::instance();
        if (!configManager.loadConfig("config.json")) {
            qWarning() << "Failed to load configuration. Using defaults.";
            configManager.setValue("sceneObjDefaultColor", "#ffffff");


            configManager.saveConfig("config.json");
        }

        SceneColorificator::defaultColor = QColor(
            configManager.getValue("sceneObjDefaultColor").toString());
    }

    MainWindow mainWindow = MainWindow();
    PresenterMain presenterMain = PresenterMain(&mainWindow);

    int ret = app.exec();
    return ret;
}
