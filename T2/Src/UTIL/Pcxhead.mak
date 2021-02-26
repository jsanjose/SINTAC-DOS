PROJ	=PCXHEAD
DEBUG	=1
CC	=qcl
CFLAGS_G	= /AT /W4 /Ze 
CFLAGS_D	= /Zi /Zr /Od 
CFLAGS_R	= /Od /Gs /DNDEBUG 
CFLAGS	=$(CFLAGS_G) $(CFLAGS_D)
LFLAGS_G	= /CP:0xfff /NOI /NOE /SE:0x80 /T /ST:0x800 CRTCOM.LIB 
LFLAGS_D	= /CO 
LFLAGS_R	= 
LFLAGS	=$(LFLAGS_G) $(LFLAGS_D)
RUNFLAGS	=
OBJS_EXT = 	
LIBS_EXT = 	

.asm.obj: ; $(AS) $(AFLAGS) -c $*.asm

all:	$(PROJ).COM

pcxhead.obj:	pcxhead.c $(H)

$(PROJ).COM:	pcxhead.obj $(OBJS_EXT)
	echo >NUL @<<$(PROJ).crf
pcxhead.obj +
$(OBJS_EXT)
$(PROJ).COM

$(LIBS_EXT);
<<
	ilink -a -e "qlink $(LFLAGS) @$(PROJ).crf" $(PROJ)

run: $(PROJ).COM
	$(PROJ) $(RUNFLAGS)

