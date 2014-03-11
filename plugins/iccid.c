#include "..\defs.h"
#include "..\config.h"
#include "..\midgard\midgard.h"
#include "..\asgard\file.h"
#include "..\liquid.h"
#include "..\framework\VAS.h"
#include "..\framework\SMS.h" 
#include "..\framework\dcs.h"
#include "iccid.h"

#if VAS_ICCID_ALLOCATED
uchar iccid_decode(void) _REENTRANT_ {			 //return response length	 
	extern uchar * vas_cistr; 	 		//current input string
	extern uchar vas_cvid;				//current variable id
	extern uchar STK_buffer[];
	uchar j = 0;
	fs_handle temp_fs;
	if(vas_cistr == NULL) goto exit_plugin;		//no response, input string not exist
	_select(&temp_fs, FID_MF);
	if(_select(&temp_fs, FID_ICCID) < 0x9F00) goto exit_plugin;
	_readbin(&temp_fs, 0, STK_buffer, 10); 
	VAS_set_variable(vas_cvid, 10, STK_buffer);			//set variable
	exit_plugin:
	VAS_exit_plugin();
	return j;					//0 => next instruction (no response),  
}
#endif