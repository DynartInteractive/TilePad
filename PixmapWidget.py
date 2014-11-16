from PyQt4.QtCore import QRect, pyqtSignal, Qt
from PyQt4.QtGui import QWidget, QPainter, QBrush, QColor

class PixmapWidget(QWidget):

	dropSignal = pyqtSignal(unicode)

	def __init__(self, parent=None):
		super(PixmapWidget, self).__init__(parent)
		self.setAcceptDrops(True)
		self.pixmap = None
		self.bgBrush = QBrush(QColor('#444'))
		self.bgPen = Qt.NoPen

	def setPixmap(self, pixmap):
		self.pixmap = pixmap
		self.update()

	def paintEvent(self, event):
		super(PixmapWidget, self).paintEvent(event)
		if not self.pixmap or self.pixmap.isNull():
			return
		p = QPainter(self)

		source = QRect(0, 0, self.pixmap.width(), self.pixmap.height())

		sw = float(source.width())
		sh = float(source.height())
		tw = float(self.width())+1
		th = float(self.height())+1
		tx = 0
		ty = 0
		if sw/tw > sh/th:
			ntw = tw
			nth = sh/sw*tw
			ty = (th-nth)/2
		else:
			nth = th
			ntw = sw/sh*th
			tx = (tw-ntw)/2

		target = QRect(tx, ty, ntw, nth)

		p.setBrush(self.bgBrush)
		p.setPen(self.bgPen)
		p.drawRect(self.rect())

		p.drawPixmap(target, self.pixmap, source)

	def dragEnterEvent(self, event):
		if event.mimeData().hasUrls():
			event.setDropAction(Qt.MoveAction)
			event.accept()
		else:
			event.ignore()

	def dropEvent(self, e):
		for url in e.mimeData().urls():
			if url.isLocalFile():
				e.acceptProposedAction()
				self.dropSignal.emit(unicode(url.toLocalFile()))
				break
