#ifndef THEMEMANAGER_H
#define THEMEMANAGER_H

#include <QObject>
#include <QApplication>
#include <QString>

class ThemeManager : public QObject {
    Q_OBJECT
public:
    enum class ThemeMode { System, Dark, Light };
    Q_ENUM(ThemeMode)

    explicit ThemeManager(QApplication* app, QObject* parent = nullptr);

    void setThemeMode(ThemeMode mode);
    ThemeMode themeMode() const;
    bool isDark() const;

    void applyTheme();

signals:
    void themeChanged(bool isDark);

private:
    bool detectSystemIsDark() const;
    QString buildStyleSheet(bool dark) const;

    QApplication* m_app;
    ThemeMode m_mode = ThemeMode::System;
    bool m_currentIsDark = false;
};

#endif // THEMEMANAGER_H
