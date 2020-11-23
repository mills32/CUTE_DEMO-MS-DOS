cd src
tcc -G -O2 -Ic:\tc\include -ml -c adlib.c gus.c cutedemo.c vga.c map.c spr.c palettes.c vector.c
tasm /dc sprc.asm
tasm /dc /m2 rotozoom.asm
tasm /dc /m2 scanline.asm
del *.LIB
tlib lib.lib +vga.obj+map.obj+spr.obj+sprc.obj+rotozoom.obj+scanline.obj+palettes.obj+vector.obj+adlib.obj+gus.obj
tlink c0l.obj cutedemo.obj lib.lib,cutedemo.exe,, cl -Lc:\tc\lib
copy cutedemo.exe ..
del *.OBJ
del *.MAP
del cutedemo.exe
cd..