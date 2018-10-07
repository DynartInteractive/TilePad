from PySide2.QtCore import Qt, Signal
from PySide2.QtGui import QColor
from PySide2.QtWidgets import QWidget, QHBoxLayout, QLineEdit, QFrame, QLabel, QColorDialog


class ColorEditBox(QFrame):

    clicked = Signal()

    def __init__(self, parent=None):
        super(ColorEditBox, self).__init__(parent)
        self.setCursor(Qt.PointingHandCursor)
        self.setFrameShape(QFrame.Box)
        self.setStyleSheet('background-color: #FF00FF')
        self.setFixedWidth(18)
        self.setFixedHeight(18)
        self.mouseDown = False

    def mousePressEvent(self, event):
        super(ColorEditBox, self).mousePressEvent(event)
        self.mouseDown = True

    def mouseReleaseEvent(self, event):
        super(ColorEditBox, self).mouseReleaseEvent(event)
        if self.mouseDown and self.rect().contains(event.pos()):
            self.clicked.emit()
        self.mouseDown = False

class ColorEdit(QWidget):

    def __init__(self, parent=None):
        super(ColorEdit, self).__init__(parent)

        self.color = QColor('#FF00FF')

        hlayout = QHBoxLayout()
        hlayout.setContentsMargins(0, 0, 0, 0)
        hlayout.setSpacing(0)

        self.edit = QLineEdit()
        self.edit.setText('FF00FF')
        self.edit.setMaximumWidth(70)
        self.edit.textEdited.connect(self.editChanged)
        self.edit.returnPressed.connect(self.editChanged)
        self.numLabel = QLabel('#')
        self.box = ColorEditBox(self)
        self.box.clicked.connect(self.boxClicked)

        hlayout.addWidget(self.numLabel)
        hlayout.addWidget(self.edit)
        hlayout.addSpacing(5)
        hlayout.addWidget(self.box)
        hlayout.addStretch()

        self.setLayout(hlayout)

    def editChanged(self):
        self.setColorText(self.edit.text())

    def boxClicked(self):
        dialog = QColorDialog()
        if dialog.exec_():
            self.setColorText(dialog.selectedColor().name())

    def setColorText(self, text):
        text = str(text).upper().replace('#', '')
        color = QColor('#' + text)
        if not color.isValid():
            return
        self.box.setStyleSheet('background-color: #' + text)
        self.edit.setText(text)
        self.color = color
        return True

    def getColor(self):
        return self.color
