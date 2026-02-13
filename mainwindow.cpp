#include "mainwindow.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QFileInfo>
#include <QDir>
#include <QFileDialog>
#include <QBuffer>
#include <QCoreApplication>
#include <QSettings>
#include <QStyle>
#include <QEvent>

#ifdef Q_OS_WIN
#include <windows.h>
#include <dwmapi.h>
#include <windowsx.h>
#endif

MainWindow::MainWindow(ThemeManager* themeManager, QWidget *parent)
    : QMainWindow(parent), m_themeManager(themeManager)
{
    QCoreApplication::setOrganizationName("Dynart");
    QCoreApplication::setOrganizationDomain("dynart.net");
    QCoreApplication::setApplicationName("TilePad");

    fileWatcher = new QFileSystemWatcher(this);
    connect(fileWatcher, &QFileSystemWatcher::fileChanged, this, &MainWindow::sourceFileChanged);

    m_titleBar = new TitleBar(this);
    setupThemeMenu();
    setupFrameless();
    createLayout();
    loadSettings();
    setWindowTitle("TilePad 0.5.1");

    connect(m_themeManager, &ThemeManager::themeChanged, this, [this](bool isDark) {
        sourcePixmapDropWidget->setDarkMode(isDark);
        resultPixmapDropWidget->setDarkMode(isDark);
    });
    sourcePixmapDropWidget->setDarkMode(m_themeManager->isDark());
    resultPixmapDropWidget->setDarkMode(m_themeManager->isDark());
}

MainWindow::~MainWindow() {
}

QSize MainWindow::sizeHint() const {
    return QSize(800, 600);
}

void MainWindow::setupFrameless() {
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);

#ifdef Q_OS_WIN
    // Re-add thick frame and caption for native resize, snap, and shadow
    HWND hwnd = (HWND)winId();
    LONG style = GetWindowLong(hwnd, GWL_STYLE);
    SetWindowLong(hwnd, GWL_STYLE, style | WS_THICKFRAME | WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX);

    // Enable DWM shadow
    MARGINS margins = {1, 1, 1, 1};
    DwmExtendFrameIntoClientArea(hwnd, &margins);

    // Enable rounded corners on Windows 11
    int cornerPref = 2; // DWMWCP_ROUND
    DwmSetWindowAttribute(hwnd, 33 /*DWMWA_WINDOW_CORNER_PREFERENCE*/,
                          &cornerPref, sizeof(cornerPref));
#endif
}

void MainWindow::setupThemeMenu() {
    auto viewMenu = m_titleBar->menuBar()->addMenu("&View");
    auto themeMenu = viewMenu->addMenu("Theme");

    auto themeGroup = new QActionGroup(this);
    themeGroup->setExclusive(true);

    m_systemThemeAction = themeMenu->addAction("Follow System");
    m_systemThemeAction->setCheckable(true);
    m_systemThemeAction->setChecked(true);

    m_darkThemeAction = themeMenu->addAction("Dark");
    m_darkThemeAction->setCheckable(true);

    m_lightThemeAction = themeMenu->addAction("Light");
    m_lightThemeAction->setCheckable(true);

    themeGroup->addAction(m_systemThemeAction);
    themeGroup->addAction(m_darkThemeAction);
    themeGroup->addAction(m_lightThemeAction);

    connect(m_systemThemeAction, &QAction::triggered, this, [this]() {
        m_themeManager->setThemeMode(ThemeManager::ThemeMode::System);
        m_themeManager->applyTheme();
    });
    connect(m_darkThemeAction, &QAction::triggered, this, [this]() {
        m_themeManager->setThemeMode(ThemeManager::ThemeMode::Dark);
        m_themeManager->applyTheme();
    });
    connect(m_lightThemeAction, &QAction::triggered, this, [this]() {
        m_themeManager->setThemeMode(ThemeManager::ThemeMode::Light);
        m_themeManager->applyTheme();
    });
}

bool MainWindow::nativeEvent(const QByteArray& eventType, void* message, qintptr* result) {
#ifdef Q_OS_WIN
    Q_UNUSED(eventType);
    MSG* msg = static_cast<MSG*>(message);

    if (msg->message == WM_NCCALCSIZE) {
        if (msg->wParam == TRUE) {
            // When maximized, adjust for taskbar
            if (IsZoomed(msg->hwnd)) {
                NCCALCSIZE_PARAMS* params = reinterpret_cast<NCCALCSIZE_PARAMS*>(msg->lParam);
                HMONITOR monitor = MonitorFromWindow(msg->hwnd, MONITOR_DEFAULTTONEAREST);
                MONITORINFO mi;
                mi.cbSize = sizeof(mi);
                GetMonitorInfo(monitor, &mi);
                params->rgrc[0] = mi.rcWork;
            }
            *result = 0;
            return true;
        }
    }

    if (msg->message == WM_NCHITTEST) {
        const int borderWidth = 6;
        RECT winrect;
        GetWindowRect(msg->hwnd, &winrect);

        long x = GET_X_LPARAM(msg->lParam);
        long y = GET_Y_LPARAM(msg->lParam);

        // Don't resize when maximized
        if (!IsZoomed(msg->hwnd)) {
            bool resizeLeft   = x >= winrect.left   && x < winrect.left   + borderWidth;
            bool resizeRight  = x <  winrect.right  && x >= winrect.right  - borderWidth;
            bool resizeTop    = y >= winrect.top    && y < winrect.top    + borderWidth;
            bool resizeBottom = y <  winrect.bottom && y >= winrect.bottom - borderWidth;

            if (resizeTop    && resizeLeft)  { *result = HTTOPLEFT;     return true; }
            if (resizeTop    && resizeRight) { *result = HTTOPRIGHT;    return true; }
            if (resizeBottom && resizeLeft)  { *result = HTBOTTOMLEFT;  return true; }
            if (resizeBottom && resizeRight) { *result = HTBOTTOMRIGHT; return true; }
            if (resizeLeft)                  { *result = HTLEFT;        return true; }
            if (resizeRight)                 { *result = HTRIGHT;       return true; }
            if (resizeTop)                   { *result = HTTOP;         return true; }
            if (resizeBottom)                { *result = HTBOTTOM;      return true; }
        }

        // Check if in title bar area (for dragging/snap)
        QPoint localPos = m_titleBar->mapFromGlobal(QPoint(x, y));
        if (m_titleBar->rect().contains(localPos)) {
            // If clicking on a child widget (button, menu), let it handle the event
            QWidget* child = m_titleBar->childAt(localPos);
            if (!child || child == m_titleBar) {
                *result = HTCAPTION;
                return true;
            }
        }
    }
#else
    Q_UNUSED(eventType);
    Q_UNUSED(message);
    Q_UNUSED(result);
#endif
    return QMainWindow::nativeEvent(eventType, message, result);
}

void MainWindow::changeEvent(QEvent* event) {
    if (event->type() == QEvent::WindowStateChange) {
        m_titleBar->updateMaximizeButton(isMaximized());
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::createLayout() {
    // Message banner
    messageLabel = new QLabel();
    messageLabel->setObjectName("messageLabel");
    messageLabel->setVisible(false);

    // --- Tile Settings group ---
    auto tileSettingsGroup = new QGroupBox("Tile Settings");
    {
        auto layout = new QHBoxLayout(tileSettingsGroup);
        layout->setSpacing(16);

        tileWidthSpinBox = new QSpinBox();
        tileWidthSpinBox->setRange(1, 512);
        tileWidthSpinBox->setValue(16);

        tileHeightSpinBox = new QSpinBox();
        tileHeightSpinBox->setRange(1, 512);
        tileHeightSpinBox->setValue(16);

        paddingSpinBox = new QSpinBox();
        paddingSpinBox->setRange(0, 64);
        paddingSpinBox->setValue(1);

        forcePotCheckBox = new QCheckBox("Force PoT");
        forcePotCheckBox->setChecked(true);
        connect(forcePotCheckBox, &QCheckBox::checkStateChanged, this, &MainWindow::forcePotCheckBoxStateChanged);

        reorderCheckBox = new QCheckBox("Reorder tiles");

        removePaddingCheckBox = new QCheckBox("Remove padding");
        connect(removePaddingCheckBox, &QCheckBox::checkStateChanged, this, &MainWindow::removePaddingCheckBoxStateChanged);

        auto addSpinPair = [&](const QString& label, QSpinBox* spin) {
            auto vbox = new QVBoxLayout();
            vbox->setSpacing(4);
            vbox->addWidget(new QLabel(label));
            vbox->addWidget(spin);
            layout->addLayout(vbox);
        };

        addSpinPair("Width", tileWidthSpinBox);
        addSpinPair("Height", tileHeightSpinBox);
        addSpinPair("Padding", paddingSpinBox);

        layout->addSpacing(8);
        layout->addWidget(forcePotCheckBox);
        layout->addWidget(reorderCheckBox);
        layout->addWidget(removePaddingCheckBox);
        layout->addStretch();
    }

    // --- Background group ---
    auto backgroundGroup = new QGroupBox("Background");
    {
        auto layout = new QHBoxLayout(backgroundGroup);
        layout->setSpacing(16);

        transparentCheckBox = new QCheckBox("Transparent");
        transparentCheckBox->setChecked(true);
        connect(transparentCheckBox, &QCheckBox::checkStateChanged, this, &MainWindow::transparentCheckBoxStateChanged);

        backgroundColorEdit = new ColorEdit();
        backgroundColorEdit->setEnabled(false);

        watchFileCheckBox = new QCheckBox("Watch file");
        connect(watchFileCheckBox, &QCheckBox::checkStateChanged, this, &MainWindow::watchFileCheckBoxStateChanged);

        layout->addWidget(transparentCheckBox);
        layout->addWidget(new QLabel("Color:"));
        layout->addWidget(backgroundColorEdit);
        layout->addStretch();
        layout->addWidget(watchFileCheckBox);
    }

    // --- Preview tab widget ---
    sourcePixmapDropWidget = new PixmapDropWidget();
    sourcePixmapDropWidget->setMinimumHeight(200);
    sourcePixmapDropWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(sourcePixmapDropWidget, &PixmapDropWidget::dropSignal, this, &MainWindow::fileDropped);

    resultPixmapDropWidget = new PixmapDropWidget();
    resultPixmapDropWidget->setMinimumHeight(200);
    resultPixmapDropWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(resultPixmapDropWidget, &PixmapDropWidget::dropSignal, this, &MainWindow::fileDropped);

    tabWidget = new QTabWidget();
    tabWidget->addTab(sourcePixmapDropWidget, "Source");
    tabWidget->addTab(resultPixmapDropWidget, "Result");

    // --- Export group ---
    auto exportGroup = new QGroupBox("Export");
    {
        auto layout = new QHBoxLayout(exportGroup);
        layout->setSpacing(8);

        exportEdit = new QLineEdit();
        exportEdit->setPlaceholderText("Export file path...");

        browseButton = new QPushButton("Browse...");
        browseButton->setObjectName("secondaryButton");
        connect(browseButton, &QPushButton::clicked, this, &MainWindow::browseButtonClicked);

        reprocessButton = new QPushButton("Reprocess");
        reprocessButton->setObjectName("secondaryButton");
        reprocessButton->setEnabled(false);
        connect(reprocessButton, &QPushButton::clicked, this, &MainWindow::reprocess);

        exportButton = new QPushButton("Export");
        exportButton->setEnabled(false);
        connect(exportButton, &QPushButton::clicked, this, &MainWindow::exportButtonClicked);

        layout->addWidget(exportEdit, 1);
        layout->addWidget(browseButton);
        layout->addWidget(reprocessButton);
        layout->addWidget(exportButton);
    }

    // --- Central assembly ---
    auto centralWidget = new QWidget();
    auto mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Title bar at the very top
    mainLayout->addWidget(m_titleBar);

    // Content area with padding
    auto contentWidget = new QWidget();
    auto contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setContentsMargins(12, 10, 12, 12);
    contentLayout->setSpacing(10);

    contentLayout->addWidget(messageLabel);
    contentLayout->addWidget(tileSettingsGroup);
    contentLayout->addWidget(backgroundGroup);
    contentLayout->addWidget(tabWidget, 1);
    contentLayout->addWidget(exportGroup);

    mainLayout->addWidget(contentWidget, 1);

    setCentralWidget(centralWidget);
}

void MainWindow::loadSettings() {
    QSettings settings;
    if (!settings.contains("tileWidth")) {
        return;
    }
    tileWidthSpinBox->setValue(settings.value("tileWidth").toInt());
    tileHeightSpinBox->setValue(settings.value("tileHeight").toInt());
    paddingSpinBox->setValue(settings.value("padding").toInt());
    forcePotCheckBox->setChecked(settings.value("forcePot").toBool());
    reorderCheckBox->setChecked(settings.value("reorder").toBool());
    removePaddingCheckBox->setChecked(settings.value("removePadding").toBool());
    transparentCheckBox->setChecked(settings.value("transparent").toBool());
    watchFileCheckBox->setChecked(settings.value("watchFile").toBool());
    backgroundColorEdit->setColorText(settings.value("backgroundColor").toString());
    exportEdit->setText(settings.value("exportPath").toString());

    QString themeModeStr = settings.value("themeMode", "system").toString();
    if (themeModeStr == "dark") {
        m_themeManager->setThemeMode(ThemeManager::ThemeMode::Dark);
        m_darkThemeAction->setChecked(true);
    } else if (themeModeStr == "light") {
        m_themeManager->setThemeMode(ThemeManager::ThemeMode::Light);
        m_lightThemeAction->setChecked(true);
    } else {
        m_themeManager->setThemeMode(ThemeManager::ThemeMode::System);
        m_systemThemeAction->setChecked(true);
    }
    m_themeManager->applyTheme();
}

void MainWindow::closeEvent(QCloseEvent* event) {
    saveSettings();
    QMainWindow::closeEvent(event);
}

void MainWindow::saveSettings() {
    QSettings settings;
    settings.setValue("tileWidth", tileWidthSpinBox->value());
    settings.setValue("tileHeight", tileHeightSpinBox->value());
    settings.setValue("padding", paddingSpinBox->value());
    settings.setValue("forcePot", forcePotCheckBox->isChecked());
    settings.setValue("removePadding", removePaddingCheckBox->isChecked());
    settings.setValue("reorder", reorderCheckBox->isChecked());
    settings.setValue("transparent", transparentCheckBox->isChecked());
    settings.setValue("watchFile", watchFileCheckBox->isChecked());
    settings.setValue("backgroundColor", backgroundColorEdit->getColor().name());
    settings.setValue("exportPath", exportEdit->text());

    QString themeModeStr;
    switch (m_themeManager->themeMode()) {
    case ThemeManager::ThemeMode::Dark:   themeModeStr = "dark";   break;
    case ThemeManager::ThemeMode::Light:  themeModeStr = "light";  break;
    default:                              themeModeStr = "system"; break;
    }
    settings.setValue("themeMode", themeModeStr);
}

void MainWindow::showError(QString text) {
    messageLabel->setProperty("messageType", "error");
    messageLabel->style()->unpolish(messageLabel);
    messageLabel->style()->polish(messageLabel);
    messageLabel->setText(text);
    messageLabel->setVisible(true);
}

void MainWindow::showInfo(QString text) {
    messageLabel->setProperty("messageType", "info");
    messageLabel->style()->unpolish(messageLabel);
    messageLabel->style()->polish(messageLabel);
    messageLabel->setText(text);
    messageLabel->setVisible(true);
}

void MainWindow::hideMessage() {
    messageLabel->setVisible(false);
}

void MainWindow::fileDropped(QString path) {
    hideMessage();
    if (!sourcePixmapDropWidget->load(path)) {
        showError("Couldn't load the image: " + path);
        return;
    }
    if (!currentSourcePath.isEmpty()) {
        fileWatcher->removePath(currentSourcePath);
    }
    currentSourcePath = path;
    if (watchFileCheckBox->isChecked()) {
        fileWatcher->addPath(currentSourcePath);
    }
    reprocessButton->setEnabled(true);
    reprocess();
    adjustUiAfterDrop(path);
}

void MainWindow::reprocess() {
    if (currentSourcePath.isEmpty()) {
        return;
    }
    hideMessage();
    QImage* image = createImageFromSource();
    QImage* resultImage;
    if (removePaddingCheckBox->isChecked()) {
        setUpRemover();
        resultImage = paddingRemover.create(image);
    } else {
        setUpGenerator();
        resultImage = paddingGenerator.create(image);
    }
    delete image;
    QPixmap* resultPixmap = new QPixmap(QPixmap::fromImage(*resultImage));
    resultPixmapDropWidget->setPixmap(resultPixmap);
    tabWidget->setCurrentIndex(1);
    update();
    showInfo("Reprocessing complete.");
}

void MainWindow::setUpGenerator() {
    paddingGenerator.setTileSize(tileWidthSpinBox->value(), tileHeightSpinBox->value());
    paddingGenerator.setPadding(paddingSpinBox->value());
    paddingGenerator.setForcePot(forcePotCheckBox->isChecked());
    paddingGenerator.setReorder(reorderCheckBox->isChecked());
    paddingGenerator.setTransparent(transparentCheckBox->isChecked());
    paddingGenerator.setBackgroundColor(backgroundColorEdit->getColor());
}

void MainWindow::setUpRemover() {
    paddingRemover.setTileSize(tileWidthSpinBox->value(), tileHeightSpinBox->value());
    paddingRemover.setPadding(paddingSpinBox->value());
}

QImage* MainWindow::createImageFromSource() {
    QPixmap* pixmap = sourcePixmapDropWidget->getPixmap();
    QByteArray bArray;
    QBuffer buffer(&bArray);
    buffer.open(QIODevice::WriteOnly);
    pixmap->save(&buffer, "PNG");
    buffer.close();
    QImage* image = new QImage(QImage::fromData(bArray));
    return image;
}

void MainWindow::adjustUiAfterDrop(QString path) {
    if (exportEdit->text().isEmpty()) {
        QFileInfo fileInfo(path);
        auto dir = fileInfo.absoluteDir().path();
        auto baseName = fileInfo.completeBaseName();
        auto suffix = fileInfo.suffix();
        exportEdit->setText(dir + "/" + baseName + ".export." + suffix);
    }
    exportButton->setEnabled(true);
}

void MainWindow::browseButtonClicked() {
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(tr("Images (*.png *.jpg *.jpeg)"));
    if (dialog.exec()) {
        auto files = dialog.selectedFiles();
        if (files.length() > 0) {
            auto file = files.at(0);
            exportEdit->setText(file);
        }
    }
}

void MainWindow::transparentCheckBoxStateChanged(Qt::CheckState state) {
    backgroundColorEdit->setEnabled(state == Qt::Unchecked);
}

void MainWindow::forcePotCheckBoxStateChanged(Qt::CheckState state) {
    reorderCheckBox->setEnabled(state == Qt::Checked);
}

void MainWindow::removePaddingCheckBoxStateChanged(Qt::CheckState state) {
    reorderCheckBox->setEnabled(state == Qt::Unchecked && forcePotCheckBox->isChecked());
    forcePotCheckBox->setEnabled(state == Qt::Unchecked);
    transparentCheckBox->setEnabled(state == Qt::Unchecked);
    backgroundColorEdit->setEnabled(state == Qt::Unchecked && !transparentCheckBox->isChecked());
}

void MainWindow::watchFileCheckBoxStateChanged(Qt::CheckState state) {
    if (currentSourcePath.isEmpty()) {
        return;
    }
    if (state == Qt::Checked) {
        fileWatcher->addPath(currentSourcePath);
    } else {
        fileWatcher->removePath(currentSourcePath);
    }
}

void MainWindow::sourceFileChanged(const QString& path) {
    if (path != currentSourcePath || !watchFileCheckBox->isChecked()) {
        return;
    }
    if (!sourcePixmapDropWidget->load(currentSourcePath)) {
        return;
    }
    fileWatcher->addPath(currentSourcePath);
    reprocess();
}

void MainWindow::exportButtonClicked() {
    auto exportPath = exportEdit->text();
    QFileInfo fileInfo(exportPath);
    auto format = fileInfo.suffix().toUpper();
    auto dir = fileInfo.dir();
    hideMessage();
    if (!dir.exists()) {
        showError("The export directory doesn't exist.");
        return;
    }
    if (format == "JPEG") {
        format = "JPG";
    }
    if (format != "PNG" && format != "JPG") {
        showError("The export extension must be .png, .jpg or .jpeg.");
        return;
    }
    QPixmap* pixmap = resultPixmapDropWidget->getPixmap();
    pixmap->save(exportPath, format.toStdString().c_str());
    showInfo("The export was successfull.");
}
