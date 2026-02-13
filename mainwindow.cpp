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
#include <QMessageBox>
#include <QToolButton>

#ifdef Q_OS_WIN
#include <windows.h>
#include <dwmapi.h>
#include <windowsx.h>
#endif

MainWindow::MainWindow(ThemeManager* themeManager, Project* project, QWidget *parent)
    : QMainWindow(parent), m_themeManager(themeManager), m_project(project)
{
    QCoreApplication::setOrganizationName("Dynart");
    QCoreApplication::setOrganizationDomain("dynart.net");
    QCoreApplication::setApplicationName("TilePad");

    fileWatcher = new QFileSystemWatcher(this);
    connect(fileWatcher, &QFileSystemWatcher::fileChanged, this, &MainWindow::sourceFileChanged);

    m_titleBar = new TitleBar(this);
    setupFileMenu();
    setupThemeMenu();
    setupFrameless();
    createLayout();
    loadAppSettings();

    // Apply project settings to UI
    applyProjectSettingsToUi();

    // Load files from project (if opened from recent/file)
    for (int i = 0; i < m_project->fileCount(); i++) {
        auto& entry = m_project->fileAt(i);
        if (!entry.sourcePath.isEmpty()) {
            entry.sourcePixmap.load(entry.sourcePath);
            QFileInfo info(entry.sourcePath);
            m_fileTabBar->addTab(info.fileName());
        }
    }
    if (m_project->fileCount() > 0) {
        switchToFile(0);
    }

    updateWindowTitle();

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
    HWND hwnd = (HWND)winId();
    LONG style = GetWindowLong(hwnd, GWL_STYLE);
    SetWindowLong(hwnd, GWL_STYLE, style | WS_THICKFRAME | WS_CAPTION | WS_MAXIMIZEBOX | WS_MINIMIZEBOX);

    MARGINS margins = {1, 1, 1, 1};
    DwmExtendFrameIntoClientArea(hwnd, &margins);

    int cornerPref = 2; // DWMWCP_ROUND
    DwmSetWindowAttribute(hwnd, 33 /*DWMWA_WINDOW_CORNER_PREFERENCE*/,
                          &cornerPref, sizeof(cornerPref));
#endif
}

void MainWindow::setupFileMenu() {
    auto fileMenu = m_titleBar->menuBar()->addMenu("&File");

    fileMenu->addAction("New Project", this, &MainWindow::newProject);
    fileMenu->addAction("Open Project...", this, &MainWindow::openProject);
    fileMenu->addSeparator();
    fileMenu->addAction("Save Project", this, &MainWindow::saveProject);
    fileMenu->addAction("Save Project As...", this, &MainWindow::saveProjectAs);
    fileMenu->addSeparator();
    fileMenu->addAction("Import Files...", this, &MainWindow::showImportDialog);
    fileMenu->addSeparator();

    m_recentMenu = fileMenu->addMenu("Recent Projects");
    updateRecentProjectsMenu();

    fileMenu->addSeparator();
    fileMenu->addAction("Exit", this, &QMainWindow::close);
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

        QPoint localPos = m_titleBar->mapFromGlobal(QPoint(x, y));
        if (m_titleBar->rect().contains(localPos)) {
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

    // --- File tab bar ---
    m_fileTabBar = new QTabBar();
    m_fileTabBar->setTabsClosable(true);
    m_fileTabBar->setExpanding(false);
    m_fileTabBar->setObjectName("fileTabBar");

    auto addTabButton = new QToolButton();
    addTabButton->setText("+");
    addTabButton->setObjectName("addTabButton");
    addTabButton->setFixedSize(28, 28);
    connect(addTabButton, &QToolButton::clicked, this, &MainWindow::showImportDialog);

    auto fileTabLayout = new QHBoxLayout();
    fileTabLayout->setContentsMargins(0, 0, 0, 0);
    fileTabLayout->setSpacing(4);
    fileTabLayout->addWidget(m_fileTabBar, 1);
    fileTabLayout->addWidget(addTabButton);

    connect(m_fileTabBar, &QTabBar::currentChanged, this, &MainWindow::switchToFile);
    connect(m_fileTabBar, &QTabBar::tabCloseRequested, this, &MainWindow::closeFileTab);

    // --- Preview tab widget (Source/Result) ---
    sourcePixmapDropWidget = new PixmapDropWidget();
    sourcePixmapDropWidget->setMinimumHeight(200);
    sourcePixmapDropWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(sourcePixmapDropWidget, &PixmapDropWidget::filesDropped, this, &MainWindow::importFiles);

    resultPixmapDropWidget = new PixmapDropWidget();
    resultPixmapDropWidget->setMinimumHeight(200);
    resultPixmapDropWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(resultPixmapDropWidget, &PixmapDropWidget::filesDropped, this, &MainWindow::importFiles);

    tabWidget = new QTabWidget();
    tabWidget->addTab(sourcePixmapDropWidget, "Source");
    tabWidget->addTab(resultPixmapDropWidget, "Result");

    // --- Export group ---
    auto exportGroup = new QGroupBox("Export");
    {
        auto groupLayout = new QVBoxLayout(exportGroup);
        groupLayout->setSpacing(6);

        // Export directory row
        auto dirRow = new QHBoxLayout();
        dirRow->setSpacing(8);
        dirRow->addWidget(new QLabel("Directory:"));
        m_exportDirEdit = new QLineEdit();
        m_exportDirEdit->setPlaceholderText("Default (same as source)");
        m_exportDirBrowseButton = new QPushButton("Browse...");
        m_exportDirBrowseButton->setObjectName("secondaryButton");
        connect(m_exportDirBrowseButton, &QPushButton::clicked, this, [this]() {
            QString dir = QFileDialog::getExistingDirectory(this, "Select Export Directory",
                m_exportDirEdit->text());
            if (!dir.isEmpty()) {
                m_exportDirEdit->setText(dir);
                m_project->settings().exportDirectory = dir;
                m_project->setModified(true);
                updateWindowTitle();
            }
        });
        dirRow->addWidget(m_exportDirEdit, 1);
        dirRow->addWidget(m_exportDirBrowseButton);
        groupLayout->addLayout(dirRow);

        // Per-file export row
        auto fileRow = new QHBoxLayout();
        fileRow->setSpacing(8);

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

        exportAllButton = new QPushButton("Export All");
        exportAllButton->setEnabled(false);
        connect(exportAllButton, &QPushButton::clicked, this, &MainWindow::exportAllButtonClicked);

        fileRow->addWidget(exportEdit, 1);
        fileRow->addWidget(browseButton);
        fileRow->addWidget(reprocessButton);
        fileRow->addWidget(exportButton);
        fileRow->addWidget(exportAllButton);
        groupLayout->addLayout(fileRow);
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
    contentLayout->addLayout(fileTabLayout);
    contentLayout->addWidget(tabWidget, 1);
    contentLayout->addWidget(exportGroup);

    mainLayout->addWidget(contentWidget, 1);

    setCentralWidget(centralWidget);
}

void MainWindow::loadAppSettings() {
    QSettings settings;

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
    if (!promptSaveIfModified()) {
        event->ignore();
        return;
    }
    saveAppSettings();
    QMainWindow::closeEvent(event);
}

void MainWindow::saveAppSettings() {
    QSettings settings;
    QString themeModeStr;
    switch (m_themeManager->themeMode()) {
    case ThemeManager::ThemeMode::Dark:   themeModeStr = "dark";   break;
    case ThemeManager::ThemeMode::Light:  themeModeStr = "light";  break;
    default:                              themeModeStr = "system"; break;
    }
    settings.setValue("themeMode", themeModeStr);
}

void MainWindow::applyProjectSettingsToUi() {
    const auto& s = m_project->settings();
    tileWidthSpinBox->setValue(s.tileWidth);
    tileHeightSpinBox->setValue(s.tileHeight);
    paddingSpinBox->setValue(s.padding);
    forcePotCheckBox->setChecked(s.forcePot);
    reorderCheckBox->setChecked(s.reorder);
    removePaddingCheckBox->setChecked(s.removePadding);
    transparentCheckBox->setChecked(s.transparent);
    backgroundColorEdit->setColorText(s.backgroundColor);
    watchFileCheckBox->setChecked(s.watchFile);
    m_exportDirEdit->setText(s.exportDirectory);
}

void MainWindow::readUiIntoProjectSettings() {
    auto& s = m_project->settings();
    s.tileWidth = tileWidthSpinBox->value();
    s.tileHeight = tileHeightSpinBox->value();
    s.padding = paddingSpinBox->value();
    s.forcePot = forcePotCheckBox->isChecked();
    s.reorder = reorderCheckBox->isChecked();
    s.removePadding = removePaddingCheckBox->isChecked();
    s.transparent = transparentCheckBox->isChecked();
    s.backgroundColor = backgroundColorEdit->getColor().name();
    s.watchFile = watchFileCheckBox->isChecked();
    s.exportDirectory = m_exportDirEdit->text();
}

void MainWindow::updateWindowTitle() {
    QString title = "TilePad";
    if (!m_project->projectPath().isEmpty()) {
        QFileInfo info(m_project->projectPath());
        title += " - " + info.fileName();
    } else {
        title += " - Untitled";
    }
    if (m_project->isModified()) {
        title += " *";
    }
    setWindowTitle(title);
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

// --- Project operations ---

void MainWindow::newProject() {
    if (!promptSaveIfModified()) {
        return;
    }

    // Clear file watcher
    if (!fileWatcher->files().isEmpty()) {
        fileWatcher->removePaths(fileWatcher->files());
    }

    m_currentFileIndex = -1;
    m_project->clear();

    // Clear file tabs
    while (m_fileTabBar->count() > 0) {
        m_fileTabBar->removeTab(0);
    }

    // Clear preview
    sourcePixmapDropWidget->setPixmap(new QPixmap());
    sourcePixmapDropWidget->update();
    resultPixmapDropWidget->setPixmap(new QPixmap());
    resultPixmapDropWidget->update();

    exportEdit->clear();
    reprocessButton->setEnabled(false);
    exportButton->setEnabled(false);
    exportAllButton->setEnabled(false);

    applyProjectSettingsToUi();
    updateWindowTitle();
    hideMessage();
}

void MainWindow::openProject() {
    if (!promptSaveIfModified()) {
        return;
    }
    QString path = QFileDialog::getOpenFileName(this, "Open Project", QString(),
                                                 "TilePad Projects (*.tilepad)");
    if (!path.isEmpty()) {
        openProjectFile(path);
    }
}

void MainWindow::openProjectFile(const QString& path) {
    if (!promptSaveIfModified()) {
        return;
    }

    // Clear current state
    if (!fileWatcher->files().isEmpty()) {
        fileWatcher->removePaths(fileWatcher->files());
    }
    m_currentFileIndex = -1;
    while (m_fileTabBar->count() > 0) {
        m_fileTabBar->removeTab(0);
    }

    Project* newProject = new Project();
    if (!newProject->load(path)) {
        showError("Could not open project: " + path);
        delete newProject;
        return;
    }

    delete m_project;
    m_project = newProject;

    applyProjectSettingsToUi();

    // Load files and create tabs
    for (int i = 0; i < m_project->fileCount(); i++) {
        auto& entry = m_project->fileAt(i);
        if (!entry.sourcePath.isEmpty()) {
            entry.sourcePixmap.load(entry.sourcePath);
            QFileInfo info(entry.sourcePath);
            m_fileTabBar->addTab(info.fileName());
        }
    }

    if (m_project->fileCount() > 0) {
        switchToFile(0);
        // Process all files
        for (int i = 0; i < m_project->fileCount(); i++) {
            processFile(i);
        }
        switchToFile(0);
    }

    updateWindowTitle();
    updateRecentProjectsMenu();
    hideMessage();
    showInfo("Project opened: " + QFileInfo(path).fileName());
}

void MainWindow::saveProject() {
    readUiIntoProjectSettings();
    storeCurrentFileState();

    if (m_project->projectPath().isEmpty()) {
        saveProjectAs();
        return;
    }
    if (m_project->save(m_project->projectPath())) {
        updateWindowTitle();
        updateRecentProjectsMenu();
        showInfo("Project saved.");
    } else {
        showError("Could not save project.");
    }
}

void MainWindow::saveProjectAs() {
    readUiIntoProjectSettings();
    storeCurrentFileState();

    QString path = QFileDialog::getSaveFileName(this, "Save Project As", QString(),
                                                 "TilePad Projects (*.tilepad)");
    if (path.isEmpty()) {
        return;
    }
    if (!path.endsWith(".tilepad")) {
        path += ".tilepad";
    }
    if (m_project->save(path)) {
        updateWindowTitle();
        updateRecentProjectsMenu();
        showInfo("Project saved: " + QFileInfo(path).fileName());
    } else {
        showError("Could not save project.");
    }
}

bool MainWindow::promptSaveIfModified() {
    if (!m_project->isModified()) {
        return true;
    }
    auto result = QMessageBox::question(this, "Save Project?",
        "The current project has unsaved changes. Save before continuing?",
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    if (result == QMessageBox::Save) {
        saveProject();
        return true;
    } else if (result == QMessageBox::Discard) {
        return true;
    }
    return false; // Cancel
}

void MainWindow::showImportDialog() {
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setNameFilter("Images (*.png *.jpg *.jpeg)");
    if (dialog.exec()) {
        importFiles(dialog.selectedFiles());
    }
}

void MainWindow::updateRecentProjectsMenu() {
    m_recentMenu->clear();
    QStringList recent = Project::recentProjects();
    if (recent.isEmpty()) {
        m_recentMenu->setEnabled(false);
        return;
    }
    m_recentMenu->setEnabled(true);
    for (const QString& path : recent) {
        if (QFileInfo::exists(path)) {
            QFileInfo info(path);
            auto action = m_recentMenu->addAction(info.fileName());
            action->setToolTip(path);
            connect(action, &QAction::triggered, this, [this, path]() {
                openProjectFile(path);
            });
        }
    }
}

// --- File tab operations ---

void MainWindow::importFiles(QStringList paths) {
    hideMessage();
    int firstNew = -1;

    // Block tab bar signals during batch import to avoid switchToFile calls mid-loop
    m_fileTabBar->blockSignals(true);

    for (const QString& path : paths) {
        int index = m_project->addFile(path);
        auto& entry = m_project->fileAt(index);

        if (!entry.sourcePixmap.load(path)) {
            showError("Could not load: " + path);
            m_project->removeFile(index);
            continue;
        }

        QFileInfo info(path);
        m_fileTabBar->addTab(info.fileName());

        if (firstNew < 0) {
            firstNew = index;
        }

        // Process the file
        processFile(index);
    }

    m_fileTabBar->blockSignals(false);

    if (firstNew >= 0) {
        int lastIndex = m_project->fileCount() - 1;
        m_fileTabBar->setCurrentIndex(lastIndex);
        switchToFile(lastIndex);
        exportAllButton->setEnabled(true);
    }

    updateWindowTitle();
}

void MainWindow::switchToFile(int index) {
    if (index < 0 || index >= m_project->fileCount()) {
        // No files - show empty state
        sourcePixmapDropWidget->setPixmap(new QPixmap());
        sourcePixmapDropWidget->update();
        resultPixmapDropWidget->setPixmap(new QPixmap());
        resultPixmapDropWidget->update();
        exportEdit->clear();
        reprocessButton->setEnabled(false);
        exportButton->setEnabled(false);
        m_currentFileIndex = -1;
        return;
    }

    // Store current file state before switching
    storeCurrentFileState();

    m_currentFileIndex = index;
    auto& entry = m_project->fileAt(index);

    // Update pixmap displays
    auto srcPix = new QPixmap(entry.sourcePixmap);
    sourcePixmapDropWidget->setPixmap(srcPix);
    sourcePixmapDropWidget->update();

    if (entry.processed) {
        auto resPix = new QPixmap(entry.resultPixmap);
        resultPixmapDropWidget->setPixmap(resPix);
    } else {
        resultPixmapDropWidget->setPixmap(new QPixmap());
    }
    resultPixmapDropWidget->update();

    updateReferenceSize(index);

    // Update export path
    exportEdit->setText(entry.exportPath);
    reprocessButton->setEnabled(true);
    exportButton->setEnabled(entry.processed);

    // Update file watcher
    if (!fileWatcher->files().isEmpty()) {
        fileWatcher->removePaths(fileWatcher->files());
    }
    if (watchFileCheckBox->isChecked() && !entry.sourcePath.isEmpty()) {
        fileWatcher->addPath(entry.sourcePath);
    }

    tabWidget->setCurrentIndex(0); // Show source tab
}

void MainWindow::storeCurrentFileState() {
    if (m_currentFileIndex < 0 || m_currentFileIndex >= m_project->fileCount()) {
        return;
    }
    auto& entry = m_project->fileAt(m_currentFileIndex);
    entry.exportPath = exportEdit->text();
}

void MainWindow::updateReferenceSize(int fileIndex) {
    if (fileIndex < 0 || fileIndex >= m_project->fileCount()) {
        sourcePixmapDropWidget->setReferenceSize(QSize());
        resultPixmapDropWidget->setReferenceSize(QSize());
        return;
    }
    auto& entry = m_project->fileAt(fileIndex);
    int maxW = entry.sourcePixmap.width();
    int maxH = entry.sourcePixmap.height();
    if (entry.processed && !entry.resultPixmap.isNull()) {
        maxW = qMax(maxW, entry.resultPixmap.width());
        maxH = qMax(maxH, entry.resultPixmap.height());
    }
    QSize ref(maxW, maxH);
    sourcePixmapDropWidget->setReferenceSize(ref);
    resultPixmapDropWidget->setReferenceSize(ref);
}

void MainWindow::closeFileTab(int index) {
    if (index < 0 || index >= m_project->fileCount()) {
        return;
    }

    // Remove watcher if this is the current file
    if (index == m_currentFileIndex) {
        auto& entry = m_project->fileAt(index);
        if (!entry.sourcePath.isEmpty() && fileWatcher->files().contains(entry.sourcePath)) {
            fileWatcher->removePath(entry.sourcePath);
        }
    }

    m_project->removeFile(index);
    m_fileTabBar->removeTab(index);

    if (m_currentFileIndex >= m_project->fileCount()) {
        m_currentFileIndex = m_project->fileCount() - 1;
    }

    if (m_project->fileCount() == 0) {
        switchToFile(-1);
        exportAllButton->setEnabled(false);
    } else {
        switchToFile(m_fileTabBar->currentIndex());
    }

    updateWindowTitle();
}

void MainWindow::processFile(int index) {
    if (index < 0 || index >= m_project->fileCount()) {
        return;
    }

    auto& entry = m_project->fileAt(index);
    if (entry.sourcePixmap.isNull()) {
        return;
    }

    // Create image from source pixmap
    QByteArray bArray;
    QBuffer buffer(&bArray);
    buffer.open(QIODevice::WriteOnly);
    entry.sourcePixmap.save(&buffer, "PNG");
    buffer.close();
    QImage sourceImage = QImage::fromData(bArray);

    QImage* resultImage;
    if (removePaddingCheckBox->isChecked()) {
        setUpRemover();
        resultImage = paddingRemover.create(&sourceImage);
    } else {
        setUpGenerator();
        resultImage = paddingGenerator.create(&sourceImage);
    }

    entry.resultPixmap = QPixmap::fromImage(*resultImage);
    // Don't delete resultImage — it's owned by paddingGenerator/paddingRemover

    entry.processed = true;
    entry.dirty = true;

    // Auto-export if export path is set
    if (!entry.exportPath.isEmpty()) {
        exportFile(index);
    }
}

void MainWindow::exportFile(int index) {
    if (index < 0 || index >= m_project->fileCount()) {
        return;
    }

    auto& entry = m_project->fileAt(index);
    if (!entry.processed || entry.resultPixmap.isNull()) {
        return;
    }

    QString exportPath = entry.exportPath;
    if (exportPath.isEmpty()) {
        return;
    }

    QFileInfo fileInfo(exportPath);
    auto format = fileInfo.suffix().toUpper();
    auto dir = fileInfo.dir();
    if (!dir.exists()) {
        return;
    }
    if (format == "JPEG") {
        format = "JPG";
    }
    if (format != "PNG" && format != "JPG") {
        return;
    }

    entry.resultPixmap.save(exportPath, format.toStdString().c_str());
    entry.dirty = false;
}

void MainWindow::reprocess() {
    if (m_currentFileIndex < 0) {
        return;
    }
    hideMessage();
    readUiIntoProjectSettings();
    storeCurrentFileState();

    processFile(m_currentFileIndex);

    // Update result display
    auto& entry = m_project->fileAt(m_currentFileIndex);
    auto resPix = new QPixmap(entry.resultPixmap);
    resultPixmapDropWidget->setPixmap(resPix);
    resultPixmapDropWidget->update();
    updateReferenceSize(m_currentFileIndex);
    tabWidget->setCurrentIndex(1);
    exportButton->setEnabled(true);

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

QImage* MainWindow::createImageFromSource(int fileIndex) {
    if (fileIndex < 0 || fileIndex >= m_project->fileCount()) {
        return new QImage();
    }
    auto& entry = m_project->fileAt(fileIndex);
    QByteArray bArray;
    QBuffer buffer(&bArray);
    buffer.open(QIODevice::WriteOnly);
    entry.sourcePixmap.save(&buffer, "PNG");
    buffer.close();
    return new QImage(QImage::fromData(bArray));
}

void MainWindow::browseButtonClicked() {
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(tr("Images (*.png *.jpg *.jpeg)"));
    if (dialog.exec()) {
        auto files = dialog.selectedFiles();
        if (files.length() > 0) {
            exportEdit->setText(files.at(0));
            if (m_currentFileIndex >= 0) {
                m_project->fileAt(m_currentFileIndex).exportPath = files.at(0);
            }
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

void MainWindow::exportButtonClicked() {
    if (m_currentFileIndex < 0) {
        return;
    }
    hideMessage();

    // Store current export path
    storeCurrentFileState();

    auto& entry = m_project->fileAt(m_currentFileIndex);
    auto exportPath = entry.exportPath;
    QFileInfo fileInfo(exportPath);
    auto format = fileInfo.suffix().toUpper();
    auto dir = fileInfo.dir();

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

    exportFile(m_currentFileIndex);
    showInfo("Export complete.");
}

void MainWindow::exportAllButtonClicked() {
    hideMessage();
    readUiIntoProjectSettings();
    storeCurrentFileState();

    int exported = 0;
    int errors = 0;

    for (int i = 0; i < m_project->fileCount(); i++) {
        auto& entry = m_project->fileAt(i);
        if (!entry.dirty || !entry.processed) {
            continue;
        }

        // Reprocess with current settings
        processFile(i);

        if (entry.exportPath.isEmpty()) {
            errors++;
            continue;
        }

        QFileInfo fileInfo(entry.exportPath);
        if (!fileInfo.dir().exists()) {
            errors++;
            continue;
        }

        exported++;
    }

    // Update current file display
    if (m_currentFileIndex >= 0) {
        auto& entry = m_project->fileAt(m_currentFileIndex);
        auto resPix = new QPixmap(entry.resultPixmap);
        resultPixmapDropWidget->setPixmap(resPix);
        resultPixmapDropWidget->update();
    }

    if (errors > 0) {
        showInfo(QString("Exported %1 files. %2 files had errors.").arg(exported).arg(errors));
    } else {
        showInfo(QString("Exported %1 files.").arg(exported));
    }
}

void MainWindow::watchFileCheckBoxStateChanged(Qt::CheckState state) {
    if (m_currentFileIndex < 0 || m_currentFileIndex >= m_project->fileCount()) {
        return;
    }
    auto& entry = m_project->fileAt(m_currentFileIndex);
    if (entry.sourcePath.isEmpty()) {
        return;
    }
    if (state == Qt::Checked) {
        fileWatcher->addPath(entry.sourcePath);
    } else {
        fileWatcher->removePath(entry.sourcePath);
    }
}

void MainWindow::sourceFileChanged(const QString& path) {
    if (m_currentFileIndex < 0 || m_currentFileIndex >= m_project->fileCount()) {
        return;
    }
    auto& entry = m_project->fileAt(m_currentFileIndex);
    if (path != entry.sourcePath || !watchFileCheckBox->isChecked()) {
        return;
    }
    if (!entry.sourcePixmap.load(entry.sourcePath)) {
        return;
    }
    fileWatcher->addPath(entry.sourcePath);

    // Update source display
    auto srcPix = new QPixmap(entry.sourcePixmap);
    sourcePixmapDropWidget->setPixmap(srcPix);
    sourcePixmapDropWidget->update();

    // Reprocess (which auto-exports)
    processFile(m_currentFileIndex);

    auto resPix = new QPixmap(entry.resultPixmap);
    resultPixmapDropWidget->setPixmap(resPix);
    resultPixmapDropWidget->update();
    updateReferenceSize(m_currentFileIndex);
}
