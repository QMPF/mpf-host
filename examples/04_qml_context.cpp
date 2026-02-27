/**
 * =============================================================================
 * 样例 04: QML Context —— 将服务暴露给 QML 层
 * =============================================================================
 *
 * 【这个样例展示了什么】
 * Host 如何将 C++ 服务对象暴露给 QML，使 QML 代码可以直接访问
 * Navigation、Theme、Settings、Menu、EventBus 等服务。
 *
 * 【为什么需要 QmlContext】
 * QML 和 C++ 是两个不同的世界。QML 不能直接 #include C++ 头文件。
 * 需要通过 Qt 的集成机制将 C++ 对象"注入"到 QML 环境中。
 *
 * 【注入方式】
 * Host 使用 setContextProperty() 将服务注册为全局 QML 对象。
 * 注册后，任何 QML 文件都可以直接使用这些名称（无需 import）。
 *
 * 【注入的全局对象】
 * - Navigation : 导航服务（页面路由）
 * - Settings   : 设置服务（键值存储）
 * - Theme      : 主题服务（颜色、间距等）
 * - AppMenu    : 菜单服务（侧边栏菜单项）
 * - EventBus   : 事件总线（跨插件通信）
 * - App        : QmlContext 辅助对象（版本号等）
 * =============================================================================
 */

/**
 * 样例：QmlContext 的 setup() 方法
 *
 * 这是 Host 中将服务暴露给 QML 的核心代码。
 */
void example_qml_context_setup()
{
    // QmlContext::setup(QQmlApplicationEngine* engine) 中：
    //
    // =========================================================================
    // 注册 App 辅助对象
    // =========================================================================
    // engine->rootContext()->setContextProperty("App", this);
    //
    // QML 中使用：
    //   Text { text: "MPF v" + App.version }
    //
    // =========================================================================
    // 注册核心服务
    // =========================================================================
    // engine->rootContext()->setContextProperty("Navigation", navigation());
    // engine->rootContext()->setContextProperty("Settings",   settings());
    // engine->rootContext()->setContextProperty("Theme",      theme());
    // engine->rootContext()->setContextProperty("AppMenu",    appMenu());
    // engine->rootContext()->setContextProperty("EventBus",   eventBus());
    //
    // 【关键实现细节】
    // QmlContext 从 ServiceRegistry 获取服务时，使用 getObject<T>()
    // 而不是 get<T>()。
    //
    // 原因：服务类使用多重继承（QObject + ITheme）。
    // get<T>() 返回 dynamic_cast<ITheme*>(qobj)，得到 ITheme* 指针。
    // 但 QML 需要 QObject* 指针才能访问 Q_PROPERTY 和 Q_INVOKABLE。
    //
    // getObject<T>() 直接返回存储的 QObject*，避免了指针偏移问题。
    //
    // QObject* QmlContext::theme() const {
    //     return m_registry->getObject<ITheme>();  // 返回 QObject*
    // }
}

/**
 * 样例：QML 中使用 Theme 对象
 *
 * Theme 是最常用的全局对象，提供统一的视觉风格。
 */
void example_theme_in_qml()
{
    // 在任意 QML 文件中直接使用 Theme：
    //
    // Rectangle {
    //     color: Theme.backgroundColor        // 背景色
    //
    //     Text {
    //         text: "Hello MPF"
    //         color: Theme.textColor           // 文字颜色
    //         font.pixelSize: 16
    //     }
    //
    //     // 间距
    //     anchors.margins: Theme.spacingMedium // 16px
    //
    //     // 圆角
    //     radius: Theme.radiusMedium           // 8px
    // }
    //
    // 【主题切换】
    // Theme.setTheme("dark")  // 切换到暗色主题
    // 所有绑定了 Theme 属性的组件会自动更新
    //
    // 【防御性编码】
    // 推荐使用三元运算符，防止 Theme 未定义：
    // color: Theme ? Theme.backgroundColor : "#FFFFFF"
    //
    // 这在独立测试 QML 组件时很有用（此时没有 Host 提供 Theme）
}

/**
 * 样例：QML 中使用 EventBus 对象
 *
 * EventBus 可以在 QML 中发布和订阅事件。
 */
void example_eventbus_in_qml()
{
    // =========================================================================
    // 在 QML 中发布事件
    // =========================================================================
    //
    // Button {
    //     text: "创建订单"
    //     onClicked: {
    //         EventBus.publish("orders/created", {
    //             "orderId": "123",
    //             "customerName": "张三"
    //         }, "com.yourco.orders")
    //     }
    // }
    //
    // =========================================================================
    // 在 QML 中订阅事件
    // =========================================================================
    //
    // 方式1：使用 Connections 监听 eventPublished 信号
    //
    // Connections {
    //     target: EventBus
    //     function onEventPublished(topic, data, senderId) {
    //         if (topic === "orders/created") {
    //             console.log("收到订单创建事件:", JSON.stringify(data))
    //             // 执行业务逻辑...
    //         }
    //     }
    // }
    //
    // 方式2：先 subscribe 再监听（带过滤）
    //
    // Component.onCompleted: {
    //     EventBus.subscribeSimple("orders/*", "com.biiz.rules")
    // }
    //
    // Component.onDestruction: {
    //     EventBus.unsubscribeAll("com.biiz.rules")
    // }
    //
    // 【注意】
    // QML 层的 Connections 会接收所有事件，需要在 onEventPublished 中
    // 手动过滤 topic。subscribe 的作用是注册订阅关系（影响 subscriberCount
    // 和 notified 返回值），但实际事件投递是通过 Qt 信号机制。
}

/**
 * 样例：QML 中使用 Navigation 和 AppMenu
 */
void example_navigation_and_menu_in_qml()
{
    // =========================================================================
    // Navigation：页面导航
    // =========================================================================
    //
    // Host 的 Main.qml 中使用 Navigation 进行页面切换：
    //
    // Loader {
    //     id: pageLoader
    //     source: Navigation.getPageUrl(Navigation.currentRoute)
    //     // 当 currentRoute 改变时，Loader 自动加载新页面
    // }
    //
    // 侧边栏菜单项点击时：
    // Navigation.setCurrentRoute("orders")  // 切换到订单页面
    //
    // =========================================================================
    // AppMenu：菜单管理
    // =========================================================================
    //
    // 侧边栏渲染菜单项：
    //
    // Repeater {
    //     model: AppMenu.itemsAsVariant()
    //     delegate: MenuItem {
    //         text: modelData.label
    //         icon: modelData.icon
    //         badge: modelData.badge
    //         onClicked: Navigation.setCurrentRoute(modelData.route)
    //     }
    // }
    //
    // =========================================================================
    // Settings：设置存储
    // =========================================================================
    //
    // 保存设置：
    // Settings.setValue("com.yourco.orders", "lastFilter", "pending")
    //
    // 读取设置：
    // var filter = Settings.value("com.yourco.orders", "lastFilter", "all")
}
