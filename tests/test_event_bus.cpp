#include <QTest>
#include <QSignalSpy>
#include <QCoreApplication>

#include "event_bus_service.h"

using namespace mpf;

class TestEventBus : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // Basic functionality tests
    void testSubscribe();
    void testUnsubscribe();
    void testUnsubscribeAll();
    void testPublishSync();
    void testPublishAsync();

    // Wildcard matching tests
    void testSingleWildcard();
    void testDoubleWildcard();
    void testMixedWildcards();
    void testMatchesTopic();

    // Subscription options tests
    void testPriority();
    void testReceiveOwnEvents();

    // Query methods tests
    void testSubscriberCount();
    void testActiveTopics();
    void testTopicStats();
    void testSubscriptionsFor();

    // Edge cases
    void testEmptyTopic();
    void testMultipleSubscribers();
    void testNoSubscribers();

private:
    EventBusService* m_eventBus = nullptr;
};

void TestEventBus::initTestCase()
{
    qDebug() << "========== EventBus Test Suite ==========";
}

void TestEventBus::cleanupTestCase()
{
    qDebug() << "========== Tests Complete ==========";
}

void TestEventBus::init()
{
    m_eventBus = new EventBusService(this);
}

void TestEventBus::cleanup()
{
    delete m_eventBus;
    m_eventBus = nullptr;
}

// =============================================================================
// Basic functionality tests
// =============================================================================

void TestEventBus::testSubscribe()
{
    QString subId = m_eventBus->subscribe("orders/created", "plugin-a");
    
    QVERIFY(!subId.isEmpty());
    QCOMPARE(m_eventBus->totalSubscribers(), 1);
    QVERIFY(m_eventBus->activeTopics().contains("orders/created"));
}

void TestEventBus::testUnsubscribe()
{
    QString subId = m_eventBus->subscribe("orders/created", "plugin-a");
    QCOMPARE(m_eventBus->totalSubscribers(), 1);

    bool result = m_eventBus->unsubscribe(subId);
    QVERIFY(result);
    QCOMPARE(m_eventBus->totalSubscribers(), 0);

    // Unsubscribe non-existent should return false
    result = m_eventBus->unsubscribe("non-existent-id");
    QVERIFY(!result);
}

void TestEventBus::testUnsubscribeAll()
{
    m_eventBus->subscribe("topic1", "plugin-a");
    m_eventBus->subscribe("topic2", "plugin-a");
    m_eventBus->subscribe("topic3", "plugin-b");

    QCOMPARE(m_eventBus->totalSubscribers(), 3);

    m_eventBus->unsubscribeAll("plugin-a");
    QCOMPARE(m_eventBus->totalSubscribers(), 1);

    // Remaining subscription should be plugin-b's
    QVERIFY(m_eventBus->subscriptionsFor("plugin-a").isEmpty());
    QCOMPARE(m_eventBus->subscriptionsFor("plugin-b").size(), 1);
}

void TestEventBus::testPublishSync()
{
    QSignalSpy spy(m_eventBus, &EventBusService::eventPublished);
    QVERIFY(spy.isValid());

    m_eventBus->subscribe("orders/created", "plugin-a");

    QVariantMap data;
    data["orderId"] = "12345";
    data["amount"] = 99.99;

    int notified = m_eventBus->publishSync("orders/created", data, "plugin-b");
    QCOMPARE(notified, 1);

    // Synchronous publish should emit immediately
    QCOMPARE(spy.count(), 1);

    QList<QVariant> args = spy.takeFirst();
    QCOMPARE(args.at(0).toString(), QString("orders/created"));
    QCOMPARE(args.at(1).toMap()["orderId"].toString(), QString("12345"));
    QCOMPARE(args.at(2).toString(), QString("plugin-b"));
}

void TestEventBus::testPublishAsync()
{
    QSignalSpy spy(m_eventBus, &EventBusService::eventPublished);
    QVERIFY(spy.isValid());

    m_eventBus->subscribe("orders/created", "plugin-a");

    QVariantMap data;
    data["orderId"] = "12345";

    int notified = m_eventBus->publish("orders/created", data, "plugin-b");
    QCOMPARE(notified, 1);

    // Async publish uses Qt::QueuedConnection, need to process events
    QCOMPARE(spy.count(), 0);  // Not yet delivered

    // Process pending events
    QCoreApplication::processEvents();

    QCOMPARE(spy.count(), 1);
}

// =============================================================================
// Wildcard matching tests
// =============================================================================

void TestEventBus::testSingleWildcard()
{
    // * matches single level
    m_eventBus->subscribe("orders/*", "plugin-a");

    QCOMPARE(m_eventBus->subscriberCount("orders/created"), 1);
    QCOMPARE(m_eventBus->subscriberCount("orders/updated"), 1);
    QCOMPARE(m_eventBus->subscriberCount("orders/deleted"), 1);

    // Should NOT match nested levels
    QCOMPARE(m_eventBus->subscriberCount("orders/items/added"), 0);

    // Should NOT match parent level
    QCOMPARE(m_eventBus->subscriberCount("orders"), 0);
}

void TestEventBus::testDoubleWildcard()
{
    // ** matches multiple levels
    m_eventBus->subscribe("orders/**", "plugin-a");

    QCOMPARE(m_eventBus->subscriberCount("orders/created"), 1);
    QCOMPARE(m_eventBus->subscriberCount("orders/items/added"), 1);
    QCOMPARE(m_eventBus->subscriberCount("orders/items/removed/all"), 1);

    // Should NOT match parent or sibling
    QCOMPARE(m_eventBus->subscriberCount("orders"), 0);
    QCOMPARE(m_eventBus->subscriberCount("products/created"), 0);
}

void TestEventBus::testMixedWildcards()
{
    // Mix of * and **
    m_eventBus->subscribe("*/items/**", "plugin-a");

    QCOMPARE(m_eventBus->subscriberCount("orders/items/added"), 1);
    QCOMPARE(m_eventBus->subscriberCount("products/items/removed"), 1);
    QCOMPARE(m_eventBus->subscriberCount("cart/items/quantity/changed"), 1);

    // Should NOT match
    QCOMPARE(m_eventBus->subscriberCount("orders/created"), 0);
    QCOMPARE(m_eventBus->subscriberCount("items/added"), 0);
}

void TestEventBus::testMatchesTopic()
{
    // Test the matchesTopic helper method directly
    QVERIFY(m_eventBus->matchesTopic("orders/created", "orders/created"));
    QVERIFY(m_eventBus->matchesTopic("orders/created", "orders/*"));
    QVERIFY(m_eventBus->matchesTopic("orders/created", "*/created"));
    QVERIFY(m_eventBus->matchesTopic("orders/created", "*/*"));
    QVERIFY(m_eventBus->matchesTopic("orders/created", "**"));
    QVERIFY(m_eventBus->matchesTopic("orders/items/added", "orders/**"));
    QVERIFY(m_eventBus->matchesTopic("orders/items/added", "**/added"));

    QVERIFY(!m_eventBus->matchesTopic("orders/created", "orders/updated"));
    QVERIFY(!m_eventBus->matchesTopic("orders/items/added", "orders/*"));
    QVERIFY(!m_eventBus->matchesTopic("orders", "orders/*"));
}

// =============================================================================
// Subscription options tests
// =============================================================================

void TestEventBus::testPriority()
{
    // Higher priority should be notified first
    // Note: Current implementation doesn't expose delivery order directly,
    // but we can verify both subscriptions receive events

    SubscriptionOptions lowPriority;
    lowPriority.priority = 1;

    SubscriptionOptions highPriority;
    highPriority.priority = 10;

    m_eventBus->subscribe("test/event", "plugin-low", lowPriority);
    m_eventBus->subscribe("test/event", "plugin-high", highPriority);

    QCOMPARE(m_eventBus->subscriberCount("test/event"), 2);

    int notified = m_eventBus->publishSync("test/event", {{"key", "value"}}, "sender");
    QCOMPARE(notified, 2);
}

void TestEventBus::testReceiveOwnEvents()
{
    // Default: should NOT receive own events
    SubscriptionOptions defaultOpts;
    m_eventBus->subscribe("test/event", "plugin-a", defaultOpts);

    int notified = m_eventBus->publishSync("test/event", {}, "plugin-a");
    QCOMPARE(notified, 0);  // Self-published, filtered out

    notified = m_eventBus->publishSync("test/event", {}, "plugin-b");
    QCOMPARE(notified, 1);  // Different sender, should receive

    // With receiveOwnEvents = true
    m_eventBus->unsubscribeAll("plugin-a");

    SubscriptionOptions receiveOwn;
    receiveOwn.receiveOwnEvents = true;
    m_eventBus->subscribe("test/event", "plugin-a", receiveOwn);

    notified = m_eventBus->publishSync("test/event", {}, "plugin-a");
    QCOMPARE(notified, 1);  // Should receive own event now
}

// =============================================================================
// Query methods tests
// =============================================================================

void TestEventBus::testSubscriberCount()
{
    QCOMPARE(m_eventBus->subscriberCount("any/topic"), 0);

    m_eventBus->subscribe("orders/*", "plugin-a");
    m_eventBus->subscribe("orders/created", "plugin-b");
    m_eventBus->subscribe("products/*", "plugin-c");

    // "orders/created" matches both "orders/*" and "orders/created"
    QCOMPARE(m_eventBus->subscriberCount("orders/created"), 2);
    QCOMPARE(m_eventBus->subscriberCount("orders/updated"), 1);
    QCOMPARE(m_eventBus->subscriberCount("products/new"), 1);
    QCOMPARE(m_eventBus->subscriberCount("unknown/topic"), 0);
}

void TestEventBus::testActiveTopics()
{
    QVERIFY(m_eventBus->activeTopics().isEmpty());

    m_eventBus->subscribe("orders/created", "plugin-a");
    m_eventBus->subscribe("orders/*", "plugin-b");
    m_eventBus->subscribe("products/**", "plugin-c");

    QStringList topics = m_eventBus->activeTopics();
    QCOMPARE(topics.size(), 3);
    QVERIFY(topics.contains("orders/created"));
    QVERIFY(topics.contains("orders/*"));
    QVERIFY(topics.contains("products/**"));
}

void TestEventBus::testTopicStats()
{
    m_eventBus->subscribe("orders/created", "plugin-a");
    m_eventBus->subscribe("orders/*", "plugin-b");

    // Before any events
    TopicStats stats = m_eventBus->topicStats("orders/created");
    QCOMPARE(stats.topic, QString("orders/created"));
    QCOMPARE(stats.subscriberCount, 2);
    QCOMPARE(stats.eventCount, 0);

    // Publish some events
    m_eventBus->publishSync("orders/created", {}, "sender");
    m_eventBus->publishSync("orders/created", {}, "sender");
    m_eventBus->publishSync("orders/created", {}, "sender");

    stats = m_eventBus->topicStats("orders/created");
    QCOMPARE(stats.eventCount, 3);
    QVERIFY(stats.lastEventTime > 0);

    // QML-friendly variant version
    QVariantMap variantStats = m_eventBus->topicStatsAsVariant("orders/created");
    QCOMPARE(variantStats["eventCount"].toLongLong(), 3);
}

void TestEventBus::testSubscriptionsFor()
{
    QVERIFY(m_eventBus->subscriptionsFor("plugin-a").isEmpty());

    QString sub1 = m_eventBus->subscribe("topic1", "plugin-a");
    QString sub2 = m_eventBus->subscribe("topic2", "plugin-a");
    QString sub3 = m_eventBus->subscribe("topic3", "plugin-b");

    QStringList pluginASubs = m_eventBus->subscriptionsFor("plugin-a");
    QCOMPARE(pluginASubs.size(), 2);
    QVERIFY(pluginASubs.contains(sub1));
    QVERIFY(pluginASubs.contains(sub2));

    QStringList pluginBSubs = m_eventBus->subscriptionsFor("plugin-b");
    QCOMPARE(pluginBSubs.size(), 1);
    QVERIFY(pluginBSubs.contains(sub3));
}

// =============================================================================
// Edge cases
// =============================================================================

void TestEventBus::testEmptyTopic()
{
    // Empty topic should still work (though not recommended)
    QString subId = m_eventBus->subscribe("", "plugin-a");
    QVERIFY(!subId.isEmpty());

    int notified = m_eventBus->publishSync("", {{"test", true}}, "sender");
    QCOMPARE(notified, 1);
}

void TestEventBus::testMultipleSubscribers()
{
    // Same topic, multiple subscribers
    m_eventBus->subscribe("shared/topic", "plugin-a");
    m_eventBus->subscribe("shared/topic", "plugin-b");
    m_eventBus->subscribe("shared/topic", "plugin-c");

    QCOMPARE(m_eventBus->subscriberCount("shared/topic"), 3);

    QSignalSpy spy(m_eventBus, &EventBusService::eventPublished);
    int notified = m_eventBus->publishSync("shared/topic", {}, "external");
    
    QCOMPARE(notified, 3);
    QCOMPARE(spy.count(), 1);  // One signal emission, three subscribers
}

void TestEventBus::testNoSubscribers()
{
    QSignalSpy spy(m_eventBus, &EventBusService::eventPublished);

    int notified = m_eventBus->publishSync("nobody/listening", {{"data", 123}}, "sender");
    
    QCOMPARE(notified, 0);
    QCOMPARE(spy.count(), 0);  // No signal if no subscribers
}

QTEST_MAIN(TestEventBus)
#include "test_event_bus.moc"
