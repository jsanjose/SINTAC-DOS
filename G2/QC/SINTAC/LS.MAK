PROJ	=LS
DEBUG	=0
CC	=qcl
CFLAGS_G	= /AL /W4 /Ze 
CFLAGS_D	= /Zi /Zr /Od 
CFLAGS_R	= /Od /Gs /DNDEBUG 
CFLAGS	=$(CFLAGS_G) $(CFLAGS_R)
LFLAGS_G	= /CP:0xfff /NOI /SE:0x80 /ST:0x1000 
LFLAGS_D	= /CO 
LFLAGS_R	= /EXEPACK 
LFLAGS	=$(LFLAGS_G) $(LFLAGS_R)
RUNFLAGS	=
H = 	version.h sintac.h ls.h tabcond.h ..\ventanas\cuadro.h ..\ventanas\menu.h ..\ventanas\raton.h \
	..\ventanas\ventana.h ..\ventanas\editor.h 
OBJS_EXT = 	
LIBS_EXT = 	

.asm.obj: ; $(AS) $(AFLAGS) -c $*.asm

all:	$(PROJ).EXE

ls.obj:	ls.c $(H)

cuadro.obj:	..\ventanas\cuadro.c $(H)

menu.obj:	..\ventanas\menu.c $(H)

raton.obj:	..\ventanas\raton.c $(H)

ventana.obj:	..\ventanas\ventana.c $(H)

editor.obj:	..\ventanas\editor.c $(H)

$(PROJ).EXE:	ls.obj cuadro.obj menu.obj raton.obj ventana.obj editor.obj $(OBJS_EXT)
	echo >NUL @<<$(PROJ).crf
ls.obj +
cuadro.obj +
menu.obj +
raton.obj +
ventana.obj +
editor.obj +
$(OBJS_EXT)
$(PROJ).EXE

$(LIBS_EXT);
<<
	qlink $(LFLAGS) @$(PROJ).crf

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)

