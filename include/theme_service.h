#pragma once

#include "mpf/interfaces/itheme.h"
#include <QHash>
#include <QJsonObject>

namespace mpf {

/**
 * @brief Theme data structure
 */
struct ThemeData
{
    QString name;
    bool isDark = false;
    
    QColor primaryColor;
    QColor accentColor;
    QColor backgroundColor;
    QColor surfaceColor;
    QColor textColor;
    QColor textSecondaryColor;
    QColor errorColor;
    QColor warningColor;
    QColor successColor;
    
    int spacingTiny = 4;
    int spacingSmall = 8;
    int spacingMedium = 16;
    int spacingLarge = 24;
    
    int radiusSmall = 4;
    int radiusMedium = 8;
    int radiusLarge = 16;
    
    static ThemeData fromJson(const QJsonObject& json);
    static ThemeData lightTheme();
    static ThemeData darkTheme();
};

/**
 * @brief Default theme service implementation
 */
class ThemeService : public QObject, public ITheme
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name NOTIFY themeChanged)
    Q_PROPERTY(bool isDark READ isDark NOTIFY themeChanged)
    Q_PROPERTY(QColor primaryColor READ primaryColor NOTIFY themeChanged)
    Q_PROPERTY(QColor accentColor READ accentColor NOTIFY themeChanged)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor NOTIFY themeChanged)
    Q_PROPERTY(QColor surfaceColor READ surfaceColor NOTIFY themeChanged)
    Q_PROPERTY(QColor textColor READ textColor NOTIFY themeChanged)
    Q_PROPERTY(QColor textSecondaryColor READ textSecondaryColor NOTIFY themeChanged)
    Q_PROPERTY(QColor errorColor READ errorColor NOTIFY themeChanged)
    Q_PROPERTY(QColor warningColor READ warningColor NOTIFY themeChanged)
    Q_PROPERTY(QColor successColor READ successColor NOTIFY themeChanged)
    Q_PROPERTY(int spacingTiny READ spacingTiny NOTIFY themeChanged)
    Q_PROPERTY(int spacingSmall READ spacingSmall NOTIFY themeChanged)
    Q_PROPERTY(int spacingMedium READ spacingMedium NOTIFY themeChanged)
    Q_PROPERTY(int spacingLarge READ spacingLarge NOTIFY themeChanged)
    Q_PROPERTY(int radiusSmall READ radiusSmall NOTIFY themeChanged)
    Q_PROPERTY(int radiusMedium READ radiusMedium NOTIFY themeChanged)
    Q_PROPERTY(int radiusLarge READ radiusLarge NOTIFY themeChanged)

public:
    explicit ThemeService(QObject* parent = nullptr);
    ~ThemeService() override;

    // ITheme interface
    QString name() const override { return m_current.name; }
    bool isDark() const override { return m_current.isDark; }

    QColor primaryColor() const override { return m_current.primaryColor; }
    QColor accentColor() const override { return m_current.accentColor; }
    QColor backgroundColor() const override { return m_current.backgroundColor; }
    QColor surfaceColor() const override { return m_current.surfaceColor; }
    QColor textColor() const override { return m_current.textColor; }
    QColor textSecondaryColor() const override { return m_current.textSecondaryColor; }
    QColor errorColor() const override { return m_current.errorColor; }
    QColor warningColor() const override { return m_current.warningColor; }
    QColor successColor() const override { return m_current.successColor; }

    int spacingTiny() const override { return m_current.spacingTiny; }
    int spacingSmall() const override { return m_current.spacingSmall; }
    int spacingMedium() const override { return m_current.spacingMedium; }
    int spacingLarge() const override { return m_current.spacingLarge; }

    int radiusSmall() const override { return m_current.radiusSmall; }
    int radiusMedium() const override { return m_current.radiusMedium; }
    int radiusLarge() const override { return m_current.radiusLarge; }

    Q_INVOKABLE void setTheme(const QString& themeName) override;
    Q_INVOKABLE QStringList availableThemes() const override;

    /**
     * @brief Register a custom theme
     */
    void registerTheme(const ThemeData& theme);

    /**
     * @brief Load themes from JSON file
     */
    bool loadThemes(const QString& path);

signals:
    void themeChanged();

private:
    ThemeData m_current;
    QHash<QString, ThemeData> m_themes;
};

} // namespace mpf
