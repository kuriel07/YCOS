
#include	"_17BD_UserCode.h"  
#include 	"defs.h"
#include	"..\ISO7816\ISO7816.h"
#include	"..\NORFlash\NORFlash.h"
#include 	"..\yggdrasil\yggdrasil.h"
#include	<intrins.h>

//====================================================
//	DES finish interrutp service routine
void	DES_ISR(void)	interrupt 0
{
	IE_DES = 0;
	return;
}

//uint16 _tick_counter;
static uchar rcv_index = 0;
#define apdu_data_buffer (iso7816_buffer+5)
#define apdu_le_value *(iso7816_buffer+262)
//====================================================
//	UART interrutp service routine
void	UART_ISR(void)	interrupt 2
{
	IE_UART = 0;  
	//switch(_iso7816_state) {
		//case ISO7816_RECEIVE_CMD:	
	if(_iso7816_state == ISO7816_RECEIVE_CMD) { 
			iso7816_buffer[rcv_index++] = UBUF;
			if(rcv_index == 5) {
				//_iso7816_case = 1;			//command only			 
				/*_iso7816_cla = iso7816_buffer[0];
				_iso7816_ins = iso7816_buffer[1];
				if(ISO7816_case() > 1) {   		//check for command with data
					_iso7816_state = ISO7816_SEND_ACK;
					rcv_index =0;
				} else {
					_iso7816_state = ISO7816_RUNNING;
					rcv_index=0;
				}*/
				rcv_index=0; 
				//apdu_le_value = 0;
				_iso7816_state = ISO7816_RUNNING;
			} 
			if(iso7816_buffer[0] == 0xff && rcv_index == 4) { 	//pps
				rcv_index=0;  
				_iso7816_state = ISO7816_RUNNING;	
			}  
			//break;
	} else if(_iso7816_state == ISO7816_RECEIVE_DATA) {
		//case ISO7816_RECEIVE_DATA: 	  
			apdu_data_buffer[rcv_index++] = UBUF;
			if(rcv_index == iso7816_buffer[4]) {
				//apdu_le_value = 0;
				_iso7816_state = ISO7816_RUNNING;
				rcv_index=0;
			}
			//break;	
		/*case ISO7816_WAIT_LE:
			apdu_le_value = UBUF;
			_iso7816_state = ISO7816_RUNNING;
			rcv_index=0;
			break; */
	}
	IE_UART = 1;
	return;
}

//====================================================
//	Interrutp 3 interrutp service routine
void	INT3_ISR(void)	interrupt 3
{
	IE_INT3 = 0;
	return;
}

//====================================================
//	Flash write finish interrutp service routine
void	FLASH_ISR(void)	interrupt 4
{
	IE_FLASH = 0;
	return;
}

//====================================================
//	Interrutp 5 service routine
void	INT5_ISR(void)	interrupt 5
{
	IE_INT5 = 0;
	return;
}

//====================================================
//	Watch dog interrutp service routine
void	WDT_ISR(void)	interrupt 6
{
	IE_WDT = 0;
	TX_NULL_BYTE_OFF(8000)
	IE_WDT = 1;
	return;
}



