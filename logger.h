#ifndef LOGGER_H
#define LOGGER_H

#include <QFile>
#include <QMutex>
#include <QString>
#include <QMessageLogContext>
#include <QtGlobal>

/**
 * @brief The Logger class provides thread-safe logging with advanced formatting.
 */
class Logger
{
public:
    /**
     * @brief Retrieves the singleton instance of Logger.
     * @return A reference to the Logger instance.
     */
    static Logger& instance();

    /**
     * @brief Logs a message with advanced formatting.
     * @param type The type of Qt message (e.g., debug, info, warning).
     * @param context The message context containing file, line, and function details.
     * @param msg The message text to log.
     */
    void log(QtMsgType type, const QMessageLogContext &context, const QString &msg);

    /**
     * @brief Opens the log file for appending log messages.
     * @param filename The file name (or path) of the log file.
     * @return True if the file was opened successfully, false otherwise.
     */
    bool openLogFile(const QString &filename);

private:
    Logger();
    ~Logger();

    // Disable copy construction and assignment
    Logger(const Logger &) = delete;
    Logger &operator=(const Logger &) = delete;

    QFile m_logFile;
    QMutex m_mutex;
};

/**
 * @brief Custom Qt message handler that redirects messages to the Logger.
 *
 * @param type The type of Qt message.
 * @param context The context in which the message was generated.
 * @param msg The actual log message.
 */
void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

#endif // LOGGER_H
