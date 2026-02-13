#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QTabWidget>
#include <QPushButton>
#include <QFileSystemWatcher>

#include "pixmapdropwidget.h"
#include "paddinggenerator.h"
#include "paddingremover.h"
#include "coloredit.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
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

private:
    void showError(QString text);
    void showInfo(QString text);
    void hideMessage();
    void createLayout();
    void setUpGenerator();
    void setUpRemover();
    QImage* createImageFromSource();
    void adjustUiAfterDrop(QString path);
    void loadSettings();
    void saveSettings();

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

    QString currentSourcePath;
    QFileSystemWatcher* fileWatcher;

    PaddingGenerator paddingGenerator;
    PaddingRemover paddingRemover;

};

#endif // MAINWINDOW_H
