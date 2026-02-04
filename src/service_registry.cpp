#include "service_registry.h"
#include <QDebug>

namespace mpf {

ServiceRegistryImpl::ServiceRegistryImpl(QObject* parent)
    : QObject(parent)
{
}

ServiceRegistryImpl::~ServiceRegistryImpl()
{
    QMutexLocker locker(&m_mutex);
    m_services.clear();
}

bool ServiceRegistryImpl::addService(const char* typeName, QObject* instance, 
                                  int version, const QString& providerId)
{
    if (!instance) {
        qWarning() << "ServiceRegistry: Cannot register null service for" << typeName;
        return false;
    }

    QString name = QString::fromLatin1(typeName);
    
    QMutexLocker locker(&m_mutex);
    
    if (m_services.contains(name)) {
        qWarning() << "ServiceRegistry: Service already registered:" << name;
        return false;
    }

    ServiceEntry entry;
    entry.interfaceName = name;
    entry.version = version;
    entry.instance = instance;
    entry.providerId = providerId;

    m_services.insert(name, entry);
    
    locker.unlock();
    emit serviceAdded(name);
    
    qDebug() << "ServiceRegistry: Registered" << name << "v" << version 
             << "from" << providerId;
    return true;
}

QObject* ServiceRegistryImpl::getService(const char* typeName, int minVersion)
{
    QString name = QString::fromLatin1(typeName);
    
    QMutexLocker locker(&m_mutex);
    
    auto it = m_services.find(name);
    if (it == m_services.end()) {
        return nullptr;
    }

    if (minVersion > 0 && it->version < minVersion) {
        qWarning() << "ServiceRegistry: Service" << name 
                   << "version" << it->version 
                   << "is below required" << minVersion;
        return nullptr;
    }

    return it->instance;
}

bool ServiceRegistryImpl::hasService(const char* typeName, int minVersion) const
{
    QString name = QString::fromLatin1(typeName);
    
    QMutexLocker locker(&m_mutex);
    
    auto it = m_services.find(name);
    if (it == m_services.end()) {
        return false;
    }

    if (minVersion > 0 && it->version < minVersion) {
        return false;
    }

    return true;
}

int ServiceRegistryImpl::serviceVersion(const char* typeName) const
{
    QString name = QString::fromLatin1(typeName);
    
    QMutexLocker locker(&m_mutex);
    
    auto it = m_services.find(name);
    if (it == m_services.end()) {
        return -1;
    }

    return it->version;
}

void ServiceRegistryImpl::removeService(const char* typeName)
{
    QString name = QString::fromLatin1(typeName);
    
    QMutexLocker locker(&m_mutex);
    
    if (m_services.remove(name) > 0) {
        locker.unlock();
        emit serviceRemoved(name);
        qDebug() << "ServiceRegistry: Removed" << name;
    }
}

QStringList ServiceRegistryImpl::registeredServices() const
{
    QMutexLocker locker(&m_mutex);
    return m_services.keys();
}

const ServiceEntry* ServiceRegistryImpl::entry(const QString& interfaceName) const
{
    QMutexLocker locker(&m_mutex);
    
    auto it = m_services.find(interfaceName);
    if (it == m_services.end()) {
        return nullptr;
    }
    
    return &it.value();
}

} // namespace mpf
