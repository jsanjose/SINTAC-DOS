PROJ	=EDS
DEBUG	=0
CC	=qcl
CFLAGS_G	= /AL /W4 /Ze 
CFLAGS_D	= /Zi /Zr /Od 
CFLAGS_R	= /Od /Gs /DNDEBUG 
CFLAGS	=$(CFLAGS_G) $(CFLAGS_R)
LFLAGS_G	= /CP:0xfff /NOI /SE:0x80 /ST:0x2000 
LFLAGS_D	= /CO 
LFLAGS_R	= /EXEPACK 
LFLAGS	=$(LFLAGS_G) $(LFLAGS_R)
RUNFLAGS	=
H = 	..\ventanas\ayuda.h ..\ventanas\cuadro.h ..\ventanas\editor.h ..\ventanas\menu.h \
	..\ventanas\raton.h ..\ventanas\ventana.h sintac.h version.h eds.h color.h 
OBJS_EXT = 	
LIBS_EXT = 	

.asm.obj: ; $(AS) $(AFLAGS) -c $*.asm

all:	$(PROJ).EXE

eds.obj:	eds.c $(H)

ayuda.obj:	..\ventanas\ayuda.c $(H)

cuadro.obj:	..\ventanas\cuadro.c $(H)

editor.obj:	..\ventanas\editor.c $(H)

menu.obj:	..\ventanas\menu.c $(H)

raton.obj:	..\ventanas\raton.c $(H)

ventana.obj:	..\ventanas\ventana.c $(H)

$(PROJ).EXE:	eds.obj ayuda.obj cuadro.obj editor.obj menu.obj raton.obj ventana.obj $(OBJS_EXT)
	echo >NUL @<<$(PROJ).crf
eds.obj +
ayuda.obj +
cuadro.obj +
editor.obj +
menu.obj +
raton.obj +
ventana.obj +
$(OBJS_EXT)
$(PROJ).EXE

$(LIBS_EXT);
<<
	qlink $(LFLAGS) @$(PROJ).crf

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)

