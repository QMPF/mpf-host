/**
 * =============================================================================
 * 样例 01: ServiceRegistry —— 服务注册与发现
 * =============================================================================
 *
 * 【这个样例展示了什么】
 * ServiceRegistry 是 MPF 框架的核心容器，负责管理所有服务实例。
 * 它实现了"服务定位器"模式，让插件可以在运行时发现和使用其他服务，
 * 而不需要在编译时知道具体实现类。
 *
 * 【为什么需要 ServiceRegistry】
 * 1. 解耦：插件只依赖 SDK 中的接口（如 INavigation），不依赖 Host 中的实现类
 * 2. 可替换：Host 可以更换服务实现而不影响插件
 * 3. 版本控制：支持按最低版本号查询，保证 API 兼容性
 * 4. 生命周期管理：集中管理所有服务的注册和销毁
 *
 * 【关键设计】
 * - SDK 中的 ServiceRegistry 是纯虚基类（见 mpf-sdk/include/mpf/service_registry.h）
 * - Host 中的 ServiceRegistryImpl 是唯一实现（继承 QObject + ServiceRegistry）
 * - 使用 typeid(T).name() 作为键，保证类型安全
 * - 服务实例必须继承 QObject（通过 dynamic_cast 检查）
 *
 * 【架构意义】
 * 插件在 initialize() 中收到的 ServiceRegistry* 指针，
 * 实际上是 Host 持有的 ServiceRegistryImpl 实例。
 * 插件通过 SDK 的模板方法 registry->get<INavigation>() 获取服务，
 * 底层调用 getService(typeid(INavigation).name(), ...)。
 * =============================================================================
 */

// ---- 以下为伪代码，展示 Host 中 ServiceRegistry 的使用方式 ----

#include <mpf/service_registry.h>
#include <mpf/interfaces/inavigation.h>
#include <mpf/interfaces/isettings.h>
#include <mpf/interfaces/itheme.h>
#include <mpf/interfaces/imenu.h>
#include <mpf/interfaces/ieventbus.h>
#include <mpf/interfaces/ilogger.h>

// Host 的具体实现头文件（插件不可见）
// #include "service_registry.h"  // ServiceRegistryImpl
// #include "navigation_service.h"
// #include "settings_service.h"
// #include "theme_service.h"
// #include "menu_service.h"
// #include "event_bus_service.h"

/**
 * 样例：Host 如何创建和填充 ServiceRegistry
 *
 * 这段代码展示了 Application::initialize() 中的核心流程。
 * Host 创建所有服务实例，然后注册到 ServiceRegistry 中。
 */
void example_host_service_registration()
{
    // =========================================================================
    // 第一步：创建 ServiceRegistry 实例
    // =========================================================================
    // ServiceRegistryImpl 同时继承 QObject 和 mpf::ServiceRegistry
    // - QObject 提供信号（serviceAdded/serviceRemoved）
    // - ServiceRegistry 提供模板方法（add/get/has）
    //
    // 只有 Host 持有 ServiceRegistryImpl*，插件只看到 ServiceRegistry*
    // auto m_registry = std::make_unique<ServiceRegistryImpl>(this);

    // =========================================================================
    // 第二步：创建核心服务
    // =========================================================================
    // 每个服务都继承 QObject + 对应的 SDK 接口
    // 例如 NavigationService : public QObject, public mpf::INavigation
    //
    // auto* navigation = new NavigationService(this);
    // auto* settings   = new SettingsService(m_configPath, this);
    // auto* theme      = new ThemeService(this);
    // auto* menu       = new MenuService(this);
    // auto* eventBus   = new EventBusService(this);

    // =========================================================================
    // 第三步：注册服务到 Registry
    // =========================================================================
    // add<T>() 的流程：
    // 1. 用 typeid(T).name() 生成键（如 "N3mpf11INavigationE"）
    // 2. dynamic_cast<QObject*>(instance) 获取 QObject 指针
    // 3. 存入 QHash<QString, ServiceEntry>
    //
    // 第二个参数是 API 版本号，用于版本兼容性检查
    // 第三个参数标识提供者（"host" 表示宿主自身提供）
    //
    // m_registry->add<INavigation>(navigation, INavigation::apiVersion(), "host");
    // m_registry->add<ISettings>(settings,     ISettings::apiVersion(),   "host");
    // m_registry->add<ITheme>(theme,           ITheme::apiVersion(),      "host");
    // m_registry->add<IMenu>(menu,             IMenu::apiVersion(),       "host");
    // m_registry->add<IEventBus>(eventBus,     IEventBus::apiVersion(),   "host");

    // =========================================================================
    // 第四步：插件如何获取服务（在插件代码中）
    // =========================================================================
    // 插件在 initialize(ServiceRegistry* registry) 中：
    //
    // auto* nav = registry->get<mpf::INavigation>();   // 获取导航服务
    // auto* menu = registry->get<mpf::IMenu>();         // 获取菜单服务
    //
    // get<T>() 的流程：
    // 1. 用 typeid(T).name() 查找
    // 2. 检查版本号
    // 3. 返回 dynamic_cast<T*>(storedQObject)
    //
    // 如果服务不存在或版本不满足，返回 nullptr
    // 插件应该检查返回值：
    // if (nav) { nav->registerRoute("orders", pageUrl); }

    // =========================================================================
    // 插件也可以注册自己的服务
    // =========================================================================
    // 例如 orders 插件可以注册 OrdersService 供其他插件使用：
    //
    // bool OrdersPlugin::initialize(mpf::ServiceRegistry* registry) {
    //     m_ordersService = std::make_unique<OrdersService>(this);
    //     // 其他插件可以通过 registry->get<OrdersServiceInterface>() 获取
    //     // 注意：需要在 SDK 中定义公共接口
    //     return true;
    // }
}

/**
 * 样例：ServiceRegistry 的版本检查机制
 *
 * 版本号用于保证 API 兼容性。当 Host 升级了某个服务的 API，
 * 旧插件仍然可以工作（只要版本号兼容）。
 */
void example_version_check()
{
    // 假设 INavigation 当前版本是 3
    // static constexpr int apiVersion() { return 3; }
    //
    // 注册时：
    // registry->add<INavigation>(nav, 3, "host");
    //
    // 查询时：
    // registry->get<INavigation>(2);  // 要求最低版本 2 → 成功（3 >= 2）
    // registry->get<INavigation>(3);  // 要求最低版本 3 → 成功（3 >= 3）
    // registry->get<INavigation>(4);  // 要求最低版本 4 → 失败（3 < 4），返回 nullptr
    //
    // 插件可以这样做版本适配：
    // auto* nav = registry->get<INavigation>(2);  // 只需要 v2 的功能
    // if (!nav) {
    //     MPF_LOG_ERROR("MyPlugin", "Navigation service v2+ required");
    //     return false;
    // }
}

/**
 * 样例：ServiceRegistry 的线程安全
 *
 * ServiceRegistryImpl 内部使用 QMutex 保护所有操作，
 * 可以安全地从任何线程调用 get/add/has。
 */
void example_thread_safety()
{
    // 内部实现使用 QMutexLocker：
    //
    // QObject* ServiceRegistryImpl::getService(const char* typeName, int minVersion) {
    //     QMutexLocker locker(&m_mutex);  // 自动加锁/解锁
    //     auto it = m_services.find(name);
    //     ...
    // }
    //
    // 这意味着插件可以在工作线程中安全地获取服务：
    // QtConcurrent::run([registry]() {
    //     auto* settings = registry->get<ISettings>();
    //     settings->setValue("myPlugin", "key", "value");
    // });
}
