#include "liquid.h"	
#include "..\asgard\file.h"
#include "..\framework\SMS.h" 
#include "..\config.h"
//#include "services\RFM.h"
#include <string.h>

uchar STK_buffer[256];
uchar _me_capabilities[STK_ME_CAPABILITY_SIZE];
#if LIQUID_ALLOCATED
uint16 _stk_menu_offset; 
uint16 _stk_menu_anchor;
uint16 _stk_menu_current;
#endif	

void SAT_profile_download(uchar * buffer, uchar len) _REENTRANT_ {
	memset(_me_capabilities, 0, STK_ME_CAPABILITY_SIZE);
	if(len >= STK_ME_CAPABILITY_SIZE) {
		len = STK_ME_CAPABILITY_SIZE;
	}
	memcpy(_me_capabilities, buffer, len);
}

uint16 liquid_error(uchar status) _REENTRANT_ {
	uchar len = 0;
	uchar i;
	if(liquid_profile_check(TERMINAL_9EXX_RESPONSE) == 0) return APDU_SUCCESS;
	
	return APDU_STK_ERROR | (uint16)len;	
}


uchar liquid_profile_check(uchar cmd) _REENTRANT_ {
	uchar i = (cmd >> 4);
	return (_me_capabilities[i] & (1<< (cmd & 0x07)));
}

uchar liquid_proactive_check(uchar cmd) _REENTRANT_ {
	switch(cmd) {		
/* STK CMD DETAIL TYPE */
		case REFRESH :
			return (_me_capabilities[2]&0x80);
		case MORE_TIME :
			return (_me_capabilities[2]&0x08);
		case POLL_INTERVAL:
			return (_me_capabilities[2]&0x20);
		case POLLING_OFF:
			return (_me_capabilities[2]&0x40);
		case SET_UP_CALL:
			return (_me_capabilities[3]&0x10);
		case SEND_SS:
			return (_me_capabilities[3]&0x04);
//- "12" = Reserved for SEND USSD;
		case SEND_SHORT_MESSAGE:
			return (_me_capabilities[3]&0x02);
		case PLAY_TONE:
		case DISPLAY_TEXT:
			return (_me_capabilities[2]&0x01);
		case GET_INKEY:
			return (_me_capabilities[2]&0x02);
		case GET_INPUT:
			return (_me_capabilities[2]&0x04);
		case SELECT_ITEM:
			return (_me_capabilities[3]&0x01);
		case SET_UP_MENU:
			return (_me_capabilities[3]&0x40);
		case PROVIDE_LOCAL_INFORMATION: 
			return (_me_capabilities[3]&0xc0);	 
		default:
			return 1;
	}
}

#if LIQUID_ALLOCATED
extern fs_handle stk_fs;
uint16 liquid_profile(void) _REENTRANT_ {
	uint16 status = APDU_SUCCESS;
	uint16 len;
	//fs_handle menu_fs;
	/* Select EF STKMenu */ 
	_select(&stk_fs, FID_MF);
	_select(&stk_fs, FID_LIQUID); 
	if(_select(&stk_fs, FID_STKMENU) < 0x9F00) return APDU_FATAL_ERROR;
	/* read root menu configuration */
	_stk_menu_offset = 0;
	_stk_menu_anchor = 0;
	_stk_menu_current = 0;
	_readbin(&stk_fs, _stk_menu_anchor, STK_buffer, sizeof(stk_config));
	if(SAT_init(0, 0) == 0) return APDU_STK_OVERLOAD;		//clear SAT temporary file and filled it with terminal response	
	if(((stk_config *)STK_buffer)->parent == 0) {  
		len = ((stk_config *)STK_buffer)->length;
		_stk_menu_offset = ((stk_config *)STK_buffer)->sibling;
		_readbin(&stk_fs, sizeof(stk_config), STK_buffer, len);
		//len = SAT_push(command->bytes, FETCH_TAG_PROSIM, len, STK_buffer);
		//fetch_len = SAT_file_push(0, FETCH_TAG_PROSIM, len, STK_buffer);
		//status = APDU_STK_RESPONSE | fetch_len;	
		status = SAT_response(STK_buffer, len);
	}
	return status;
} 
#endif


void liquid_set_response_data(uchar status, uchar * buffer, uint16 length) _REENTRANT_ {
	fs_handle temp;
	struct response_prop {
	  uint16 length;
	  uchar status;
	} prop;
	_select(&temp, FID_MF); 
	_select(&temp, FID_LIQUID); 
	if(_select(&temp, FID_RES) < 0x9F00) { goto exit_set_response; }
	prop.length = length;
	prop.status = status;
	_writebin(&temp, 3, buffer, length);
	_writebin(&temp, 0, &prop, sizeof(struct response_prop));
	exit_set_response:
	return;
}
