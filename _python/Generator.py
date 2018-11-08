from PySide2.QtCore import Qt
from PySide2.QtGui import QImage, QPainter


class Generator(object):

    def __init__(self):
        self.tileWidth = 16
        self.tileHeight = 16
        self.forcePot = False
        self.isTransparent = True
        self.bgColor = None
        self.reorder = False
        self.padding = 0
        self._cols = 0
        self._rows = 0
        self._gridHeight = 0
        self._gridWidth = 0
        self._targetHeight = 0
        self._targetWidth = 0
        self._target = None

    def create(self, source):
        self._findSizes(source)
        self._createTargetImage()
        self._drawTiles(source)
        self._drawEdges()
        return self._target

    def _drawEdges(self):
        cols = int(self._targetWidth / self._gridWidth)
        rows = int(self._targetHeight / self._gridHeight)
        for offset in range(1, self.padding + 1):
            for j in range(rows):
                # horizontally (top)
                y = j * self._gridHeight + self.padding
                for x in range(self._targetWidth):
                    self._target.setPixel(x, y - offset, self._target.pixel(x, y))
                # horizontally (bottom)
                y = j * self._gridHeight + self.tileHeight + self.padding - 1
                for x in range(self._targetWidth):
                    self._target.setPixel(x, y + offset, self._target.pixel(x, y))
        for offset in range(1, self.padding + 1):
            for j in range(cols):
                # vertically (left)
                x = j * self._gridWidth + self.padding
                for y in range(self._targetHeight):
                    self._target.setPixel(x - offset, y, self._target.pixel(x, y))
                # vertically (right)
                x = j * self._gridWidth + self.tileWidth + self.padding - 1
                for y in range(self._targetHeight):
                    self._target.setPixel(x + offset, y, self._target.pixel(x, y))

    def _drawTiles(self, source):
        x = self.padding
        y = self.padding
        sx = 0
        sy = 0
        doReorder = self.forcePot & self.reorder
        painter = QPainter(self._target)
        for j in range(self._rows):
            for i in range(self._cols):
                painter.drawPixmap(x, y, source, sx, sy, self.tileWidth, self.tileHeight)
                x += self._gridWidth
                if doReorder and x >= self._targetWidth - self._gridWidth:
                    x = self.padding
                    y += self._gridHeight
                sx += self.tileWidth
            if not doReorder:
                x = self.padding
                y += self._gridHeight
            sx = 0
            sy += self.tileHeight

    def _findSizes(self, source):
        self._cols = int(source.width() / self.tileWidth)
        self._rows = int(source.height() / self.tileHeight)
        self._gridWidth = self.tileWidth + self.padding * 2
        self._gridHeight = self.tileHeight + self.padding * 2
        self._targetWidth = self._cols * self._gridWidth
        self._targetHeight = self._rows * self._gridHeight
        if not self.forcePot:
            return self._targetHeight, self._targetWidth
        size = 1
        widthOk = False
        heightOk = False
        for i in range(1, 31):
            if not widthOk and self._targetWidth < size:
                self._targetWidth = size
                widthOk = True
            if not heightOk and self._targetHeight < size:
                self._targetHeight = size
                heightOk = True
            if widthOk and heightOk:
                break
            size *= 2
        return self._targetHeight, self._targetWidth

    def _createTargetImage(self):
        self._target = QImage(self._targetWidth, self._targetHeight, QImage.Format_ARGB32)
        if self.isTransparent:
            self._target.fill(Qt.transparent)
        else:
            self._target.fill(self.bgColor)

