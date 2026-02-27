#include "event_bus_service.h"
#include "cross_dll_safety.h"

#include <QDateTime>
#include <QMetaObject>
#include <QUuid>
#include <QDebug>

#include <algorithm>

namespace mpf {

using CrossDllSafety::deepCopy;

EventBusService::EventBusService(QObject* parent)
    : QObject(parent)
{
}

EventBusService::~EventBusService() = default;

int EventBusService::publish(const QString& topic,
                              const QVariantMap& data,
                              const QString& senderId)
{
    Event event;
    event.topic = topic;
    event.senderId = senderId;
    event.data = data;
    event.timestamp = QDateTime::currentMSecsSinceEpoch();

    return deliverEvent(event, false);  // async
}

int EventBusService::publishSync(const QString& topic,
                                  const QVariantMap& data,
                                  const QString& senderId)
{
    Event event;
    event.topic = topic;
    event.senderId = senderId;
    event.data = data;
    event.timestamp = QDateTime::currentMSecsSinceEpoch();

    return deliverEvent(event, true);  // sync
}

int EventBusService::deliverEvent(const Event& event, bool synchronous)
{
    QList<const Subscription*> matches;

    {
        QMutexLocker locker(&m_mutex);

        // Update topic stats
        TopicData& stats = m_topicStats[event.topic];
        stats.topic = event.topic;
        stats.eventCount++;
        stats.lastEventTime = event.timestamp;

        // Find matching subscriptions
        matches = findMatchingSubscriptions(event.topic);
    }

    if (matches.isEmpty()) {
        return 0;
    }

    // Sort by priority (descending - higher priority first)
    std::sort(matches.begin(), matches.end(),
              [](const Subscription* a, const Subscription* b) {
                  return a->options.priority > b->options.priority;
              });

    int notified = 0;

    for (const Subscription* sub : matches) {
        // Skip if sender doesn't want own events
        if (!sub->options.receiveOwnEvents && sub->subscriberId == event.senderId) {
            continue;
        }

        // Invoke the callback if provided
        if (sub->handler) {
            if (synchronous) {
                sub->handler(event);
            } else {
                // Capture handler by value for async invocation
                auto handler = sub->handler;
                auto eventCopy = event;
                QMetaObject::invokeMethod(this, [handler, eventCopy]() {
                    handler(eventCopy);
                }, Qt::QueuedConnection);
            }
        }

        notified++;
    }

    // Emit signal for signal-based subscribers (QML etc.)
    if (synchronous) {
        emit eventPublished(event.topic, event.data, event.senderId);
    } else {
        QMetaObject::invokeMethod(this, [this, event]() {
            emit eventPublished(event.topic, event.data, event.senderId);
        }, Qt::QueuedConnection);
    }

    return notified;
}

QString EventBusService::subscribe(const QString& pattern,
                                    const QString& subscriberId,
                                    EventHandler handler,
                                    const SubscriptionOptions& options)
{
    Subscription sub;
    sub.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    // Deep copy strings from plugin to ensure they're in host's heap
    sub.pattern = deepCopy(pattern);
    sub.subscriberId = deepCopy(subscriberId);
    sub.handler = std::move(handler);
    sub.options = options;
    sub.regex = compilePattern(pattern);

    {
        QMutexLocker locker(&m_mutex);
        m_subscriptions.insert(sub.id, sub);
        m_subscriberIndex[sub.subscriberId].append(sub.id);
    }

    qDebug() << "EventBus: Subscribed" << subscriberId << "to" << pattern
             << "id:" << sub.id;

    emit subscriptionAdded(sub.id, pattern);
    emit subscribersChanged();
    emit topicsChanged();

    // Deep copy before returning
    return deepCopy(sub.id);
}

bool EventBusService::unsubscribe(const QString& subscriptionId)
{
    QString subscriberId;

    {
        QMutexLocker locker(&m_mutex);

        auto it = m_subscriptions.find(subscriptionId);
        if (it == m_subscriptions.end()) {
            return false;
        }

        subscriberId = it->subscriberId;
        m_subscriptions.erase(it);
        m_subscriberIndex[subscriberId].removeAll(subscriptionId);

        if (m_subscriberIndex[subscriberId].isEmpty()) {
            m_subscriberIndex.remove(subscriberId);
        }
    }

    qDebug() << "EventBus: Unsubscribed" << subscriptionId;

    emit subscriptionRemoved(subscriptionId);
    emit subscribersChanged();
    emit topicsChanged();

    return true;
}

void EventBusService::unsubscribeAll(const QString& subscriberId)
{
    QStringList ids;

    {
        QMutexLocker locker(&m_mutex);
        ids = m_subscriberIndex.take(subscriberId);

        for (const QString& id : ids) {
            m_subscriptions.remove(id);
        }
    }

    for (const QString& id : ids) {
        emit subscriptionRemoved(id);
    }

    if (!ids.isEmpty()) {
        qDebug() << "EventBus: Unsubscribed all for" << subscriberId
                 << "(" << ids.size() << "subscriptions)";
        emit subscribersChanged();
        emit topicsChanged();
    }
}

int EventBusService::subscriberCount(const QString& topic) const
{
    QMutexLocker locker(&m_mutex);

    int count = 0;
    for (auto it = m_subscriptions.constBegin(); it != m_subscriptions.constEnd(); ++it) {
        if (it->regex.match(topic).hasMatch()) {
            count++;
        }
    }
    return count;
}

QStringList EventBusService::activeTopics() const
{
    QMutexLocker locker(&m_mutex);

    QSet<QString> patterns;
    for (auto it = m_subscriptions.constBegin(); it != m_subscriptions.constEnd(); ++it) {
        patterns.insert(it->pattern);
    }
    return deepCopy(patterns.values());
}

TopicStats EventBusService::topicStats(const QString& topic) const
{
    QMutexLocker locker(&m_mutex);

    TopicStats stats;
    stats.topic = topic;
    stats.subscriberCount = 0;

    // Count subscribers
    for (auto it = m_subscriptions.constBegin(); it != m_subscriptions.constEnd(); ++it) {
        if (it->regex.match(topic).hasMatch()) {
            stats.subscriberCount++;
        }
    }

    // Get event stats
    auto dataIt = m_topicStats.find(topic);
    if (dataIt != m_topicStats.end()) {
        stats.eventCount = dataIt->eventCount;
        stats.lastEventTime = dataIt->lastEventTime;
    }

    return stats;
}

QStringList EventBusService::subscriptionsFor(const QString& subscriberId) const
{
    QMutexLocker locker(&m_mutex);
    return deepCopy(m_subscriberIndex.value(subscriberId));
}

bool EventBusService::matchesTopic(const QString& topic, const QString& pattern) const
{
    QRegularExpression regex = compilePattern(pattern);
    return regex.match(topic).hasMatch();
}

QString EventBusService::subscribeSimple(const QString& pattern, const QString& subscriberId)
{
    return subscribe(pattern, subscriberId, nullptr, SubscriptionOptions{});
}

QVariantMap EventBusService::topicStatsAsVariant(const QString& topic) const
{
    return deepCopy(topicStats(topic).toVariantMap());
}

int EventBusService::totalSubscribers() const
{
    QMutexLocker locker(&m_mutex);
    return m_subscriptions.size();
}

// ===== Request/Response =====

bool EventBusService::registerHandler(const QString& topic,
                                       const QString& handlerId,
                                       RequestHandler handler)
{
    QMutexLocker locker(&m_mutex);

    if (m_requestHandlers.contains(topic)) {
        qWarning() << "EventBus: Handler already registered for topic:" << topic;
        return false;
    }

    RequestHandlerEntry entry;
    entry.topic = deepCopy(topic);
    entry.handlerId = deepCopy(handlerId);
    entry.handler = std::move(handler);
    m_requestHandlers.insert(topic, std::move(entry));

    qDebug() << "EventBus: Registered request handler for" << topic << "by" << handlerId;
    return true;
}

bool EventBusService::unregisterHandler(const QString& topic)
{
    QMutexLocker locker(&m_mutex);
    bool removed = m_requestHandlers.remove(topic) > 0;
    if (removed) {
        qDebug() << "EventBus: Unregistered request handler for" << topic;
    }
    return removed;
}

void EventBusService::unregisterAllHandlers(const QString& handlerId)
{
    QMutexLocker locker(&m_mutex);

    QStringList toRemove;
    for (auto it = m_requestHandlers.constBegin(); it != m_requestHandlers.constEnd(); ++it) {
        if (it->handlerId == handlerId) {
            toRemove.append(it.key());
        }
    }

    for (const QString& topic : toRemove) {
        m_requestHandlers.remove(topic);
    }

    if (!toRemove.isEmpty()) {
        qDebug() << "EventBus: Unregistered all handlers for" << handlerId
                 << "(" << toRemove.size() << "handlers)";
    }
}

std::optional<QVariantMap> EventBusService::request(const QString& topic,
                                                     const QVariantMap& data,
                                                     const QString& senderId,
                                                     int timeoutMs)
{
    Q_UNUSED(timeoutMs)  // Synchronous call, timeout not implemented yet

    RequestHandler handler;
    {
        QMutexLocker locker(&m_mutex);
        auto it = m_requestHandlers.find(topic);
        if (it == m_requestHandlers.end()) {
            qDebug() << "EventBus: No handler for request topic:" << topic;
            return std::nullopt;
        }
        handler = it->handler;
    }

    Event event;
    event.topic = topic;
    event.senderId = senderId;
    event.data = data;
    event.timestamp = QDateTime::currentMSecsSinceEpoch();

    try {
        return deepCopy(handler(event));
    } catch (const std::exception& e) {
        qWarning() << "EventBus: Request handler threw exception:" << e.what();
        return std::nullopt;
    }
}

bool EventBusService::hasHandler(const QString& topic) const
{
    QMutexLocker locker(&m_mutex);
    return m_requestHandlers.contains(topic);
}

QRegularExpression EventBusService::compilePattern(const QString& pattern) const
{
    // Convert topic pattern to regex:
    // ** -> .+    (matches multiple levels, must be done first)
    // *  -> [^/]+ (matches single level)

    QString regex = QRegularExpression::escape(pattern);
    regex.replace("\\*\\*", "<<DOUBLE_STAR>>");  // Placeholder to avoid conflicts
    regex.replace("\\*", "[^/]+");
    regex.replace("<<DOUBLE_STAR>>", ".+");
    regex = "^" + regex + "$";

    return QRegularExpression(regex);
}

QList<const EventBusService::Subscription*> EventBusService::findMatchingSubscriptions(const QString& topic) const
{
    // Note: must be called with m_mutex held
    QList<const Subscription*> result;

    for (auto it = m_subscriptions.constBegin(); it != m_subscriptions.constEnd(); ++it) {
        if (it->regex.match(topic).hasMatch()) {
            result.append(&(*it));
        }
    }

    return result;
}

} // namespace mpf
