PROJ	=DS
DEBUG	=0
CC	=qcl
CFLAGS_G	= /AC /W4 /Ze /DDEBUGGER=1 /DRUNTIME=0 
CFLAGS_D	= /Zi /Zr /Od 
CFLAGS_R	= /Od /Gs /DNDEBUG 
CFLAGS	=$(CFLAGS_G) $(CFLAGS_R)
LFLAGS_G	= /CP:0xfff /NOI /SE:0x80 /ST:0x1000 
LFLAGS_D	= /CO 
LFLAGS_R	= /EXEPACK 
LFLAGS	=$(LFLAGS_G) $(LFLAGS_R)
RUNFLAGS	=
H = 	version.h sintac.h ds.h tabcond.h wing.h condacto.h impchr.h defchr.h pcx.h 
OBJS_EXT = 	
LIBS_EXT = 	

.asm.obj: ; $(AS) $(AFLAGS) -c $*.asm

all:	$(PROJ).EXE

ds.obj:	ds.c $(H)

wing.obj:	wing.c $(H)

condacto.obj:	condacto.c $(H)

impchr.obj:	impchr.c $(H)

pcx.obj:	pcx.c $(H)

$(PROJ).EXE:	ds.obj wing.obj condacto.obj impchr.obj pcx.obj $(OBJS_EXT)
	echo >NUL @<<$(PROJ).crf
ds.obj +
wing.obj +
condacto.obj +
impchr.obj +
pcx.obj +
$(OBJS_EXT)
$(PROJ).EXE

$(LIBS_EXT);
<<
	qlink $(LFLAGS) @$(PROJ).crf

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)

