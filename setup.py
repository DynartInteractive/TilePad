# -*- coding: utf-8 -*-
import os

import sys
import shutil
from cx_Freeze import setup, Executable

base = None
if sys.platform == 'win32':
	base = 'Win32GUI'

options = {
	'build_exe': {
		'includes': 'atexit'
	}
}

executables = [
	Executable('TilePad.py', base=base, icon='_source/icon.ico')
]

setup(name='TilePad',
	  version='0.3.0',
	  description='TilePad 0.3.0',
	  options=options,
	  executables=executables
	  )

# copy resources
try:
	shutil.rmtree('build/exe.win32-2.7/images')
	#shutil.rmtree('build/exe.win-amd64-2.7/docs')
	#shutil.rmtree('build/exe.win-amd64-2.7/ffmpeg')
	#os.remove('build/exe.win-amd64-2.7/LICENSE')
except:
	pass
shutil.copytree('images', 'build/exe.win32-2.7/images')
#shutil.copytree('docs', 'build/exe.win-amd64-2.7/docs')
#shutil.copytree('ffmpeg', 'build/exe.win-amd64-2.7/ffmpeg')
#shutil.copy('LICENSE', 'build/exe.win-amd64-2.7/LICENSE')