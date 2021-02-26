PROJ	=LKS
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
H = 	color.h ..\ventanas\ventana.h ..\ventanas\raton.h ..\ventanas\cuadro.h sintac.h \
	lks.h version.h 
OBJS_EXT = 	
LIBS_EXT = 	

.asm.obj: ; $(AS) $(AFLAGS) -c $*.asm

all:	$(PROJ).EXE

lks.obj:	lks.c $(H)

ventana.obj:	..\ventanas\ventana.c $(H)

raton.obj:	..\ventanas\raton.c $(H)

cuadro.obj:	..\ventanas\cuadro.c $(H)

$(PROJ).EXE:	lks.obj ventana.obj raton.obj cuadro.obj $(OBJS_EXT)
	echo >NUL @<<$(PROJ).crf
lks.obj +
ventana.obj +
raton.obj +
cuadro.obj +
$(OBJS_EXT)
$(PROJ).EXE

$(LIBS_EXT);
<<
	qlink $(LFLAGS) @$(PROJ).crf

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)

