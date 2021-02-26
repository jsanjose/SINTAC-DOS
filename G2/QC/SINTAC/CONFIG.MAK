PROJ	=CONFIG
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
H = 	config.h color.h sintac.h version.h ..\ventanas\cuadro.h ..\ventanas\ventana.h \
	..\ventanas\menu.h ..\ventanas\raton.h 
OBJS_EXT = 	
LIBS_EXT = 	

.asm.obj: ; $(AS) $(AFLAGS) -c $*.asm

all:	$(PROJ).EXE

config.obj:	config.c $(H)

cuadro.obj:	..\ventanas\cuadro.c $(H)

menu.obj:	..\ventanas\menu.c $(H)

ventana.obj:	..\ventanas\ventana.c $(H)

raton.obj:	..\ventanas\raton.c $(H)

$(PROJ).EXE:	config.obj cuadro.obj menu.obj ventana.obj raton.obj $(OBJS_EXT)
	echo >NUL @<<$(PROJ).crf
config.obj +
cuadro.obj +
menu.obj +
ventana.obj +
raton.obj +
$(OBJS_EXT)
$(PROJ).EXE

$(LIBS_EXT);
<<
	qlink $(LFLAGS) @$(PROJ).crf

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)

