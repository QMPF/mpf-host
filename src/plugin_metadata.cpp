#include "plugin_metadata.h"
#include <QJsonArray>

namespace mpf {

// PluginDependency

PluginDependency PluginDependency::fromJson(const QJsonObject& json)
{
    PluginDependency dep;
    
    QString typeStr = json.value("type").toString("plugin");
    dep.type = (typeStr == "service") ? Type::Service : Type::Plugin;
    dep.id = json.value("id").toString();
    dep.minVersion = QVersionNumber::fromString(json.value("min").toString("0.0.0"));
    dep.optional = json.value("optional").toBool(false);
    
    return dep;
}

QJsonObject PluginDependency::toJson() const
{
    QJsonObject json;
    json["type"] = (type == Type::Service) ? "service" : "plugin";
    json["id"] = id;
    json["min"] = minVersion.toString();
    if (optional) {
        json["optional"] = true;
    }
    return json;
}

// PluginMetadata

PluginMetadata::PluginMetadata(const QJsonObject& json)
{
    parse(json);
}

void PluginMetadata::parse(const QJsonObject& json)
{
    m_json = json;
    
    // Identity
    m_id = json.value("id").toString();
    m_name = json.value("name").toString(m_id);
    m_description = json.value("description").toString();
    m_version = QVersionNumber::fromString(json.value("version").toString("1.0.0"));
    m_vendor = json.value("vendor").toString();
    
    // Dependencies
    QJsonArray reqArray = json.value("requires").toArray();
    for (const auto& reqVal : reqArray) {
        QJsonObject reqObj = reqVal.toObject();
        m_requires.append(PluginDependency::fromJson(reqObj));
    }
    
    m_minHostVersion = QVersionNumber::fromString(
        json.value("minHostVersion").toString("1.0.0"));
    m_minFoundationVersion = QVersionNumber::fromString(
        json.value("minFoundationVersion").toString("1.0.0"));
    
    // Capabilities
    QJsonArray providesArray = json.value("provides").toArray();
    for (const auto& val : providesArray) {
        m_provides.append(val.toString());
    }
    
    QJsonArray qmlArray = json.value("qmlModules").toArray();
    for (const auto& val : qmlArray) {
        m_qmlModules.append(val.toString());
    }
    
    m_entryQml = json.value("entryQml").toString();
    
    // Loading hints
    m_priority = json.value("priority").toInt(0);
    m_loadOnStartup = json.value("loadOnStartup").toBool(true);
}

QStringList PluginMetadata::validate() const
{
    QStringList errors;
    
    if (m_id.isEmpty()) {
        errors << "Missing required field: id";
    }
    
    if (m_version.isNull()) {
        errors << "Invalid version format";
    }
    
    // Check for circular or self-dependencies
    for (const auto& dep : m_requires) {
        if (dep.id == m_id) {
            errors << QString("Plugin cannot depend on itself: %1").arg(dep.id);
        }
        if (dep.id.isEmpty()) {
            errors << "Dependency has empty id";
        }
    }
    
    return errors;
}

} // namespace mpf
