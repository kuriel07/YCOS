$NOMOD51
;------------------------------------------------------------------------------
;  This file is part of the C51 Compiler package
;  Copyright (c) 1988-2002 Keil Elektronik GmbH and Keil Software, Inc.
;------------------------------------------------------------------------------
;  STARTUP.A51:  This code is executed after processor reset.
;
;  To translate this file use A51 with the following invocation:
;
;     A51 STARTUP.A51
;
;  To link the modified STARTUP.OBJ file to you$NOMOD51
;------------------------------------------------------------------------------
;  This file is part of the C51 Compiler package
;  Copyright (c) 1988-2002 Keil Elektronik GmbH and Keil Software, Inc.
;------------------------------------------------------------------------------
;  STARTUP.A51:  This code is executed after processor reset.
;
;  To translate this file use A51 with the following invocation:
;
;     A51 STARTUP.A51
;
;  To link the modified STARTUP.OBJ file to your application use the following
;  BL51 invocation:
;
;     BL51 <your object file list>, STARTUP.OBJ <controls>
;
;------------------------------------------------------------------------------
;
;  User-defined Power-On Initialization of Memory
;
;  With the following EQU statements the initialization of memory
;  at processor reset can be defined:
;
;		; the absolute start-address of IDATA memory is always 0
IDATALEN	EQU	100H	; the length of IDATA memory in bytes.
;
XDATASTART	EQU	0H	; the absolute start-address of XDATA memory
XDATALEN	EQU	00800H	; the length of XDATA memory in bytes.
;
PDATASTART	EQU	0H	; the absolute start-address of PDATA memory
PDATALEN	EQU	0H	; the length of PDATA memory in bytes.
;
;  Notes:  The IDATA space overlaps physically the DATA and BIT areas of the
;          8051 CPU. At minimum the memory space occupied from the C51 
;          run-time routines must be set to zero.
;------------------------------------------------------------------------------
;
;  Reentrant Stack Initilization
;
;  The following EQU statements define the stack pointer for reentrant
;  functions and initialized it:
;
;  Stack Space for reentrant functions in the SMALL model.
IBPSTACK	EQU	0	; set to 1 if small reentrant is used.
IBPSTACKTOP	EQU	0FFH+1	; set top of stack to highest location+1.
;
;  Stack Space for reentrant functions in the LARGE model.	
XBPSTACK	EQU	1	; set to 1 if large reentrant is used.
XBPSTACKTOP	EQU	07FFH+1; set top of stack to highest location+1.
;
;  Stack Space for reentrant functions in the COMPACT model.	
PBPSTACK	EQU	0	; set to 1 if compact reentrant is used.
PBPSTACKTOP	EQU	07FFH+1; set top of stack to highest location+1.
;
;------------------------------------------------------------------------------
;
;  Page Definition for Using the Compact Model with 64 KByte xdata RAM
;
;  The following EQU statements define the xdata page used for pdata
;  variables. The EQU PPAGE must conform with the PPAGE control used
;  in the linker invocation.
;
PPAGEENABLE	EQU	0	; set to 1 if pdata object are used.
PPAGE		EQU	0	; define PPAGE number.
;
;------------------------------------------------------------------------------
; Standard SFR Symbols 
ACC     DATA    0E0H
B       DATA    0F0H
SP      DATA    81H
DPL     DATA    82H
DPH     DATA    83H
P2		DATA    0A0H
CLKSEL	DATA	8FH

NAME	?C_STARTUP


?C_C51STARTUP	SEGMENT   CODE
?STACK		SEGMENT   IDATA
EXTRN	CODE 	(MAIN)
?C_INITSEG	SEGMENT 	CODE

		RSEG	?STACK
		DS	40H


		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		EXTRN CODE (?C_START)
		PUBLIC	?C_STARTUP

;=======================================================
		CSEG	AT	0x00
?C_STARTUP:
		LJMP	CStartup
;=======================================================

;=======================================================
EXTRN	CODE 	(DES_ISR)
		CSEG	AT	0x03
INTERRUPT0:
		LJMP	DES_ISR
		RETI
		NOP
		NOP
		NOP
		NOP
;=======================================================

;=======================================================
EXTRN	CODE 	(TMR0_ISR)
		CSEG	AT	0x0B
INTERRUPT1:
		LJMP	TMR0_ISR
		RETI
		NOP
		NOP
		NOP
		NOP
;=======================================================

;=======================================================
EXTRN	CODE 	(UART_ISR)
		CSEG	AT	0x13
INTERRUPT2:
		LJMP	UART_ISR
		RETI
		NOP
		NOP
		NOP
		NOP
;=======================================================

;=======================================================
EXTRN	CODE 	(INT3_ISR)
		CSEG	AT	0x1B
INTERRUPT3:
		LJMP	INT3_ISR
		RETI
		NOP
		NOP
		NOP
		NOP
;=======================================================

;=======================================================
EXTRN	CODE 	(FLASH_ISR)
		CSEG	AT	0x23
INTERRUPT4:
		LJMP	FLASH_ISR
		RETI
		NOP
		NOP
		NOP
		NOP
;=======================================================

;=======================================================
EXTRN	CODE 	(INT5_ISR)
		CSEG	AT	0x2B
INTERRUPT5:
		LJMP	INT5_ISR
		RETI
		NOP
		NOP
		NOP
		NOP
;=======================================================

;=======================================================
EXTRN	CODE 	(WDT_ISR)
		CSEG	AT	0x33
INTERRUPT6:
		LJMP	WDT_ISR
		RETI
		NOP
		NOP
		NOP
		NOP
;=======================================================

;=======================================================
		RSEG	?C_C51STARTUP
		CSEG	AT	0x2000
		_XDATA_GROUP_	SEGMENT		XDATA

CStartup:

		MOV		CLKSEL,	#80H

IF IDATALEN <> 0
		MOV		R0,		#IDATALEN - 1
		CLR		A
IDATALOOP:	
		MOV		@R0,	A
		DJNZ	R0,		IDATALOOP
ENDIF

IF XDATALEN <> 0

		MOV		DPTR,	#XDATASTART
		MOV		R7,		#LOW (_XDATA_GROUP_)
  		MOV		R6,		#HIGH (_XDATA_GROUP_)

		MOV		A,		R7
		JNZ		INCHI
JUDGEHI:
		MOV		A,		R6
		JZ		NONXRAMINIT0

INCHI:
		INC		R6

		CLR		A
XDATALOOP:	
		MOVX	@DPTR,	A
		INC		DPTR
		DJNZ	R7,		XDATALOOP
		DJNZ	R6,		XDATALOOP

NONXRAMINIT0:
		
ENDIF

IF PPAGEENABLE <> 0
		MOV		P2,		#PPAGE
ENDIF

IF PDATALEN <> 0
		MOV		R0,		#PDATASTART
		MOV		R7,		#LOW (PDATALEN)
		CLR		A
PDATALOOP:	
		MOVX	@R0,	A
		INC		R0
		DJNZ	R7,		PDATALOOP
ENDIF

IF IBPSTACK <> 0
EXTRN DATA (?C_IBP)

		MOV	?C_IBP,#LOW IBPSTACKTOP
ENDIF

IF XBPSTACK <> 0
EXTRN DATA (?C_XBP)

		MOV	?C_XBP,#HIGH XBPSTACKTOP
		MOV	?C_XBP+1,#LOW XBPSTACKTOP
ENDIF

IF PBPSTACK <> 0
EXTRN DATA (?C_PBP)
		MOV	?C_PBP,#LOW PBPSTACKTOP
ENDIF

;	===============================================
;	Non-zero initialized read/write data initialization
INITIIALIZEDDATA:
	MOV 	DPTR,	#?C_INITSEG
	MOV		R6,	#00H

LOOPINIT:
	CLR		A
	MOVC	A,	@A+DPTR 
	JZ		JUMPTOMAIN						;	No initialized data,jump to main()
	
	INC 	DPTR 
	MOV		R7,	A							;	Save the length in R7
	ANL		A,	#3FH
	JNB		ACC.5,	LEN5BIT					;	Length in 5-bit

LEN13BIT:	
	MOV		A,	R7
	MOV		R6,	A							;	High 5-bit of length
	CLR		A
	MOVC	A,	@A+DPTR 
	INC 	DPTR
	MOV		R7,	A							;	Save the low 8-bit of length in R7
	MOV		A,	R6
	ANL		A,	#7FH
	JNB		ACC.6,	ADDR8BITL				;	Bit 6 is 0b,8-bit address for IDATA or DATA
	ANL		A,	#1FH						;	Bit 6 is 1b,16-bit address for XDATA
	MOV		R6,	A
	SJMP	ADDR16BIT

LEN5BIT:
	MOV		A,	R7
	ANL		A,	#7FH
	JNB		ACC.6,	ADDR8BITS				;	Bit 6 is 0b,8-bit address for IDATA or DATA
	ANL		A,	#1FH						;	Bit 6 is 1b,16-bit address for XDATA
	MOV		R7,	A

ADDR16BIT:
	MOV		A,	R7
	JZ		$+3								;	R6 is increased if and only if the R7 is not zero
	INC		R6
	CLR		A
	MOVC 	A,	@A+DPTR
	MOV		R2,	A							;	High byte of XDATA address
	INC		DPTR
	CLR 	A
	MOVC 	A,	@A+DPTR
	MOV		R0,	A							;	Low byte of XDATA address
	CLR		A
	INC		DPTR

INITXDATA:
	CLR		A
	MOVC 	A,	@A+DPTR                		;   Load the first byte of initialized XDATA
	INC 	DPTR 
    MOV 	R3,	A

    XCH 	A,	R0 							;	Save CODE DPTR to R0(L) and R2(H)
    XCH 	A,	DPL 						;	Set XDATA DPTR from R0(L) and R2(H)	
    XCH 	A,	R0 
    XCH 	A,	R2 
    XCH 	A,	DPH 
    XCH 	A,	R2

    MOV 	A,	R3
    MOVX 	@DPTR,	A 						;	Initialize one byte in XDATA RAM
    INC 	DPTR 

    XCH 	A,	R0 							;	Save XDATA DPTR to R0(L) and R2(H) 
    XCH 	A,	DPL 						;	Set CODE DPTR from R0(L) and R2(H)	 
    XCH 	A,	R0 
    XCH 	A,	R2 
    XCH 	A,	DPH 
    XCH 	A,	R2

	DJNZ 	R7,	INITXDATA
	DJNZ 	R6,	INITXDATA
	SJMP	LOOPINIT 

ADDR8BITL:
	MOV		A,	R6
	ANL		A,	#1FH
	MOV		R6,	A							;	Save the high 5-bit of length in R6
	SJMP	ADDR8BIT

ADDR8BITS:
	MOV		A,	R7
	ANL		A,	#1FH
	MOV		R7,	A

ADDR8BIT:
	INC		R6
	CLR 	A
	MOVC 	A,	@A+DPTR
	MOV		R0,	A							;	8-bit IDATA address
	INC 	DPTR

INITIDATA:
	CLR 	A
	MOVC 	A,	@A+DPTR
	INC 	DPTR
	MOV		@R0,	A
	INC		R0

    DJNZ 	R7,	INITIDATA
	DJNZ 	R6,	INITIDATA
    SJMP	LOOPINIT						;	8-bit offset limited   
;	End of initialization of non-zero initialized data
;	===============================================

JUMPTOMAIN:
		MOV	SP,#?STACK-1
	;	LJMP	?C_START

		LJMP	MAIN
;	===============================================
		END
       