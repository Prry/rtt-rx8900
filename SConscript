from building import *
Import('rtconfig')

src   = []
cwd   = GetCurrentDir()

# add rx8900 src files.
if GetDepend('PKG_USING_RX8900'):
    src += Glob('src/rx8900.c')

# add rx8900 include path.
path  = [cwd + '/inc']

# add src and include to group.
group = DefineGroup('rx8900', src, depend = ['PKG_USING_RX8900'], CPPPATH = path)

Return('group')
