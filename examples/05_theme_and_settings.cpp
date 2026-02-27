/**
 * =============================================================================
 * 样例 05: Theme 和 Settings 服务
 * =============================================================================
 *
 * 【这个样例展示了什么】
 * Theme 和 Settings 是 Host 提供的两个重要服务：
 * - Theme: 统一的视觉风格系统，支持亮/暗主题切换
 * - Settings: 插件隔离的键值存储，持久化到本地文件
 *
 * 【Theme 的设计理念】
 * 所有 UI 组件都应该使用 Theme 提供的颜色和间距，
 * 而不是硬编码值。这样切换主题时所有 UI 会自动更新。
 *
 * 【Settings 的命名空间隔离】
 * Settings 使用 pluginId 作为命名空间，
 * 不同插件的设置互不干扰。
 * =============================================================================
 */

/**
 * 样例：ThemeService 的内部实现
 */
void example_theme_implementation()
{
    // =========================================================================
    // ThemeService 继承关系
    // =========================================================================
    // class ThemeService : public QObject, public mpf::ITheme
    //
    // 通过 Q_PROPERTY 将所有颜色和间距暴露给 QML：
    //
    // Q_PROPERTY(QColor primaryColor       READ primaryColor    NOTIFY themeChanged)
    // Q_PROPERTY(QColor backgroundColor    READ backgroundColor NOTIFY themeChanged)
    // Q_PROPERTY(QColor textColor          READ textColor       NOTIFY themeChanged)
    // Q_PROPERTY(int spacingMedium         READ spacingMedium   NOTIFY themeChanged)
    // Q_PROPERTY(int radiusMedium          READ radiusMedium    NOTIFY themeChanged)
    // ... 等等
    //
    // 当调用 setTheme("dark") 时，emit themeChanged() 信号，
    // 所有绑定了 Theme.xxx 的 QML 属性会自动更新。

    // =========================================================================
    // 内置主题
    // =========================================================================
    // ThemeData::lightTheme() 返回亮色主题预设：
    // - primaryColor:     #2196F3 (Material Blue)
    // - backgroundColor:  #FAFAFA
    // - textColor:        #212121
    // - surfaceColor:     #FFFFFF
    //
    // ThemeData::darkTheme() 返回暗色主题预设：
    // - primaryColor:     #90CAF9 (Light Blue)
    // - backgroundColor:  #121212
    // - textColor:        #E0E0E0
    // - surfaceColor:     #1E1E1E

    // =========================================================================
    // 自定义主题
    // =========================================================================
    // Host 可以注册自定义主题：
    //
    // ThemeData custom;
    // custom.name = "ocean";
    // custom.primaryColor = QColor("#0077B6");
    // custom.backgroundColor = QColor("#CAF0F8");
    // custom.textColor = QColor("#03045E");
    // ... 
    // themeService->registerTheme(custom);
    //
    // 或从 JSON 文件加载：
    // themeService->loadThemes("/path/to/themes.json");
}

/**
 * 样例：SettingsService 的使用
 */
void example_settings_usage()
{
    // =========================================================================
    // C++ 层使用 Settings
    // =========================================================================
    //
    // 在插件代码中：
    // auto* settings = registry->get<mpf::ISettings>();
    //
    // // 写入设置（自动按 pluginId 隔离）
    // settings->setValue("com.yourco.orders", "pageSize", 20);
    // settings->setValue("com.yourco.orders", "defaultSort", "createdAt");
    //
    // // 读取设置（提供默认值）
    // int pageSize = settings->value("com.yourco.orders", "pageSize", 10).toInt();
    // QString sort = settings->value("com.yourco.orders", "defaultSort", "id").toString();
    //
    // // 检查是否存在
    // if (settings->contains("com.yourco.orders", "apiToken")) {
    //     // ...
    // }
    //
    // // 获取插件的所有键
    // QStringList keys = settings->keys("com.yourco.orders");
    //
    // // 删除设置
    // settings->remove("com.yourco.orders", "tempKey");
    //
    // // 强制同步到磁盘
    // settings->sync();

    // =========================================================================
    // QML 层使用 Settings
    // =========================================================================
    //
    // // 保存用户偏好
    // Settings.setValue("com.yourco.orders", "viewMode", "grid")
    //
    // // 读取用户偏好
    // var mode = Settings.value("com.yourco.orders", "viewMode", "list")
    //
    // // 在 Component.onCompleted 中恢复状态
    // Component.onCompleted: {
    //     var savedFilter = Settings.value("com.yourco.orders", "lastFilter", "all")
    //     filterCombo.currentIndex = filterCombo.model.indexOf(savedFilter)
    // }
    //
    // // 在 Component.onDestruction 中保存状态
    // Component.onDestruction: {
    //     Settings.setValue("com.yourco.orders", "lastFilter",
    //                       filterCombo.currentText)
    //     Settings.sync()
    // }

    // =========================================================================
    // 命名空间隔离
    // =========================================================================
    // 不同插件的设置完全隔离：
    //
    // settings->setValue("com.yourco.orders", "theme", "dark");
    // settings->setValue("com.biiz.rules",    "theme", "light");
    //
    // // 互不影响：
    // settings->value("com.yourco.orders", "theme");  // → "dark"
    // settings->value("com.biiz.rules",    "theme");  // → "light"
    //
    // SettingsService 内部使用 QSettings，
    // 键名格式为 "pluginId/key"，存储在系统标准配置位置：
    // - macOS: ~/Library/Preferences/com.mpf.QtModularPluginFramework.plist
    // - Linux: ~/.config/MPF/QtModularPluginFramework.conf
    // - Windows: Registry HKEY_CURRENT_USER\Software\MPF\QtModularPluginFramework
}
