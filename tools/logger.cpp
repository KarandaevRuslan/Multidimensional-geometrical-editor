#include "logger.h"
#include <QDateTime>
#include <QTextStream>
#include <cstdio>

Logger& Logger::instance() {
    static Logger _instance;
    return _instance;
}

Logger::Logger() {
}

Logger::~Logger() {
    if (m_logFile.isOpen())
        m_logFile.close();
}

bool Logger::openLogFile(const QString &filename) {
    QMutexLocker locker(&m_mutex);
    m_logFile.setFileName(filename);
    return m_logFile.open(QIODevice::Append | QIODevice::Text);
}

void Logger::log(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    QMutexLocker locker(&m_mutex);
    QString timeStamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString level;
    switch (type) {
    case QtDebugMsg:    level = "DEBUG";    break;
    case QtInfoMsg:     level = "INFO";     break;
    case QtWarningMsg:  level = "WARNING";  break;
    case QtCriticalMsg: level = "CRITICAL"; break;
    case QtFatalMsg:    level = "FATAL";    break;
    }

    QString logMessage = QString("%1 [%2] (%3, %4): %5")
                             .arg(timeStamp)
                             .arg(level)
                             .arg(context.line)
                             .arg(context.function ? context.function : "")
                             .arg(msg);

    if (m_logFile.isOpen()) {
        QTextStream out(&m_logFile);
        out << logMessage << "\n";
        out.flush();
    }

    fprintf(stdout, "%s\n", logMessage.toLocal8Bit().constData());

    if (type == QtFatalMsg)
        abort();
}

void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    Logger::instance().log(type, context, msg);
}
