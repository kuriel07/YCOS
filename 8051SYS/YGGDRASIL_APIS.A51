;EXTRN	CODE 	(TMR0_ISR)
EXTRN	CODE 	(ISO7816_Main)
IE_TMR0	BIT	0A8H.1
TCON_TF0	BIT	088H.5
DPL0	DATA	082H
DPH0	DATA	083H

;  Stack Space for reentrant functions in the LARGE model.	
XBPSTACK	EQU	1	; set to 1 if large reentrant is used.
XBPSTACKTOP	EQU	07FFH+1; set top of stack to highest location+1.



?STACK		SEGMENT   IDATA
	;RSEG	?STACK
	;DS	40H
;=======================================================
;void * m_alloc(uint16 size), memory allocator API
;=======================================================
EXTRN	CODE 	(_?m_alloc)
		CSEG	AT	0x03B
SERVICE0:
		LJMP	_?m_alloc
		RET
;=======================================================
;void m_free(void * ptr), memory allocator API
;=======================================================
EXTRN	CODE 	(_?m_free)
		CSEG	AT	0x3F
SERVICE1:
		LJMP	_?m_free
		RET

;=======================================================

;=======================================================
EXTRN	CODE 	(_?_select)
		CSEG	AT	0x47
SERVICE3:
		LJMP	_?_select
		RET
;=======================================================

;=======================================================
EXTRN	CODE 	(_?_readbin)
		CSEG	AT	0x4B
SERVICE4:
		LJMP	_?_readbin
		RET
;=======================================================

;=======================================================
EXTRN	CODE 	(_?_writebin)
		CSEG	AT	0x4F
SERVICE5:
		LJMP	_?_writebin
		RET
;=======================================================

;=======================================================
EXTRN	CODE 	(_?_readrec)
		CSEG	AT	0x53
SERVICE6:
		LJMP	_?_readrec
		RET
;=======================================================

;=======================================================
EXTRN	CODE 	(_?_readrec_next)
		CSEG	AT	0x57
SERVICE7:
		LJMP	_?_readrec_next
		RET
;=======================================================

;=======================================================
EXTRN	CODE 	(_?_readrec_prev)
		CSEG	AT	0x5B
SERVICE8:
		LJMP	_?_readrec_prev
		RET
;=======================================================

;=======================================================
EXTRN	CODE 	(_?_writerec)
		CSEG	AT	0x5F
SERVICE9:
		LJMP	_?_writerec
		RET
;=======================================================

;=======================================================
EXTRN	CODE 	(_?_writerec_next)
		CSEG	AT	0x63
SERVICE10:
		LJMP	_?_writerec_next
		RET
;=======================================================

;=======================================================
EXTRN	CODE 	(_?_writerec_prev)
		CSEG	AT	0x67
SERVICE11:
		LJMP	_?_writerec_prev
		RET
;uint16 _seek(uchar type, uchar * buffer, uchar * pattern, uchar size);
;=======================================================

;=======================================================
EXTRN	CODE 	(_?_seek)
		CSEG	AT	0x6B
SERVICE12:
		LJMP	_?_seek
		RET
;uint16 _increase(uint32 ival, uchar * res, uchar size);
;=======================================================

;=======================================================
EXTRN	CODE 	(_?_increase)
		CSEG	AT	0x6F
SERVICE13:
		LJMP	_?_increase
		RET
;uint16 _createfilebin(uint16 fid, uchar ACC_READ, uchar ACC_WRT, uchar ACC_INC, uchar ACC_INV, uchar ACC_RHB, uchar *buffer, uint16 size);
;=======================================================

;=======================================================
EXTRN	CODE 	(_?_createfilebin)
		CSEG	AT	0x73
SERVICE14:
		LJMP	_?_createfilebin
		RET
;uint16 _createfilerec(uint16 fid, uchar ACC_READ, uchar ACC_WRT, uchar ACC_INC, uchar ACC_INV, uchar ACC_RHB, uchar total_rec, uchar rec_size);
;=======================================================

;=======================================================
EXTRN	CODE 	(_?_createfilerec)
		CSEG	AT	0x77
SERVICE15:
		LJMP	_?_createfilerec
		RET
;uint16 _createfilecyclic(uint16 fid, uchar ACC_READ, uchar ACC_WRT, uchar ACC_INC, uchar ACC_INV, uchar ACC_RHB, uchar total_rec, uchar rec_size);
;=======================================================

;=======================================================
EXTRN	CODE 	(_?_createfilecyclic)
		CSEG	AT	0x7B
SERVICE16:
		LJMP	_?_createfilecyclic
		RET
;uint16 _createdirectory(uint16 fid, uchar *dirname, uchar len_name);
;=======================================================

;=======================================================
EXTRN	CODE 	(_?_createdirectory)
		CSEG	AT	0x7F
SERVICE17:
		LJMP	_?_createdirectory
		RET		
;uint16 _remove(uint16 file_id);
;=======================================================

;=======================================================
EXTRN	CODE 	(_?_remove)
		CSEG	AT	0x83
SERVICE18:
		LJMP	_?_remove
		RET
;uint16 _invalidate();
;=======================================================

;=======================================================
EXTRN	CODE 	(_?_invalidate)
		CSEG	AT	0x87
SERVICE19:
		LJMP	_?_invalidate
		RET
;uint16 _rehabilitate(); 
;=======================================================

;=======================================================
EXTRN	CODE 	(_?_rehabilitate)
		CSEG	AT	0x8B
SERVICE20:
		LJMP	_?_rehabilitate
		RET
;uint16 _check_access(); 
;=======================================================

;=======================================================
EXTRN	CODE 	(_?_check_access)
		CSEG	AT	0x8F
SERVICE21:
		LJMP	_?_check_access
		RET
;=======================================================
;uchar chv_verify(uchar chv_no, uchar *pin);
;=======================================================
EXTRN	CODE 	(_?chv_verify)
		CSEG	AT	0x93
SERVICE22:
		LJMP	_?chv_verify
		RET
;=======================================================
;uchar chv_enable(uchar chv_no, uchar *pin);
;=======================================================
EXTRN	CODE 	(_?chv_enable)
		CSEG	AT	0x97
SERVICE23:
		LJMP	_?chv_enable
		RET
;=======================================================
;uchar chv_disable(uchar chv_no, uchar *pin);
;=======================================================
EXTRN	CODE 	(_?chv_disable)
		CSEG	AT	0x9B
SERVICE24:
		LJMP	_?chv_disable
		RET
;=======================================================
;uchar chv_get_status(uchar chv_no);
;=======================================================
EXTRN	CODE 	(_?chv_get_status)
		CSEG	AT	0x9F
SERVICE25:
		LJMP	_?chv_get_status
		RET

;=======================================================
;uchar chv_unblock(uchar chv_no, uchar *pin, uchar *puk);
;=======================================================
EXTRN	CODE 	(_?chv_unblock)
		CSEG	AT	0xA3
SERVICE26:
		LJMP	_?chv_unblock
		RET
;=======================================================
;uchar chv_change(uchar chv_no, uchar *old, uchar *new)
;=======================================================
EXTRN	CODE 	(_?chv_change)
		CSEG	AT	0xA7
SERVICE27:
		LJMP	_?chv_change
		RET
;=======================================================
;BYTE DES_Decrypt(BYTE mode, BYTE * key, BYTE * inbuf, BYTE * outbuf)
;=======================================================
EXTRN 	CODE 	(_?DES_Decrypt)
		CSEG 	AT	0xAB
SERVICE28:
		LJMP	_?DES_Decrypt
		RET
;=======================================================
;BYTE DES_Encrypt(BYTE mode, BYTE * key, BYTE * inbuf, BYTE * outbuf)
;=======================================================
EXTRN 	CODE 	(_?DES_Encrypt)
		CSEG 	AT	0xAF
SERVICE29:
		LJMP	_?DES_Encrypt
		RET
;=======================================================
;Random_Number_Generator(BYTE *RNGBuf, BYTE length)
;=======================================================
EXTRN 	CODE 	(_?Random_Number_Generator)
		CSEG 	AT	0xB3
SERVICE30:
		LJMP	_?Random_Number_Generator
		RET
;=======================================================
;uint16	CalCRC(BYTE * SrcAddr, HALFW length)
;=======================================================
EXTRN 	CODE 	(_?CalCRC)
		CSEG 	AT	0xB7
SERVICE31:
		LJMP	_?CalCRC
		RET	
;=======================================================
;uint16	Auth_COMP128(uchar * inbuf, uchar * outbuf)
;=======================================================
EXTRN 	CODE 	(_?Auth_COMP128)
		CSEG 	AT	0xBB
SERVICE32:
		LJMP	_?Auth_COMP128
		RET	
;=======================================================
;void TDES_Decrypt(uchar * key1, uchar * key2, uchar * inbuf, uchar * outbuf)
;=======================================================
EXTRN 	CODE 	(_?TDES_Decrypt)
		CSEG 	AT	0xBF
SERVICE33:
		LJMP	_?TDES_Decrypt
		RET	
;=======================================================
;void TDES_Encrypt(uchar * key1, uchar * key2, uchar * inbuf, uchar * outbuf)
;=======================================================
EXTRN 	CODE 	(_?TDES_Encrypt)
		CSEG 	AT	0xC3
SERVICE34:
		LJMP	_?TDES_Encrypt
		RET		
;=======================================================
;void MAC_Algorithm3(uchar len, uchar * key, uchar * message, uchar * output)
;=======================================================
EXTRN 	CODE 	(_?MAC_Algorithm3)
		CSEG	AT	0xC7
SERVICE35:
		LJMP	_?MAC_Algorithm3
		RET	
			
;=======================================================
;void * file_get_current_header(void)
;=======================================================
EXTRN 	CODE 	(_?file_get_current_header)
		CSEG 	AT	0xE3
SERVICE42:
		LJMP	_?file_get_current_header
		RET	
;=======================================================
;chv_file * chv_get_config(uchar chv_no);
;=======================================================
EXTRN	CODE 	(_?chv_get_config)
		CSEG	AT	0xE7
SERVICE43:
		LJMP	_?chv_get_config
		RET
;=======================================================

;=======================================================
EXTRN	CODE 	(_?Set_Response)
		CSEG	AT	0xF3
SERVICE46:
		LJMP	_?Set_Response
		RET		
;=======================================================

;=======================================================
EXTRN	CODE 	(_?Get_Data)
		CSEG	AT	0xF7
SERVICE47:
		LJMP	_?Get_Data
		RET

;=======================================================
;uint16 Write_Temp_Space(uint16 pos, ucharx * bytes, uint16 length), write program to temporary space
;=======================================================
EXTRN	CODE 	(_?Write_Temp_Space)
		CSEG	AT	0xFB
SERVICE48:
		LJMP	_?Write_Temp_Space
		RET
;=======================================================
;uint16 Load_Temp_Space() , load program from temp space to program space
;=======================================================
EXTRN	CODE 	(_?Load_Temp_Space)
		CSEG	AT	0xFF
SERVICE49:
		LJMP	_?Load_Temp_Space
		RET
;=======================================================
;uint16 fs_format(uint32 size)
;=======================================================
EXTRN 	CODE 	(_?fs_format)
		CSEG 	AT	0x103
SERVICE50:
		LJMP	_?fs_format
		RET	
;=======================================================
;uint32 fs_freespace(void)
;=======================================================
EXTRN 	CODE 	(_?fs_freespace)
		CSEG 	AT	0x107
SERVICE51:
		LJMP	_?fs_freespace
		RET		
;=======================================================
;void lsprintf(uchar * str, uchar * fmt, ...)
;=======================================================
EXTRN 	CODE 	(_?ls_printf)
		CSEG 	AT	0x10B
SERVICE52:
		LJMP	_?ls_printf
		RET	
		NOP
		NOP
		NOP
		NOP
				
	//PUBLIC	TMR0_ISR
;EXTRN	CODE 	(TMR0_ISR)
	;CSEG	AT	0x200 

;==========================================================================
; User App Timeout mechanism
;==========================================================================
INCREMENT_DX:
	XCH      A,B
	MOV      R0,A
	INC      DPTR
	MOVX     A,@DPTR
	ADD      A,R0
	MOVX     @DPTR,A
	XCH      A,B
	MOV      R0,A
	MOV      A,DPL0
	DEC      DPL0
	JNZ      INC_DX_0
	DEC      DPH0
INC_DX_0:
	MOVX     A,@DPTR
	ADDC     A,R0
	MOVX     @DPTR,A
	RET      
	NAME	TMR0_ISR
	PUBLIC 	TMR0_ISR 
EXTRN 	XDATA	(_tick_counter)
EXTRN 	XDATA	(_iso7816_state)
;?C_C51STARTUP	SEGMENT   CODE
TMR0_ISR:
	PUSH 	ACC
	PUSH 	B
	PUSH 	DPH
	PUSH 	DPL
	PUSH 	PSW
	MOV  	PSW,#00H
	USING	0
	PUSH 	AR0
	PUSH 	AR6
	PUSH 	AR7
			; SOURCE LINE # 16
; {
; 	IE_TMR0 = 0;
			; SOURCE LINE # 18
	CLR  	IE_TMR0
; 	_tick_counter++;
			; SOURCE LINE # 19
	MOV  	DPTR,#_tick_counter
	CLR  	A
	MOV  	B,#01H
	LCALL	INCREMENT_DX
	;LCALL	?C?IILDX
; 	if(_tick_counter < 300) { 
			; SOURCE LINE # 20
	MOV  	DPTR,#_tick_counter
	MOVX 	A,@DPTR
	MOV  	R6,A
	INC  	DPTR
	MOVX 	A,@DPTR
	MOV  	R7,A
	CLR  	C
	MOV  	A,R7
	;SUBB 	A,#02CH	  ;300d = 12Ch		--> lower byte
	;SUBB 	A,#058H	  ;600d = 258h		--> lower byte
	SUBB 	A,#0B0H	  ;1200d = 4B0h		--> lower byte
	MOV  	A,R6
	;SUBB 	A,#01H	  ;--> upper byte  
	;SUBB 	A,#02H	  ;--> upper byte
	SUBB 	A,#04H	  ;--> upper byte
	JNC  	EXIT_TMR0_ISR1
; 		TH0 = 0xC3;			//50000 clock, reload value
			; SOURCE LINE # 21
	MOV  	TH0,#0C3H
; 		TL0 = 0x50;		
			; SOURCE LINE # 22
	MOV  	TL0,#050H
; 		TCON_TF0 = 0;		//clear flag
			; SOURCE LINE # 23
	CLR  	TCON_TF0
; 		IE_TMR0 = 1;
			; SOURCE LINE # 24
	SETB 	IE_TMR0
; 	} else {
			; SOURCE LINE # 25
	SJMP 	EXIT_TMR0_ISR3
EXIT_TMR0_ISR1:
; 	 	_iso7816_state =  ISO7816_STOP;			//cancel pending operation
			; SOURCE LINE # 26
	MOV  	DPTR,#_iso7816_state
	MOV  	A,#0DH
	MOVX 	@DPTR,A
; 		//#pragma	asm
; 		TH0 = 0x00;			//50000 clock, reload value
			; SOURCE LINE # 28
	MOV  	TH0,#00H
; 		TL0 = 0x00;		
			; SOURCE LINE # 29
	MOV  	TL0,#00H
; 		TCON_TF0 = 0;		//clear flag
			; SOURCE LINE # 30
	CLR  	TCON_TF0
; 		IE_TMR0 = 0;
			; SOURCE LINE # 31
	CLR  	IE_TMR0
; 		//SP = 0x23;
; 		#pragma asm
	POP AR7;
	POP AR6;
	POP AR0;
	POP PSW	;
	POP DPL0;
	POP DPH0;
	POP B;
	POP ACC;
	POP ACC; 
	POP ACC;
	  ;MOV SP, #023h
	  ;Timeout sequence, reset all stack to default, return to ISO7816_Main
	EXTRN DATA (?C_XBP)
	;EXTRN DATA (?STACK)
	MOV	SP,#?STACK-1
	MOV	?C_XBP,#HIGH XBPSTACKTOP
	MOV	?C_XBP+1,#LOW XBPSTACKTOP
	MOV DPTR, #ISO7816_Main;
	PUSH DPL;
	PUSH DPH;
	RETI
; 		//MOV SP, #023h;
	  
; 		//MOV PSW, #0;
	  
; 		//MOV A, #0;
	  
; 		//MOV B, #0;
	  
; 		//MOV	SP,#?STACK-1
	  
; 		//AJMP	ISO7816_Main;
	  
; 		//LJMP	MAIN
	  
; 		#pragma endasm
; 		//*((uchar *)(SP - 8)) = 0x20;
; 		//*((uchar *)(SP - 8 + 1)) = 0x00;
; 		//PC = 0x2000;
; 	}
			; SOURCE LINE # 59
EXIT_TMR0_ISR2:
; 	return;
; }
			; SOURCE LINE # 61
EXIT_TMR0_ISR3:
	POP  	AR7
	POP  	AR6
	POP  	AR0
	POP  	PSW
	POP  	DPL
	POP  	DPH
	POP  	B
	POP  	ACC
	RETI 	
; END OF TMR0_ISR

;Initialize_User_App
		CSEG	AT	0x8000 
		RET

		END                    