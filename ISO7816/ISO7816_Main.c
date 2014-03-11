#include "_17BD_UserCode.h"
#include "defs.h"
#include "config.h"	
#include "..\yggdrasil\yggdrasil.h"
#include "..\drivers\ioman.h"
#include "..\ISO7816\ISO7816.h"
#include "..\NORFlash\NORFlash.h"
#include "..\misc\mem.h"
#include <intrins.h>

//BYTE	SWptr;
HALFWX	Foffset;
BYTEX	PPSFlag;
//BYTEX 	gCommand[5];
//BYTEX	IOBuf[512];
//BYTEX	FlashBuffer[512];
//uchar _iso7816_le;
uchar _iso7816_ins;
uchar _iso7816_cla;
//uchar _iso7816_case;
BYTEX	_iso7816_state;
BYTEX 	iso7816_buffer[263];
BYTEX	FlashBuffer[512];
//sfr	xbp = 0x7e;
//BYTEX	IOBuf[512];
//BYTEX	IOBuf[] = (iso7816_buffer+5);

//BYTEC	ATR[]={0x3B,0x18,0x96,0x00,0x17,0xBD,0x10,0x00,0x00,0x90,0x00};
//BYTEC	ATR[]={0x3B,0x1A,0x96,0x00,0x17,0xBD,0x10,0x59,0x43,0x4F,0x53,0x02,0x00};

//TS = 0x3B, direct conversion, 3F=inverse
//T0 = 0x1A, 1=TA available, A=length for history bytes
//TA1 = 0xA8, Fmax=5Mhz, Fi=512, Di=32, 3.57Mhz/16, 230K
//BYTEC	ATR[]= {0x3B, 0x1A, 0x95, 0x00, 0x17, 0xBD, 0x10, 0x59, 0x43, 0x4F, 0x53, 0x31, 0x00};
#if VAS_ALLOCATED 
BYTEC	ATR[]= {0x3B, ATR_LENGTH, ATR_PPS, 0x00, ATR_CHIP_ID, ATR_CHIP_TYPE, ATR_CHIP_VER, ATR_OS_NAME, ATR_OS_YEAR, ATR_OS_CUSTOMER, ATR_OS_REV, ATR_OS_VER, ATR_FS_VER, ATR_APP_FRAMEWORKS, ATR_WIB_PLUGINS, ATR_CHECKSUM, 0x00}; 
#else
BYTEC	ATR[]= {0x3B, ATR_LENGTH, ATR_PPS, 0x00, ATR_CHIP_ID, ATR_CHIP_TYPE, ATR_CHIP_VER, ATR_OS_NAME, ATR_OS_YEAR, ATR_OS_CUSTOMER, ATR_OS_REV, ATR_OS_VER, ATR_FS_VER, ATR_APP_FRAMEWORKS, ATR_CHECKSUM, 0x00};
#endif 
//BYTEC	ATR[]={0x3B,0x1A,0x11,0x00,0x17,0xBD,0x10,0x59,0x43,0x4F,0x53,0xB4,0x00};
//BYTEC	ATR[]={0x3B, 0x1D, 0x95, 0x04, 0x01, 0x7F, 0x3F, 0x10, 0x04, 0x17, 0x01, 0x04, 0x21, 0x31, 0x11, 0x20 };
//BYTEC	SW[12]={0x90,0x00,0x6D,0x00,0x6C,0x00,0x65,0x01,0x6A,0x00,0x65,0x04};
//				   Success, Invalid INS, P3 err, Write err, P1P2 err, Verify err

void ISO7816_init(void) {
	IE_EA = 1;				//global interrupt enable bit
	IE_UART = 1;			//enable uart interrupt
	//CLKSEL = 0x00;			//7.5Mhz internal clock
	//rP1=0;
	//MMU_SEL = 0;
	Initialize_Hardware();
	_iso7816_state = ISO7816_DORMANT;
}  


/*uchar ISO7816_case(void) {
	switch(_iso7816_cla) {
	 	case CLA_GSM11:
			return gsm_class_case[_iso7816_ins];
		case CLA_YGGDRASIL:
			return yggdrasil_class_case[_iso7816_ins];
		default:
			return 0;			
	}
	return 0;
} */
//====================================================
extern uchar get_resp_length;
//	Main process
void ISO7816_main(void)
{
	//register uint16 i=0;
	//register uint16 len;
	uint16 sw;
	while(1) {
		switch(_iso7816_state) {
		 	case ISO7816_DORMANT: 
				VDCON = 0x00;						//	VD closed
				FDCON = 0X00;						//	FD closed
				IoInit(0x11);						//	H/W initial	 
				//rP0 = 0x00;							//	disable output, low voltage level
				Tx_n_Bytes(1, ATR);		   			//kirim 3B	 (ask for T=0 protocol)	
				Initialize_Hardware();							  
				Initialize_Operating_System();	 	 
				Send_ATR();	//	Send ATR
				_iso7816_state = ISO7816_RECEIVE_CMD;
				Sleep_Mode();
				break;
			case ISO7816_WAIT_PPS:

				break;
			case ISO7816_READY:		
				//Sleep_Mode();			//	Power down mode after thread has been executed
				/*_iso7816_cla = iso7816_buffer[0] = receive_byte();		//CLA
				_iso7816_ins = iso7816_buffer[1] = receive_byte();		//INS
				iso7816_buffer[2] = receive_byte();		//P1
				iso7816_buffer[3] = receive_byte();		//P2
				iso7816_buffer[4] = receive_byte();		//P3
				len = (5 + iso7816_buffer[4]);
				
				if(len > 5) {
					//if(iso7816_buffer[1] == 0xA4) {
					send_byte(iso7816_buffer[1]);
					//send_byte(0);
					for(i=5;i<len;i++) {
						iso7816_buffer[i] = receive_byte();
					} 
					//}
				}
				//iso7816_buffer[5] = receive_byte();
				//iso7816_buffer[6] = receive_byte();
				_iso7816_state = ISO7816_RUNNING;  */
				break;
			case ISO7816_RECEIVE_CMD:
				/*if(_os_config.os_state & YGG_ST_LOAD_APP) {	   //load app into user program space
					//Sleep_Mode();	
					Load_User_App();
					_os_config.os_state &= ~(YGG_ST_LOAD_APP);
					Save_State();
				}
				if(_os_config.os_state & YGG_ST_SLEEP) {		   //sleep MCU
					Sleep_Mode();
					_os_config.os_state &= ~(YGG_ST_SLEEP);
				} */
				//do nothing
				break;
			case ISO7816_SEND_ACK:
				send_byte(iso7816_buffer[1]);			//send acknowledgement in order to and wait for data
				_iso7816_state = YGG_RECEIVE_DATA;
				break;
			case ISO7816_RECEIVE_DATA:
				//do nothing
				break;
			case ISO7816_WAIT_LE:		//wait for at least 1 etu 
				break;
			case ISO7816_RUNNING:
				switch(iso7816_buffer[0]) {
					case 0xff:	 						//default iso7816 system class
						PPS_Handler(iso7816_buffer);
						break;
					default:
						TX_NULL_BYTE_ON(8000)
						StartTimeoutSequence();
						//if(iso7816_buffer[1] == 0xC0 && iso7816_buffer[0] == _iso7816_cla) {									//get response
							/*if(iso7816_buffer[4] > get_resp_length) {				//check for response length	  
								//return (APDU_WRONG_LENGTH | get_resp_length); 	//wrong length .OR. requested length
								sw = (APDU_WRONG_LENGTH | get_resp_length); 	//wrong length .OR. requested length
							} else {
								get_resp_length = iso7816_buffer[4];
								//Set_Response(command->bytes, get_resp_length); 
								_iso7816_ins = iso7816_buffer[1];
								memcopy(iso7816_buffer, iso7816_buffer + 5, 0, get_resp_length);
								response_length = get_resp_length;
								get_resp_length = 0;
								sw = APDU_SUCCESS;
							}*/
						//} else {
						_iso7816_cla = iso7816_buffer[0];
						_iso7816_ins = iso7816_buffer[1];
						sw = Yggdrasil_Decode((apdu_command *)iso7816_buffer);
						//}
						
						if(_os_config.os_state & YGG_ST_LOAD_APP) {
							if(Load_User_App() == APDU_SUCCESS) {
								_os_config.os_state	&= ~YGG_ST_LOAD_APP;
								Save_State();
							}
						}
						EndTimeoutSequence();
						TX_NULL_BYTE_OFF(8000)
						ioman_transmit(response_length, _iso7816_ins, iso7816_buffer, sw);
						//Response();
						//TxStatus(SWptr);					//	Tx SW
						break;
				}
				_iso7816_state = ISO7816_RECEIVE_CMD;
				Sleep_Mode();
				break;
			case ISO7816_STOP:			//if the operation didn't response within 2 sec then cancel all pending operation
				EndTimeoutSequence();
				TX_NULL_BYTE_OFF(8000)
				ioman_transmit(0, _iso7816_ins, iso7816_buffer, APDU_FATAL_ERROR);
				_iso7816_state = ISO7816_RECEIVE_CMD;
				Initialize_Operating_System();
				break;
		}
	}
	/*if(ISO7816_Time == 0)
	{
		IoInit(0x11);						//	H/W initial
		Send_ATR();							//	Send ATR
		PPSFlag = 0;  						//	Enable PPS
		ISO7816_Time ++;
		return;								//	Return to the main process after sending ATR	
	}
	if(RcvAPDU())							//	Rcv APDU,include PPS
	{	
		Foffset = (P1 << 8) + P2;			//	Start address of flash reading/writing
		SWptr = SUCCESS;					//	9000

		TX_NULL_BYTE_ON(8000)
		CMMD_Handle();
		TX_NULL_BYTE_OFF(8000)
		
		Response();
		TxStatus(SWptr);					//	Tx SW
	} */
}

//====================================================
//	ISO/IEC 7816 commands handling
void	Send_ATR(void)
{
	uint16 i;
	//for(i=0;i<400;i++);				//delay ATR	
	/*for(i=0;i<10000;i++) {				//delay ATR	
		_nop_();
	}*/
	Tx_n_Bytes(sizeof(ATR)-1, ATR+1);	 //kirim data selanjutnya
}

//====================================================
//	Receive APDU
#if 0
BYTE	RcvAPDU(void)
{
	iso7816_buffer[0] = receive_byte();
	if(iso7816_buffer[0] == 0xFF)
	{	
		iso7816_buffer[1] = receive_byte();
		iso7816_buffer[2] = receive_byte();
		iso7816_buffer[3] = receive_byte();
		if(iso7816_buffer[1] == 0x10 && PPSFlag == 0)		//	PPS
		{	
			PPS();							//	Set baud rate
			PPSFlag = 1;					//	PPS is enabled for the 1st time only
		}
	}
	else if(iso7816_buffer[0] == 0x00)
	{	
		iso7816_buffer[1] = receive_byte();
		iso7816_buffer[2] = receive_byte();
		iso7816_buffer[3] = receive_byte();
		iso7816_buffer[4] = receive_byte();

		if((iso7816_buffer[1] == WRFLASH || iso7816_buffer[1] == UDFLASH || iso7816_buffer[1] == ERFLASH || (iso7816_buffer[1] == DESOPER && iso7816_buffer[3] != 0x10)) && iso7816_buffer[4] != 0) 
		{								
			send_byte(iso7816_buffer[1]);
			Rx_n_Bytes(iso7816_buffer[4]);
		}
		return	1;
	}
	return	0;
}
#endif

//====================================================
//	ISO/IEC 7816 commands handling
//	Format: 00 5x P1 P2 P3 



USHORT PPS_Handler(BYTEX * buffer) {
	switch(buffer[1]) {
		case 0x10:
			Set_PPS(buffer);
			return APDU_SUCCESS;
		default:   
 			return APDU_INSTRUCTION_INVALID;
	}
}

//====================================================
//	Tx response data
/*void	Response(void)
{
	if((INS == RDFLASH || INS == CRCCALL || INS == GETRDMN || (INS == DESOPER && P2 == 0x10)) && SWptr == SUCCESS)
	{	
		send_byte(INS);						//	Procedure byte in Case 2 APDU,followed with Le data

		if(INS == RDFLASH && P3 == 0x00)
			Tx_n_Bytes(0x100,IOBuf);
		else
			Tx_n_Bytes(P3,IOBuf);
	}
}*/
