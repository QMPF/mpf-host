#pragma once

#include "mpf/interfaces/imenu.h"
#include <QList>
#include <QHash>
#include <QMutex>

namespace mpf {

/**
 * @brief Default menu service implementation
 */
class MenuService : public QObject, public IMenu
{
    Q_OBJECT
    Q_PROPERTY(QVariantList items READ itemsAsVariant NOTIFY menuChanged)
    Q_PROPERTY(int count READ count NOTIFY menuChanged)

public:
    explicit MenuService(QObject* parent = nullptr);
    ~MenuService() override;

    // IMenu interface
    bool registerItem(const MenuItem& item) override;
    void unregisterItem(const QString& id) override;
    void unregisterPlugin(const QString& pluginId) override;
    bool updateItem(const QString& id, const QVariantMap& updates) override;
    void setBadge(const QString& id, const QString& badge) override;
    void setEnabled(const QString& id, bool enabled) override;
    
    QList<MenuItem> items() const override;
    QVariantList itemsAsVariant() const override;
    QVariantList itemsInGroup(const QString& group) const override;
    QStringList groups() const override;
    int count() const override;

signals:
    void menuChanged();

private:
    void sortItems();
    
    mutable QMutex m_mutex;
    QList<MenuItem> m_items;
    QHash<QString, int> m_indexMap;  // id -> index for fast lookup
};

} // namespace mpf
