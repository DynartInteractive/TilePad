#include "mainwindow.h"

#include <QWidget>
#include <QHBoxLayout>
#include <QFileInfo>
#include <QDir>
#include <QFileDialog>
#include <QBuffer>
#include <QCoreApplication>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    QCoreApplication::setOrganizationName("Dynart");
    QCoreApplication::setOrganizationDomain("dynart.net");
    QCoreApplication::setApplicationName("TilePad");
    createLayout();
    loadSettings();
}

MainWindow::~MainWindow() {
}

QSize MainWindow::sizeHint() const {
    return QSize(800, 600);
}

void MainWindow::createLayout() {
    messageLabel = new QLabel("Error message");
    showInfo("Drop an image file to the grey area!");

    tileWidthSpinBox = new QSpinBox();
    tileWidthSpinBox->setFixedWidth(50);
    tileWidthSpinBox->setValue(16);

    tileHeightSpinBox = new QSpinBox();
    tileHeightSpinBox->setFixedWidth(50);
    tileHeightSpinBox->setValue(16);

    paddingSpinBox = new QSpinBox();
    paddingSpinBox->setFixedWidth(50);
    paddingSpinBox->setValue(1);

    forcePotCheckBox = new QCheckBox("Force PoT");
    forcePotCheckBox->setChecked(true);
    connect(forcePotCheckBox, SIGNAL(stateChanged(int)), this, SLOT(forcePotCheckBoxStateChanged(int)));

    reorderCheckBox = new QCheckBox("Reorder tiles");

    transparentCheckBox = new QCheckBox("Transparent");
    transparentCheckBox->setChecked(true);
    connect(transparentCheckBox, SIGNAL(stateChanged(int)), this, SLOT(transparentCheckBoxStateChanged(int)));

    backgroundColorEdit = new ColorEdit();
    backgroundColorEdit->setEnabled(false);

    sourcePixmapDropWidget = new PixmapDropWidget();
    sourcePixmapDropWidget->setMinimumHeight(300);
    sourcePixmapDropWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(sourcePixmapDropWidget, SIGNAL(dropSignal(QString)), this, SLOT(fileDropped(QString)));

    resultPixmapDropWidget = new PixmapDropWidget();
    resultPixmapDropWidget->setMinimumHeight(300);
    resultPixmapDropWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(resultPixmapDropWidget, SIGNAL(dropSignal(QString)), this, SLOT(fileDropped(QString)));

    tabWidget = new QTabWidget();
    tabWidget->addTab(sourcePixmapDropWidget, "Source");
    tabWidget->addTab(resultPixmapDropWidget, "Result");
    tabWidget->setStyleSheet("QTabWidget::pane { border: 0 solid white; } ");

    exportEdit = new QLineEdit();

    browseButton = new QPushButton("Browse ...");
    connect(browseButton, SIGNAL(clicked()), this, SLOT(browseButtonClicked()));

    exportButton = new QPushButton("Export");
    exportButton->setEnabled(false);
    connect(exportButton, SIGNAL(clicked()), this, SLOT(exportButtonClicked()));

    auto h = new QHBoxLayout();
    h->setContentsMargins(5, 5, 5, 5);
    h->addSpacing(5);
    h->addWidget(new QLabel("Tile width:"));
    h->addSpacing(5);
    h->addWidget(tileWidthSpinBox);
    h->addSpacing(15);
    h->addWidget(new QLabel("Tile height:"));
    h->addSpacing(5);
    h->addWidget(tileHeightSpinBox);
    h->addSpacing(15);
    h->addWidget(new QLabel("Padding:"));
    h->addSpacing(5);
    h->addWidget(paddingSpinBox);
    h->addSpacing(15);
    h->addWidget(forcePotCheckBox);
    h->addSpacing(15);
    h->addWidget(reorderCheckBox);
    h->addStretch();

    auto h2 = new QHBoxLayout();
    h2->setContentsMargins(5, 5, 5, 5);
    h2->addSpacing(5);
    h2->addWidget(transparentCheckBox);
    h2->addSpacing(15);
    h2->addWidget(new QLabel("Background color:"));
    h2->addSpacing(5);
    h2->addWidget(backgroundColorEdit);
    h2->addStretch();

    auto h3 = new QHBoxLayout();
    h3->setContentsMargins(5, 5, 5, 5);
    h3->addWidget(exportEdit);
    h3->addSpacing(5);
    h3->addWidget(browseButton);
    h3->addSpacing(5);
    h3->addWidget(exportButton);

    auto centralWidget = new QWidget();
    auto v = new QVBoxLayout(centralWidget);
    v->setContentsMargins(0, 0, 0 ,0);
    v->setSpacing(0);
    v->addWidget(messageLabel);
    v->addLayout(h);
    v->addLayout(h2);
    v->addWidget(tabWidget);
    v->addLayout(h3);
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
    transparentCheckBox->setChecked(settings.value("transparent").toBool());
    backgroundColorEdit->setColorText(settings.value("backgroundColor").toString());
    exportEdit->setText(settings.value("exportPath").toString());
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
    settings.setValue("reorder", reorderCheckBox->isChecked());
    settings.setValue("transparent", transparentCheckBox->isChecked());
    settings.setValue("backgroundColor", backgroundColorEdit->getColor().name());
    settings.setValue("exportPath", exportEdit->text());
}

void MainWindow::showError(QString text) {
    messageLabel->setStyleSheet("color: #fff; background: #a00; font-weight: bold; padding: 5px");
    messageLabel->setVisible(true);
    messageLabel->setText(text);
}

void MainWindow::showInfo(QString text) {
    messageLabel->setStyleSheet("color: #fff; background: #4286f4; font-weight: bold; padding: 5px");
    messageLabel->setVisible(true);
    messageLabel->setText(text);
}

void MainWindow::hideMessage() {
    messageLabel->setVisible(false);
}

void MainWindow::fileDropped(QString path) {
    // error handling
    hideMessage();
    if (!sourcePixmapDropWidget->load(path)) {
        showError("Couldn't load the image: " + path);
        return;
    }
    setUpGenerator();
    QImage* image = createImageFromSource();
    QImage* resultImage = paddingGenerator.create(image);
    delete image;
    QPixmap* resultPixmap = new QPixmap(QPixmap::fromImage(*resultImage));
    adjustUiAfterDrop(path, resultPixmap);
    //delete resultImage;
}

void MainWindow::setUpGenerator() {
    paddingGenerator.setTileSize(tileWidthSpinBox->value(), tileHeightSpinBox->value());
    paddingGenerator.setPadding(paddingSpinBox->value());
    paddingGenerator.setForcePot(forcePotCheckBox->isChecked());
    paddingGenerator.setReorder(reorderCheckBox->isChecked());
    paddingGenerator.setTransparent(transparentCheckBox->isChecked());
    paddingGenerator.setBackgroundColor(backgroundColorEdit->getColor());
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

void MainWindow::adjustUiAfterDrop(QString path, QPixmap* resultPixmap) {
    resultPixmapDropWidget->setPixmap(resultPixmap);
    tabWidget->setCurrentIndex(1);
    if (exportEdit->text().isEmpty()) {
        QFileInfo fileInfo(path);
        auto dir = fileInfo.absoluteDir().path();
        auto baseName = fileInfo.completeBaseName();
        auto suffix = fileInfo.suffix();
        exportEdit->setText(dir + "/" + baseName + ".export." + suffix);
    }
    exportButton->setEnabled(true);
    update();
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

void MainWindow::transparentCheckBoxStateChanged(int state) {
    backgroundColorEdit->setEnabled(state == Qt::Unchecked);
}

void MainWindow::forcePotCheckBoxStateChanged(int state) {
    reorderCheckBox->setEnabled(state == Qt::Checked);
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

