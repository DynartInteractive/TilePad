import os

from PySide.QtCore import QCoreApplication, Qt, QSettings, QByteArray, QFileSystemWatcher
from PySide.QtGui import QMainWindow, QIcon, QVBoxLayout, QWidget, QPixmap, QPainter, QImage, QColor, QFontMetrics, QFont, \
	QPen, QSpinBox, QLabel, QSizePolicy, QHBoxLayout, QPushButton, QImageReader, QMessageBox, QFileDialog, QCheckBox

from ColorEdit import ColorEdit

from PixmapWidget import PixmapWidget

class MainWindow(QMainWindow):

	def __init__(self, app, parent=None):
		super(MainWindow, self).__init__(parent)
		self.imagesDir = app.dir + '/images/'
		self.setWindowIcon(QIcon(self.imagesDir + 'icon.png'))
		self.path = ''
		self.settings = QSettings()

		self.setMinimumWidth(540)

		self.supportedFormats = []
		for f in QImageReader.supportedImageFormats():
			self.supportedFormats.append(unicode(f))

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
		self.transparentCheckbox.setChecked(False)
		self.transparentCheckbox.stateChanged.connect(self.transparentChanged)

		self.backgroundColorEdit = ColorEdit()
		self.backgroundColorLabel = QLabel("Background color:")

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
		self.tileWidthSpinBox.setValue(self.settings.value('tileWidth', 16))
		self.tileHeightSpinBox.setValue(self.settings.value('tileHeight', 16))
		self.paddingSpinBox.setValue(self.settings.value('padding', 1))
		self.forcePotCheckBox.setChecked(True if self.settings.value('forcePot', 'true') == 'true' else False)
		self.reorderTilesCheckBox.setChecked(True if self.settings.value('reorderTiles', 'false') == 'true' else False)
		self.transparentCheckbox.setChecked(True if self.settings.value('transparent', 'false') == 'true' else False)
		self.backgroundColorEdit.setColorText(unicode(self.settings.value('backgroundColor', '#FF00FF')))
		self.restoreGeometry(QByteArray(self.settings.value('MainWindow/geometry')))
		self.restoreState(QByteArray(self.settings.value('MainWindow/windowState')))


		# layout
		hlayout1 = QHBoxLayout()
		hlayout1.setContentsMargins(5, 5, 5, 5)
		hlayout1.addWidget(QLabel("Tile width:"))
		hlayout1.addSpacing(5)
		hlayout1.addWidget(self.tileWidthSpinBox)
		hlayout1.addSpacing(15)
		hlayout1.addWidget(QLabel("Tile height:"))
		hlayout1.addSpacing(5)
		hlayout1.addWidget(self.tileHeightSpinBox)
		hlayout1.addSpacing(15)
		hlayout1.addWidget(QLabel("Padding:"))
		hlayout1.addSpacing(5)
		hlayout1.addWidget(self.paddingSpinBox)
		hlayout1.addSpacing(15)
		hlayout1.addWidget(self.forcePotCheckBox)
		hlayout1.addSpacing(15)
		hlayout1.addWidget(self.reorderTilesCheckBox)
		hlayout1.addStretch()

		hlayout2 = QHBoxLayout()
		hlayout2.setContentsMargins(5, 5, 5, 5)
		hlayout2.addWidget(self.transparentCheckbox)
		hlayout2.addSpacing(15)
		hlayout2.addWidget(self.backgroundColorLabel)
		hlayout2.addSpacing(5)
		hlayout2.addWidget(self.backgroundColorEdit)
		hlayout2.addStretch()

		hlayout3 = QHBoxLayout()
		hlayout3.setContentsMargins(5, 5, 5, 5)
		hlayout3.addWidget(self.generateAndExportButton)

		vlayout = QVBoxLayout()
		vlayout.setContentsMargins(0, 0, 0, 0)
		vlayout.setSpacing(0)
		vlayout.addLayout(hlayout1)
		vlayout.addLayout(hlayout2)
		vlayout.addWidget(self.pixmapWidget)
		vlayout.addLayout(hlayout3)

		w = QWidget()
		w.setLayout(vlayout)
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
		path = unicode(path)
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

		# init
		source = self.pixmapWidget.pixmap
		tileWidth = self.tileWidthSpinBox.value()
		tileHeight = self.tileHeightSpinBox.value()
		cols = source.width() / tileWidth
		rows = source.height() / tileHeight
		pad = self.paddingSpinBox.value()
		gridWidth = (tileWidth + pad*2)
		gridHeight = (tileHeight + pad*2)
		targetWidth = cols * gridWidth
		targetHeight = rows * gridHeight

		# force "power of two" to the target size
		forcePot = self.forcePotCheckBox.isChecked()
		if forcePot:
			size = 1
			widthOk = False
			heightOk = False
			for i in range(1, 31):
				if not widthOk and targetWidth < size:
					targetWidth = size
					widthOk = True
				if not heightOk and targetHeight < size:
					targetHeight = size
					heightOk = True
				if widthOk and heightOk:
					break
				size *= 2

		# create the target image
		target = QImage(targetWidth, targetHeight, QImage.Format_ARGB32)
		if self.transparentCheckbox.isChecked():
			target.fill(Qt.transparent)
		else:
			target.fill(self.backgroundColorEdit.getColor())
		painter = QPainter(target)

		# drawing tiles with empty padding
		x = pad
		y = pad
		sx = 0
		sy = 0
		reorder = forcePot & self.reorderTilesCheckBox.isChecked()
		for j in range(rows):
			for i in range(cols):
				painter.drawPixmap(x, y, source, sx, sy, tileWidth, tileHeight)
				x += gridWidth
				if reorder and x >= targetWidth - gridWidth:
					x = pad
					y += gridHeight
				sx += tileWidth
			if not reorder:
				x = pad
				y += gridHeight
			sx = 0
			sy += tileHeight
		del painter

		cols = targetWidth / gridWidth
		rows = targetHeight / gridHeight

		# padding with edges
		for offset in range(1, pad + 1):
			result = QImage(target)
			for j in range(rows):

				# horizontally (top)
				y = j * gridHeight + pad
				for x in range(targetWidth):
					xo = x % gridWidth
					if pad <= xo < gridWidth - pad:
						result.setPixel(x, y - offset, target.pixel(x, y))
					else:
						# corners
						cx = -1
						if xo <= pad:
							cx = (x / gridWidth) * gridWidth + pad
						elif xo >= gridWidth - pad:
							cx = (x / gridWidth) * gridWidth + pad + tileWidth - 1
						if cx != -1:
							result.setPixel(x, y - offset, target.pixel(cx, y))

				# horizontally (bottom)
				y = j * gridHeight + tileHeight + pad - 1
				for x in range(targetWidth):
					xo = x % gridWidth
					if pad <= xo < gridWidth - pad:
						result.setPixel(x, y + offset, target.pixel(x, y))
					else:
						# corners
						cx = -1
						if xo <= pad:
							cx = (x / gridWidth) * gridWidth + pad
						elif xo >= gridWidth - pad:
							cx = (x / gridWidth) * gridWidth + pad + tileWidth - 1
						if cx != -1:
							result.setPixel(x, y + offset, target.pixel(cx, y))

			for j in range(cols):

				# vertically (left)
				x = j * gridWidth + pad
				for y in range(targetHeight):
					yo = y % gridHeight
					if pad <= yo < gridHeight - pad:
						result.setPixel(x - offset, y, target.pixel(x, y))

				# horizontally (bottom)
				x = j * gridWidth + tileWidth + pad - 1
				for y in range(targetHeight):
					yo = y % gridHeight
					if pad <= yo < gridHeight - pad:
						result.setPixel(x + offset, y, target.pixel(x, y))

			# copy result to target
			target = QImage(result)

		# export
		name, ext = os.path.splitext(self.path)
		targetPath = os.path.dirname(self.path) + name + ".export.png"
		targetPath = unicode(QFileDialog.getSaveFileName(self, 'Export', targetPath, 'PNG (*.png)'))
		if targetPath:
			target.save(targetPath)
			showPixmap = QPixmap.fromImage(target)
			if self.showPixmapWidget:
				self.showPixmapWidget.deleteLater()
				del self.showPixmapWidget
			self.showPixmapWidget = PixmapWidget()
			self.showPixmapWidget.setWindowIcon(self.windowIcon())
			self.showPixmapWidget.setWindowTitle(os.path.basename(targetPath))
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
		self.settings.setValue('MainWindow/geometry', self.saveGeometry())
		self.settings.setValue('MainWindow/windowState', self.saveState())

		super(MainWindow, self).closeEvent(event)
