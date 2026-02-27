/**
 * =============================================================================
 * 样例 02: EventBus —— 事件总线与跨插件通信
 * =============================================================================
 *
 * 【这个样例展示了什么】
 * EventBus 是 MPF 框架中插件间通信的核心机制。
 * 它实现了发布/订阅模式，让插件可以松耦合地交换消息。
 *
 * 【为什么需要 EventBus】
 * 插件之间不能直接引用（编译时隔离），但业务上需要通信。
 * 例如：订单创建后，规则插件需要自动检查规则。
 * EventBus 让这种通信成为可能，而不需要插件之间有任何编译时依赖。
 *
 * 【IEventBus 接口定义的通信模式】
 * 1. 发布/订阅（Pub/Sub）：一对多广播，fire-and-forget
 * 2. 通配符匹配：* 匹配单级，** 匹配多级
 *
 * 注：IEventBus 接口还定义了请求/响应（Request/Response）模式，
 * 但当前 Host 的 EventBusService 实现主要使用发布/订阅 + Qt 信号机制。
 *
 * 【当前实现说明】
 * Host 的 EventBusService 实现了基于 Qt 信号的事件分发：
 * - subscribe() 注册订阅关系（pattern + subscriberId）
 * - publish() 发布事件时，匹配所有订阅并发射 eventPublished 信号
 * - QML 层通过 Connections 监听 eventPublished 信号
 * - C++ 层通过 connect(eventBus, &EventBusService::eventPublished, ...) 监听
 *
 * 【Topic 命名约定】
 * 推荐使用类似 URL 的分层命名：
 * - "orders/created"     → 订单创建事件
 * - "orders/updated"     → 订单更新事件
 * - "rules/check"        → 规则检查事件
 * - "system/shutdown"    → 系统关闭事件
 * =============================================================================
 */

#include <mpf/interfaces/ieventbus.h>
#include <QObject>
#include <QDebug>

/**
 * 样例：EventBus 的发布和订阅（C++ 层）
 *
 * 展示插件如何在 C++ 代码中使用 EventBus 进行通信。
 */
void example_eventbus_cpp_usage()
{
    // =========================================================================
    // 获取 EventBus 服务
    // =========================================================================
    // 在插件的 initialize() 或 start() 中：
    //
    // mpf::IEventBus* eventBus = m_registry->get<mpf::IEventBus>();

    // =========================================================================
    // 订阅事件
    // =========================================================================
    // subscribe() 注册一个订阅，返回订阅 ID（用于取消订阅）
    //
    // 参数说明：
    // - pattern: 主题模式，支持通配符
    //   - "orders/created"  → 精确匹配
    //   - "orders/*"        → 匹配 orders/ 下的任意单级
    //   - "orders/**"       → 匹配 orders/ 下的任意多级
    // - subscriberId: 订阅者标识（通常是插件 ID）
    // - options: 订阅选项（优先级、是否异步等）
    //
    // QString subId = eventBus->subscribe(
    //     "orders/created",           // 主题模式
    //     "com.biiz.rules",           // 订阅者 ID
    //     nullptr,                    // 回调（nullptr 则用信号）
    //     mpf::SubscriptionOptions{}  // 默认选项
    // );
    //
    // 或将 EventBus 转为 EventBusService 对象后使用简化版本：
    // auto* busObj = dynamic_cast<QObject*>(eventBus);
    // 在 QML 中可以直接调用: EventBus.subscribeSimple("orders/created", "com.biiz.rules")

    // =========================================================================
    // 监听事件（通过 Qt 信号）
    // =========================================================================
    // EventBusService 会在事件发布时发射 eventPublished 信号
    // 插件需要连接这个信号来接收事件
    //
    // 方式1：在 C++ 中连接信号
    // QObject* eventBusObj = dynamic_cast<QObject*>(eventBus);
    // connect(eventBusObj, SIGNAL(eventPublished(QString,QVariantMap,QString)),
    //         this,        SLOT(onEventReceived(QString,QVariantMap,QString)));
    //
    // 方式2：使用 QML Connections（见 QML 样例）

    // =========================================================================
    // 发布事件
    // =========================================================================
    // publish() 异步发布事件，返回通知的订阅者数量
    //
    // int notified = eventBus->publish(
    //     "orders/created",           // 主题
    //     {                           // 事件数据（QVariantMap）
    //         {"orderId", "abc123"},
    //         {"customerName", "张三"},
    //         {"totalAmount", 299.99}
    //     },
    //     "com.yourco.orders"         // 发送者 ID
    // );
    //
    // qDebug() << "通知了" << notified << "个订阅者";

    // =========================================================================
    // 同步发布
    // =========================================================================
    // publishSync() 同步发布，阻塞直到所有处理器完成
    // 适用于需要确保所有订阅者都已处理完成的场景
    //
    // int notified = eventBus->publishSync(
    //     "orders/validated",
    //     {{"orderId", "abc123"}, {"valid", true}},
    //     "com.yourco.orders"
    // );

    // =========================================================================
    // 取消订阅
    // =========================================================================
    // 在插件 stop() 中取消所有订阅：
    //
    // eventBus->unsubscribeAll("com.biiz.rules");
    //
    // 或取消特定订阅：
    // eventBus->unsubscribe(subId);
}

/**
 * 样例：通配符主题匹配
 *
 * 通配符让订阅者可以监听一类事件，而不是单个事件。
 */
void example_wildcard_matching()
{
    // 假设发布了以下事件：
    // publish("orders/created", ...)
    // publish("orders/updated", ...)
    // publish("orders/items/added", ...)
    // publish("rules/triggered", ...)

    // 订阅 "orders/*" 会匹配：
    // ✓ orders/created
    // ✓ orders/updated
    // ✗ orders/items/added （* 只匹配单级）
    // ✗ rules/triggered

    // 订阅 "orders/**" 会匹配：
    // ✓ orders/created
    // ✓ orders/updated
    // ✓ orders/items/added （** 匹配多级）
    // ✗ rules/triggered

    // 订阅 "**" 会匹配所有事件（谨慎使用！）

    // 内部实现：通配符被转换为正则表达式
    // "orders/*"  → "^orders/[^/]+$"
    // "orders/**" → "^orders/.+$"
}

/**
 * 样例：EventBus 的查询功能
 *
 * EventBus 提供了丰富的查询方法，用于调试和监控。
 */
void example_eventbus_query()
{
    // 查询某个主题的订阅者数量：
    // int count = eventBus->subscriberCount("orders/created");
    //
    // 获取所有活跃主题：
    // QStringList topics = eventBus->activeTopics();
    //
    // 获取某个订阅者的所有订阅：
    // QStringList subs = eventBus->subscriptionsFor("com.biiz.rules");
    //
    // 获取主题统计信息：
    // mpf::TopicStats stats = eventBus->topicStats("orders/created");
    // qDebug() << "订阅者:" << stats.subscriberCount;
    // qDebug() << "事件数:" << stats.eventCount;
    // qDebug() << "最后事件时间:" << stats.lastEventTime;
    //
    // 检查主题是否匹配模式：
    // bool matches = eventBus->matchesTopic("orders/created", "orders/*");
    // // matches == true
}

/**
 * 样例：EventBus 的 SubscriptionOptions
 *
 * 控制事件的投递行为。
 */
void example_subscription_options()
{
    // mpf::SubscriptionOptions options;
    //
    // options.async = true;           // 异步投递（默认），事件通过 Qt 事件队列投递
    //                                 // false = 同步投递，在发布线程直接调用
    //
    // options.priority = 10;          // 优先级（数字越大越先收到）
    //                                 // 默认 0，可以用来确保某些处理器先执行
    //
    // options.receiveOwnEvents = false; // 是否接收自己发送的事件（默认不接收）
    //                                   // 避免消息循环
    //
    // QString subId = eventBus->subscribe("orders/**", "com.biiz.rules", nullptr, options);
}
