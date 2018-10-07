import os
import sys
import ctypes

ctypes.windll.shell32.SetCurrentProcessExplicitAppUserModelID('net.dynart.tilepad')

from PySide2.QtCore import QCoreApplication
from PySide2.QtWidgets import QApplication

from MainWindow import MainWindow

class TilePad(object):

	Instance = None

	@staticmethod
	def Run():
		TilePad.Instance = TilePad()
		return TilePad.Instance.run()

	def __init__(self):
		QCoreApplication.setOrganizationName('DynArt')
		QCoreApplication.setApplicationName('TilePad')
		QCoreApplication.setApplicationVersion('0.3.1')
		if getattr(sys, 'frozen', False):
			self.dir = os.path.dirname(sys.executable)
		else:
			self.dir = os.path.dirname(__file__)
		self.dir = self.dir.replace('\\', '/')
		self.qApp = QApplication(sys.argv)
		self.mainWindow = MainWindow(self)

	def run(self):
		self.mainWindow.show()
		return self.qApp.exec_()

sys.exit(TilePad.Run())