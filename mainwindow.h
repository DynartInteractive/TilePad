#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QTabWidget>
#include <QPushButton>

#include "pixmapdropwidget.h"
#include "paddinggenerator.h"
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
    void browseButtonClicked();
    void transparentCheckBoxStateChanged(int state);
    void forcePotCheckBoxStateChanged(int state);
    void exportButtonClicked();

private:
    void showError(QString text);
    void showInfo(QString text);
    void hideMessage();
    void createLayout();
    void setUpGenerator();
    QImage* createImageFromSource();
    void adjustUiAfterDrop(QString path, QPixmap* resultPixmap);
    void loadSettings();
    void saveSettings();

    QLabel* messageLabel;
    QSpinBox* tileWidthSpinBox;
    QSpinBox* tileHeightSpinBox;
    QSpinBox* paddingSpinBox;
    QCheckBox* forcePotCheckBox;
    QCheckBox* reorderCheckBox;
    QCheckBox* transparentCheckBox;
    ColorEdit* backgroundColorEdit;
    QTabWidget* tabWidget;
    PixmapDropWidget* sourcePixmapDropWidget;
    PixmapDropWidget* resultPixmapDropWidget;
    QLineEdit* exportEdit;
    QPushButton* browseButton;
    QPushButton* exportButton;

    PaddingGenerator paddingGenerator;

};

#endif // MAINWINDOW_H
