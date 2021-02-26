
	IDEAL
	P386

	INCLUDE "ESI.INC"

	SEGMENT 	ESI_DATA	USE16 'FAR_DATA'

		; Datos del mezclador: volumetable, canales...

		; Valores para "CHANNEL::Active"
		;	0	No procesar este canal ni los siguientes
		;	1	Procesar como mono
		;	2	Procesar como stereo - izquierda
		;	3	Procesar como stereo - derecha
		;	4	Procesar como stereo - seg£n "volumes"
		;	0FFh	Canal "desactivado"


	VolumeTable	DB	16384 DUP (?)

	LABEL	FinalTable	BYTE

		DB 128 DUP (0)
		i = 0
		REPT 256
		   DB i
		   i = i + 1
		ENDM
		DB 128 DUP (255)

	PanningTable1	DB	63, 63, 63, 63, 63, 63, 63, 63
			DB	63, 47, 35, 26, 19, 14, 8, 0
	PanningTable2	DB	0, 8, 14, 19, 26, 35, 47, 63
			DB	63, 63, 63, 63, 63, 63, 63, 63

	LABEL	_Channels	CHANNEL

		MACRO ChannelDef X
		    Channel&X	CHANNEL <>
		ENDM

		i = 1
		REPT NumChannels
		    ChannelDef %i
		    i = i + 1
		ENDM

		ChannelEnd	CHANNEL <0>

	_GlobalVolume	DB	63

	PeriodTable	DW	1712, 1616, 1524, 1440, 1356, 1280
			DW	1208, 1140, 1076, 1016, 960, 907
	AQPeriodTable	DW	1556, 1469, 1385, 1308, 1232, 1163
			DW	1098, 1036, 978, 923, 872, 824

		; Datos del gestor de instrumentos

	LABEL	_Samples	SAMPLEHEADER

		MACRO SampleDef X
		    Sample&X	SAMPLEHEADER <>
		ENDM

		i = 1

		REPT MaxSamples
		    SampleDef %i
		    i = i + 1
		ENDM

	SampleLoad	SAMPLEHEADER <>

		; Informaci¢n de DMA

		IdealBufferSize = 32768

	DMABasePort	DB	0, 2, 4, 6
	DMAPagePort	DB	87h, 83h, 81h, 82h

	DMABuffer	DB	IdealBufferSize DUP (128)

	DMAModeByte	DB	59h		; 58h + canal
	DMAOffset	DW	?
	_DMASize		DW	?
	DMAMaxSize	DW	?
	DMAPage 	DB	?

	_DMABufferPtr	DW	offset DMABuffer, ESI_DATA
	ChunkPtr	DW	?
	ChunkSize	DW	?

	DMAPos		DW	?
	DMAOkPos	DW	?
	DMADif		DW	0
	OldDMADif	DW	0

		; Informaci¢n de IRQ usada por el DSP

	IrqInt		DB	8, 9, 10, 11, 12, 13, 14, 15
			DB	112, 113, 114
	OldIrq		DD	?
	IC2Used 	DB	?		; 1 para IRQ 10

	OldPort21	DB	?
	OldPortA1	DB	?

	_IrqCount	DW	0
	_IntCount	DW	0

		; Datos SB

	_SB2x0		DW	0220h
	_SB2x4		DW	0224h
	_SB2xC		DW	022Ch
	_SB2xE		DW	022Eh
	_SBDMAChannel	DB	1
	_SBIrq		DB	7

	_SBHiSpeed	DB	?
	_SBTimeConstant DB	?
	_Hz		DW	43000

	SBStarted	DB	0
	_SBDetected	DB	0
	_SBStereo	DB	0

	Mezclas 	DW	offset MezclasMono	; Puntero NEAR a rutina
	Filtrar 	DW	offset Filtrar0

		; Variables del mezclador

		PITConstant = 05B08h

	OldInt8 	DD	?
	PITCount	DW	0
	PITIConstant	DW	PITConstant

	SInc		DW	?
	SFInc		DW	?
	SLoopOff	DW	?
	SLoopSize	DW	?
	SEnd		DW	?
	VolTabBase	DW	?
	VolTabBase2	DW	?

	PeriodTableSeg	DW	?

		; Variables de la canci¢n activa

			MaxOrders = 256

	_SongName	DB	29 DUP (0)
	_OrderNum	DW	0
	_PatNum 	DW	0
	_InsNum 	DW	0

	_Orders 	DB	MaxOrders DUP (0FFh)
	_PatternSegTable DW	256 DUP (0)
	_PatternSize	DW	0		; Variable temporal
	_PatternBuffer	DW	0
	I_GlobalVolume	DB	0
	ISpeed		DB	0
	ITempo		DB	0
	ChannelSets	DB	32 DUP (0)
	_Stereo 	DB	1

	SongLoad	DB	1024 DUP (0)

			; En ejecuci¢n

	PlayingSong	DB	0

	_Order		DB	0
	_Pattern	DB	0
	_PatternRow	DB	0
	_PatternMustEnd DB	0
	_PatternStartRow DB	0
	_PatternSeg	DW	0
	_PatternOff	DW	0
	SpeedCount	DB	0
	_Speed		DB	0

	PlayRow 	DB	160 DUP (?)

		; Debugging & Testing

	DebugSample	DW	?
	DebugCount	DW	0
	DebugKey	DB	0

	ENDS		ESI_DATA

	SEGMENT 	ESI_CODE	USE16 'CODE'

		ASSUME CS:ESI_CODE, DS:ESI_DATA

	; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	; Funciones de acceso a memoria
	; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

		; Estas funciones permite gestionar bloques
		; de hasta 64K de memoria DOS o EMS

		MemInit 	DB	0
		EMSFound	DB	0
		EMSDriverName	DB	"EMMXXXX0", 0
		EMSPageFrame	DW	0
		EMSAvailable	DW	0
		DOSAvailable	DW	0

		ContextHandle	DW	0

InitMem:

	; Modifica AX

		push dx
		push bx
		push ds
		mov ah, 048h
		sub bx, bx
		dec bx
		int 021h
		mov [DOSAvailable], bx

		mov ax, cs
		mov ds, ax
		mov ax, 03D00h
		mov dx, offset EMSDriverName
		int 021h
		jc @@NoEMS
		mov bx, ax
		push bx
		mov ax, 4400h
		int 021h
		pop bx
		jc @@EMSBad
		cmp dx, 80h
		jz @@EMSBad
		push bx
		mov ax, 4407h
		int 021h
		pop bx
		or al, al
		jz @@EMSBad
		mov [EMSFound], 1
		mov ax, 03E00h
		int 021h
		mov ah, 41h
		int 67h
		mov [EMSPageFrame], bx
		mov [MemInit], 1
		call GetFreeEMS
		mov [EMSAvailable], ax

		mov ah, 43h
		mov bx, 1
		int 067h
		or ah, ah
		jnz @@EMSBad
		mov [ContextHandle], dx

		mov ah, 047h
		mov dx, [ContextHandle]
		int 067h
		or ah, ah
		jnz @@EMSBad

		mov ah, 048h
		mov dx, [ContextHandle]
		int 067h
		or ah, ah
		jnz @@EMSBad

		pop ds
		pop bx
		pop dx
		ret

    @@EMSBad:	mov ax, 03E00h
		int 21h
    @@NoEMS:	mov [MemInit], 1
		mov [EMSFound], 0
		pop ds
		pop bx
		pop dx
		ret

EndMem:

		cmp [MemInit], 0
		je @@End
		cmp [EMSFound], 0
		je @@End
		push dx
		push ax
		mov dx, [ContextHandle]
		mov ah, 045h
		int 067h
		pop ax
		pop dx
	@@End:	ret

AllocMem:

	; BX - N§ de p rrafos (16 bytes) a reservar
	; Salida: AX - Handle/Segmento, C=1 si error

	; No modifica ning£n registro excepto AX

		cmp [MemInit], 0
		jne @@1
		call InitMem
	@@1:	cmp [EMSFound], 0
		je @@3
		push bx
		movzx ebx, bx
		test ebx, 1023
		jz @@2
		add ebx, 1024
	@@2:	shr ebx, 10
		mov ah, 043h
		int 067h
		pop bx
		or ah, ah
		jnz @@3
		mov ax, dx
		ret
	@@3:	mov ah, 048h
		int 021h
		ret

DeAllocMem:

	; AX - Segmento o handle EMS

		or ah, ah
		jz @@EMS
		push es
		mov es, ax
		mov ah, 049h
		int 021h
		pop es
		ret
	@@EMS:	mov dx, ax
		mov ah, 045h
		int 067h
		ret

AccessMem:

	; AX - Segmento o Handle EMS
	; Salida: AX - Segmento

		or ah, ah
		jz @@PFEMS
		ret
	@@PFEMS:push bx
		push cx
		push dx
		mov dh, 04Ch
		xchg ax, dx
		int 067h
		mov cx, bx
		sub bx, bx
		sub al, al
	@@AMem: mov ah, 044h
		int 067h
		inc bx
		inc al
		loop @@AMem
		pop dx
		pop cx
		pop bx
		mov ax, [EMSPageFrame]
		ret

GetFreeEMS:

	; Salida: AX - Free EMS

		cmp [MemInit], 0
		jne @@1
		call InitMem
	@@1:	cmp [EMSFound], 0
		je @@2
		mov ah, 42h
		int 067h
		mov ax, bx
		ret
	@@2:	sub ax, ax
		ret

	; --------------------------- Interface con C


PROC	_AllocMem	FAR

	ARG	memsize:WORD

		push bp
		mov bp, sp
		push ds
		mov ax, ESI_DATA
		mov ds, ax
		mov bx, [memsize]
		test bx, 15
		jz @@1
		add bx, 16
	@@1:	shr bx, 4
		call AllocMem
		pop ds
		pop bp
		ret

ENDP	_AllocMem

PROC	_FreeMem	FAR

	ARG	handle:WORD

		push bp
		mov bp, sp
		mov ax, [handle]
		call near DeAllocMem
		pop bp
		ret

ENDP	_FreeMem

PROC	_AccessMem	FAR

	ARG	handle:WORD

		push bp
		mov bp, sp
		mov ax, [handle]
		call near AccessMem
		pop bp
		sub dx, dx
		xchg ax, dx
		ret

ENDP	_AccessMem

PROC	_GetFreeEMS	FAR

		push ds
		mov ax, ESI_DATA
		mov ds, ax
		call near GetFreeEMS
		pop ds
		ret

ENDP	_GetFreeEMS

	; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	; Inicializaci¢n de los datos de la SoundBlaster
	; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

SetSoundBlaster:

		mov [_SBDetected], 1
		cmp [_Stereo], 1
		jne @@ssend

		mov [Mezclas], offset MezclasMono
		mov dx, [_SB2x4]
		mov al, 022h
		out dx, al
		inc dx
		in al, dx
		mov bl, al
		mov al, 0
		out dx, al
		in al, dx
		cmp al, 011h
		jnz @@ssend
		mov al, 099h
		out dx, al
		in al, dx
		cmp al, 099h
		jne @@ssend
		mov al, bl
		out dx, al
		mov [_SBStereo], 1
		cmp [_Stereo], 0
		je @@ssend
		mov [Mezclas], offset MezclasStereo
		dec dx
		mov al, 0eh
		out dx, al
		inc dx
		in al, dx
		or al, 2
		out dx, al		; Activar Stereo

		mov di, offset _Channels
		mov cx, NumChannels/2
		mov di, offset _Channels
		mov cx, NumChannels/2
	@@setpan:
		mov [(channel ptr di).Panning], 2
		add di, size channel
		mov [(channel ptr di).Panning], 13
		add di, size channel
		loop @@setpan

	@@ssend:ret

	; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	; Inicializaci¢n
	; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

PROC	_SBStart	FAR

		push si
		push di
		push ds
		mov ax, ESI_DATA
		mov ds, ax

		mov [SBStarted], 0
		call SetSoundBlaster
		cmp [_SBDetected], 1
		je @@1
		stc
		mov ax, -1
		pop ds
		pop di
		pop si
		ret

	@@1:	call SetupDMABuffer
		cli
		call EnableIRQ
		sti

		mov dx, [_SB2xC]
	sbww0:	in al, dx
		shl al, 1
		jc sbww0
		mov al, 0D1h
		out dx, al
		mov cx, 0
		loop $
		mov cx, 0
		loop $

		; Activar frecuencia

		mov bx, [_Hz]
		call SetHz
		call SetupSoundTables

		; Comenzar con el sonido

		cli
		call StartDMA
		call StartDSP
		call StartVector8Int
		sti
		sub ax, ax
		clc

		pop ds
		pop di
		pop si
		ret

ENDP	_SBStart

PROC	_SBEnd	FAR

		push si
		push di
		push ds
		mov ax, ESI_DATA
		mov ds, ax

		cli
		call EndVector8Int
		call ResetDSP

		mov dx, [_SB2xC]
	sbw1:	in al, dx
		shl al, 1
		jc sbw1
		mov al, 0D3h
		out dx, al

		call DisableIrq
		sti

		cmp [PeriodTableSeg], 0
		je @@sbe2
		mov ax, [PeriodTableSeg]
		call DeAllocMem
	@@sbe2:

		mov cx, MaxSamples
	@@sbe1: mov bx, MaxSamples
		sub bx, cx
		push cx
		call KillInstrument
		pop cx
		loop @@sbe1

		mov si, offset _PatternSegTable
		mov cx, [_PatNum]
		jcxz @@sbe
	@@sbe0: lodsw
		or ax, ax
		jz @@sbe
		call DeAllocMem
		mov [word ptr si-2], 0
		loop @@sbe0
	@@sbe:

		pop ds
		pop di
		pop si

		call EndMem
		ret

ENDP	_SBEnd

SetupDMABuffer:

		mov ax, offset DMABuffer
		mov [_DMABufferPtr], ax
		mov dx, ESI_DATA
		mov bl, dh
		shr bl, 4
		shl dx, 4
		add ax, dx
		adc bl, 0
		mov [DMAPage], bl
		mov [DMAOffset], ax
		neg ax
		cmp ax, IdealBufferSize
		jbe @@nosup
		mov ax, IdealBufferSize
	@@nosup:mov [DMAMaxSize], ax
		cmp ax, IdealBufferSize / 2
		jae ab1
		add [_DMABufferPtr], ax
		mov ax, IdealBufferSize
		sub ax, [DMAMaxSize]
		mov [DMAMaxSize], ax
		mov [DMAOffset], 0
		inc [DMAPage]
	ab1:	clc
		ret

SetHz:

	; BX - Hz

		cmp [SBStarted], 0
		je @@7
		push bx
		cli
		call EndVector8Int
		call ResetDSP
		call DisableIRQ
		sti
		mov ax, ESI_DATA
		mov ds, ax
		mov es, ax
		mov di, offset DMABuffer
		mov cx, IdealBufferSize /2
		mov ax, 8080h
		rep stosw
		pop bx
		mov [_Hz], bx
		call _SBStart
		ret

	@@7:	movzx ebx, bx
		mov eax, 1000000
		cdq
		div ebx
		mov edx, eax
		mov eax, 256
		sub eax, edx
		mov [_SBTimeConstant], al
		mov [_SBHiSpeed], 0
		cmp al, 219
		jb sh0
		mov [_SBHiSpeed], 1
	sh0:	mov eax, -1000000
		movzx ebx, [_SBTimeConstant]
		sub ebx, 256
		cdq
		idiv ebx
		mov [_Hz], ax
		movzx ebx, ax
		cmp [_Stereo], 1
		jne @@nostereo
		shr [_Hz], 1
	@@nostereo:
		mov eax, PITConstant
		mul ebx
		cdq
		mov ebx, 01234DDh		; 1193181d
		div ebx
		and al, 0FEh
		mov [ChunkSize], ax
		movzx ebx, ax
		movzx eax, [DMAMaxSize]
		cdq
		div ebx
		cdq
		mul [ChunkSize]
		mov [_DMASize], ax
		mov ax, [_DMABufferPtr]
		mov [ChunkPtr], ax
		call SendDSPTimeConstant
		ret

PROC	_SetHz	FAR

	ARG	hz:WORD

		push bp
		mov bp, sp
		push ds
		mov ax, ESI_DATA
		mov ds, ax
		mov bx, [hz]
		call SetHz
		pop ds
		pop bp
		ret

ENDP	_SetHz

PROC    SetupSoundTables        FAR

		; VolumeTable

		push di
		push es

		mov di, offset VolumeTable
		sub bx, bx
	st0:	sub cx, cx
	st1:	mov ax, cx
		sub ax, 128
		imul bx
		cwd
		push bx
		mov bx, 63
		idiv bx
		pop bx
		add ax, 128
		mov [di], al
		inc di
		inc cl
		jnz st1
		inc bx
		cmp bx, 64
		jl st0

		; PeriodTable

		mov [PeriodTableSeg], 0
		mov bx, 4096
		call AllocMem
		jnc ptnerr
		pop es
		pop di
		ret
	ptnerr: mov [PeriodTableSeg], ax
		call AccessMem
		mov es, ax
		mov ebx, 2
		mov di, 4
	dts1:	push ebx
		mov eax, 14353408
		sub edx, edx
		div ebx
		sub edx, edx
		shld edx, eax, 16
		shl eax, 16
		movzx ebx, [_Hz]
		div ebx
		pop ebx
		mov [es:di], eax
		add ebx, 2
		add di, 4
		jnc dts1

		pop es
		pop di
		ret

ENDP    SetupSoundTables

PROC    StartVector8Int         FAR

		mov bx, PITConstant
		mov [PITIConstant], bx
		mov al, 036h
		out 043h, al
		mov al, bl
		out 040h, al
		mov al, bh
		out 040h, al

		sub ax, ax
		mov es, ax
		mov ax, cs
		shl eax, 16
		add eax, offset Vector8Int
		mov edx, [es:20h]
		mov [OldInt8], edx
		mov [es:20h], eax
		mov [PITCount], 0
		ret

ENDP    StartVector8Int

PROC    EndVector8Int           FAR

		mov al, 036h
		out 43h, al
		sub al, al
		out 040h, al
		out 040h, al
		sub ax, ax
		mov es, ax
		mov eax, [OldInt8]
		mov [es:20h], eax
		ret

ENDP    EndVector8Int

	; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	; Rutinas varias: programaci¢n del DSP y del DMA
	; y preparaci¢n de la IRQ
	; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

EnableIrq:

		; Recoger datos del IC para preservarlos
		; y habilitar la IRQ

		movzx bx, [_SBIrq]
		movzx ax, [byte ptr IrqInt+bx]
		shl ax, 2
		mov di, ax
		sub ax, ax
		mov es, ax
		mov eax, [es:di]
		mov [OldIrq], eax
		mov ax, ESI_CODE
		sal eax, 16
		mov ax, offset Irq
		mov [es:di], eax

		in al, 0A1h
		mov [OldPortA1], al
		mov dl, al
		in al, 021h
		mov [OldPort21], al
		mov cl, [_SBIrq]
		mov ah, 0FEh
		rol ah, cl
		cmp cl, 8
		jge sbs1
		and al, ah
		out 021h, al
		ret
	sbs1:	and al, 0FDh
		out 021h, al
		mov al, dl
		and al, ah
		out 0A1h, al
		ret

DisableIRQ:
		mov al, [OldPortA1]
		out 0A1h, al
		mov al, [OldPort21]
		out 021h, al
		xor ax, ax
		mov es, ax
		movzx bx, [_SBIrq]
		movzx ax, [IrqInt+bx]
		shl ax, 4
		mov di, ax
		mov eax, [OldIrq]
		mov [es:di], eax
		ret

StartDMA:

		cmp [_SBDMAChannel], 3
		je StartDMA3
		cmp [_SBDMAChannel], 1
		je StartDMA1

	StartDMA0:

		mov al, 4
		out 0Ah, al
		xor al, al
		out 0Ch, al
		mov al, [DMAModeByte]
		out 0Bh, al
		mov ax,[DMAOffset]
		out 0, al
		xchg ah, al
		out 0, al
		mov ax, [_DMASize]
		dec ax
		out 1, al
		xchg ah, al
		out 1, al
		mov al, [DMAPage]
		out 087h, al
		mov al, 0
		out 0Ah, al
		ret

	StartDMA1:

		mov al, 5
		out 0Ah, al
		xor al, al
		out 0Ch, al
		mov al, [DMAModeByte]
		out 0Bh, al
		mov ax, [DMAOffset]
		out 2, al
		xchg ah, al
		out 2, al
		mov ax, [_DMASize]
		dec ax
		out 3, al
		xchg ah, al
		out 3, al
		mov al, [DMAPage]
		out 083h, al
		mov al, 1
		out 0Ah, al
		ret

	StartDMA3:

		mov al, 7
		out 0Ah, al
		xor al, al
		out 0Ch, al
		mov al, [DMAModeByte]
		out 0Bh, al
		mov ax, [DMAOffset]
		out 6, al
		xchg ah, al
		out 6, al
		mov ax, [_DMASize]
		dec ax
		out 7, al
		xchg ah, al
		out 7, al
		mov al, [DMAPage]
		out 082h, al
		mov al, 3
		out 0Ah, al
		ret

GetDMACount:

		movzx dx, [_SBDMAChannel]
		add dx, dx
		inc dx
		mov al, 0FFh
		out 0Ch, al
		in al, dx
		xchg ah, al
		in al, dx
		xchg ah, al
		ret

StartDSP:

		; Programaci¢n del DSP

		mov dx, [_SB2xC]
	sbi0:	in al, dx
		shl al, 1
		jc sbi0
		mov al, 048h
		cmp [_SBHiSpeed], 1
		je sbh
		mov al, 014h
	sbh:	out dx, al			; Out 048h (HiSpeed) or 014h
	sbi1:	in al, dx
		shl al, 1
		jc sbi1
		mov ax, [_DMASize]
		dec ax
		out dx, al			; Out LO(_DMASize)
	sbi2:	in al, dx
		shl al, 1
		jc sbi2
		xchg ah, al
		out dx, al			; Out HI(_DMASize)
		cmp [_SBHiSpeed], 1
		jne sbnh
	sbi3:	in al, dx
		shl al, 1
		jc sbi3
		mov al, 091h
		out dx, al			; Out 091h (HiSpeed)
	sbnh:
		mov dx, [_SB2xE]
		in al, dx
		ret

ResetDSP:

		push dx
		push cx
		mov dx, [_SB2x0]
		add dx, 6
		mov al, 1
		out dx, al
		mov cx, 02000h
	rsd0:	nop
		nop
		dec cx
		jnz rsd0
		mov al, 0
		out dx, al
		mov cx, 02000h
	rsd1:	nop
		nop
		dec cx
		jnz rsd1
		mov cx, 02000h
		add dx, 0Eh-6
	rsd2:	in al, dx
		shl al, 1
		jc rsd3
		loop rsd2
	rsd3:	pop cx
		pop dx
		ret

SendDSPTimeConstant:

		mov dx, [_SB2xC]
	sh1:	in al, dx
		shl al, 1
		jc sh1
		mov al, 040h
		out dx, al
	sh2:	in al, dx
		shl al, 1
		jc sh2
		mov al, [_SBTimeConstant]
		out dx, al

		ret

	; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	; IRQ
	; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

Irq:

		cli
		push ax
		push dx
		push ds

		mov ax, ESI_DATA
		mov ds, ax
		cmp [_SBDetected], 0
		je nodsp
		call StartDSP
	nodsp:	inc [_IrqCount]

		; Decirle al IC que ya est 

	nice:	mov al, 020h
		cmp [IC2Used], 0
		je nic2
		out 0A0h, al
	nic2:	out 020h, al

		sti
		pop ds
		pop dx
		pop ax
		iret

	; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	; Rutina de interrupci¢n 8
	; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

Vector8Int:

		pushad
		push ds
		push es
		mov ax, ESI_DATA
		mov ds, ax
		cld

		add [PITCount], PITConstant
		jc vbye
		mov al, 020h
		out 020h, al
		jmp vbye2

	vbye:	pushf
		call [OldInt8]
	vbye2:

		; Averigua si el DMA ya empez¢ con el £ltimo Chunk
		; rellenado y modifica la constante del PIT

		mov ax, [_DMASize]
		add ax, [_DMABufferPtr]
		sub ax, [ChunkPtr]
		mov bx, ax
		call GetDMACount
		mov [DMAPos], ax
		mov [DMAOkPos], bx
		cmp bx, ax
		jb x1s
	x1n:	mov dx, bx
		sub dx, ax
		mov cx, [_DMASize]
		add cx, ax
		sub cx, bx
		cmp dx, cx
		jb x1nn
	x1ns:	neg cx
		mov [DMADif], cx
		jmp xok
	x1nn:	mov [DMADif], dx
		jmp xok
	x1s:	mov dx, ax
		sub dx, bx
		mov cx, [_DMASize]
		add cx, bx
		sub cx, ax
		cmp dx, cx
		jb x1sn
	x1ss:	mov [DMADif], cx
		jmp xok
	x1sn:	neg dx
		mov [DMADif], dx
	xok:	cmp [DMADif], 0
		jg xx2
		cmp [PITIConstant], PITConstant
		ja z1
		mov [PITIConstant], PITConstant
	    z1: add [PITIConstant], 4
		jmp xx3
	xx2:	cmp [PITIConstant], PITConstant
		jb z2
		mov [PITIConstant], PITConstant
	    z2: sub [PITIConstant], 16
	xx3:

		mov cx, [PITIConstant]
		mov al, 36h
		out 43h, al
		mov ax, [PITIConstant]
		out 40h, al
		xchg ah, al
		out 40h, al

		; Prepara la EMS

		cmp [EMSFound], 1
		jne @@NoPEMS
		mov ah, 047h
		mov dx, [ContextHandle]
		int 067h
	@@NoPEMS:

		; Averigua la posici¢n del siguiente Chunk

		call ProcessSong

        gomezclas:

		mov ax, [ChunkSize]
		add [ChunkPtr], ax
		mov ax, [_DMASize]
		add ax, [_DMABufferPtr]
		sub ax, [ChunkSize]
		inc ax
		cmp ax, [ChunkPtr]
		ja vi1
		mov ax, [_DMABufferPtr]
		mov [ChunkPtr], ax
	vi1:

		; Llama a la rutina de mezclas

		call [Mezclas]
		mov ax, ESI_DATA
		mov ds, ax
		call [Filtrar]

		cmp [SBStarted], 0
		jne vi2
		inc [SBStarted]
		jmp gomezclas
	vi2:

		; Termina ­bye!

		cmp [EMSFound], 1
		jne @@NoREMS
		mov ah, 048h
		mov dx, [ContextHandle]
		int 067h
	@@NoREMS:

		inc [_IntCount]
		pop es
		pop ds
		popad
		iret

	; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	; Rutina de mezclas: mono
	; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

MezclasMono:

		cld
		mov ax, ds
		mov es, ax
		mov di, [ChunkPtr]
		mov cx, [ChunkSize]
		mov al, 80h
		rep stosb

		mov bx, offset _Channels

	chan:			; Procesar canal

		mov al, [(channel ptr bx).Active]
		cmp al, 0
		jne ch1
		jmp MezclasMonoEnd
	ch1:	cmp al, 0FFh
		jne ch2
		jmp nextchan
	ch2:	mov ax, [(channel ptr bx).SampleSeg]
		or ax, ax
		jnz ch3
		jmp nextchan
	ch3:

		mov di, [ChunkPtr]
		mov si, [word ptr (channel ptr bx).Volume]
		and si, 07Fh
		cmp si, 63
		jna @@ch4
		mov si, 63
	@@ch4:	mov ah, [_GlobalVolume]
		sub al, al
		add si, ax
		mov ah, [VolumeTable + 128 + si]
		sub ah, 128
		add ax, offset VolumeTable
		mov [VolTabBase], ax
		mov ax, [(channel ptr bx).SampleInc]
		mov [SInc], ax
		mov ax, [(channel ptr bx).SampleFInc]
		mov [SFInc], ax
		mov dx, [(channel ptr bx).SampleFOffset]
		mov ax, [(channel ptr bx).SampleSeg]
		call AccessMem
		mov es, ax
		mov ax, [(channel ptr bx).SampleSize]
		mov [SEnd], ax
		mov si, [(channel ptr bx).SampleOffset]
		mov cx, [ChunkSize]
		push bx

		mov ax, 0FFFFh
		mov [SLoopOff], ax
		cmp [(channel ptr bx).SampleLoop], 0
		je smix
		mov ax, [(channel ptr bx).SampleLoopEnd]
		cmp ax, [(channel ptr bx).SampleLoopStart]
		jbe smix
		sub ax, [(channel ptr bx).SampleLoopStart]
		mov [SLoopSize], ax
		mov ax, [(channel ptr bx).SampleLoopEnd]
		mov [SLoopOff], ax
		mov [SEnd], ax

	smix:	mov ax, [VolTabBase]

	mix:	movzx bx, [byte ptr es:si]
		add dx, [SFInc]
		adc si, [SInc]
		jc mixbr
	mixp:	cmp si, [SEnd]
		jae mixbr
		add bx, ax
		movzx bx, [bx]
		add bl, [di]
		adc bh, 0
		mov bl, [FinalTable+bx]
		mov [di], bl
	mixc:	inc di
		dec cx
		jnz mix

		pop bx
		mov [(channel ptr bx).SampleOffset], si
		mov [(channel ptr bx).SampleFOffset], dx

	nextchan:

		add bx, SIZE channel
		jmp chan

	mixl:	sub si, [SLoopSize]
		jmp mixp
	mixbr:	cmp [SLoopOff], 0FFFFh
		jne mixl
		pop bx
		mov [(channel ptr bx).SampleSeg], 0
		jmp nextchan

MezclasMonoEnd:

		ret

	; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	; Rutina de mezclas: stereo
	; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

MACRO	GlobalStereo	X

	; BX - Instrumento (con Panning)
	; X - Tabla (1 o 2)
	; Salida: AX = 128*Volumen global del canal elegido

		mov ah, [_GlobalVolume]
		sub al, al
		push bx
		movzx bx, [(channel ptr bx).Panning]
		and bx, 0Fh
		add al, [PanningTable&X+bx]
		adc ah, 0
		mov bx, ax
		mov ah, [VolumeTable + 128 + bx]
		sub ah, 128
		sub al, al
		pop bx

ENDM	Global2Panned

MezclasStereo:

		cld
		mov ax, ds
		mov es, ax
		mov di, [ChunkPtr]
		mov cx, [ChunkSize]
		mov al, 80h
		rep stosb

		mov bx, offset _Channels

	@@chan: 		; Procesar canal

		mov al, [(channel ptr bx).Active]
		cmp al, 0
		jne @@ch1
		jmp MezclasStereoEnd
	@@ch1:	cmp al, 0FFh
		jne @@ch2
		jmp @@nextchan
	@@ch2:	mov ax, [(channel ptr bx).SampleSeg]
		or ax, ax
		jnz @@ch3
		jmp @@nextchan
	@@ch3:	mov di, [ChunkPtr]
		mov si, [word ptr (channel ptr bx).Volume]
		and si, 07Fh
		cmp si, 63
		jna @@ch4
		mov si, 63
	@@ch4:	GlobalStereo 1
		push si
		add si, ax
		mov ah, [VolumeTable + 128 + si]
		sub ah, 128
		add ax, offset VolumeTable
		mov [VolTabBase], ax
		pop si
		GlobalStereo 2
		add si, ax
		mov ah, [VolumeTable + 128 + si]
		sub ah, 128
		add ax, offset VolumeTable
		mov [VolTabBase2], ax

		mov ax, [(channel ptr bx).SampleInc]
		mov [SInc], ax
		mov ax, [(channel ptr bx).SampleFInc]
		mov [SFInc], ax
		mov dx, [(channel ptr bx).SampleFOffset]
		mov ax, [(channel ptr bx).SampleSeg]
		call AccessMem
		mov es, ax
		mov ax, [(channel ptr bx).SampleSize]
		mov [SEnd], ax
		mov si, [(channel ptr bx).SampleOffset]
		mov cx, [ChunkSize]
		shr cx, 1
		push bx

		mov ax, 0FFFFh
		mov [SLoopOff], ax
		cmp [(channel ptr bx).SampleLoop], 0
		je @@smix
		mov ax, [(channel ptr bx).SampleLoopEnd]
		cmp ax, [(channel ptr bx).SampleLoopStart]
		jbe @@smix
		sub ax, [(channel ptr bx).SampleLoopStart]
		mov [SLoopSize], ax
		mov ax, [(channel ptr bx).SampleLoopEnd]
		mov [SLoopOff], ax
		mov [SEnd], ax

	@@smix: mov ax, [VolTabBase]

	@@mix:	movzx ax, [byte ptr es:si]
		add dx, [SFInc]
		adc si, [SInc]
		jc @@mixbr
	@@mixp: cmp si, [SEnd]
		jae @@mixbr
		mov bx, ax
		add bx, [VolTabBase]
		movzx bx, [bx]
		add bl, [di]
		adc bh, 0
		mov bl, [FinalTable+bx]
		mov [di], bl
		inc di
		mov bx, ax
		add bx, [VolTabBase2]
		movzx bx, [bx]
		add bl, [di]
		adc bh, 0
		mov bl, [FinalTable+bx]
		mov [di], bl
		inc di
		dec cx
		jnz @@mix

		pop bx
		mov [(channel ptr bx).SampleOffset], si
		mov [(channel ptr bx).SampleFOffset], dx

	@@nextchan:

		add bx, SIZE channel
		jmp @@chan

	@@mixl: sub si, [SLoopSize]
		jmp @@mixp
	@@mixbr:cmp [SLoopOff], 0FFFFh
		jne @@mixl
		pop bx
		mov [(channel ptr bx).SampleSeg], 0
		jmp @@nextchan

MezclasStereoEnd:

		ret

	; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	; Rutinas de filtros
	; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

Filtrar0:	ret

Filtrar1:

		mov bx, 1
		cmp [_Stereo], 1
		jne @@1
		mov bx, 2
	@@1:

		mov cx, [ChunkSize]
		dec cx
		mov si, [ChunkPtr]
	f1buc:	movzx ax, [si]
		add ax, ax
		add al, [si]
		adc ah, 0
		add al, [si+bx]
		adc ah, 0
		shr ax, 2
		mov [si], al
		inc si
		dec cx
		jnz f1buc
		ret

Filtrar2:

		mov cx, [ChunkSize]
		dec cx
		dec cx
		dec cx
		mov si, [ChunkPtr]
	f2buc:	movzx ax, [si]
		add al, [si+1]
		adc ah, 0
		add al, [si+2]
		adc ah, 0
		add al, [si+3]
		adc ah, 0
		shr ax, 2
		mov [si], al
		inc si
		dec cx
		jnz f2buc
		ret

Filtrar3:

		mov cx, [ChunkSize]
		sub cx, 7
		mov si, [ChunkPtr]
	f3buc:	movzx ax, [si]
		add al, [si+1]
		adc ah, 0
		add al, [si+2]
		adc ah, 0
		add al, [si+3]
		adc ah, 0
		add al, [si+4]
		adc ah, 0
		add al, [si+5]
		adc ah, 0
		add al, [si+6]
		adc ah, 0
		add al, [si+7]
		adc ah, 0
		shr ax, 3
		mov [si], al
		inc si
		dec cx
		jnz f3buc
		ret

PROC	SetFilter	FAR

		push ds
		push ax
		mov ax, ESI_DATA
		mov ds, ax
		pop ax

		cmp ax, 0
		je sf0
		cmp ax, 1
		je sf1
		cmp ax, 2
		je sf2
		cmp ax, 3
		je sf3
		pop ds
		ret
	sf0:	mov [Filtrar], offset Filtrar0
		pop ds
		ret
	sf1:	mov [Filtrar], offset Filtrar1
		pop ds
		ret
	sf2:	mov [Filtrar], offset Filtrar2
		pop ds
		ret
	sf3:	mov [Filtrar], offset Filtrar3
		pop ds
		ret

ENDP	SetFilter

	; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	; Rutina para procesar canci¢n
	; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

		ChannelCount	DB	0

	EfectTableTick  DW	OFFSET pet_end
			DW      OFFSET pet_SetSpeed
                        DW      OFFSET pet_OrderJump
                        DW      OFFSET pet_Break
                        DW      OFFSET pet_VolSlide
                        DW      OFFSET pet_SlideDown
                        DW      OFFSET pet_SlideUp
			DW      OFFSET pet_end
			DW      OFFSET pet_end
			DW      OFFSET pet_end
			DW      OFFSET pet_end
			DW      OFFSET pet_end
			DW      OFFSET pet_end
			DW      OFFSET pet_end
			DW      OFFSET pet_end
                        DW      OFFSET pet_Offset
			DW      OFFSET pet_end
			DW      OFFSET pet_end
			DW      OFFSET pet_end
                        DW      OFFSET pet_Multiple
			DW      OFFSET pet_end
			DW      OFFSET pet_end
                        DW      OFFSET pet_SetGlobalVol
			DW      OFFSET pet_end
			DW      OFFSET pet_end
			DW      OFFSET pet_end
			DW      OFFSET pet_end

        EfectTable      DW      OFFSET pef_end
			DW      OFFSET pef_end
			DW      OFFSET pef_end
			DW      OFFSET pef_end
			DW      OFFSET pef_VolSlide
			DW      OFFSET pef_SlideDown
			DW      OFFSET pef_SlideUp
			DW      OFFSET pef_TonePort
			DW      OFFSET pef_end          ; Vibrato
			DW      OFFSET pef_Tremor
			DW      OFFSET pef_Arpeggio
			DW      OFFSET pef_end          ; Vib_VolSl
			DW      OFFSET pef_end          ; Tone_VolSl
			DW      OFFSET pef_end
			DW      OFFSET pef_end
			DW      OFFSET pef_end
			DW      OFFSET pef_end
			DW      OFFSET pef_Retrig
                        DW      OFFSET pef_end          ; Tremolo
                        DW      OFFSET pef_Multiple
                        DW      OFFSET pef_end
                        DW      OFFSET pef_end
                        DW      OFFSET pef_end
                        DW      OFFSET pef_end
                        DW      OFFSET pef_end
                        DW      OFFSET pef_end
                        DW      OFFSET pef_end

PROC	ProcessSong	NEAR

		cmp [PlayingSong], 1
		jne @@End

		cmp [SpeedCount], 0
		je @@Tick
		dec [SpeedCount]
		jnz @@Frame

                ; Aqu¡ llega cada vez que n§tick = speed y hay que
		; leer la siguiente nota, etc.
                ; ------------------------------------------------

	@@Tick: mov al, [_Speed]
		mov [SpeedCount], al

		inc [_PatternRow]
		cmp [_PatternMustEnd], 1
		je @@_PatternEnd
		cmp [_PatternRow], 64
		jl @@ProcessRow

	@@_PatternEnd:
		inc [_Order]
	@@3:	movzx bx, [_Order]
		mov al, [_Orders + bx]
		cmp al, 0FEh
		je @@3
		cmp al, 0FFh
		jne @@5
		inc [_Order]
		jnc @@3
		dec [_Order]
	@@4:	inc [_Order]
		movzx bx, [_Order]
		mov al, [_Orders + bx]
		cmp al, 0FEh
		je @@4
		cmp al, 0FFh
		jne @@5
		mov [PlayingSong], 0
		jmp @@End
	@@5:	mov [_Pattern], al
		mov al, [_PatternStartRow]
		mov [_PatternRow], al
		mov [_PatternStartRow], 0
		mov [_PatternMustEnd], 0

	@@ProcessRow:
		movzx bx, [_Pattern]
		add bx, bx
		mov ax, [_PatternSegTable + bx]
		movzx dx, [_PatternRow]
		shl dx, 5
		mov si, dx
		shl si, 2
		add si, dx
		call AccessMem
		mov ds, ax
		mov ax, ESI_DATA
		mov es, ax
		mov di, offset PlayRow
		mov cx, 160
		rep movsb
		mov ax, ESI_DATA
		mov ds, ax

	; -------------- Procesar una fila: tocar las notas (tick)

		mov [ChannelCount], 0
	psn0:	movzx bx, [ChannelCount]
		shl bx, 2
		add bl, [ChannelCount]
		adc bh, 0
		add bx, offset PlayRow

		cmp [(entry ptr bx).Note], 0FFh
		je psnnot
		mov al, [(entry ptr bx).Note]
		cmp al, KeyOff
		jne psnota
		movzx cx, [ChannelCount]
		call StopSample
		jmp pssig
	psnota: cmp [(entry ptr bx).Effect], Ef_TonePort
		jne psplnot
		movzx cx, [ChannelCount]
		NumChannel2ChannelOffset
		mov [(channel ptr si).Note], al
		mov al, [(entry ptr bx).Volume]
		cmp al, 0FFh
		jne @@xx
		movzx bx, [(entry ptr bx).Instrument]
		cmp bl, 0FFh
		je pssig
		NumInstrument2InstrumentOffset
		mov al, [(sampleheader ptr bx).Volume]
	@@xx:	mov [(channel ptr si).Volume], al
		jmp pssig
	psplnot:sub ecx, ecx
		movzx cx, [(entry ptr bx).Volume]
		shl ecx, 16
		movzx cx, [ChannelCount]
		movzx bx, [(entry ptr bx).Instrument]
		call PlayInstrument
		jmp pssig
	psnnot: cmp [(entry ptr bx).Instrument], 0FFh
		je psni
		push bx
		movzx bx, [(entry ptr bx).Instrument]

		NumInstrument2InstrumentOffset

		mov al, [(sampleheader ptr bx).Volume]
		movzx cx, [ChannelCount]
		call SetVolume
		pop bx
	psni:	cmp [(entry ptr bx).Volume], 0FFh
		je pssig
		mov al, [(entry ptr bx).Volume]
		movzx cx, [ChannelCount]
		call SetVolume
	pssig:	inc [ChannelCount]
		cmp [ChannelCount], NumChannels
		jne psn0

	; -------------- Procesar efectos (tick)

		mov [ChannelCount], 0
		mov di, offset PlayRow
		mov bx, offset _Channels
	pef1:	mov ah, [(entry ptr di).EffectInfo]
		cmp ah, 0
		je @@Lef
		cmp ah, 0FFh
		je @@Lef
		mov [(channel ptr bx).EffectInfo], ah
		jmp @@NLef
	@@Lef:	cmp [(entry ptr di).Effect], 4
		jb @@NLef
		cmp [(entry ptr di).Effect], Ef_Multiple
		je @@NLef
		cmp [(entry ptr di).EffectInfo], 0
		jne @@NLef
		mov ah, [(channel ptr bx).EffectInfo]
	@@NLef: mov al, [(entry ptr di).Effect]
		cmp [(channel ptr bx).Effect], al
		je @@NL2
		mov [(channel ptr bx).Effect], al
		mov [(channel ptr bx).EffectCount], 0
	@@NL2:  push si
		cmp al, 0FFh
		je pet_end

		movzx si, al
		add si, si
		jmp [EfectTableTick+si]

       pet_Multiple:
		mov al, ah
		shr al, 4
		and ah, 0Fh
		cmp al, MEf_Panning
		je pet_setpanning
       pet_End: pop si
		add di, 5
		add bx, size Channel
		inc [ChannelCount]
		cmp [ChannelCount], NumChannels
		jne pef1

                jmp @@Frame

	; -------------- Aqu¡ termina el proceso de cada tick

        ; Aqu¡ llega cada frame            pef = process efect frame
        ; ----------------------------------------------------------

	@@Frame:			; Procesar efectos (frame)

		mov [ChannelCount], 0
		mov di, offset PlayRow
		mov bx, offset _Channels
	pef2:   push si
		mov al, [(entry ptr di).Effect]
		cmp al, 0FFh
		je pef_end
		cmp [(entry ptr di).Effect], 4
		jb @@LefX
		cmp [(entry ptr di).Effect], Ef_Multiple
		je @@LefX
		cmp [(entry ptr di).EffectInfo], 0
		jne @@LefX
		mov ah, [(channel ptr bx).EffectInfo]
		jmp @@XLef
	@@LefX: mov ah, [(entry ptr di).EffectInfo]
	@@XLef:

		movzx si, al
		add si, si
		jmp [EfectTable+si]

       pef_End:

		pop si
		inc [(channel ptr bx).EffectCount]
		add di, 5
		add bx, size Channel
		inc [ChannelCount]
		cmp [ChannelCount], NumChannels
		jne pef2

	@@End:
		ret

        pef_Multiple:

		mov al, ah
		shr al, 4
		and ah, 0Fh
		cmp al, MEf_NoteCut
		je pef_notecut
                jmp pef_End

		; -------------------------------------------
		; Efectos procesados cada tick: AH = InfoByte
		; -------------------------------------------

        pet_setspeed:
		mov [_Speed], ah
		mov [SpeedCount], ah
                jmp pet_end
        pet_orderjump:
		dec ah
		mov [_Order], ah
		mov [_PatternMustEnd], 1
		jmp pet_end
        pet_break:
		cmp ah, 63
                ja pet_end
		mov [_PatternMustEnd], 1
		mov [_PatternStartRow], ah
                jmp pet_end
        pet_volslide:
		mov dl, ah
		and dl, 0Fh
		cmp dl, 0Fh
                je pet_volfineslideup
		mov dl, ah
		and dl, 0F0h
		cmp dl, 0F0h
                je pet_volfineslidedown
                jmp pet_end                     ; Los dem s slides
						; se tratan cada frame
        pet_volfineslideup:
		shr ah, 4
		or ah, ah
		jnz vfsuz
		mov ah, 0Fh
	vfsuz:	mov dh, [(channel ptr bx).Volume]
		add dh, ah
		cmp dh, 63
		jbe vfsux
		mov dh, 63
	vfsux:	mov [(channel ptr bx).Volume], dh
                jmp pet_end

        pet_volfineslidedown:
		and ah, 0Fh
		jnz vfsdz
		mov ah, 0Fh
	vfsdz:	mov dh, [(channel ptr bx).Volume]
		sub dh, ah
		jnc vfsdx
		mov dh, 0
	vfsdx:	mov [(channel ptr bx).Volume], dh
                jmp pet_end

        pet_offset:
		sub al, al
		mov [(channel ptr bx).SampleOffset], ax
                jmp pet_end

        pet_setglobalvol:
		cmp ah, 63
		jnb sgvx
		mov ah, 63
	sgvx:	mov [_GlobalVolume], ah
                jmp pet_end

        pet_slideup:
		mov dh, ah
		and dh, 0F0h
		cmp dh, 0F0h
                jne pet_end
		movzx dx, ah
		shl dx, 2
		mov ax, [(channel ptr bx).Period]
		sub ax, dx
                jc pet_end
		mov [(channel ptr bx).Period], ax
		call Period2Speed
		mov [(channel ptr bx).SampleFInc], ax
		shr ax, 16
		mov [(channel ptr bx).SampleInc], ax
                jmp pet_end
	pet_slidedown:
		mov dh, ah
		and dh, 0F0h
		cmp dh, 0F0h
                jne pet_end
		movzx ax, ah
		shl ax, 2
		add ax, [(channel ptr bx).Period]
                jc pet_end
		mov [(channel ptr bx).Period], ax
		call Period2Speed
		mov [(channel ptr bx).SampleFInc], ax
		shr ax, 16
		mov [(channel ptr bx).SampleInc], ax
                jmp pet_end
        pet_setpanning:
		mov [(channel ptr bx).Panning], ah
                jmp pet_end

		; --------------------------------------------
		; Efectos procesados cada frame: AH = InfoByte
		; --------------------------------------------

        ; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ VolSlide
        pef_volslide:
		mov dl, ah
		and dl, 0Fh
		jz pef_volslideframeup
		mov dl, ah
		and dl, 0F0h
		jz pef_volslideframedown
                jmp pef_end
	pef_volslideframeup:
		shr ah, 4
		add [(channel ptr bx).Volume], ah
		cmp [(channel ptr bx).Volume], 64
                jl pef_end
		mov [(channel ptr bx).Volume], 63
                jmp pef_end
	pef_volslideframedown:
		and ah, 0Fh
		sub [(channel ptr bx).Volume], ah
                jnc pef_end
		mov [(channel ptr bx).Volume], 0
                jmp pef_end

        ; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ Retrig
        pef_retrig:
		mov dh, ah
		and dh, 0Fh
		cmp dh, [(channel ptr bx).EffectCount]
                jne pef_end
		mov [(channel ptr bx).EffectCount], 0
		mov [(channel ptr bx).SampleOffset], 0
		mov [(channel ptr bx).SampleFOffset], 0
	noretrig:
		mov cl, ah
		and cl, 0F0h
		shr cl, 4
		cmp cl, 0
                je pef_end
		cmp cl, 5
		jg nrv1
		mov al, 1
		shl al, cl
		sub [(channel ptr bx).Volume], al
                jnc pef_end
		mov [(channel ptr bx).Volume], 0
                jmp pef_end
	nrv1:	cmp cl, 6
		jg nrv2
		mov al, [(channel ptr bx).Volume]
		shl al, 1
		sub ah, ah
		shr cl, 1
		div cl
		mov [(channel ptr bx).Volume], al
                jmp pef_end
	nrv2:	cmp cl, 7
		jg nrv3
		mov al, [(channel ptr bx).Volume]
		shr al, 1
		mov [(channel ptr bx).Volume], al
                jmp pef_end
	nrv3:	cmp cl, 0Dh
		jg nrv4
		sub cl, 9
                jc pef_end
		mov al, 1
		shl al, cl
		add [(channel ptr bx).Volume], al
		cmp [(channel ptr bx).Volume], 63
                jng pef_end
		mov [(channel ptr bx).Volume], 63
                jmp pef_end
	nrv4:	cmp cl, 0Eh
		jne nrv5
		mov al, [(channel ptr bx).Volume]
		shr al, 1
		mov cl, 3
		sub ah, ah
		mul cl
		mov [(channel ptr bx).Volume], al
		cmp [(channel ptr bx).Volume], 63
                jng pef_end
		mov [(channel ptr bx).Volume], 63
                jmp pef_end
	nrv5:	mov al, [(channel ptr bx).Volume]
		shl al, 1
		mov [(channel ptr bx).Volume], 63
                jng pef_end
		mov [(channel ptr bx).Volume], 63
                jmp pef_end

        ; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ Slide Up
        pef_slideup:
		cmp ah, 0DFh
		ja pefsup2
		movzx dx, ah
		shl dx, 2
	   __s: mov ax, [(channel ptr bx).Period]
		sub ax, dx
                jc pef_end
		mov [(channel ptr bx).Period], ax
		call Period2Speed
		mov [(channel ptr bx).SampleFInc], ax
		shr eax, 16
		mov [(channel ptr bx).SampleInc], ax
                jmp pef_end
	   pefsup2:
		cmp ah, 0EFh
                ja pef_end
		and ah, 0Fh
		movzx dx, ah
		jmp __s

        ; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ Slide Down
        pef_slidedown:
		cmp ah, 0DFh
		ja pefinf2
		movzx ax, ah
		shl ax, 2
	   __t: add ax, [(channel ptr bx).Period]
                jc pef_end
		mov [(channel ptr bx).Period], ax
		call Period2Speed
		mov [(channel ptr bx).SampleFInc], ax
		shr eax, 16
		mov [(channel ptr bx).SampleInc], ax
                jmp pef_end
	   pefinf2:
		cmp ah, 0EFh
                ja pef_end
		and ah, 0Fh
		movzx ax, ah
		jmp __t

        ; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ Tone Portamento
        pef_toneport:
		movzx cx, ah
		shl cx, 2
		mov al, [(channel ptr bx).Note]
		mov dx, [(channel ptr bx).C2SPD]
		call Note2Period
		mov dx, [(channel ptr bx).Period]
		cmp ax, dx
		ja pef_toneportup
	pef_toneportdown:
		sub dx, cx
		jc __z
		cmp dx, ax
		jae __x
	   __z: mov dx, ax
	   __x: mov [(channel ptr bx).Period], dx
		mov ax, dx
		call Period2Speed
		mov [(channel ptr bx).SampleFInc], ax
		shr eax, 16
		mov [(channel ptr bx).SampleInc], ax
                jmp pef_end
	pef_toneportup:
		add dx, cx
		jc __z
		cmp dx, ax
		jbe __x
		mov dx, ax
		jmp __x

        ; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ Tremor
        pef_tremor:
		mov dh, [(channel ptr bx).EffectCount]
	   tremorcomp:
		mov dl, ah
		shr dl, 4
		cmp dh, dl
		je pef_mute
		mov al, ah
		and al, 0Fh
		add dl, al
		cmp dh, dl
                jb pef_end
		mov [(channel ptr bx).EffectCount], 0FFh
	pef_unmute:
		mov al, [(channel ptr bx).OldVolume]
		mov [(channel ptr bx).Volume], al
                jmp pef_end
	pef_mute:
		cmp [(channel ptr bx).Volume], 0
                je pef_end
		mov al, [(channel ptr bx).Volume]
		mov [(channel ptr bx).OldVolume], al
		mov [(channel ptr bx).Volume], 0
                jmp pef_end

        ; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ Arpeggio
        pef_arpeggio:
		mov dl, [(channel ptr bx).EffectCount]
		cmp dl, 0
		je pef_arpeggionorm
		cmp dl, 1
		je pef_arpeggio1
		and ah, 0Fh
		mov [(channel ptr bx).EffectCount], -1
	   pef_arpeggio2:
		mov al, [(channel ptr bx).Note]
		mov dl, al
		and al, 0Fh
		and dl, 0F0h
		add al, ah
	   Ar3: cmp al, 11
		jng Ar4
		sub al, 11
		add dl, 010h
		jmp Ar3
	   Ar4: add al, dl
		mov dx, [(channel ptr bx).C2SPD]
		call Note2Period
		call Period2Speed
		mov [(channel ptr bx).SampleFInc], ax
		shr eax, 16
		mov [(channel ptr bx).SampleInc], ax
                jmp pef_end
	   pef_arpeggionorm:
		sub ah, ah
		jmp pef_arpeggio2
	   pef_arpeggio1:
		shr ah, 4
		jmp pef_arpeggio2

        ; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ NoteCut
	   pef_notecut:
		cmp ah, [(channel ptr bx).EffectCount]
                jne pef_end
		mov [(channel ptr bx).SampleSeg], 0
                jmp pef_end

ENDP	ProcessSong

PROC	_StopSong	FAR

		push ds
		mov ax, ESI_DATA
		mov ds, ax
		mov [PlayingSong], 0
		pop ds
		ret

ENDP	_StopSong

PROC	_StartSong	FAR

		push si
		push di
		push ds

		mov ax, ESI_DATA
		mov es, ax
		mov ds, ax
		mov di, offset PlayRow
		mov cx, 160
		mov al, 0FFh
		rep stosb

		mov [PlayingSong], 1
		mov [_Order], 0FFh
	iord3:	inc [_Order]
		movzx bx, [_Order]
		mov al, [_Orders + bx]
		cmp al, 0FEh
		je iord3
		cmp al, 0FFh
		jne ordno
		mov [PlayingSong], 0
		jmp psngen
	ordno:	mov [_Pattern], al
		mov [_PatternRow], 0FFh
		mov al, [ISpeed]
		mov [_Speed], al
		mov [SpeedCount], 1
		mov al, [I_GlobalVolume]
		mov [_GlobalVolume], al

	psngen: pop ds
		pop di
		pop si
		ret

ENDP	_StartSong

	; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	; Rutinas PLAY, para rellenar info. en CHANNEL
	; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

		; Las siguientes rutinas ser n llamadas
		; con DS = ESI_DATA. Algunas destruyen todos los
		; registros comunes (AX, BX, CX, DX, SI, DI)


SetVolume:

	; CX - n§ canal (0 - ...)
	; AL - volumen

		cmp al, 63
		jng vnmsv
		mov al, 63
	vnmsv:	NumChannel2ChannelOffset
		mov [(channel ptr si).Volume], al
		ret

StopSample:

	; CX - n§ canal (0 - ...)

		NumChannel2ChannelOffset
		mov [(channel ptr si).SampleSeg], 0
		ret

PROC	_PlayInstrument FAR

	ARG	inst:word, canal:word, volumen:word, nota:word, octava:word, panning:word

		push bp
		mov bp, sp
		push si
		push di
		push ds
		mov ax, ESI_DATA
		mov ds, ax

		mov bx, [inst]
		mov cx, [volumen]
		shl ecx, 16
		mov cx, [canal]
		mov ax, [nota]
		mov ah, [byte ptr octava]
		shl ah, 4
		and al, 0Fh
		or al, ah
		call near PlayInstrument
		mov ax, [panning]
		and al, 0Fh
		mov [(channel ptr si).Panning], al

		pop ds
		pop di
		pop si
		pop bp
		ret

ENDP	_PlayInstrument

PlayInstrument:

	; BX - N§ de instrumento (0FFh = £ltimo instrumento/volumen usado)
	; ECX - Canal | (Volumen << 16) (Volumen 0FFh = volumen base)
	; AL - (Octava << 4) | Nota  (0FFh = conservar nota anterior)


		NumChannel2ChannelOffset
		cmp bx, MaxSamples
		jb @@1
		movzx bx, [(channel ptr si).Instrument]
		mov eax, ecx
		shr eax, 16
		cmp al, 0FFh
		jne @@1
		and ecx, 0FFFFh
		movzx eax, [(channel ptr si).Volume]
		shl eax, 16
		or ecx, eax
	@@1:	cmp bx, MaxSamples
		jb @@2
		ret
	@@2:	mov [(channel ptr si).Instrument], bl
		NumInstrument2InstrumentOffset

		cmp [(sampleheader ptr bx).Type], 1
		je @@3
		ret
	@@3:

		cmp al, 0FFh
		je nonote

		mov [(channel ptr si).Note], al
		mov dx, [word ptr (SampleHeader ptr bx).C2SPD]
		mov [(channel ptr si).C2SPD], dx
		call Note2Period
		mov [(channel ptr si).Period], ax
		call Period2Speed
		mov [(channel ptr si).SampleFInc], ax
		shr eax, 16
		mov [(channel ptr si).SampleInc], ax

	nonote: mov eax, ecx
		shr eax, 16
		cmp al, 0FFh
		jne _vnff
		mov al, [(SampleHeader ptr bx).Volume]
	_vnff:	mov [(channel ptr si).Volume], al
		mov ax, [word ptr (SampleHeader ptr bx).Size]
		mov [(channel ptr si).SampleSize], ax
		mov dx, [word ptr (SampleHeader ptr bx).LoopEnd]
		shl edx, 16
		mov dx, [word ptr (SampleHeader ptr bx).LoopBeg]
		mov [dword ptr (channel ptr si).SampleLoopStart], edx
		mov ax, [(SampleHeader ptr bx).MemSeg]
		mov [(channel ptr si).SampleSeg], ax
		mov [(channel ptr si).SampleOffset], 0
		mov [(channel ptr si).SampleFOffset], 0
		mov al, 1
		or edx, edx
		jnz ps2
		dec al
	ps2:	mov [(channel ptr si).SampleLoop], al
		ret

Period2Speed:

	; Entrada: AX - Per¡odo
	; Salida:  EAX - Incs

		push bx
		movzx ebx, ax
		cmp [PeriodTableSeg], 0
		je pts0
		cmp bx, 07FFFh
		ja pts0
		and bx, 07FFEh
		shl bx, 1
		mov ax, [PeriodTableSeg]
		call AccessMem
		mov es, ax
		mov eax, [es:bx]
		jmp eaxok
	pts0:	mov eax, 14353408
		sub edx, edx
		div ebx
		sub edx, edx
		shld edx, eax, 16
		shl eax, 16
		movzx ebx, [_Hz]
		div ebx
	eaxok:	pop bx
		ret

Note2Period:

	; DX - C2SPD (8363 base)
	; AL - (Octava << 4) | Nota
	; Devuelve el per¡odo en AX

		push cx
		push bx
		mov cl, al
		shr cl, 4
		and al, 0Fh
		movzx bx, al
		add bx, bx
		movzx eax, [PeriodTable + bx]
		shl eax, 4
		shr eax, cl
		cmp dx, 8363
		je gp0
		or dx, dx
		jz gp0
		push dx
		mov ebx, 8363
		mul ebx
		sub edx, edx
		pop bx
		div ebx
	gp0:	pop bx
		pop cx
		ret

	; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	; Gestor de instrumentos
	; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

		LSFError	DW	0FFFFh

PROC	LoadSCRSFile	FAR

	; Entrada... DS:DX - ASCIIZ del fichero a abrir
	; Devuelve el n§ de instrumento en AX, y el SampleHeader en DS:SI
	;	AX = 0FFFFh si error al abrir
	;	AX = 0FFFEh si formato no soportado
	;	AX = 0FFFDh si varianto de formato no soportada

		mov [LSFError], 0FFFFh
		mov ax, 03D00h
		int 021h
		mov bx, ax
		jnc opened
		mov ax, [LSFError]
		ret
      errclose: mov ah, 03Eh
		int 021h
		mov ax, ESI_DATA
		mov ds, ax
		mov ax, [LSFError]
		ret
	opened: mov ax, ESI_DATA
		mov ds, ax
		dec [LSFError]
		mov dx, offset SampleLoad
		mov cx, size SampleHeader
		mov ah, 03Fh
		int 021h
		cmp ax, size SampleHeader
		jne errclose
		dec [LSFError]
		mov ax, ds
		mov es, ax
		mov di, offset _Samples + (MaxSamples-1) * size SampleHeader
		mov cx, MaxSamples
	stst:	cmp [(SampleHeader ptr di).Type], 0FFh
		je tstok
		sub di, size SampleHeader
		loop stst
		jmp errclose
	tstok:	mov si, offset SampleLoad
		mov cx, size SampleHeader
		push di
		rep movsb
		pop si
		push bx
		mov bx, [word ptr SampleLoad.Size]
		shr bx, 4
		test [SampleLoad.Size], 0Fh
		jz tstok2
		inc bx
	tstok2: call AllocMem
		pop bx
		jc errclose
		mov [(SampleHeader ptr si).MemSeg], ax
		mov cx, [word ptr SampleLoad.Size]
		mov ds, ax
		sub dx, dx
		mov ah, 03Fh
		int 021h
		jc errfree
		push ax
		mov ax, ESI_DATA
		mov ds, ax
		pop ax
		cmp ax, [word ptr SampleLoad.Size]
		jne errfree
		call TestSampleLoad
		mov [LSFError], ax
		cmp ax, 0
		jne errfree
		mov ah, 03Eh
		int 021h
		mov ax, si
		sub ax, offset _Samples
		sub dx, dx
		mov bx, size SampleHeader
		div bx
		ret
       errfree: mov ax, [(SampleHeader ptr si).MemSeg]
		call DeAllocMem
		jmp errclose

ENDP	LoadSCRSFile

PROC	_CreateInstrument	FAR

	ARG	handle:WORD, isize:WORD, c2spd:WORD, loopstart:WORD, loopend:WORD

		push bp
		mov bp, sp
		push ds
		push si
		push di

		mov ax, ESI_DATA
		mov ds, ax

		mov si, offset _Samples + MaxSamples * size SampleHeader
		mov ax, MaxSamples
	@@1:	sub si, size SampleHeader
		dec ax
		jc @@end
		cmp [(sampleheader ptr si).Type], 0FFh
		jne @@1
		mov [(sampleheader ptr si).Type], 1
		mov [(sampleheader ptr si).Pack], 0
		mov [(sampleheader ptr si).Volume], 63
		mov dx, [handle]
		mov [(sampleheader ptr si).MemSeg], dx
		movzx edx, [c2spd]
		mov [(sampleheader ptr si).C2SPD], edx
		mov dx, [loopstart]
		mov [(sampleheader ptr si).LoopBeg], edx
		mov dx, [loopend]
		mov [(sampleheader ptr si).LoopEnd], edx
		mov [(sampleheader ptr si).Flags], 1
		cmp dx, [loopstart]
		ja @@2
		mov [(sampleheader ptr si).Flags], 0
	@@2:	mov dx, [isize]
		mov [(sampleheader ptr si).Size], edx

	@@end:	pop di
		pop si
		pop ds
		pop bp
		ret

ENDP	_CreateInstrument

TestSampleLoad:

	; Entrada: DS:SampleLoad - Datos a comprobar
	; Salida:  AX=FFFE no es un instrumento
	;	   AX=FFFD no es un instrumento v lido

		mov ax, 0FFFEh
		cmp [SampleLoad.Firma], 'S'
		jne tend
		cmp [SampleLoad.Firma2], 'C'
		jne tend
		cmp [SampleLoad.Firma3], 'R'
		jne tend
		cmp [SampleLoad.Firma4], 'S'
		jne tend
		mov ax, 0FFFDh
		cmp [SampleLoad.Type], 1
		jne tend
		cmp [SampleLoad.Pack], 0
		jne tend
		test [SampleLoad.Flags], 6
		jnz tend
		cmp [SampleLoad.Size], 0FFFFh
		jg tend
		sub ax, ax
		cmp [SampleLoad.Volume], 63
		ja tend
		mov [SampleLoad.Volume], 63
	tend:	ret

KillInstrument:

	; Entrada: BX - N§ instrumento
	; Salida:  DS:BX - "Slot" del instrumento


		NumInstrument2InstrumentOffset
		cmp [(SampleHeader ptr bx).MemSeg], 0
		je kend
		mov ax, [(SampleHeader ptr bx).MemSeg]
		call DeAllocMem
		mov [(SampleHeader ptr bx).MemSeg], 0
		mov [(SampleHeader ptr bx).Type], 0FFh
		mov [(SampleHeader ptr bx).SampleName], 0
	kend:	ret

	; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
	; Recuperar de disco un m¢dulo Song
	; ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ

		LoadWarning	DW	0
		LoadError	DW	0
		InsCount	DW	0
		PatCount	DW	0

Unpack_Pattern:

	; DS:0 -> Patr¢n comprimido
	; ES:0 -> 10K para descomprimir

		mov cx, 64
		xor si, si
		xor di, di
	upr:	push di
		push cx
		mov al, 0FFh
		mov cx, 160
		rep stosb
		pop cx
		pop di
	upt:	movzx ax, [byte ptr si]
		or ax, ax
		jz rowok
		and ax, 1Fh
		mov bx, ax
		shl bx, 2
		add bx, ax
		mov ah, [byte ptr si]
		inc si
		mov [byte ptr es:di+bx], 0FFh
		mov [byte ptr es:di+bx+1], 0FFh
		test ah, 32
		jz up1
		mov al, [si]
		inc si
		mov [es:di+bx], al
		mov al, [si]
		inc si
		dec al			; Instrumento 0.(x-1) y no 1.x
		mov [es:di+bx+1], al
	up1:	mov [byte ptr es:di+bx+2], 0FFh
		test ah, 64
		jz up2
		mov al, [si]
		inc si
		mov [es:di+bx+2], al
	up2:	mov [byte ptr es:di+bx+3], 0FFh
		test ah, 128
		jz upt
		mov al, [si]
		inc si
		mov [es:di+bx+3], al
		mov al, [si]
		inc si
		mov [es:di+bx+4], al
		jmp upt
	rowok:	add di, 160
		inc si
		loop upr

		sub ax, ax
		ret

UpdateEfect00:

	; ES:0 patr¢n

		mov ax, es
		mov ds, ax

		mov si, 160
		mov cx, 63
	ue00:	mov dx, 32
	ue00x:	mov ax, [si+3]
		cmp al, 0
		je ue00z
		cmp al, 0FFh
		je ue00z
		cmp ah, 0
		jne ue00z
		cmp al, [si-160+3]
		jne ue00z
		mov ah, [si-160+4]
		mov [si+4], ah
	ue00z:	add si, 5
		dec dx
		jnz ue00x
		loop ue00

		sub ax, ax
		ret

PROC	LoadS3M 	FAR

	; Entrada:  DS:DX - Nombre del fichero Song
	; Salida:   AX - FFFFh error al abrir fichero
	;	    AX - FFFEh no es un fichero Song
	;	    AX - FFFDh variante no permitida
	;	    AX - FFFCh fichero corrupto
	;	    AX - FFFBh sin memoria

	;	    AX - 0001h sample err¢neo	(warning)

		mov [LoadWarning], 0
		mov [LoadError], 0FFFFh
		mov ax, 03D00h
		int 021h
		mov bx, ax
		jnc opok
		mov ax, [LoadError]
		ret
	errc:	mov ah, 03Eh
		int 021h
		mov ax, ESI_DATA
		mov ds, ax
		mov ax, [LoadError]
		ret
	opok:	mov ax, ESI_DATA
		mov ds, ax
		dec [LoadError]
		mov dx, offset SongLoad
		mov cx, 060h
		mov ah, 03Fh
		int 021h
		jc errc
		cmp ax, 60h
		jne errc
		cmp [SongLoad+2Ch], 'S'
		jne errc
		cmp [SongLoad+2Dh], 'C'
		jne errc
		cmp [SongLoad+2Eh], 'R'
		jne errc
		cmp [SongLoad+2Fh], 'M'
		jne errc
		dec [LoadError]
		cmp [SongLoad+1Dh], 16
		jne errc
		cmp [SongLoad+2Ah], 2
		jne errc
		cmp [word ptr SongLoad + 20h], MaxOrders
		jg errc
		dec [LoadError]

		; Load Song name & channel settings

		mov ax, ds
		mov es, ax
		mov si, offset SongLoad
		mov di, offset _SongName
		mov cx, 28
		rep movsb
		sub al, al
		stosb
		mov si, offset SongLoad + 40h
		mov di, offset ChannelSets
		mov cx, 32
		rep movsb

		; Load Song vars

		mov ax, [word ptr SongLoad + 20h]
		mov [_OrderNum], ax
		mov ax, [word ptr SongLoad + 22h]
		mov [_InsNum], ax
		mov ax, [word ptr SongLoad + 24h]
		mov [_PatNum], ax
		movzx ax, [SongLoad+30h]
		push bx
		mov bl, [SongLoad+33h]
		and bl, 07Fh
		mul bl
		shr ax, 7
		mov [I_GlobalVolume], al
		pop bx
		mov al, [SongLoad+31h]
		mov [ISpeed], al
		mov al, [SongLoad+32h]
		mov [ITempo], al

		; Load orders

		mov di, offset _Orders
		mov al, 0FFh
		mov cx, MaxOrders
		rep stosb
		mov cx, [_OrderNum]
		mov dx, offset _Orders
		mov ah, 03Fh
		int 021h
		jc errc
		cmp ax, [_OrderNum]
		jne errc

		; Load Instruments+_Patterns offsets

		mov cx, [_InsNum]
		add cx, [_PatNum]
		add cx, cx
		push cx
		mov dx, offset SongLoad
		mov ah, 03Fh
		int 021h
		pop cx
		jc errc
		cmp ax, cx
		jne errc

		; Load instruments

		mov ax, [_InsNum]
		mov [InsCount], ax
		mov si, offset SongLoad
		mov di, offset _Samples
	li:	mov dx, [si]
		sub cx, cx
		shld cx, dx, 4
		shl dx, 4
		mov ax, 4200h
		int 021h
		jc errc
		mov cx, size SampleHeader
		mov dx, offset SampleLoad
		mov ah, 03Fh
		int 021h
		jc errc
		cmp ax, size SampleHeader
		jne errc
		call TestSampleLoad
		cmp ax, 0
		jne snok
		push bx
		mov bx, [word ptr SampleLoad.Size]
		shr bx, 4
		inc bx
		call AllocMem
		pop bx
		jc errmem
		push ds
		pop es
		mov cx, size SampleHeader
		push si
		push di
		mov si, offset SampleLoad
		rep movsb
		pop di
		pop si
		sub edx, edx
		mov dl, [(byte ptr (SampleHeader ptr di).MemSeg)-1]
		shl edx, 16
		mov dx, [(SampleHeader ptr di).MemSeg]
		mov [(byte ptr ((SampleHeader ptr di).MemSeg))-1], 0
		mov [(SampleHeader ptr di).MemSeg], ax
		shl edx, 4
		mov ecx, edx
		shr ecx, 16
		mov ax, 4200h
		int 021h
		sub dx, dx
		mov cx, [word ptr (SampleHeader ptr di).Size]
		mov ax, [(SampleHeader ptr di).MemSeg]
		call AccessMem
		push ds
		mov ds, ax
		mov ah, 03Fh
		int 021h
		pop ds
		jc errc
		cmp ax, [word ptr (SampleHeader ptr di).Size]
		jne errc
	scnt:	inc si
		inc si
		add di, size SampleHeader
		dec [InsCount]
		jnz li

		; Load patterns

		mov ax, [_PatNum]
		mov [PatCount], ax
		mov di, offset _PatternSegTable
	lp:	mov dx, [si]
		sub cx, cx
		shld cx, dx, 4
		shl dx, 4
		mov ax, 4200h
		int 021h
		jc errc
		mov dx, offset _PatternSize
		mov cx, 2
		mov ah, 03Fh
		int 021h
		jc errc
		cmp ax, 2
		jne errc
		mov ax, [_PatternBuffer]
		or ax, ax
		jnz pbok
		push bx
		mov bx, 640
		mov ah, 048h
		int 021h
		pop bx
		jc errmem
		mov [_PatternBuffer], ax
	pbok:	mov cx, [_PatternSize]
		cmp cx, 0
		je errc
		cmp cx, 10240
		ja errc
		push ds
		mov ds, ax
		mov es, ax
		sub dx, dx
		mov ah, 03Fh
		int 021h
		pop ds
		jc errc
		cmp ax, [_PatternSize]
		jne errc
		push bx
		mov bx, 640
		call AllocMem
		pop bx
		jc errmem
		mov [di], ax
		inc di
		inc di
		push ds
		push es
		pop ds
		call AccessMem
		mov es, ax
		push bx
		push si
		push di
		call Unpack_Pattern
		pop di
		pop si
		pop bx
		pop ds
		or ax, ax
		jnz errc
		inc si
		inc si
		dec [PatCount]
		jnz lp

		mov ah, 03Eh
		int 021h

		sub ax, ax
		ret

	snok:	or [LoadWarning], 1
		jmp scnt

	errmem: mov [LoadError], 0FFFBh
		jmp errc

ENDP	LoadS3M

PROC	_LoadS3M	FAR

	ARG	Filename:DWORD

		push bp
		mov bp, sp
		push si
		push di
		push ds
		lds dx, [FileName]
		call LoadS3M
		pop ds
		pop di
		pop si
		pop bp
		ret

ENDP	_LoadS3M

	; Rutina que procesa la variable de entorno BLASTER

PROC	FIND_SB_STRING	FAR

	; ds:si - entorno

	cmp [byte ptr si], 'B'
	je fsbs0
fsbs1:	lodsb
	or al, al
	jnz fsbs1
	cmp [byte ptr si], 0
	jne find_sb_string
	mov si, -1
	ret

fsbs0:	cmp [byte ptr si+1], 'L'
	jne fsbs1
	cmp [byte ptr si+2], 'A'
	jne fsbs1
	cmp [byte ptr si+3], 'S'
	jne fsbs1
	cmp [byte ptr si+4], 'T'
	jne fsbs1
	cmp [byte ptr si+5], 'E'
	jne fsbs1
	cmp [byte ptr si+6], 'R'
	jne fsbs1
	cmp [byte ptr si+7], '='
	jne fsbs1
	add si, 8
	ret

ENDP	FIND_SB_STRING

PROC	process_sb_string	FAR

	; ds:si - cadena

	push	es
	mov	ax, esi_data
	mov	es, ax

	mov	[es:_sbdmachannel], -1
	mov	[es:_sbirq], 0
	mov	[es:_sb2x0], 0

psb14ba:

	cmp	[byte ptr si], 31
	jle	psb155c
	cmp	[byte ptr si], 90
	jg	psb155c

	cmp	[byte ptr si], 'a'
	jnz	psb1508

    psb14d1:
	inc	si
	movzx	ax, [si]
	cmp	al, '0'
	jl	psb14ba
	cmp	al, '9'
	jg	psb14ba
	sub	ax, '0'
	shl	[es:_sb2x0], 4
	add	[es:_sb2x0], ax
	jmp	psb14d1

    psb1508:
	cmp	[byte ptr si], 'd'
	jnz	psb1509
	inc	si
	movzx	ax, [si]
	sub	ax, '0'
	mov	[es:_sbdmachannel], al
	jmp	psb14ba

    psb1509:
	cmp	[byte ptr si], 'i'
	jnz	psb1510
	inc	si
	movzx	ax, [si]
	sub	ax, '0'
	mov	[es:_sbirq], al
	jmp	psb14ba

    psb1510:
	inc	si
	jmp	psb14ba

    psb155c:
	cmp	[es:_sbdmachannel], -1
	jne	dok
	mov	[es:_sbdmachannel], 1
    dok:cmp	[es:_sbirq], 0
	jne	iok
	mov	[es:_sbirq], 7
    iok:cmp	[es:_sb2x0], 0
	jne	sok
	mov	[es:_sb2x0], 0220h

    sok:pop	es
	ret

ENDP	PROCESS_SB_STRING

PROC    _ProcessSBString       FAR

        ARG     string:DWORD

		push bp
		mov bp, sp
		push si
		push ds
                lds si, [string]
                call process_sb_string
		pop ds
		pop si
		pop bp
		ret

ENDP    _ProcessSBString

	ENDS	ESI_CODE

	PUBLIC		_SBStart, _SBEnd, _LoadS3M, _StartSong, _StopSong

	PUBLIC		_SB2x0, _SBDMAChannel, _SBIrq
	PUBLIC		_DMABufferPtr, _DMASize
	PUBLIC		_Channels, _Samples
	PUBLIC		_GlobalVolume
	PUBLIC		_SongName, _OrderNum, _PatNum, _InsNum
	;PUBLIC 	 _Orders, _PatternSegTable

	PUBLIC		_GetFreeEMS

	PUBLIC		_Stereo, _Hz

	PUBLIC		_AllocMem, _FreeMem, _AccessMem

	PUBLIC		_CreateInstrument, _PlayInstrument

        PUBLIC          _ProcessSBString

	SEGMENT ZSEG USE16
	ENDS ZSEG

	END
