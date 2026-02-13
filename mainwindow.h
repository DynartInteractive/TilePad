#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QTabWidget>
#include <QTabBar>
#include <QPushButton>
#include <QFileSystemWatcher>
#include <QActionGroup>
#include <QMenu>

#include "pixmapdropwidget.h"
#include "paddinggenerator.h"
#include "paddingremover.h"
#include "coloredit.h"
#include "thememanager.h"
#include "titlebar.h"
#include "project.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(ThemeManager* themeManager, Project* project, QWidget *parent = nullptr);
    ~MainWindow();

    virtual QSize sizeHint() const;
    virtual void closeEvent(QCloseEvent* event);

public slots:
    void importFiles(QStringList paths);
    void reprocess();
    void browseButtonClicked();
    void transparentCheckBoxStateChanged(Qt::CheckState state);
    void forcePotCheckBoxStateChanged(Qt::CheckState state);
    void removePaddingCheckBoxStateChanged(Qt::CheckState state);
    void exportButtonClicked();
    void exportAllButtonClicked();
    void watchFileCheckBoxStateChanged(Qt::CheckState state);
    void sourceFileChanged(const QString& path);

protected:
    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;
    void changeEvent(QEvent* event) override;

private:
    void showError(QString text);
    void showInfo(QString text);
    void hideMessage();
    void setupFrameless();
    void setupFileMenu();
    void setupThemeMenu();
    void createLayout();
    void setUpGenerator();
    void setUpRemover();
    QImage* createImageFromSource(int fileIndex);
    void loadAppSettings();
    void saveAppSettings();
    void applyProjectSettingsToUi();
    void readUiIntoProjectSettings();
    void updateWindowTitle();

    // Project operations
    void newProject();
    void openProject();
    void openProjectFile(const QString& path);
    void saveProject();
    void saveProjectAs();
    bool promptSaveIfModified();
    void showImportDialog();
    void updateRecentProjectsMenu();

    // File tab operations
    void switchToFile(int index);
    void closeFileTab(int index);
    void processFile(int index);
    void exportFile(int index);
    void storeCurrentFileState();
    void updateReferenceSize(int fileIndex);

    int m_currentFileIndex = -1;

    ThemeManager* m_themeManager;
    Project* m_project;
    TitleBar* m_titleBar;

    QLabel* messageLabel;
    QSpinBox* tileWidthSpinBox;
    QSpinBox* tileHeightSpinBox;
    QSpinBox* paddingSpinBox;
    QCheckBox* forcePotCheckBox;
    QCheckBox* reorderCheckBox;
    QCheckBox* removePaddingCheckBox;
    QCheckBox* transparentCheckBox;
    ColorEdit* backgroundColorEdit;
    QTabBar* m_fileTabBar;
    QTabWidget* tabWidget;
    PixmapDropWidget* sourcePixmapDropWidget;
    PixmapDropWidget* resultPixmapDropWidget;
    QLineEdit* m_exportDirEdit;
    QPushButton* m_exportDirBrowseButton;
    QLineEdit* exportEdit;
    QCheckBox* watchFileCheckBox;
    QPushButton* reprocessButton;
    QPushButton* browseButton;
    QPushButton* exportButton;
    QPushButton* exportAllButton;

    QAction* m_systemThemeAction;
    QAction* m_darkThemeAction;
    QAction* m_lightThemeAction;
    QMenu* m_recentMenu;

    QFileSystemWatcher* fileWatcher;

    PaddingGenerator paddingGenerator;
    PaddingRemover paddingRemover;
};

#endif // MAINWINDOW_H
