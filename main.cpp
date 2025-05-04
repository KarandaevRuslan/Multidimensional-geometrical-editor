#include <QApplication>
#include <QSurfaceFormat>
#include <QScreen>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QOpenGLWindow>
#include <QStyleFactory>
#include <QOperatingSystemVersion>
#include <QSysInfo>
#include <QProcess>
#include <QDebug>
#include <QMetaType>
#include <QDir>

#include "tools/configManager.h"
#include "tools/logger.h"
#include "view/mainWindow.h"
#include "model/sceneColorificator.h"
#include "model/opengl/graphics/sceneGeometryManager.h"
#include "presenterMain.h"
#include "view/sceneRenderer.h"

QString detectLinuxDesktopEnvironment()
{
#ifdef Q_OS_LINUX
    QByteArray xdg = qgetenv("XDG_CURRENT_DESKTOP");
    if (!xdg.isEmpty()) {
        QString env = QString::fromUtf8(xdg).toLower();
        if (env.contains("kde"))       return "KDE";
        if (env.contains("gnome"))     return "GNOME";
        if (env.contains("xfce"))      return "XFCE";
        if (env.contains("cinnamon"))  return "Cinnamon";
        if (env.contains("lxde"))      return "LXDE";
    }
    QByteArray session = qgetenv("DESKTOP_SESSION");
    if (!session.isEmpty())
        return QString::fromUtf8(session).toLower();
#endif
    return QString();
}

QString chooseWindowsStyle(const QStringList &availableStyles)
{
#if QT_VERSION_MAJOR >= 6
    constexpr bool isQt6 = true;
#else
    constexpr bool isQt6 = false;
#endif

#if defined(Q_OS_WIN)
    auto winVer = QOperatingSystemVersion::current();

    if (isQt6
        && winVer >= QOperatingSystemVersion::Windows11
        && availableStyles.contains("windows11", Qt::CaseInsensitive))
    {
        return "windows11";
    }
    if (winVer >= QOperatingSystemVersion::Windows10
        && availableStyles.contains("Fusion", Qt::CaseInsensitive))
    {
        return "Fusion";
    }
    if (winVer >= QOperatingSystemVersion::Windows7
        && availableStyles.contains("windowsvista", Qt::CaseInsensitive))
    {
        return "windowsvista";
    }
    if (availableStyles.contains("Windows", Qt::CaseInsensitive))
        return "Windows";
#endif
    return QString();
}

void setAppropriateStyle()
{
    QStringList styles = QStyleFactory::keys();
    qDebug() << "Available styles:" << styles;

    QString chosen;
#if defined(Q_OS_WIN)
    chosen = chooseWindowsStyle(styles);
#elif defined(Q_OS_MAC)
    chosen = styles.contains("macintosh", Qt::CaseInsensitive)
                 ? "macintosh" : "Fusion";
#elif defined(Q_OS_LINUX)
    QString de = detectLinuxDesktopEnvironment();
    qDebug() << de;
    if (de == "KDE" && styles.contains("Breeze"))
        chosen = "Breeze";
    else if ((de == "GNOME" || de == "Cinnamon")
             && styles.contains("GTK+"))
        chosen = "GTK+";
    else if (styles.contains("Fusion"))
        chosen = "Fusion";
    else if (!styles.isEmpty())
        chosen = styles.first();
#else
    chosen = "Fusion";
#endif

    if (!chosen.isEmpty()) {
        qDebug() << "Setting style to:" << chosen;
        QApplication::setStyle(QStyleFactory::create(chosen));
    } else {
        qWarning() << "No suitable style found, using default.";
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/images/app_icon.png"));

    qRegisterMetaType<std::shared_ptr<NDShape>>("std::shared_ptr<NDShape>");
    qRegisterMetaType<std::shared_ptr<Projection>>("std::shared_ptr<Projection>");
    qRegisterMetaType<std::vector<Rotator>>("std::vector<Rotator>");

    QCoreApplication::setApplicationName("NDEditor");
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

    QString dataDir;
    #ifdef Q_OS_WIN
    dataDir = QCoreApplication::applicationDirPath();
    #else
    dataDir = QStandardPaths::writableLocation(
        QStandardPaths::AppDataLocation
        );
    #endif

    if (!QDir().mkpath(dataDir)) {
        qWarning() << "Can not create directory: " << dataDir;
    }

    const QString logPath    = QDir(dataDir).filePath("application.log");
    const QString configPath = QDir(dataDir).filePath("config.json");

    // Logger
    if (!Logger::instance().openLogFile(logPath)) {
        qWarning() <<  "Could not open log file.";
    }
    qInstallMessageHandler(customMessageHandler);

    QString currentStyle = app.style()->objectName();
    qDebug() << "Default Qt style:" << currentStyle;
    setAppropriateStyle();

    // Config
    {
        ConfigManager &configManager = ConfigManager::instance();
        if (!configManager.loadConfig(configPath)) {
            qWarning() << "Failed to load configuration. Using defaults.";
        }

        auto readOrDefault = [&](const char* key, const QString& def){
            QString v = configManager.getValue(key).toString();
            if (v.isEmpty() || !QColor(v).isValid()) {
                configManager.setValue(key, def);
                return def;
            }
            return v;
        };

        QString sceneObjColorStr    = readOrDefault("sceneObjDefaultColor",    "#ffffff");
        QString clearColorStr       = readOrDefault("sceneRendererClearColor", "#8f8f8f");
        QString overlayPenColorStr  = readOrDefault("sceneOverlayNumberPenColor", "#000000");

        QString tmpConfig = configPath + ".tmp";
        if (configManager.saveConfig(tmpConfig) && QFile::rename(tmpConfig, configPath)) {
            qDebug() << "Config saved to" << configPath;
        } else {
            qWarning() << "Error while saving config.";
        }

        SceneColorificator::defaultColor = QColor(sceneObjColorStr);
        SceneRenderer::clearSceneColor = QColor(clearColorStr);
        SceneGeometryManager::sceneOverlayNumberPen = QPen(QColor(overlayPenColorStr));

        qDebug() << "defaultColor ="
                 << SceneColorificator::defaultColor.name(QColor::HexArgb);
        qDebug() << "clearSceneColor ="
                 << SceneRenderer::clearSceneColor.name(QColor::HexArgb);
        qDebug() << "overlayNumberPenColor ="
                 << SceneGeometryManager::sceneOverlayNumberPen.color().name(QColor::HexArgb);
    }

    MainWindow mainWindow = MainWindow();
    PresenterMain presenterMain = PresenterMain(&mainWindow);

    int ret = app.exec();
    return ret;
}
