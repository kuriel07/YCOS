#include "..\defs.h"
#include "..\config.h"
#include "..\midgard\midgard.h"
#include "..\asgard\file.h"
#include "..\liquid.h"
#include "..\framework\VAS.h"
#include "..\framework\SMS.h" 
#include "..\framework\dcs.h"
#include "..\framework\des.h"
#include "encr.h"
#include <string.h>

#if VAS_ENCR_ALLOCATED	  

uchar encr_decode(void) _REENTRANT_ {			 //return response length	  
	extern uchar * vas_cistr; 	 		//current input string
	extern uchar vas_cvid;				//current variable id
	extern uchar STK_buffer[];
	uchar len;							//input string length
	uchar plen; 						//padded length
	uchar key_id;
	uchar i;
	fs_handle temp_fs;
	uchar * key = NULL;
	if(vas_cistr == NULL) goto exit_plugin;
	len = vas_cistr[0];
	key_id = vas_cistr[1];
	len = len -1;								//total input string length without key_id
	plen = 8 - (len % 8);						//padding length
	memcpy(STK_buffer, vas_cistr + 2,len);		//copy input string
	for(i=len;i<(len+plen);i++) {
		STK_buffer[i] = 0;						//fill padding bytes
	}
	len = (len + plen);							//total current length
	key = m_alloc(16); 							//allocate key buffer
	_select(&temp_fs, FID_MF);
	_select(&temp_fs, FID_LIQUID); 
	if(_select(&temp_fs, FID_WIBKEY) < 0x9F00) goto exit_plugin;
	_readrec(&temp_fs, key_id, key, 16);
	DES_MemOperation(len, DES_MODE_TDES | DES_MODE_CBC | DES_MODE_ENCRYPT, key, STK_buffer, STK_buffer + len + 1);
	m_free(key);								//freed up key buffer
	STK_buffer[len] = plen;			//padding number
	VAS_set_variable(vas_cvid, len + 1, STK_buffer + len);			//set variable
	exit_plugin:
	VAS_exit_plugin();
	return 0;					//0 => next instruction (no response), 
}
#endif