#include "configmanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QDebug>
#include <QMutexLocker>

/**
 * @brief Returns the singleton instance of ConfigManager. *
 * @return ConfigManager& Reference to the singleton instance.
 */
ConfigManager& ConfigManager::instance()
{
    static ConfigManager instance;
    return instance;
}

ConfigManager::ConfigManager()
{
}

bool ConfigManager::loadConfig(const QString &fileName)
{
    QMutexLocker locker(&m_mutex);

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open file for reading:" << fileName;
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        qWarning() << "Failed to parse JSON from file:" << fileName;
        return false;
    }

    if (!doc.isObject()) {
        qWarning() << "JSON content is not an object in file:" << fileName;
        return false;
    }

    m_config = doc.object();
    return true;
}

bool ConfigManager::saveConfig(const QString &fileName) const
{
    QMutexLocker locker(&m_mutex);

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Could not open file for writing:" << fileName;
        return false;
    }

    QJsonDocument doc(m_config);
    file.write(doc.toJson());
    file.close();
    return true;
}

QJsonObject ConfigManager::getConfig() const
{
    QMutexLocker locker(&m_mutex);
    return m_config;
}

void ConfigManager::setConfig(const QJsonObject &config)
{
    QMutexLocker locker(&m_mutex);
    m_config = config;
}

QJsonValue ConfigManager::getValue(const QString &key, const QJsonValue &defaultValue) const
{
    QMutexLocker locker(&m_mutex);
    return m_config.contains(key) ? m_config.value(key) : defaultValue;
}

void ConfigManager::setValue(const QString &key, const QJsonValue &value)
{
    QMutexLocker locker(&m_mutex);
    m_config.insert(key, value);
}
