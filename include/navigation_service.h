#pragma once

#include "mpf/interfaces/inavigation.h"
#include <QList>

class QQmlApplicationEngine;

namespace mpf {

/**
 * @brief Simple navigation service for Loader-based page switching
 * 
 * Plugins register their main page URL via registerRoute().
 * QML uses getPageUrl() to load pages via Loader.
 * Internal navigation within plugins uses Popup/Dialog.
 */
class NavigationService : public QObject, public INavigation
{
    Q_OBJECT

public:
    explicit NavigationService(QQmlApplicationEngine* engine, QObject* parent = nullptr);
    ~NavigationService() override;

    // Route registration (called by plugins)
    Q_INVOKABLE void registerRoute(const QString& route, const QString& qmlComponent) override;
    
    // Get page URL for a route (used by QML Loader)
    Q_INVOKABLE QString getPageUrl(const QString& route) const;
    
    // Current route tracking
    Q_INVOKABLE QString currentRoute() const override;
    Q_INVOKABLE void setCurrentRoute(const QString& route);

    // Legacy interface - no longer used but kept for INavigation compatibility
    Q_INVOKABLE bool push(const QString&, const QVariantMap& = {}) override { return false; }
    Q_INVOKABLE bool pop() override { return false; }
    Q_INVOKABLE void popToRoot() override {}
    Q_INVOKABLE bool replace(const QString&, const QVariantMap& = {}) override { return false; }
    Q_INVOKABLE int stackDepth() const override { return 0; }
    Q_INVOKABLE bool canGoBack() const override { return false; }

signals:
    void navigationChanged(const QString& route, const QVariantMap& params);
    void canGoBackChanged(bool canGoBack);

private:
    QQmlApplicationEngine* m_engine;
    QString m_currentRoute;
    
    struct RouteEntry {
        QString pattern;
        QString component;
    };
    QList<RouteEntry> m_routes;
};

} // namespace mpf
