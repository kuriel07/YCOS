
#include	"_17BD_UserCode.h"  
#include 	"defs.h"
#include	"..\ISO7816\ISO7816.h"
#include	"..\NORFlash\NORFlash.h"
#include 	"..\yggdrasil\yggdrasil.h"
#include	<intrins.h>

//BYTE	ISO7816_Time;

uint16 _tick_counter = 0;
//extern uint16 C_XBP;
//====================================================
//	Timer 0 interrutp service routine
//extern void ISO7816_main(void);
//extern void _C_START(void);
/*void TMR0_ISR(void) interrupt 1
{
	IE_TMR0 = 0;
	_tick_counter++;
	if(_tick_counter < 300) { 
		TH0 = 0xC3;			//50000 clock, reload value
		TL0 = 0x50;		
		TCON_TF0 = 0;		//clear flag
		IE_TMR0 = 1;
	} else {
	 	_iso7816_state =  ISO7816_STOP;			//cancel pending operation
		//#pragma	asm
		TH0 = 0x00;			//50000 clock, reload value
		TL0 = 0x00;		
		TCON_TF0 = 0;		//clear flag
		IE_TMR0 = 0;
		//SP = 0x23;
		#pragma asm
		POP 7;
		POP 6;
		POP ACC;
		POP PSW	;
		POP DPL0;
		POP DPH0;
		POP B;
		POP ACC;
		POP ACC; 	//clear return address
		POP ACC;
		MOV DPTR, #ISO7816_Main;
		PUSH DPL;
		PUSH DPH;
		RETI
		//MOV SP, #023h;
		//MOV PSW, #0;
		//MOV A, #0;
		//MOV B, #0;
		//MOV	SP,#?STACK-1
		//AJMP	ISO7816_Main;
		//LJMP	MAIN
		#pragma endasm
		//*((uchar *)(SP - 8)) = 0x20;
		//*((uchar *)(SP - 8 + 1)) = 0x00;
		//PC = 0x2000;
	}
	return;
} */

//====================================================
//	Main process
void	main(void)
{	
	HW_Init();						//	H/W initial
	//ISO7816_Time = 0;				//	0: 1st time,ATR
	ISO7816_init();
	//while(1)
	//{  
		//if(ISO7816_Time > 0)
		//{
		//	Sleep_Mode();			//	Power down mode after thread has been executed			
		//}
	ISO7816_main();
	//}
}

//====================================================
//	ISO/IEC 7816 commands handling
void	HW_Init(void)
{
	IE_EA = 1;						//	Interrupt enabed by corresponding control bit
	IP = 0x00;						//	No priviledge
	UCR2 = 0x03;					//	Retry 3 times
	RNGCTL = 0x01;					//	Close RNG
	CLKCON = 0x01;					//	Open DES clock 
}
/*
//====================================================
//	Copy memory
void	mem_cpy(BYTE * dst,BYTE * src,USHORT len)
{
	USHORT	i;

	for(i = 0;i < len;i ++)
		dst[i] = *src++;
}

//====================================================
//	Compare memory
BYTE	mem_cmp(BYTE * dst,BYTE * src,USHORT len)
{
	USHORT	i;

	for(i = 0;i < len;i ++)
	{
		if(dst[i] != *src++)
			return 0;
	}
	return 1;
} 
*/

#define	MEMXDATA	0x01
#define	MEMCODE		0xFF
//====================================================
//	Copy memory
//	DPTR0	=	dst
//	DPTR1	=	src
//	R0		=	backupDPS
//	R1 R2	=	length	
#if 0
void	mem_cpy(BYTE * dst, BYTE * src, USHORT len)
{
	if(*(BYTEX *)(&dst) == MEMXDATA)
	{
		switch(*(BYTEX *)(&src))
		{
			case	MEMXDATA:								//	XDATA to XDATA
			{	
				#pragma	asm
						MOV		R0,		DPS	
						MOV		DPS,	0	
						MOV		DPTR,	#?_MEM_CPY?BYTE+6	//	Get length
						MOVX	A,		@DPTR
						MOV		R1,		A
						INC		DPTR
						MOVX	A,		@DPTR
						MOV		R2,		A
						JZ		$+1+1+1
						INC		R1
						MOV		DPTR,	#?_MEM_CPY?BYTE+1	//	DPTR 0 for dst
						MOVX	A,		@DPTR
						MOV		R3,		A
						INC		DPTR
						MOVX	A,		@DPTR
						MOV		DPL0,	A
						MOV		DPH0,	R3
						INC		DPS
						MOV		DPTR,	#?_MEM_CPY?BYTE+4	//	DPTR 1 for src
						MOVX	A,		@DPTR
						MOV		R3,		A
						INC		DPTR
						MOVX	A,		@DPTR
						MOV		DPL1,	A
						MOV		DPH1,	R3
						MOV		DPS,	#01H
					LOOP_CPY_X:
						MOVX	A,		@DPTR				//	DPTR 0 for dst
						INC		DPTR
						INC		DPS
						MOVX	@DPTR,	A					//	DPTR 1 for src
						INC		DPTR
						INC		DPS
						DJNZ	R2,		LOOP_CPY_X
						DJNZ	R1,		LOOP_CPY_X
						MOV		DPS,	R0			
				#pragma	endasm	  
				break;
			}	
			case	MEMCODE:								//	CODE to XDATA
			{
			 	#pragma	asm
						MOV		R0,		DPS	
						MOV		DPS,	0	
						MOV		DPTR,	#?_MEM_CPY?BYTE+6	//	Get length
						MOVX	A,		@DPTR
						MOV		R1,		A
						INC		DPTR
						MOVX	A,		@DPTR
						MOV		R2,		A
						JZ		$+1+1+1
						INC		R1
						MOV		DPTR,	#?_MEM_CPY?BYTE+1	//	DPTR 0 for dst
						MOVX	A,		@DPTR
						MOV		R3,		A
						INC		DPTR
						MOVX	A,		@DPTR
						MOV		DPL0,	A
						MOV		DPH0,	R3
						INC		DPS
						MOV		DPTR,	#?_MEM_CPY?BYTE+4	//	DPTR 1 for src
						MOVX	A,		@DPTR
						MOV		R3,		A
						INC		DPTR
						MOVX	A,		@DPTR
						MOV		DPL1,	A
						MOV		DPH1,	R3
						MOV		DPS,	#01H
					LOOP_CPY_C:
						CLR		A
						MOVC	A,		@A+DPTR				//	DPTR 0 for dst
						INC		DPTR
						INC		DPS
						MOVX	@DPTR,	A					//	DPTR 1 for src
						INC		DPTR
						INC		DPS
						DJNZ	R2,		LOOP_CPY_C
						DJNZ	R1,		LOOP_CPY_C	
						MOV		DPS,	R0		
				#pragma	endasm	 
				break;
			}	
		}
	}
	else
		while(1);											//	ERROR
}

//====================================================
//	Compare memory
//	DPTR0	=	dst
//	DPTR1	=	src
//	R0		=	backupDPS
//	R1 R2	=	length
//	R3		=	compare data
BYTE	mem_cmp(BYTE * dst,BYTE * src,USHORT len)
{
	if(*(BYTEX *)(&dst) == MEMXDATA)
	{
		switch(*(BYTEX *)(&src))
		{
			case	MEMXDATA:								//	XDATA vs XDATA		
			{	
				#pragma	asm	
						MOV		R0,		DPS	
						MOV		DPS,	0	
						MOV		DPTR,	#?_MEM_CMP?BYTE+6	//	Get length
						MOVX	A,		@DPTR
						MOV		R1,		A
						INC		DPTR
						MOVX	A,		@DPTR
						MOV		R2,		A
						JZ		$+1+1+1
						INC		R1
						MOV		DPTR,	#?_MEM_CMP?BYTE+1	//	DPTR 0 for dst
						MOVX	A,		@DPTR
						MOV		R3,		A
						INC		DPTR
						MOVX	A,		@DPTR
						MOV		DPL0,	A
						MOV		DPH0,	R3
						INC		DPS
						MOV		DPTR,	#?_MEM_CMP?BYTE+4	//	DPTR 1 for src
						MOVX	A,		@DPTR
						MOV		R3,		A
						INC		DPTR
						MOVX	A,		@DPTR
						MOV		DPL1,	A
						MOV		DPH1,	R3
						INC		DPS
					LOOP_CMP_XX:
						MOVX	A,		@DPTR				//	DPTR 0
						MOV		R3,		A				 
						INC		DPTR
						INC		DPS
						MOVX	A,		@DPTR				//	DPTR 1
						INC		DPTR
						INC		DPS
						CLR		C
						SUBB	A,		R3
						JZ		CONTINUE_XX
						MOV		R7,		0
						MOV		DPS,	R0	
						RET
					CONTINUE_XX:
						DJNZ	R2,		LOOP_CMP_XX
						DJNZ	R1,		LOOP_CMP_XX
						MOV		DPS,	R0			
				#pragma	endasm					
				break;
			}
			case	MEMCODE:								//	XDATA vs CODE		
			{	
				#pragma	asm	
						MOV		R0,		DPS	
						MOV		DPS,	0	
						MOV		DPTR,	#?_MEM_CMP?BYTE+6	//	Get length
						MOVX	A,		@DPTR
						MOV		R1,		A
						INC		DPTR
						MOVX	A,		@DPTR
						MOV		R2,		A
						JZ		$+1+1+1
						INC		R1
						MOV		DPTR,	#?_MEM_CMP?BYTE+1	//	DPTR 0 for dst
						MOVX	A,		@DPTR
						MOV		R3,		A
						INC		DPTR
						MOVX	A,		@DPTR
						MOV		DPL0,	A
						MOV		DPH0,	R3
						INC		DPS
						MOV		DPTR,	#?_MEM_CMP?BYTE+4	//	DPTR 1 for src
						MOVX	A,		@DPTR
						MOV		R3,		A
						INC		DPTR
						MOVX	A,		@DPTR
						MOV		DPL1,	A
						MOV		DPH1,	R3
						INC		DPS
					LOOP_CMP_CX:
						MOVX	A,		@DPTR				//	DPTR 0
						MOV		R3,		A				 
						INC		DPTR
						INC		DPS
						CLR		A
						MOVC	A,		@A+DPTR				//	DPTR 1
						INC		DPTR
						INC		DPS
						CLR		C
						SUBB	A,		R3
						JZ		CONTINUE_CX
						MOV		R7,		0
						MOV		DPS,	R0	
						RET
					CONTINUE_CX:
						DJNZ	R2,		LOOP_CMP_CX
						DJNZ	R1,		LOOP_CMP_CX
						MOV		DPS,	R0			
				#pragma	endasm					
				break;
			}
		}
	}
	else if(*(BYTEX *)(&dst) == MEMCODE)
	{
		switch(*(BYTEX *)(&src))
		{
			case	MEMXDATA:								//	CODE vs XDATA				
			{	
				#pragma	asm	
						MOV		R0,		DPS	
						MOV		DPS,	0	
						MOV		DPTR,	#?_MEM_CMP?BYTE+6	//	Get length
						MOVX	A,		@DPTR
						MOV		R1,		A
						INC		DPTR
						MOVX	A,		@DPTR
						MOV		R2,		A
						JZ		$+1+1+1
						INC		R1
						MOV		DPTR,	#?_MEM_CMP?BYTE+1	//	DPTR 0 for dst
						MOVX	A,		@DPTR
						MOV		R3,		A
						INC		DPTR
						MOVX	A,		@DPTR
						MOV		DPL0,	A
						MOV		DPH0,	R3
						INC		DPS
						MOV		DPTR,	#?_MEM_CMP?BYTE+4	//	DPTR 1 for src
						MOVX	A,		@DPTR
						MOV		R3,		A
						INC		DPTR
						MOVX	A,		@DPTR
						MOV		DPL1,	A
						MOV		DPH1,	R3
						INC		DPS
					LOOP_CMP_XC:
						CLR		A
						MOVC	A,		@A+DPTR				//	DPTR 0
						MOV		R3,		A				 
						INC		DPTR
						INC		DPS
						MOVX	A,		@DPTR				//	DPTR 1
						INC		DPTR
						INC		DPS
						CLR		C
						SUBB	A,		R3
						JZ		CONTINUE_XC
						MOV		R7,		0
						MOV		DPS,	R0	
						RET
					CONTINUE_XC:
						DJNZ	R2,		LOOP_CMP_XC
						DJNZ	R1,		LOOP_CMP_XC
						MOV		DPS,	R0			
				#pragma	endasm					
				break;
			}
			case	MEMCODE:								//	CODE vs	CODE		
			{	
				#pragma	asm	
						MOV		R0,		DPS	
						MOV		DPS,	0	
						MOV		DPTR,	#?_MEM_CMP?BYTE+6	//	Get length
						MOVX	A,		@DPTR
						MOV		R1,		A
						INC		DPTR
						MOVX	A,		@DPTR
						MOV		R2,		A
						JZ		$+1+1+1
						INC		R1
						MOV		DPTR,	#?_MEM_CMP?BYTE+1	//	DPTR 0 for dst
						MOVX	A,		@DPTR
						MOV		R3,		A
						INC		DPTR
						MOVX	A,		@DPTR
						MOV		DPL0,	A
						MOV		DPH0,	R3
						INC		DPS
						MOV		DPTR,	#?_MEM_CMP?BYTE+4	//	DPTR 1 for src
						MOVX	A,		@DPTR
						MOV		R3,		A
						INC		DPTR
						MOVX	A,		@DPTR
						MOV		DPL1,	A
						MOV		DPH1,	R3
						INC		DPS
					LOOP_CMP_CC:
						CLR		A
						MOVC	A,		@A+DPTR				//	DPTR 0
						MOV		R3,		A				 
						INC		DPTR
						INC		DPS
						CLR		A
						MOVC	A,		@A+DPTR				//	DPTR 1
						INC		DPTR
						INC		DPS
						CLR		C
						SUBB	A,		R3
						JZ		CONTINUE_CC
						MOV		R7,		0
						MOV		DPS,	R0	
						RET
					CONTINUE_CC:
						DJNZ	R2,		LOOP_CMP_CC
						DJNZ	R1,		LOOP_CMP_CC
						MOV		DPS,	R0			
				#pragma	endasm					
				break;
			}
		}
	}
	else
		while(1);

	return 1;
}
#endif
