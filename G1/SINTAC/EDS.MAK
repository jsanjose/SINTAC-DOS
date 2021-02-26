PROJ	=EDS
DEBUG	=1
CC	=qcl
CFLAGS_G	= /AC /W4 /Ze 
CFLAGS_D	= /Zi /Zr /Od 
CFLAGS_R	= /Od /Gs /DNDEBUG 
CFLAGS	=$(CFLAGS_G) $(CFLAGS_D)
LFLAGS_G	= /CP:0xfff /NOI /SE:0x80 /ST:0x2800 
LFLAGS_D	= /CO 
LFLAGS_R	= /EXEPACK 
LFLAGS	=$(LFLAGS_G) $(LFLAGS_D)
RUNFLAGS	=
H = 	eds.h version.h sintac.h scr.h color.h 
OBJS_EXT = 	
LIBS_EXT = 	

.asm.obj: ; $(AS) $(AFLAGS) -c $*.asm

all:	$(PROJ).EXE

eds.obj:	eds.c $(H)

scr.obj:	scr.c $(H)

$(PROJ).EXE:	eds.obj scr.obj $(OBJS_EXT)
	echo >NUL @<<$(PROJ).crf
eds.obj +
scr.obj +
$(OBJS_EXT)
$(PROJ).EXE

$(LIBS_EXT);
<<
	ilink -a -e "qlink $(LFLAGS) @$(PROJ).crf" $(PROJ)

run: $(PROJ).EXE
	$(PROJ) $(RUNFLAGS)

