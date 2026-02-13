#include "thememanager.h"

#include <QStyleHints>
#include <QSettings>

struct ThemePalette {
    QString windowBg;
    QString panelBg;
    QString inputBg;
    QString dropZoneBg;
    QString dropZoneHoverBg;

    QString textPrimary;
    QString textSecondary;
    QString textPlaceholder;

    QString border;
    QString borderFocused;

    QString accent;
    QString accentHover;
    QString accentPressed;
    QString accentText;

    QString errorBg;
    QString errorText;
    QString infoBg;
    QString infoText;

    QString tabBg;
    QString tabActiveBg;
    QString tabText;
    QString tabActiveText;
    QString tabBorder;

    QString checkboxBorder;
    QString checkboxCheckedBg;

    QString scrollbarBg;
    QString scrollbarHandle;
};

static const ThemePalette DarkPalette {
    "#1e1e1e",  // windowBg
    "#2a2a2a",  // panelBg
    "#333333",  // inputBg
    "#252525",  // dropZoneBg
    "#2f3640",  // dropZoneHoverBg

    "#e0e0e0",  // textPrimary
    "#a0a0a0",  // textSecondary
    "#666666",  // textPlaceholder

    "#444444",  // border
    "#5a9cf5",  // borderFocused

    "#4a90d9",  // accent
    "#5a9cf5",  // accentHover
    "#3a7bc8",  // accentPressed
    "#ffffff",  // accentText

    "#c0392b",  // errorBg
    "#ffffff",  // errorText
    "#2a6496",  // infoBg
    "#ffffff",  // infoText

    "#2a2a2a",  // tabBg
    "#1e1e1e",  // tabActiveBg
    "#888888",  // tabText
    "#e0e0e0",  // tabActiveText
    "#444444",  // tabBorder

    "#666666",  // checkboxBorder
    "#4a90d9",  // checkboxCheckedBg

    "#2a2a2a",  // scrollbarBg
    "#555555",  // scrollbarHandle
};

static const ThemePalette LightPalette {
    "#f5f5f5",  // windowBg
    "#ffffff",  // panelBg
    "#ffffff",  // inputBg
    "#eaeaea",  // dropZoneBg
    "#dce8f5",  // dropZoneHoverBg

    "#1a1a1a",  // textPrimary
    "#555555",  // textSecondary
    "#aaaaaa",  // textPlaceholder

    "#cccccc",  // border
    "#4a90d9",  // borderFocused

    "#4a90d9",  // accent
    "#3a7bc8",  // accentHover
    "#2a6ab5",  // accentPressed
    "#ffffff",  // accentText

    "#e74c3c",  // errorBg
    "#ffffff",  // errorText
    "#3498db",  // infoBg
    "#ffffff",  // infoText

    "#eaeaea",  // tabBg
    "#ffffff",  // tabActiveBg
    "#777777",  // tabText
    "#1a1a1a",  // tabActiveText
    "#cccccc",  // tabBorder

    "#bbbbbb",  // checkboxBorder
    "#4a90d9",  // checkboxCheckedBg

    "#f0f0f0",  // scrollbarBg
    "#cccccc",  // scrollbarHandle
};

ThemeManager::ThemeManager(QApplication* app, QObject* parent)
    : QObject(parent), m_app(app)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    connect(app->styleHints(), &QStyleHints::colorSchemeChanged,
            this, [this]() {
        if (m_mode == ThemeMode::System) {
            applyTheme();
        }
    });
#endif
}

void ThemeManager::setThemeMode(ThemeMode mode) {
    m_mode = mode;
}

ThemeManager::ThemeMode ThemeManager::themeMode() const {
    return m_mode;
}

bool ThemeManager::isDark() const {
    return m_currentIsDark;
}

void ThemeManager::applyTheme() {
    bool dark;
    switch (m_mode) {
    case ThemeMode::Dark:   dark = true;  break;
    case ThemeMode::Light:  dark = false; break;
    case ThemeMode::System:
    default:                dark = detectSystemIsDark(); break;
    }

    if (dark != m_currentIsDark || m_app->styleSheet().isEmpty()) {
        m_currentIsDark = dark;
        m_app->setStyleSheet(buildStyleSheet(dark));
        emit themeChanged(dark);
    }
}

bool ThemeManager::detectSystemIsDark() const {
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    auto scheme = m_app->styleHints()->colorScheme();
    return (scheme == Qt::ColorScheme::Dark);
#else
    QSettings reg("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows"
                   "\\CurrentVersion\\Themes\\Personalize",
                   QSettings::NativeFormat);
    return reg.value("AppsUseLightTheme", 1).toInt() == 0;
#endif
}

QString ThemeManager::buildStyleSheet(bool dark) const {
    const auto& p = dark ? DarkPalette : LightPalette;

    return QStringLiteral(
        /* ---- Global ---- */
        "QWidget {"
        "  background-color: %1;"
        "  color: %2;"
        "  font-family: 'Segoe UI', sans-serif;"
        "  font-size: 13px;"
        "}"

        "QMainWindow {"
        "  background-color: %1;"
        "}"

        /* ---- QGroupBox (card panels) ---- */
        "QGroupBox {"
        "  background-color: %3;"
        "  border: 1px solid %4;"
        "  border-radius: 6px;"
        "  margin-top: 14px;"
        "  padding: 12px 10px 10px 10px;"
        "  font-weight: 600;"
        "  font-size: 12px;"
        "  color: %5;"
        "}"
        "QGroupBox::title {"
        "  subcontrol-origin: margin;"
        "  subcontrol-position: top left;"
        "  left: 12px;"
        "  padding: 0 6px;"
        "  color: %5;"
        "  background: transparent;"
        "}"

        /* ---- QSpinBox ---- */
        "QSpinBox {"
        "  background-color: %6;"
        "  border: 1px solid %4;"
        "  border-radius: 4px;"
        "  padding: 4px 6px;"
        "  color: %2;"
        "  min-width: 55px;"
        "  min-height: 22px;"
        "}"
        "QSpinBox:focus {"
        "  border-color: %7;"
        "}"
        "QSpinBox::up-button, QSpinBox::down-button {"
        "  border: none;"
        "  width: 16px;"
        "}"
        "QSpinBox::up-arrow {"
        "  image: none;"
        "  border-left: 4px solid transparent;"
        "  border-right: 4px solid transparent;"
        "  border-bottom: 5px solid %5;"
        "  width: 0; height: 0;"
        "}"
        "QSpinBox::down-arrow {"
        "  image: none;"
        "  border-left: 4px solid transparent;"
        "  border-right: 4px solid transparent;"
        "  border-top: 5px solid %5;"
        "  width: 0; height: 0;"
        "}"

        /* ---- QLineEdit ---- */
        "QLineEdit {"
        "  background-color: %6;"
        "  border: 1px solid %4;"
        "  border-radius: 4px;"
        "  padding: 5px 8px;"
        "  color: %2;"
        "  min-height: 22px;"
        "}"
        "QLineEdit:focus {"
        "  border-color: %7;"
        "}"

        /* ---- QPushButton ---- */
        "QPushButton {"
        "  background-color: %8;"
        "  color: %9;"
        "  border: none;"
        "  border-radius: 4px;"
        "  padding: 6px 16px;"
        "  font-weight: 600;"
        "  min-height: 24px;"
        "}"
        "QPushButton:hover {"
        "  background-color: %10;"
        "}"
        "QPushButton:pressed {"
        "  background-color: %11;"
        "}"
        "QPushButton:disabled {"
        "  background-color: %4;"
        "  color: %12;"
        "}"
        "QPushButton#secondaryButton {"
        "  background-color: %3;"
        "  color: %2;"
        "  border: 1px solid %4;"
        "}"
        "QPushButton#secondaryButton:hover {"
        "  background-color: %6;"
        "}"

        /* ---- QCheckBox (toggle switch) ---- */
        "QCheckBox {"
        "  spacing: 8px;"
        "  color: %2;"
        "  background: transparent;"
        "}"
        "QCheckBox::indicator {"
        "  width: 36px;"
        "  height: 20px;"
        "  border: 2px solid %13;"
        "  border-radius: 12px;"
        "  background-color: transparent;"
        "}"
        "QCheckBox::indicator:checked {"
        "  background-color: %14;"
        "  border-color: %14;"
        "}"
        "QCheckBox:disabled {"
        "  color: %12;"
        "}"
        "QCheckBox::indicator:disabled {"
        "  border-color: %12;"
        "}"

        /* ---- QTabWidget ---- */
        "QTabWidget::pane {"
        "  border: 1px solid %15;"
        "  border-top: none;"
        "  background-color: %16;"
        "}"
        "QTabBar::tab {"
        "  background-color: %17;"
        "  color: %18;"
        "  border: 1px solid %15;"
        "  border-bottom: none;"
        "  border-top-left-radius: 4px;"
        "  border-top-right-radius: 4px;"
        "  padding: 6px 20px;"
        "  margin-right: 2px;"
        "  min-width: 80px;"
        "}"
        "QTabBar::tab:selected {"
        "  background-color: %19;"
        "  color: %20;"
        "  border-bottom: 2px solid %8;"
        "}"
        "QTabBar::tab:hover:!selected {"
        "  background-color: %6;"
        "}"

        /* ---- QLabel ---- */
        "QLabel {"
        "  background: transparent;"
        "  color: %5;"
        "}"

        /* ---- Title bar ---- */
        "#titleBar {"
        "  background-color: %1;"
        "  border-bottom: 1px solid %4;"
        "}"
        "#titleBarLabel {"
        "  font-weight: 700;"
        "  font-size: 13px;"
        "  color: %2;"
        "  background: transparent;"
        "}"

        /* ---- Title bar menu ---- */
        "#titleBarMenu {"
        "  background: transparent;"
        "  border: none;"
        "  color: %2;"
        "  font-size: 12px;"
        "}"
        "#titleBarMenu::item {"
        "  padding: 4px 10px;"
        "  border-radius: 4px;"
        "  background: transparent;"
        "}"
        "#titleBarMenu::item:selected {"
        "  background-color: %6;"
        "}"

        /* ---- Window control buttons ---- */
        "#minimizeButton, #maximizeButton, #closeButton {"
        "  background: transparent;"
        "  border: none;"
        "  border-radius: 0;"
        "  padding: 0;"
        "  min-height: 0;"
        "}"
        "#minimizeButton:hover, #maximizeButton:hover {"
        "  background-color: %6;"
        "}"
        "#closeButton:hover {"
        "  background-color: #c42b1c;"
        "}"

        /* ---- QMenu (dropdown) ---- */
        "QMenu {"
        "  background-color: %3;"
        "  color: %2;"
        "  border: 1px solid %4;"
        "  padding: 4px 0;"
        "}"
        "QMenu::item {"
        "  padding: 6px 30px 6px 20px;"
        "}"
        "QMenu::item:selected {"
        "  background-color: %8;"
        "  color: %9;"
        "}"

        /* ---- Scrollbar ---- */
        "QScrollBar:vertical {"
        "  background: %21;"
        "  width: 10px;"
        "  border: none;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background: %22;"
        "  border-radius: 5px;"
        "  min-height: 30px;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "  height: 0;"
        "}"

        /* ---- Message banner ---- */
        "QLabel#messageLabel[messageType=\"error\"] {"
        "  background-color: %23;"
        "  color: %24;"
        "  font-weight: bold;"
        "  padding: 8px 12px;"
        "}"
        "QLabel#messageLabel[messageType=\"info\"] {"
        "  background-color: %25;"
        "  color: %26;"
        "  font-weight: bold;"
        "  padding: 8px 12px;"
        "}"

        /* ---- ColorEdit swatch ---- */
        "QPushButton#colorSwatch {"
        "  border: 1px solid %4;"
        "  border-radius: 3px;"
        "  min-width: 22px;"
        "  min-height: 22px;"
        "  max-width: 22px;"
        "  max-height: 22px;"
        "  padding: 0;"
        "}"
        "QPushButton#colorSwatch:hover {"
        "  border-color: %7;"
        "}"
    )
    .arg(p.windowBg)          // %1
    .arg(p.textPrimary)       // %2
    .arg(p.panelBg)           // %3
    .arg(p.border)            // %4
    .arg(p.textSecondary)     // %5
    .arg(p.inputBg)           // %6
    .arg(p.borderFocused)     // %7
    .arg(p.accent)            // %8
    .arg(p.accentText)        // %9
    .arg(p.accentHover)       // %10
    .arg(p.accentPressed)     // %11
    .arg(p.textPlaceholder)   // %12
    .arg(p.checkboxBorder)    // %13
    .arg(p.checkboxCheckedBg) // %14
    .arg(p.tabBorder)         // %15
    .arg(p.tabActiveBg)       // %16
    .arg(p.tabBg)             // %17
    .arg(p.tabText)           // %18
    .arg(p.tabActiveBg)       // %19
    .arg(p.tabActiveText)     // %20
    .arg(p.scrollbarBg)       // %21
    .arg(p.scrollbarHandle)   // %22
    .arg(p.errorBg)           // %23
    .arg(p.errorText)         // %24
    .arg(p.infoBg)            // %25
    .arg(p.infoText);         // %26
}
