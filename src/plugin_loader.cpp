#include "plugin_loader.h"
#include <mpf/interfaces/iplugin.h>
#include "plugin_metadata.h"

#include <QFileInfo>
#include <QJsonDocument>
#include <QDebug>

namespace mpf {

PluginLoader::PluginLoader(const QString& path, QObject* parent)
    : QObject(parent)
    , m_path(path)
    , m_loader(std::make_unique<QPluginLoader>(path, this))
    , m_metadata(std::make_unique<PluginMetadata>())
{
}

PluginLoader::~PluginLoader()
{
    if (isLoaded()) {
        unload();
    }
}

bool PluginLoader::load()
{
    if (m_state == State::Loaded) {
        return true;
    }

    // Load metadata from plugin
    QJsonObject metaJson = m_loader->metaData().value("MetaData").toObject();
    *m_metadata = PluginMetadata(metaJson);
    
    // Validate metadata
    QStringList errors = m_metadata->validate();
    if (!errors.isEmpty()) {
        m_errorString = QString("Invalid metadata: %1").arg(errors.join("; "));
        m_state = State::Error;
        emit errorOccurred(m_errorString);
        return false;
    }

    // Load the plugin
    if (!m_loader->load()) {
        m_errorString = m_loader->errorString();
        m_state = State::Error;
        emit errorOccurred(m_errorString);
        return false;
    }

    // Get the plugin instance
    QObject* instance = m_loader->instance();
    if (!instance) {
        m_errorString = "Failed to get plugin instance";
        m_state = State::Error;
        m_loader->unload();
        emit errorOccurred(m_errorString);
        return false;
    }

    m_plugin = qobject_cast<IPlugin*>(instance);
    if (!m_plugin) {
        m_errorString = "Plugin does not implement IPlugin interface";
        m_state = State::Error;
        m_loader->unload();
        emit errorOccurred(m_errorString);
        return false;
    }

    m_state = State::Loaded;
    emit stateChanged(m_state);
    return true;
}

bool PluginLoader::unload()
{
    if (m_state == State::Unloaded) {
        return true;
    }

    m_plugin = nullptr;
    
    if (!m_loader->unload()) {
        m_errorString = m_loader->errorString();
        emit errorOccurred(m_errorString);
        return false;
    }

    m_state = State::Unloaded;
    emit stateChanged(m_state);
    return true;
}

} // namespace mpf
