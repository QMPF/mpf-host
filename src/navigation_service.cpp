#include "navigation_service.h"
#include <QQmlApplicationEngine>
#include <QDebug>

namespace mpf {

NavigationService::NavigationService(QQmlApplicationEngine* engine, QObject* parent)
    : QObject(parent)
    , m_engine(engine)
{
}

NavigationService::~NavigationService() = default;

void NavigationService::registerRoute(const QString& route, const QString& qmlPageUrl)
{
    RouteEntry entry{route, qmlPageUrl};
    m_routes.append(entry);
    qDebug() << "NavigationService: Registered route" << route << "->" << qmlPageUrl;
}

QString NavigationService::getPageUrl(const QString& route) const
{
    for (const RouteEntry& entry : m_routes) {
        if (entry.pattern == route) {
            qDebug() << "NavigationService: getPageUrl" << route << "->" << entry.pageUrl;
            return entry.pageUrl;
        }
    }
    
    qWarning() << "NavigationService: No page URL found for route:" << route;
    return QString();
}

QString NavigationService::currentRoute() const
{
    return m_currentRoute;
}

void NavigationService::setCurrentRoute(const QString& route)
{
    if (m_currentRoute != route) {
        m_currentRoute = route;
        emit navigationChanged(route, {});
    }
}

} // namespace mpf
