#include "plugin_manager.h"
#include "plugin_loader.h"
#include "service_registry.h"
#include "plugin_metadata.h"
#include <mpf/interfaces/iplugin.h>

#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <algorithm>

namespace mpf {

PluginManager::PluginManager(ServiceRegistry* registry, QObject* parent)
    : QObject(parent)
    , m_registry(registry)
{
}

PluginManager::~PluginManager()
{
    stopAll();
    unloadAll();
}

int PluginManager::discover(const QString& path)
{
    QDir dir(path);
    if (!dir.exists()) {
        qWarning() << "Plugin directory does not exist:" << path;
        return 0;
    }

    int count = 0;
    
    // Platform-specific plugin patterns
#if defined(Q_OS_WIN)
    QStringList filters = {"*.dll"};
#elif defined(Q_OS_MACOS)
    QStringList filters = {"*.dylib", "*.bundle"};
#else
    QStringList filters = {"*.so"};
#endif

    for (const QFileInfo& info : dir.entryInfoList(filters, QDir::Files)) {
        QString pluginPath = info.absoluteFilePath();
        
        auto loader = std::make_unique<PluginLoader>(pluginPath, this);
        
        // Try to read metadata without fully loading
        QPluginLoader tempLoader(pluginPath);
        QJsonObject meta = tempLoader.metaData().value("MetaData").toObject();
        
        if (meta.isEmpty()) {
            qDebug() << "Skipping non-MPF plugin:" << info.fileName();
            continue;
        }

        PluginMetadata metadata(meta);
        if (!metadata.isValid()) {
            qWarning() << "Invalid plugin metadata:" << info.fileName();
            continue;
        }

        QString id = metadata.id();
        
        if (m_pluginMap.contains(id)) {
            qWarning() << "Duplicate plugin ID:" << id;
            continue;
        }

        m_pluginMap[id] = loader.get();
        m_loaders.push_back(std::move(loader));
        
        emit pluginDiscovered(id);
        count++;
    }

    return count;
}

bool PluginManager::loadAll()
{
    QStringList order = computeLoadOrder();
    
    bool allLoaded = true;
    for (const QString& id : order) {
        PluginLoader* loader = m_pluginMap.value(id);
        if (!loader) continue;

        // Check dependencies
        QStringList unsatisfied = checkDependencies(loader->metadata());
        if (!unsatisfied.isEmpty()) {
            QString error = QString("Unsatisfied dependencies: %1").arg(unsatisfied.join(", "));
            emit pluginError(id, error);
            allLoaded = false;
            continue;
        }

        if (!loader->load()) {
            emit pluginError(id, loader->errorString());
            allLoaded = false;
            continue;
        }

        emit pluginLoaded(id);
    }

    return allLoaded;
}

bool PluginManager::initializeAll()
{
    QStringList order = computeLoadOrder();
    
    bool allInitialized = true;
    for (const QString& id : order) {
        PluginLoader* loader = m_pluginMap.value(id);
        if (!loader || !loader->isLoaded()) continue;
        if (loader->state() >= PluginLoader::State::Initialized) continue;

        IPlugin* plugin = loader->plugin();
        if (!plugin) continue;

        if (!plugin->initialize(m_registry)) {
            emit pluginError(id, "Initialization failed");
            allInitialized = false;
            continue;
        }

        loader->setState(PluginLoader::State::Initialized);
        emit pluginInitialized(id);
    }

    return allInitialized;
}

bool PluginManager::startAll()
{
    QStringList order = computeLoadOrder();
    
    bool allStarted = true;
    for (const QString& id : order) {
        PluginLoader* loader = m_pluginMap.value(id);
        if (!loader || loader->state() != PluginLoader::State::Initialized) continue;

        IPlugin* plugin = loader->plugin();
        if (!plugin) continue;

        if (!plugin->start()) {
            emit pluginError(id, "Start failed");
            allStarted = false;
            continue;
        }

        loader->setState(PluginLoader::State::Started);
        emit pluginStarted(id);
    }

    return allStarted;
}

void PluginManager::stopAll()
{
    // Stop in reverse order
    QStringList order = computeLoadOrder();
    std::reverse(order.begin(), order.end());
    
    for (const QString& id : order) {
        PluginLoader* loader = m_pluginMap.value(id);
        if (!loader || loader->state() != PluginLoader::State::Started) continue;

        IPlugin* plugin = loader->plugin();
        if (plugin) {
            plugin->stop();
        }

        loader->setState(PluginLoader::State::Initialized);
        emit pluginStopped(id);
    }
}

void PluginManager::unloadAll()
{
    // Unload in reverse order
    QStringList order = computeLoadOrder();
    std::reverse(order.begin(), order.end());
    
    for (const QString& id : order) {
        PluginLoader* loader = m_pluginMap.value(id);
        if (!loader || !loader->isLoaded()) continue;

        loader->unload();
        emit pluginUnloaded(id);
    }

    m_pluginMap.clear();
    m_loaders.clear();
}

QList<PluginLoader*> PluginManager::plugins() const
{
    QList<PluginLoader*> result;
    for (const auto& loader : m_loaders) {
        result.append(loader.get());
    }
    return result;
}

PluginLoader* PluginManager::plugin(const QString& id) const
{
    return m_pluginMap.value(id);
}

QStringList PluginManager::qmlModuleUris() const
{
    QStringList uris;
    for (const auto& loader : m_loaders) {
        if (loader->isLoaded() && loader->plugin()) {
            QString uri = loader->plugin()->qmlModuleUri();
            if (!uri.isEmpty()) {
                uris.append(uri);
            }
        }
    }
    return uris;
}

QString PluginManager::entryQml(const QString& pluginId) const
{
    PluginLoader* loader = m_pluginMap.value(pluginId);
    if (loader && loader->isLoaded() && loader->plugin()) {
        return loader->plugin()->entryQml();
    }
    return QString();
}

QStringList PluginManager::checkDependencies(const PluginMetadata& metadata) const
{
    QStringList unsatisfied;
    
    for (const PluginDependency& dep : metadata.requires()) {
        if (dep.optional) continue;
        
        if (dep.type == PluginDependency::Type::Plugin) {
            PluginLoader* depPlugin = m_pluginMap.value(dep.id);
            if (!depPlugin) {
                unsatisfied.append(QString("plugin:%1").arg(dep.id));
            } else if (depPlugin->metadata().version() < dep.minVersion) {
                unsatisfied.append(QString("plugin:%1>=%2")
                    .arg(dep.id, dep.minVersion.toString()));
            }
        } else {
            // Service dependency - check at runtime
            // We can't fully check this until initialization
        }
    }
    
    return unsatisfied;
}

QStringList PluginManager::loadOrder() const
{
    return computeLoadOrder();
}

QStringList PluginManager::computeLoadOrder() const
{
    QStringList order;
    QHash<QString, int> state; // 0=unvisited, 1=visiting, 2=visited
    
    for (auto it = m_pluginMap.begin(); it != m_pluginMap.end(); ++it) {
        state[it.key()] = 0;
    }
    
    for (auto it = m_pluginMap.begin(); it != m_pluginMap.end(); ++it) {
        if (!topologicalSort(it.key(), state, order)) {
            qWarning() << "Circular dependency detected involving:" << it.key();
        }
    }
    
    return order;
}

bool PluginManager::topologicalSort(const QString& id, 
                                     QHash<QString, int>& state, 
                                     QStringList& order) const
{
    if (state.value(id) == 2) return true;  // Already visited
    if (state.value(id) == 1) return false; // Circular dependency
    
    state[id] = 1; // Visiting
    
    PluginLoader* loader = m_pluginMap.value(id);
    if (loader) {
        for (const PluginDependency& dep : loader->metadata().requires()) {
            if (dep.type == PluginDependency::Type::Plugin) {
                if (m_pluginMap.contains(dep.id)) {
                    if (!topologicalSort(dep.id, state, order)) {
                        return false;
                    }
                }
            }
        }
    }
    
    state[id] = 2; // Visited
    order.append(id);
    return true;
}

} // namespace mpf
