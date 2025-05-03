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

#include "tools/configManager.h"
#include "tools/logger.h"
#include "view/mainWindow.h"
#include "model/sceneColorificator.h"
#include "presenterMain.h"

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
        && availableStyles.contains("Windows", Qt::CaseInsensitive))
    {
        return "Windows";
    }
    if (availableStyles.contains("windowsvista", Qt::CaseInsensitive))
        return "windowsvista";
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
    if (de == "KDE"     && styles.contains("Breeze"))
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

    QString currentStyle = app.style()->objectName();
    qDebug() << "Default Qt style:" << currentStyle;
    setAppropriateStyle();

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
