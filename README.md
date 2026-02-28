# MPF Host

> 📖 **[MPF 开发环境完整教程](https://github.com/QMPF/mpf-dev/blob/main/docs/USAGE.md)** — 安装指南、命令参考、开发流程、IDE 配置、常见问题

Qt Modular Plugin Framework - 宿主应用程序

## 概述

MPF Host 是插件框架的宿主应用，负责：
- 加载和管理插件（通过 `PluginManager`，支持依赖拓扑排序）
- 实现核心服务接口（Navigation, Menu, Theme, Settings, EventBus, Logger）
- 提供 QML Shell（Main.qml + SideMenu + Loader-based 页面切换）
- 运行时自动读取 `~/.mpf-sdk/dev.json` 发现源码构建的组件
- 跨 DLL 内存安全（`CrossDllSafety::deepCopy()` 用于所有从插件传入的数据）

## 依赖

- Qt 6.8+（Core, Gui, Qml, Quick, QuickControls2）
- MPF foundation-sdk
- MPF ui-components（Host **直接链接**以避免跨 DLL 堆问题；插件通过 QML import 运行时访问）

## 构建

```bash
# 1. 安装 mpf-dev 和 SDK
mpf-dev setup

# 2. 初始化项目（生成 CMakeUserPresets.json）
mpf-dev init

# 3. 构建
cmake --preset dev
cmake --build build
```

## 运行

```bash
# 方式一：通过 mpf-dev（自动注入开发路径）
mpf-dev run

# 方式二：直接运行（Host 自动读取 dev.json）
./build/bin/mpf-host
```

## 源码开发注册

```bash
# 注册 Host 构建输出到 dev.json
mpf-dev link host ./build
```

## 启动流程

1. `main()` — 设置 Basic 样式，创建 `Application`
2. `Application::initialize()`:
   - `setupPaths()` — 检测 SDK 路径、读取 dev.json、构建搜索路径
   - `setupLogging()` — 创建 Logger 实例
   - 创建 `ServiceRegistryImpl`，注册 6 个核心服务
   - 创建 `QQmlApplicationEngine`
   - `setupQmlContext()` — 注入 `App`, `Navigation`, `Theme`, `AppMenu`, `Settings`, `EventBus` 到 QML
   - `loadPlugins()` — 发现→加载→初始化→启动
   - `loadMainQml()` — 加载 `MPF/Host/Main.qml`

## SDK/插件路径发现

Host 按以下优先级搜索路径：

1. `MPF_SDK_ROOT` 环境变量（`mpf-dev run` 设置）
2. `~/.mpf-sdk/current`（自动检测，支持 Qt Creator 直接调试）
3. 相对于可执行文件的路径（本地构建/安装模式）

`dev.json` 中注册的源码组件路径会被自动添加到搜索路径中。

## 核心服务

| 服务 | QML 上下文名 | 接口 | 说明 |
|------|-------------|------|------|
| NavigationService | `Navigation` | `INavigation` | Loader-based 路由管理 |
| MenuService | `AppMenu` | `IMenu` | 侧边栏菜单管理 |
| ThemeService | `Theme` | `ITheme` | 主题切换（Light/Dark） |
| SettingsService | `Settings` | `ISettings` | 配置持久化（QSettings + INI） |
| EventBusService | `EventBus` | `IEventBus` | 跨插件事件总线（pub/sub + request/response） |
| Logger | — | `ILogger` | 分级日志 |

此外还有 `App` 对象（`QmlContext`）暴露版本号和所有服务的 QObject 引用。

## QML Shell

- `Main.qml` — ApplicationWindow + SideMenu + Loader 内容区 + WelcomePage
- `SideMenu.qml` — 从 `AppMenu.items` 动态渲染菜单项，支持折叠
- `MenuItemCustom.qml` — 单个菜单项（图标、标签、徽章、选中高亮）
- `ErrorDialog.qml` — 错误提示对话框

## 插件管理

`PluginManager` 负责完整的插件生命周期：

1. **discover()** — 扫描目录中的 `.dylib`/`.so`/`.dll` 文件，读取 MetaData
2. **loadAll()** — 按拓扑排序顺序加载（依赖先加载）
3. **initializeAll()** — 调用 `IPlugin::initialize(registry)`
4. **startAll()** — 调用 `IPlugin::start()`
5. **stopAll()** — 逆序调用 `IPlugin::stop()`

## 测试

```bash
cd tests
cmake -B build -DCMAKE_PREFIX_PATH="/path/to/qt6;/path/to/sdk"
cmake --build build
./build/test_event_bus              # EventBus 单元测试
./build/test_plugin_dependencies    # 插件依赖测试
```

## 许可证

MIT License
