#include "..\defs.h"
#include "..\config.h"
#include "..\midgard\midgard.h"
#include "..\asgard\file.h"
#include "..\liquid.h"
#include "..\framework\VAS.h"
#include "..\framework\SMS.h" 
#include "..\framework\dcs.h"
#include "..\framework\des.h"
#include "decr.h"
#include <string.h>

#if VAS_DECR_ALLOCATED

uchar decr_decode(void) _REENTRANT_ {			 //return response length  
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
	plen = vas_cistr[2];
	len = (len - 2);		//length of encrypted string	
	//memcpy(STK_buffer, vas_cistr + 3,len);		//copy encrypted string

	key = m_alloc(16); 							//allocate key buffer
	_select(&temp_fs, FID_MF);
	_select(&temp_fs, FID_LIQUID); 
	if(_select(&temp_fs, FID_WIBKEY) < 0x9F00) goto exit_plugin;
	_readrec(&temp_fs, key_id, key, 16);
	DES_MemOperation(len, DES_MODE_TDES | DES_MODE_CBC | DES_MODE_DECRYPT, key, vas_cistr + 3, STK_buffer);
	m_free(key);								//freed up key buffer
	len = (len - plen);
	VAS_set_variable(vas_cvid, len, STK_buffer);			//set variable
	exit_plugin:
	VAS_exit_plugin();
	return 0; 
}
#endif