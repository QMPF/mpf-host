#pragma once

#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QVersionNumber>

namespace mpf {

/**
 * @brief Plugin dependency declaration
 */
struct PluginDependency
{
    enum class Type {
        Plugin,     // Depends on another plugin
        Service     // Depends on a service interface
    };

    Type type;
    QString id;             // Plugin ID or service interface name
    QVersionNumber minVersion;
    bool optional = false;  // If true, plugin can load without this dependency

    static PluginDependency fromJson(const QJsonObject& json);
    QJsonObject toJson() const;
};

/**
 * @brief Parsed plugin metadata
 * 
 * Contains all information about a plugin extracted from its metadata JSON.
 */
class PluginMetadata
{
public:
    PluginMetadata() = default;
    explicit PluginMetadata(const QJsonObject& json);

    bool isValid() const { return !m_id.isEmpty(); }

    // Identity
    QString id() const { return m_id; }
    QString name() const { return m_name; }
    QString description() const { return m_description; }
    QVersionNumber version() const { return m_version; }
    QString vendor() const { return m_vendor; }

    // Dependencies
    QList<PluginDependency> requires() const { return m_requires; }
    QVersionNumber minHostVersion() const { return m_minHostVersion; }
    QVersionNumber minFoundationVersion() const { return m_minFoundationVersion; }

    // Capabilities
    QStringList provides() const { return m_provides; }
    QStringList qmlModules() const { return m_qmlModules; }
    QString entryQml() const { return m_entryQml; }

    // Loading hints
    int priority() const { return m_priority; }
    bool loadOnStartup() const { return m_loadOnStartup; }

    // Raw JSON
    QJsonObject toJson() const { return m_json; }

    // Validation
    QStringList validate() const;

private:
    void parse(const QJsonObject& json);

    QString m_id;
    QString m_name;
    QString m_description;
    QVersionNumber m_version;
    QString m_vendor;
    
    QList<PluginDependency> m_requires;
    QVersionNumber m_minHostVersion;
    QVersionNumber m_minFoundationVersion;
    
    QStringList m_provides;
    QStringList m_qmlModules;
    QString m_entryQml;
    
    int m_priority = 0;
    bool m_loadOnStartup = true;
    
    QJsonObject m_json;
};

} // namespace mpf
