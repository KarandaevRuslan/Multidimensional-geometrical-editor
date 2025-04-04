#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <QJsonObject>
#include <QJsonValue>
#include <QString>
#include <QMutex>

/**
 * @brief The ConfigManager class provides functionality to load, save, and manage
 * configuration data stored in JSON format.
 */
class ConfigManager
{
public:
    /**
     * @brief Returns the singleton instance of ConfigManager.
     * @return Reference to the singleton instance.
     */
    static ConfigManager& instance();

    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    /**
     * @brief Loads configuration data from a JSON file in a thread-safe manner.
     * @param fileName The file path of the JSON configuration file.
     * @return true if the configuration was loaded successfully, false otherwise.
     */
    bool loadConfig(const QString &fileName);

    /**
     * @brief Saves the current configuration data to a JSON file in a thread-safe manner.
     * @param fileName The file path to save the JSON configuration.
     * @return true if the configuration was saved successfully, false otherwise.
     */
    bool saveConfig(const QString &fileName) const;

    /**
     * @brief Retrieves the current configuration data.
     * @return A QJsonObject containing the configuration settings.
     */
    QJsonObject getConfig() const;

    /**
     * @brief Sets the configuration data.
     * @param config A QJsonObject containing the new configuration settings.
     */
    void setConfig(const QJsonObject &config);

    /**
     * @brief Retrieves a specific value from the configuration.
     * @param key The key associated with the desired value.
     * @param defaultValue The value to return if the key does not exist.
     * @return The QJsonValue associated with the key, or defaultValue if the key is not found.
     */
    QJsonValue getValue(const QString &key, const QJsonValue &defaultValue = QJsonValue()) const;

    /**
     * @brief Sets a specific value in the configuration.
     * @param key The key to set.
     * @param value The value to assign to the key.
     */
    void setValue(const QString &key, const QJsonValue &value);

private:
    /**
     * @brief Private constructor to enforce singleton pattern.
     */
    ConfigManager();

    mutable QMutex m_mutex;
    QJsonObject m_config;
};

#endif // CONFIG_MANAGER_H
