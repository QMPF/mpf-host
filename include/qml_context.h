#pragma once

#include <QObject>
#include <QString>

class QQmlApplicationEngine;

namespace mpf {

class ServiceRegistry;
class ServiceRegistryImpl;
class INavigation;
class ISettings;
class ITheme;
class IMenu;
class IEventBus;

/**
 * @brief Sets up QML context with services
 * 
 * Exposes services to QML as singletons accessible via App.Services.
 */
class QmlContext : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(QObject* navigation READ navigation CONSTANT)
    Q_PROPERTY(QObject* settings READ settings CONSTANT)
    Q_PROPERTY(QObject* theme READ theme CONSTANT)
    Q_PROPERTY(QObject* appMenu READ appMenu CONSTANT)
    Q_PROPERTY(QObject* eventBus READ eventBus CONSTANT)

public:
    explicit QmlContext(ServiceRegistry* registry, QObject* parent = nullptr);
    ~QmlContext() override;

    /**
     * @brief Set up QML context on engine
     */
    void setup(QQmlApplicationEngine* engine);

    QString version() const;
    QObject* navigation() const;
    QObject* settings() const;
    QObject* theme() const;
    QObject* appMenu() const;
    QObject* eventBus() const;

private:
    ServiceRegistryImpl* m_registry;
};

} // namespace mpf
