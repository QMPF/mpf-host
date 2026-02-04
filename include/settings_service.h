#pragma once

#include "mpf/interfaces/isettings.h"
#include <QSettings>
#include <memory>

namespace mpf {

/**
 * @brief Default settings service implementation
 * 
 * Uses QSettings for persistent storage with plugin namespace isolation.
 */
class SettingsService : public QObject, public ISettings
{
    Q_OBJECT

public:
    explicit SettingsService(QObject* parent = nullptr);
    explicit SettingsService(const QString& configPath, QObject* parent = nullptr);
    ~SettingsService() override;

    // ISettings interface
    QVariant value(const QString& pluginId, 
                   const QString& key, 
                   const QVariant& defaultValue = {}) const override;
    void setValue(const QString& pluginId, 
                  const QString& key, 
                  const QVariant& value) override;
    void remove(const QString& pluginId, const QString& key) override;
    bool contains(const QString& pluginId, const QString& key) const override;
    QStringList keys(const QString& pluginId) const override;
    void sync() override;

signals:
    void settingChanged(const QString& pluginId, const QString& key, const QVariant& value);

private:
    QString makeKey(const QString& pluginId, const QString& key) const;
    
    std::unique_ptr<QSettings> m_settings;
};

} // namespace mpf
