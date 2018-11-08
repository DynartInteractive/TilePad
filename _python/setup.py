import sys
from cx_Freeze import setup, Executable

base = None
if sys.platform == 'win32':
	#base = 'Win32GUI'
    base = 'Console'

options = {
	'build_exe': {
		'includes': 'atexit'
	}
}

executables = [
	Executable('TilePad.py', base=base, icon='_source/icon.ico')
]

setup(
	name='TilePad',
	version='0.3.0',
	description='TilePad 0.3.0',
	options=options,
	executables=executables, requires=['PySide2']
)

