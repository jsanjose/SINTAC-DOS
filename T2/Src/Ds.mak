PROJ	=DS
DEBUG	=1
CC	=qcl
CFLAGS_G	= /AC /W4 /Ze /DDEBUGGER=ON 
CFLAGS_D	= /Zi /Zr /Od 
CFLAGS_R	= /Od /Gs /DNDEBUG 
CFLAGS	=$(CFLAGS_G) $(CFLAGS_D)
LFLAGS_G	= /CP:0xfff /NOI /SE:0x80 /ST:0x800 
LFLAGS_D	= /CO 
LFLAGS_R	= /EXEPACK 
LFLAGS	=$(LFLAGS_G) $(LFLAGS_D)
RUNFLAGS	=av.dat
H = 	version.h sintac.h ds.h win.h tabcond.h 
OBJS_EXT = 	
LIBS_EXT = 	

.asm.obj: ; $(AS) $(AFLAGS) -c $*.asm

all:	$(PROJ).EXE

ds.obj:	ds.c $(H)

win.obj:	win.c $(H)

$(PROJ).EXE:	ds.obj win.obj $(OBJS_EXT)
	echo >NUL @<<$(PROJ).crf
ds.obj +
win.obj +
$(OBJS_EXT)
$(PROJ).EXE

$(LIBS_EXT);
<<
	ilink -a -e "qlink $(LFLAGS) @$(PROJ).crf" $(PROJ)

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)

