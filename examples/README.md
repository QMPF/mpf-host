# MPF Host 样例代码

本目录包含 MPF 宿主应用的架构样例，帮助理解 Host 如何组织和管理整个框架。

## 文件说明

| 文件 | 展示内容 |
|------|----------|
| `01_service_registry.cpp` | ServiceRegistry 的实现原理与服务注册 |
| `02_event_bus.cpp` | EventBus 实现与事件分发机制 |
| `03_plugin_lifecycle.cpp` | 插件发现、加载、初始化的完整流程 |
| `04_qml_context.cpp` | 如何将服务暴露给 QML 层 |
| `05_theme_and_settings.cpp` | Theme/Settings 服务的实现与使用 |

## 关键概念

### Host 的角色
Host 是整个 MPF 框架的**核心枢纽**，负责：
1. **创建并拥有**所有核心服务（Navigation, Settings, Theme, Menu, EventBus, Logger）
2. **注册服务**到 ServiceRegistry
3. **发现和加载**插件（.dll/.dylib/.so）
4. **暴露服务**给 QML 层（通过 context property）
5. **管理生命周期**（初始化 → 启动 → 停止）

### 依赖方向
```
Host 不依赖任何插件（零耦合）
Host 只依赖 mpf-sdk（接口定义）
插件通过 SDK 接口与 Host 交互（依赖倒置）
```

## 学习顺序

建议按文件编号顺序阅读：
1. 先理解 ServiceRegistry 如何存储和查找服务
2. 再理解 EventBus 如何实现跨插件通信
3. 然后看插件的完整生命周期
4. 最后看 QML 集成和主题系统
