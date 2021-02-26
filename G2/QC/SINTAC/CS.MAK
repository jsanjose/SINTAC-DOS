PROJ	=CS
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
H = 	sintac.h version.h cs.h compil.h tabcond.h cserr.h color.h ..\ventanas\ventana.h \
	..\ventanas\cuadro.h ..\ventanas\raton.h 
OBJS_EXT = 	
LIBS_EXT = 	

.asm.obj: ; $(AS) $(AFLAGS) -c $*.asm

all:	$(PROJ).EXE

cs.obj:	cs.c $(H)

compil.obj:	compil.c $(H)

cuadro.obj:	..\ventanas\cuadro.c $(H)

raton.obj:	..\ventanas\raton.c $(H)

ventana.obj:	..\ventanas\ventana.c $(H)

$(PROJ).EXE:	cs.obj compil.obj cuadro.obj raton.obj ventana.obj $(OBJS_EXT)
	echo >NUL @<<$(PROJ).crf
cs.obj +
compil.obj +
cuadro.obj +
raton.obj +
ventana.obj +
$(OBJS_EXT)
$(PROJ).EXE

$(LIBS_EXT);
<<
	qlink $(LFLAGS) @$(PROJ).crf

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)

