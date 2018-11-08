from PySide2.QtCore import Qt, Signal
from PySide2.QtGui import QColor
from PySide2.QtWidgets import QWidget, QHBoxLayout, QLineEdit, QFrame, QLabel, QColorDialog, QPushButton


class ColorEdit(QWidget):

    def __init__(self, parent=None):
        super(ColorEdit, self).__init__(parent)

        self.color = QColor('#FF00FF')

        self.edit = QLineEdit()
        self.edit.setText('FF00FF')
        self.edit.setMaximumWidth(70)
        self.edit.textEdited.connect(self.editChanged)
        self.edit.returnPressed.connect(self.editChanged)

        self.box = QPushButton()
        self.box.setFlat(True)
        self.box.setFixedSize(18, 18)
        self.box.setStyleSheet('background-color: #FF00FF; border: 1px solid black;')
        self.box.setCursor(Qt.PointingHandCursor)
        self.box.clicked.connect(self.boxClicked)

        hlayout = QHBoxLayout()
        hlayout.setContentsMargins(0, 0, 0, 0)
        hlayout.setSpacing(0)
        hlayout.addWidget(QLabel('#'))
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
        self.box.setStyleSheet('background-color: #' + text + '; border: 1px solid black;')
        self.edit.setText(text)
        self.color = color
        return True

    def getColor(self):
        return self.color
