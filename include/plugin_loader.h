#pragma once

#include <QObject>
#include <QString>
#include <QPluginLoader>
#include <memory>

namespace mpf {

class IPlugin;
class PluginMetadata;

/**
 * @brief Handles loading a single plugin
 */
class PluginLoader : public QObject
{
    Q_OBJECT

public:
    enum class State {
        Unloaded,
        Loaded,
        Initialized,
        Started,
        Error
    };

    explicit PluginLoader(const QString& path, QObject* parent = nullptr);
    ~PluginLoader() override;

    /**
     * @brief Load the plugin library
     * @return true if loading succeeded
     */
    bool load();

    /**
     * @brief Unload the plugin library
     * @return true if unloading succeeded
     */
    bool unload();

    /**
     * @brief Get the plugin instance
     * @return Plugin instance or nullptr
     */
    IPlugin* plugin() const { return m_plugin; }

    /**
     * @brief Get plugin metadata
     */
    const PluginMetadata& metadata() const { return *m_metadata; }

    /**
     * @brief Get plugin file path
     */
    QString path() const { return m_path; }

    /**
     * @brief Get current state
     */
    State state() const { return m_state; }

    /**
     * @brief Get error message (if state is Error)
     */
    QString errorString() const { return m_errorString; }

    /**
     * @brief Check if plugin is loaded
     */
    bool isLoaded() const { return m_state >= State::Loaded && m_state != State::Error; }

    /**
     * @brief Set state (called by PluginManager)
     */
    void setState(State state) { m_state = state; }

signals:
    void stateChanged(State state);
    void errorOccurred(const QString& error);

private:
    QString m_path;
    std::unique_ptr<QPluginLoader> m_loader;
    std::unique_ptr<PluginMetadata> m_metadata;
    IPlugin* m_plugin = nullptr;
    State m_state = State::Unloaded;
    QString m_errorString;
};

} // namespace mpf
