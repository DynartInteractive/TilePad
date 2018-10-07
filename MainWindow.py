# ssh key test
import os

from PySide2.QtCore import QCoreApplication, QSettings, QByteArray, QFileSystemWatcher
from PySide2.QtWidgets import QMainWindow, QVBoxLayout, QWidget, QSpinBox, QLabel, QSizePolicy, QHBoxLayout, \
    QPushButton, QMessageBox, QFileDialog, QCheckBox
from PySide2.QtGui import QIcon, QPixmap, QPainter, QColor, QFontMetrics, QFont, QPen, QImageReader

from ColorEdit import ColorEdit
from Generator import Generator
from PixmapWidget import PixmapWidget

class MainWindow(QMainWindow):

    def __init__(self, app, parent=None):
        super(MainWindow, self).__init__(parent)
        self.imagesDir = app.dir + '/images/'
        self.setWindowIcon(QIcon(self.imagesDir + 'icon.png'))
        self.path = ''

        self.settings = QSettings()
        self.lastDir = self.settings.value('lastDir', '')

        self.setMinimumWidth(540)

        self.supportedFormats = []
        for f in QImageReader.supportedImageFormats():
            self.supportedFormats.append(str(f.data(), encoding="utf-8"))

        self.fileWatcher = QFileSystemWatcher()
        self.fileWatcher.fileChanged.connect(self.fileChanged)

        # widgets
        self.showPixmapWidget = None

        self.tileWidthSpinBox = QSpinBox()
        self.tileWidthSpinBox.setValue(16)
        self.tileWidthSpinBox.setFixedWidth(50)
        self.tileWidthSpinBox.setMinimum(1)

        self.tileHeightSpinBox = QSpinBox()
        self.tileHeightSpinBox.setValue(16)
        self.tileHeightSpinBox.setFixedWidth(50)
        self.tileHeightSpinBox.setMinimum(1)

        self.paddingSpinBox = QSpinBox()
        self.paddingSpinBox.setFixedWidth(50)
        self.paddingSpinBox.setMinimum(1)

        self.transparentCheckbox = QCheckBox("Transparent")
        self.transparentCheckbox.setChecked(True)
        self.transparentCheckbox.stateChanged.connect(self.transparentChanged)

        self.backgroundColorEdit = ColorEdit()
        self.backgroundColorEdit.setEnabled(False)
        self.backgroundColorLabel = QLabel("Background color:")
        self.backgroundColorLabel.setEnabled(False)

        self.forcePotCheckBox = QCheckBox("Force PoT")
        self.forcePotCheckBox.setChecked(True)
        self.forcePotCheckBox.stateChanged.connect(self.forcePotChanged)

        self.reorderTilesCheckBox = QCheckBox("Reorder tiles")

        self.generateAndExportButton = QPushButton("Generate and export")
        self.generateAndExportButton.setFixedHeight(32)
        self.generateAndExportButton.clicked.connect(self.generateAndExportClicked)
        self.generateAndExportButton.setEnabled(False)

        self.pixmapWidget = PixmapWidget()
        self.pixmapWidget.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)
        self.pixmapWidget.setPixmap(self.createDropTextPixmap())
        self.pixmapWidget.dropSignal.connect(self.fileDropped)
        self.pixmapWidget.setMinimumHeight(300)

        # load settings
        self.tileWidthSpinBox.setValue(int(self.settings.value('tileWidth', 16)))
        self.tileHeightSpinBox.setValue(int(self.settings.value('tileHeight', 16)))
        self.paddingSpinBox.setValue(int(self.settings.value('padding', 1)))
        self.forcePotCheckBox.setChecked(True if self.settings.value('forcePot', 'true') == 'true' else False)
        self.reorderTilesCheckBox.setChecked(True if self.settings.value('reorderTiles', 'false') == 'true' else False)
        self.transparentCheckbox.setChecked(True if self.settings.value('transparent', 'false') == 'true' else False)
        self.backgroundColorEdit.setColorText(str(self.settings.value('backgroundColor', '#FF00FF')))
        self.restoreGeometry(QByteArray(self.settings.value('MainWindow/geometry')))
        self.restoreState(QByteArray(self.settings.value('MainWindow/windowState')))

        # layout
        hl1 = QHBoxLayout()
        hl1.setContentsMargins(5, 5, 5, 5)
        hl1.addWidget(QLabel("Tile width:"))
        hl1.addSpacing(5)
        hl1.addWidget(self.tileWidthSpinBox)
        hl1.addSpacing(15)
        hl1.addWidget(QLabel("Tile height:"))
        hl1.addSpacing(5)
        hl1.addWidget(self.tileHeightSpinBox)
        hl1.addSpacing(15)
        hl1.addWidget(QLabel("Padding:"))
        hl1.addSpacing(5)
        hl1.addWidget(self.paddingSpinBox)
        hl1.addSpacing(15)
        hl1.addWidget(self.forcePotCheckBox)
        hl1.addSpacing(15)
        hl1.addWidget(self.reorderTilesCheckBox)
        hl1.addStretch()

        hl2 = QHBoxLayout()
        hl2.setContentsMargins(5, 5, 5, 5)
        hl2.addWidget(self.transparentCheckbox)
        hl2.addSpacing(15)
        hl2.addWidget(self.backgroundColorLabel)
        hl2.addSpacing(5)
        hl2.addWidget(self.backgroundColorEdit)
        hl2.addStretch()

        hl3 = QHBoxLayout()
        hl3.setContentsMargins(5, 5, 5, 5)
        hl3.addWidget(self.generateAndExportButton)

        vl = QVBoxLayout()
        vl.setContentsMargins(0, 0, 0, 0)
        vl.setSpacing(0)
        vl.addLayout(hl1)
        vl.addLayout(hl2)
        vl.addWidget(self.pixmapWidget)
        vl.addLayout(hl3)

        w = QWidget()
        w.setLayout(vl)
        self.setCentralWidget(w)

        self.setTitle()

    def setTitle(self):
        p = ' - ' + os.path.basename(self.path) if self.path else ''
        self.setWindowTitle(QCoreApplication.applicationName() + ' ' + QCoreApplication.applicationVersion() + p)

    def createDropTextPixmap(self):
        pixmap = QPixmap(481, 300)
        pixmap.fill(QColor("#333333"))
        painter = QPainter(pixmap)
        font = QFont("Arial")
        font.setPixelSize(28)
        font.setBold(True)
        fm = QFontMetrics(font)
        painter.setFont(font)
        painter.setPen(QPen(QColor("#888888"), 1))
        text = "Drop the tileset image here"
        x = (pixmap.width()-fm.width(text))/2
        y = (pixmap.height()+fm.height())/2
        painter.drawText(x, y, text)
        del painter
        return pixmap

    def fileDropped(self, path):
        path = str(path)
        name, ext = os.path.splitext(path)
        ext = ext[1:]
        if not ext in self.supportedFormats:
            QMessageBox.warning(self, "Warning", "The dropped file is not supported")
            return
        pixmap = QPixmap(path)
        if pixmap.isNull():
            QMessageBox.warning(self, "Warning", "Can't load the image")
            return
        if self.path:
            self.fileWatcher.removePath(self.path)
        self.path = path
        self.fileWatcher.addPath(self.path)
        self.pixmapWidget.setPixmap(pixmap)
        self.generateAndExportButton.setEnabled(True)
        self.setTitle()
        self.activateWindow()

    def fileChanged(self, path):
        #self.fileDropped(path)
        pass

    def transparentChanged(self):
        e = self.transparentCheckbox.isChecked()
        self.backgroundColorEdit.setEnabled(not e)
        self.backgroundColorLabel.setEnabled(not e)

    def forcePotChanged(self):
        e = self.forcePotCheckBox.isChecked()
        self.reorderTilesCheckBox.setEnabled(e)

    def generateAndExportClicked(self):

        g = Generator()
        g.tileWidth = self.tileWidthSpinBox.value()
        g.tileHeight = self.tileHeightSpinBox.value()
        g.forcePot = self.forcePotCheckBox.isChecked()
        g.isTransparent = self.transparentCheckbox.isChecked()
        g.bgColor = self.backgroundColorEdit.getColor()
        g.reorder = self.reorderTilesCheckBox.isChecked()
        g.padding = self.paddingSpinBox.value()

        target = g.create(self.pixmapWidget.pixmap);

        # export
        self.lastDir = os.path.dirname(self.path)
        targetPath = QFileDialog.getSaveFileName(self, 'Export', self.lastDir, 'PNG (*.png)')
        if targetPath:
            target.save(targetPath[0])
            showPixmap = QPixmap.fromImage(target)
            if self.showPixmapWidget:
                self.showPixmapWidget.deleteLater()
                del self.showPixmapWidget
            self.showPixmapWidget = PixmapWidget()
            self.showPixmapWidget.setWindowIcon(self.windowIcon())
            self.showPixmapWidget.setWindowTitle(os.path.basename(targetPath[0]))
            self.showPixmapWidget.resize(showPixmap.width(), showPixmap.height())
            self.showPixmapWidget.setPixmap(showPixmap)
            self.showPixmapWidget.show()

    def closeEvent(self, event):
        if self.showPixmapWidget:
            self.showPixmapWidget.close()

        # save settings
        self.settings.setValue('tileWidth', self.tileWidthSpinBox.value())
        self.settings.setValue('tileHeight', self.tileHeightSpinBox.value())
        self.settings.setValue('padding', self.paddingSpinBox.value())
        self.settings.setValue('forcePot', self.forcePotCheckBox.isChecked())
        self.settings.setValue('reorderTiles', self.reorderTilesCheckBox.isChecked())
        self.settings.setValue('transparent', self.transparentCheckbox.isChecked())
        self.settings.setValue('backgroundColor', self.backgroundColorEdit.getColor().name())
        self.settings.setValue('lastDir', self.lastDir)
        self.settings.setValue('MainWindow/geometry', self.saveGeometry())
        self.settings.setValue('MainWindow/windowState', self.saveState())

        super(MainWindow, self).closeEvent(event)
