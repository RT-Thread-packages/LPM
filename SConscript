# RT-Thread building script for component

from building import *

cwd = GetCurrentDir()
src = Glob('src/*.c')
CPPPATH = [cwd + '/inc']

group = DefineGroup('lpm', src, depend = ['PKG_USING_LPM'], CPPPATH = CPPPATH)

Return('group')