#include "qml_context.h"
#include "service_registry.h"
#include <mpf/version.h>
#include <mpf/interfaces/inavigation.h>
#include <mpf/interfaces/isettings.h>
#include <mpf/interfaces/itheme.h>
#include <mpf/interfaces/imenu.h>
#include <mpf/interfaces/ieventbus.h>

#include <QQmlApplicationEngine>
#include <QQmlContext>

namespace mpf {

QmlContext::QmlContext(ServiceRegistry* registry, QObject* parent)
    : QObject(parent)
    , m_registry(static_cast<ServiceRegistryImpl*>(registry))
{
}

QmlContext::~QmlContext() = default;

void QmlContext::setup(QQmlApplicationEngine* engine)
{
    // Register this as "App" singleton
    engine->rootContext()->setContextProperty("App", this);
    
    // Expose services to QML (as QObject*)
    engine->rootContext()->setContextProperty("Navigation", navigation());
    engine->rootContext()->setContextProperty("Settings", settings());
    engine->rootContext()->setContextProperty("Theme", theme());
    engine->rootContext()->setContextProperty("AppMenu", appMenu());
    engine->rootContext()->setContextProperty("EventBus", eventBus());
}

QString QmlContext::version() const
{
    return MPF_VERSION_STRING;
}

QObject* QmlContext::navigation() const
{
    // Use getObject to get the stored QObject* directly,
    // avoiding reinterpret_cast issues with multiple inheritance
    return m_registry->getObject<INavigation>();
}

QObject* QmlContext::settings() const
{
    return m_registry->getObject<ISettings>();
}

QObject* QmlContext::theme() const
{
    return m_registry->getObject<ITheme>();
}

QObject* QmlContext::appMenu() const
{
    return m_registry->getObject<IMenu>();
}

QObject* QmlContext::eventBus() const
{
    return m_registry->getObject<IEventBus>();
}

} // namespace mpf
