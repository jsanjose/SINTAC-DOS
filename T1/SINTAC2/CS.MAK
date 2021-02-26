PROJ	=CS
DEBUG	=1
CC	=qcl
CFLAGS_G	= /AC /W4 /Ze 
CFLAGS_D	= /Zi /Zr /Od 
CFLAGS_R	= /Od /DNDEBUG 
CFLAGS	=$(CFLAGS_G) $(CFLAGS_D)
LFLAGS_G	= /CP:0xfff /NOI /SE:0x80 /ST:0x1000 
LFLAGS_D	= /CO 
LFLAGS_R	= /EXEPACK 
LFLAGS	=$(LFLAGS_G) $(LFLAGS_D)
RUNFLAGS	=
H = 	sintac.h version.h cs.h compil.h cnd.h tabcond.h condacto.h 
OBJS_EXT = 	
LIBS_EXT = 	

.asm.obj: ; $(AS) $(AFLAGS) -c $*.asm

all:	$(PROJ).EXE

cs.obj:	cs.c $(H)

compil.obj:	compil.c $(H)

condacto.obj:	condacto.c $(H)

$(PROJ).EXE:	cs.obj compil.obj condacto.obj $(OBJS_EXT)
	echo >NUL @<<$(PROJ).crf
cs.obj +
compil.obj +
condacto.obj +
$(OBJS_EXT)
$(PROJ).EXE

$(LIBS_EXT);
<<
	ilink -a -e "qlink $(LFLAGS) @$(PROJ).crf" $(PROJ)

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)

