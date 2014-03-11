
#include	"_17BD_UserCode.h"
#include	"..\ISO7816\ISO7816.h"
#include	"..\NORFlash\NORFlash.h"
#include 	"defs.h"
#include 	<string.h>
#include	<intrins.h>

const BYTEC	VectorTable[0x60] = {0x02,0x00,0x3B,0x32,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x32,0x00,0x00,0x00,0x00,
						   0x00,0x00,0x00,0x32,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x32,0x00,0x00,0x00,0x00,
						   0x00,0x00,0x00,0x32,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x32,0x00,0x00,0x00,0x00,
						   0x00,0x00,0x00,0x32,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x78,0xFF,0xE4,0xF6,0xD8,
						   0xFD,0x90,0x00,0x00,0x7F,0x1A,0x7E,0x04,0xEF,0x70,0x03,0xEE,0x60,0x08,0x0E,0xE4,
						   0xF0,0xA3,0xDF,0xFC,0xDE,0xFA,0x75,0x90,0x03,0x75,0xC3,0x01,0x02,0x80,0x00,0xFF};
//====================================================
//	Initial I/O
void	IoInit(BYTE FIDI) _REENTRANT_ 
{
	switch(FIDI)
	{										
		case(0x11): UBRC = FIDI;	break;		//	9600 Bps
		case(0x12): UBRC = FIDI;	break;		//
		case(0x13): UBRC = FIDI;	break;		//
		case(0x18): UBRC = FIDI;	break;
		case(0x91): UBRC = FIDI;	break;
		case(0x92): UBRC = FIDI;	break;
		case(0x94): UBRC = FIDI;	break;		//	57600 Bps
		case(0x95): UBRC = FIDI;	break;		//	115200 Bps
		case(0x96): UBRC = FIDI;	break;		//	223K Bps
	}
}

//====================================================
//	PPS acknowledge
void    PPS_ACK(void)
{
	Tx_n_Bytes(4,iso7816_buffer);
}

//====================================================
//	PPS
void	PPS(void)
{	
	HALFWX P12;

	P12=((HALFW)iso7816_buffer[2] << 8) + (HALFW)iso7816_buffer[3];
	switch(P12)
	{	
		case 0x11FE:	{ PPS_ACK(); IoInit(0x11); break; }
		case 0x947B:	{ PPS_ACK(); IoInit(0x94); break; }
		case 0x957A:	{ PPS_ACK(); IoInit(0x95); break; }
		case 0x9679:	{ PPS_ACK(); IoInit(0x96); break; }
		default: return;
	}
} 

//====================================================
//	PPS						  
void	Set_PPS(BYTEX * buffer)	_REENTRANT_
{	
	HALFWX P12;
	P12=((HALFW)buffer[2] << 8) + (HALFW)buffer[3];
	//if(P12 == 0x11FE)	{ PPS_ACK(); IoInit(0x11); return; }
	//if(P12 == 0x947B)	{ PPS_ACK(); IoInit(0x94); return; }
	//if(P12 == 0x957A)	{ PPS_ACK(); IoInit(0x95); return; }
	//if(P12 == 0x9679)	{ PPS_ACK(); IoInit(0x96); return; }
	switch(buffer[2]) {
		case(0x11): PPS_ACK(); IoInit(0x11); return;		//	9600 Bps
		case(0x12): PPS_ACK(); IoInit(0x12); return;		//
		case(0x13): PPS_ACK(); IoInit(0x13); return;		//
		case(0x18): PPS_ACK(); IoInit(0x18); return;
		case(0x91): PPS_ACK(); IoInit(0x91); return;
		case(0x92): PPS_ACK(); IoInit(0x92); return;
		case(0x94): PPS_ACK(); IoInit(0x94); return;		//	57600 Bps
		case(0x95): PPS_ACK(); IoInit(0x95); return;		//	115200 Bps
		case(0x96): PPS_ACK(); IoInit(0x96); return;		//	223K Bps	
	}
	return;
}

//====================================================
//	Receive one byte by UART
BYTE	receive_byte(void)
{
	UCR = 0x00;						//	UART mode,H/W check parity err,Rx
	while(!(USR&0x02))
	{;}								//	Wait for receicing
	return	UBUF;					//	Return the received byte
}

//====================================================
//	Receive n bytes by UART
void    Rx_n_Bytes(short n) _REENTRANT_ 
{	
	short	i = 0;

	for(i = 0;i < n;i ++)
		IOBuf[i] = receive_byte();
}

//====================================================
//	Transmit one byte by UART
void	send_byte(char c) _REENTRANT_ 
{
	UCR = 0x20;						//	UART mode,H/W check parity err,Tx
	UCR2 = 0x0f;					//	1 ETU guard time, resend unlimited
	UBUF = c;						//	Load the transmitted byte into buffer
	while(!USR_TBE)
	{;}	
	UCR = 0x00;						//	Keep Rx mode
}

//====================================================
//	Transmit n bytes by UART
void 	Tx_n_Bytes(USHORT n, BYTE * databuf) _REENTRANT_ 
{
	USHORT	i;

	for(i = 0;i < n;i ++)
		send_byte(databuf[i]);	
}

//====================================================
//	Transmit status by UART
void 	Tx_Status(USHORT sw) _REENTRANT_ 
{
	//USHORT	i;

	//for(i = 0;i < n;i ++)
	send_byte((BYTE)(sw>>8));
	send_byte((BYTE)(sw&0xff));	
}

//====================================================
//	Chip enters idle mode, be awaken up by UART interrupt
void	Sleep_Mode(void)
{
	BYTE	backupVDCON = VDCON;
	BYTE	backupFDCON = FDCON;

	VDCON = 0x00;					//	VD closed
	FDCON = 0X00;					//	FD closed

	SLEEP = 0x01;					//	Idle mode

	_nop_();
	_nop_();
	_nop_();


	VDCON = backupVDCON;			
	FDCON = backupFDCON;						
}

void ISO_DelayETU(void) {
	ISOTDAT0 = ISOTRLD0 = 0xFF;
	ISOTDAT1 = ISOTRLD1 = 0x7F;				//
	ISOTMSK |= Bit0_En;						//	ISO timing to send NULL byte interrupt disabled
	ISOTCON &= (Bit3_Dis&Bit2_Dis);					//	Counting mode  use mode 1 , clear Tflag
	ISOTCON |= (Bit1_En);					//	Counting mode			use mode 1
	ISOTCON |= Bit0_En;						//	Timer start
	while((ISOTCON & (1<<3)) == 0);
}

//====================================================
//	ISO/IEC 7816 interface ISO Tx NULL byte automatically
void	ISO_AutoTxNULL(BYTE mode, HALFW ETUcount) _REENTRANT_ 
{
	if(mode == 1)
	{
		ISOTDAT0 = ISOTRLD0 = ETUcount;
		ISOTDAT1 = ISOTRLD1 = (ETUcount>>8);	//	Every ETUcount to send 1 NULL byte
		ISOTMSK |= Bit0_En;						//	ISO timing to send NULL byte interrupt disabled
		ISOTCON &= (Bit1_Dis&Bit2_Dis);			//	Counting mode
		ISOTCON |= Bit0_En;						//	Timer start
	}
	else
	{
		ISOTCON &= Bit0_Dis;
	}
}

//====================================================
//	Change bank
void	SetBase(BYTE BankNum) _REENTRANT_ 
{
	if(BankNum < 8)
	{
		MMU_SEL = 0x01;
		rP3	= BankNum; 				//	Set P3
	}
	else
	{
		MMU_SEL = 0x00;
		//SWptr = P1P2ERR;			//	P1,P2 error
	}	
}

//====================================================
//	Compute CRC by hardware
uint16	CalCRC(BYTE * SrcAddr, HALFW length) _REENTRANT_
{
	uint16 crcval;
	while(length != 0)
	{
		CRCDAT = *(BYTEX *)SrcAddr;
		SrcAddr ++;
		length --;
	}
	crcval = (BYTE)CRCDAT;
	crcval <<= 8;
	crcval |= (BYTE)CRCDAT;
	//DstAddr[0] = CRCDAT;						//	Read CRC
	//DstAddr[1] = CRCDAT;
	return crcval;
}

//====================================================
//	Erase pages in flash memory.
BYTE	Erase_Pages(HALFW FlashAddr, HALFW PageNum) _REENTRANT_
{
	HALFW i;

	for (i = 0;i < PageNum;i++)				//	Erase pages	  
	{
		Erase_Page((BYTEX *)FlashAddr);

		FlashAddr += PageSize;
		/*if(SWptr != 0)
		{
			return 	SWptr;
		} */
	}
}

//====================================================
//	Read flash memory
void	ReadFlash(BYTEX *ramAddr, HALFW FlashAddr, HALFW  length) 
{
	if(length == 0)
		length = 0x100;

	if((FlashAddr >= FlashStart) && (FlashAddr < FlashLimit)) memcpy(ramAddr,(BYTEX *)FlashAddr,length);
		//mem_cpy(ramAddr,(BYTEX *)FlashAddr,length);
}

//====================================================
//	Update flash memory
BYTE	UpdateFlash(HALFW foffset, BYTEX * RAMbuf, BYTE length) 
{
	HALFW	FlashAddr, FlashAddrEnd;
	HALFW	dataleft = 0, toUpdate = 0;
	uchar updtcntr = 0;
	FlashAddr =  foffset;
	FlashAddrEnd = FlashAddr + length - 1;		//	End of the target address

	//***************************************************//	
//	Update RAM
	if((foffset >= RAMBase) && (foffset < RAMLimit))
	{
		//mem_cpy((BYTEX *)foffset,RAMbuf,length);
		memcpy((BYTEX *)foffset, RAMbuf, length);
		return SUCCESS;
	}

	//***************************************************//	
//	Update flash
	else if((FlashAddrEnd < FlashLimit))		//	Flash and OTP memory area
	{   
		//mem_cpy(FlashBuffer,RAMbuf,length);
		
	    FlashAddr &= (0xFE00 | PageSize); 
	    foffset = (HALFW)foffset & (PageSize - 1);
	    
		for(dataleft = length; dataleft; dataleft  -= toUpdate)
		{
			toUpdate = (foffset + dataleft) < PageSize ? dataleft : (PageSize - foffset);
						
			//mem_cpy(FlashBuffer,(BYTEX *)FlashAddr,PageSize);
			memcpy(FlashBuffer, (BYTEX *)FlashAddr, PageSize);

			//mem_cpy(FlashBuffer + foffset,RAMbuf,toUpdate);
			memcpy(FlashBuffer + foffset, RAMbuf, toUpdate);
			RAMbuf += toUpdate; 		/* update rambuf pointer */
			updtcntr = 0;
			erase_page:	
			if(Erase_Page((BYTEX *)FlashAddr) != SUCCESS)
			{
				//SWptr = WRERROR;				//	6501
				updtcntr++;
				if(updtcntr > 3) return WRERROR; 
				//return WRERROR;
				goto erase_page;			
			}
			
			//updtcntr = 0;
			write_bytes:
			if(Write_Bytes((BYTEX *)FlashAddr, FlashBuffer, PageSize) != SUCCESS)
			{
				//SWptr = WRERROR;				//	6501
				updtcntr++;
				if(updtcntr > 3) return WRERROR; 
				//return WRERROR;
				goto erase_page;			
			}
		
			//if(memcmp((BYTEX *)FlashAddr, FlashBuffer, PageSize) != SUCCESS)	
			{									//	Check by reading back
				//SWptr = RDERROR;				//	6504
				//return RDERROR;
				//goto erase_page;	
			}	
					
			foffset = (BYTE)(toUpdate + foffset) & (PageSize - 1);
			FlashAddr += PageSize;
		}
	}
	else
	{
		return P1P2ERR;						//	6A00
	}
	return SUCCESS;
}

#if 1
void DES_Decrypt(BYTE * key, BYTE * inbuf, BYTE * outbuf) _REENTRANT_ {
	/*CLKCON |= Bit0_En;					   	//	Open DES CLK
	DES_DATEN = 1;							//	Enable data input
	DESD0 = inbuf[7];
	DESD1 = inbuf[6];
	DESD2 = inbuf[5];
	DESD3 = inbuf[4];	
	DESD4 = inbuf[3];
	DESD5 = inbuf[2];
	DESD6 = inbuf[1];
	DESD7 = inbuf[0];
	DES_DATEN = 0;							//	Disable data input	
	if(mode) {
		DES_KEY1EN = 1;
		DESD0 = key1[7];
		DESD1 = key1[6];
		DESD2 = key1[5];	
		DESD3 = key1[4];
		DESD4 = key1[3];
		DESD5 = key1[2];
		DESD6 = key1[1];	
		DESD7 = key1[0];
		DES_KEY1EN = 0;
	
		DES_KEY2EN = 1;
		DESD0 = key2[7];
		DESD1 = key2[6];
		DESD2 = key2[5];	
		DESD3 = key2[4];
		DESD4 = key2[3];
		DESD5 = key2[2];
		DESD6 = key2[1];	
		DESD7 = key2[0];
		DES_KEY2EN = 0;
		DES_TDES = 1;						//	Tripple DES
	} else {	
		DESCTL |= 0xC0;						//	Key1,Key2 enable
		DESD0 = key[7];
		DESD1 = key[6];
		DESD2 = key[5];	
		DESD3 = key[4];
		DESD4 = key[3];
		DESD5 = key[2];
		DESD6 = key[1];	
		DESD7 = key[0];
		DESCTL &= 0x3F;						//	Key1,Key2 disable
		DES_TDES = 0;						//	Tripple DES
	}
	//if(oper == 0x00)
	//DES_MODE = 0;						//	Encryption
	//if(oper == 0x01)
	DES_MODE = 1;						//	Decryption
	DES_TDES = 0;						//	Tripple DES	(disabled)
	DES_START = 1;							//	Start DES
	while(!DES_END)							//	DES caculation is pending
	{;}		
	//CLKCON &= Bit0_Dis;						//	Close DES CLK								
	//CLKCON |= Bit0_En;					   	//	Open DES CLK
	outbuf[7] = DESD0;
	outbuf[6] = DESD1;		
	outbuf[5] = DESD2;
	outbuf[4] = DESD3;
	outbuf[3] = DESD4;
	outbuf[2] = DESD5;
	outbuf[1] = DESD6;
	outbuf[0] = DESD7;
	DES_END = 0;							//	Clear DES_END
	CLKCON &= Bit0_Dis;						//	Close DES CLK	
	return;*/
	DES_Operation(0, 1, inbuf, key, key, outbuf);	
}

void DES_Encrypt(BYTE * key, BYTE * inbuf, BYTE * outbuf) _REENTRANT_ {
	/*CLKCON |= Bit0_En;					   	//	Open DES CLK
	DES_DATEN = 1;							//	Enable data input
	DESD0 = inbuf[7];
	DESD1 = inbuf[6];
	DESD2 = inbuf[5];
	DESD3 = inbuf[4];	
	DESD4 = inbuf[3];
	DESD5 = inbuf[2];
	DESD6 = inbuf[1];
	DESD7 = inbuf[0];
	DES_DATEN = 0;							//	Disable data input
	if(mode) {
		DES_KEY1EN = 1;
		DESD0 = key1[7];
		DESD1 = key1[6];
		DESD2 = key1[5];	
		DESD3 = key1[4];
		DESD4 = key1[3];
		DESD5 = key1[2];
		DESD6 = key1[1];	
		DESD7 = key1[0];
		DES_KEY1EN = 0;
	
		DES_KEY2EN = 1;
		DESD0 = key2[7];
		DESD1 = key2[6];
		DESD2 = key2[5];	
		DESD3 = key2[4];
		DESD4 = key2[3];
		DESD5 = key2[2];
		DESD6 = key2[1];	
		DESD7 = key2[0];
		DES_KEY2EN = 0;
		DES_TDES = 1;						//	Tripple DES
	} else {	
		DESCTL |= 0xC0;						//	Key1,Key2 enable
		DESD0 = key[7];
		DESD1 = key[6];
		DESD2 = key[5];	
		DESD3 = key[4];
		DESD4 = key[3];
		DESD5 = key[2];
		DESD6 = key[1];	
		DESD7 = key[0];
		DESCTL &= 0x3F;						//	Key1,Key2 disable
		DES_TDES = 0;						//	Tripple DES
	}
	//if(oper == 0x00)
	DES_MODE = 0;						//	Encryption
	//if(oper == 0x01)
	//DES_MODE = 1;						//	Decryption
	DES_START = 1;							//	Start DES
	while(!DES_END)							//	DES caculation is pending
	{;}		
	CLKCON &= Bit0_Dis;						//	Close DES CLK								
	CLKCON |= Bit0_En;					   	//	Open DES CLK
	outbuf[7] = DESD0;
	outbuf[6] = DESD1;		
	outbuf[5] = DESD2;
	outbuf[4] = DESD3;
	outbuf[3] = DESD4;
	outbuf[2] = DESD5;
	outbuf[1] = DESD6;
	outbuf[0] = DESD7;
	DES_END = 0;							//	Clear DES_END
	CLKCON &= Bit0_Dis;						//	Close DES CLK
	return;*/
	DES_Operation(0, 0, inbuf, key, key, outbuf);	
}

void TDES_Decrypt(BYTE * key1, BYTE * key2, BYTE * inbuf, BYTE * outbuf) _REENTRANT_ {
	DES_Operation(1, 1, inbuf, key1, key2, outbuf);	
}

void TDES_Encrypt(BYTE * key1, BYTE * key2, BYTE * inbuf, BYTE * outbuf) _REENTRANT_ {
	DES_Operation(1, 0, inbuf, key1, key2, outbuf);	
}

#endif

//====================================================
//	DES operation
//	mode=P1:	0x00-DES,0x01-T-DES
//	oper=P2:	0x00-Encrypt,0x01-Decrypt,0x10-Read Result
//	len=P3
BYTE DES_Operation(BYTE mode, BYTE oper, BYTE * inbuf, BYTE * key1, BYTE * key2, BYTE * outbuf)
{	
	CLKCON |= Bit0_En;					   	//	Open DES CLK
	DES_DATEN = 1;							//	Enable data input
	DESD0 = inbuf[7];
	DESD1 = inbuf[6];
	DESD2 = inbuf[5];
	DESD3 = inbuf[4];	
	DESD4 = inbuf[3];
	DESD5 = inbuf[2];
	DESD6 = inbuf[1];
	DESD7 = inbuf[0];
	DES_DATEN = 0;							//	Disable data input

	if(mode == 0x01)						//	Tripple DES
	{	
		DES_KEY1EN = 1;
		DESD0 = key1[7];
		DESD1 = key1[6];
		DESD2 = key1[5];	
		DESD3 = key1[4];
		DESD4 = key1[3];
		DESD5 = key1[2];
		DESD6 = key1[1];	
		DESD7 = key1[0];
		DES_KEY1EN = 0;

		DES_KEY2EN = 1;
		DESD0 = key2[7];
		DESD1 = key2[6];
		DESD2 = key2[5];	
		DESD3 = key2[4];
		DESD4 = key2[3];
		DESD5 = key2[2];
		DESD6 = key2[1];	
		DESD7 = key2[0];
		DES_KEY2EN = 0;

		if(oper == 0x00)
		DES_MODE = 0;						//	Encryption
		if(oper == 0x01)
		DES_MODE = 1;						//	Decryption
		DES_TDES = 1;						//	Tripple DES
	}
	else									//	Single DES
	{	
		DESCTL |= 0xC0;						//	Key1,Key2 enable
		DESD0 = key1[7];
		DESD1 = key1[6];
		DESD2 = key1[5];	
		DESD3 = key1[4];
		DESD4 = key1[3];
		DESD5 = key1[2];
		DESD6 = key1[1];	
		DESD7 = key1[0];
		DESCTL &= 0x3F;						//	Key1,Key2 disable

		if(oper == 0x00)
		DES_MODE = 0;						//	Encryption
		if(oper == 0x01)
		DES_MODE = 1;						//	Decryption
		DES_TDES = 0;						//	Tripple DES
	}
	DES_START = 1;							//	Start DES
	while(!DES_END)							//	DES caculation is pending
	{;}		
	//CLKCON &= Bit0_Dis;						//	Close DES CLK
							
	//CLKCON |= Bit0_En;					   	//	Open DES CLK
	outbuf[7] = DESD0;
	outbuf[6] = DESD1;		
	outbuf[5] = DESD2;
	outbuf[4] = DESD3;
	outbuf[3] = DESD4;
	outbuf[2] = DESD5;
	outbuf[1] = DESD6;
	outbuf[0] = DESD7;
	DES_END = 0;							//	Clear DES_END
	CLKCON &= Bit0_Dis;						//	Close DES CLK
}

void MAC_Algorithm3(uchar len, uchar * key, uchar * message, uchar * output) _REENTRANT_ {
	//unsigned char output[8];
	uchar i, j;
    uchar xx[8];
    //uchar block[8];
    //uchar offset = 0;
	memset(xx, 0, 8);
    //memcpy(xx, iv, 8);
    // Chain and encrypt 5 8-bit blocks
    for (i = 0; i < len; i += 8) {
        //memcpy(block, message + i , 8);
		for(j = 0; j < 8; j++) {
			xx[j] ^= message[i + j];
		}
        //set xx `xor {xx} {mj}` # chain
        //xor_block(xx, block);
		DES_Operation(0, 0, xx, key, key, output);
        //set xx `des -k {k0} -c {xx}` #encrypt
        //des_ecb_crypt(xx, output, DES_ENCRYPT, k0);
        memcpy(xx, output, 8);
    }

	DES_Operation(0, 0, xx, key + 8, key + 8, output);
    //des_ecb_crypt(xx, output, DES_DECRYPT, k1);
    memcpy(xx, output, 8);
	
	DES_Operation(0, 0, xx, key, key, output);
    //des_ecb_crypt(xx, output, DES_ENCRYPT, k0);
    memcpy(xx, output, 8);
    //print_hex(xx, 8);
}

//====================================================
//	Generate random number
void Random_Number_Generator(BYTE *RNGBuf, BYTE length) _REENTRANT_ {
	BYTE	WaitTime;

	RNGCTL &= 0xFE;							//	Open RNG

	while(length > 0)
	{
		WaitTime = 10;
		while(WaitTime --)
		{
			_nop_();
			_nop_();
			_nop_();
			_nop_();
			_nop_();

			_nop_();
			_nop_();
			_nop_();
			_nop_();
			_nop_();
		}
		*RNGBuf = RNGDAT;
		RNGBuf++;
		length--;
	}

	RNGCTL |= 0x01;							//	Close RNG
}

//====================================================
//	Return to BL
void	ReturnToBL(void)
{
	IE = 0;
	MMU_SEL = 0x01;
	rP3 = 0x06;
	Foffset = 0x8000;
	//IOBuf[0] = 0x00;
	//IOBuf[1] = 0x01;
	Erase_Page((BYTEX *)Foffset);
	//mem_cpy(IOBuf,VectorTable,0x60);
	memcpy(IOBuf,VectorTable,0x60);
	Write_Bytes((BYTEX *)(0x8000),IOBuf,0x60);
}

void StartTimeoutSequence(void) {
	_tick_counter = 0;
 	TMOD = 1; 		//16 bit timer mode
	TH0 = 0xC3;		//50000 clock
	TL0 = 0x50;	
	IE_TMR0 = 1; 		//enable interrupt timer 0	
	TCON_TF0 = 0;
	TCON_TR0 = 1;
}

void EndTimeoutSequence(void) {
	_tick_counter = 0;
	TCON_TR0 = 0;
	TH0 = 0x00;	
	TL0 = 0x00;		
	TCON_TF0 = 0;
	IE_TMR0 = 0; 		//disable interrupt timer 0	
}
