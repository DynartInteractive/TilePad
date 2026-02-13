#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QTabWidget>
#include <QPushButton>
#include <QFileSystemWatcher>
#include <QActionGroup>

#include "pixmapdropwidget.h"
#include "paddinggenerator.h"
#include "paddingremover.h"
#include "coloredit.h"
#include "thememanager.h"
#include "titlebar.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(ThemeManager* themeManager, QWidget *parent = nullptr);
    ~MainWindow();

    virtual QSize sizeHint() const;
    virtual void closeEvent(QCloseEvent* event);

public slots:
    void fileDropped(QString path);
    void reprocess();
    void browseButtonClicked();
    void transparentCheckBoxStateChanged(Qt::CheckState state);
    void forcePotCheckBoxStateChanged(Qt::CheckState state);
    void removePaddingCheckBoxStateChanged(Qt::CheckState state);
    void exportButtonClicked();
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
    void setupThemeMenu();
    void createLayout();
    void setUpGenerator();
    void setUpRemover();
    QImage* createImageFromSource();
    void adjustUiAfterDrop(QString path);
    void loadSettings();
    void saveSettings();

    ThemeManager* m_themeManager;
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
    QTabWidget* tabWidget;
    PixmapDropWidget* sourcePixmapDropWidget;
    PixmapDropWidget* resultPixmapDropWidget;
    QLineEdit* exportEdit;
    QCheckBox* watchFileCheckBox;
    QPushButton* reprocessButton;
    QPushButton* browseButton;
    QPushButton* exportButton;

    QAction* m_systemThemeAction;
    QAction* m_darkThemeAction;
    QAction* m_lightThemeAction;

    QString currentSourcePath;
    QFileSystemWatcher* fileWatcher;

    PaddingGenerator paddingGenerator;
    PaddingRemover paddingRemover;
};

#endif // MAINWINDOW_H
