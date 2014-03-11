#include "..\defs.h"
#include "..\config.h"
#include "..\midgard\midgard.h"
#include "..\asgard\file.h"
#include "..\liquid.h"
#include "..\framework\VAS.h"
#include "..\framework\SMS.h" 
#include "..\framework\dcs.h"
#include "ditr.h"

#if VAS_DITR_ALLOCATED

uchar ditr_decode(void) _REENTRANT_ {			 //return response length, 0 on no response (automatically execute next command)	 
	extern uchar * vas_cistr; 	 		//current input string
	extern uchar vas_cvid;				//current variable id
	uchar j = 0;
	if(vas_cistr == NULL) goto exit_plugin;
	j += VAS_push_header(j, (DISPLAY_TEXT & 0x7F), 0x80, STK_DEV_DISPLAY);
	j += SAT_file_push(j, (STK_TAG_TEXT_STRING & 0x7F), vas_cistr[0], vas_cistr + 1);
	exit_plugin:
	VAS_exit_plugin;
	return j;					//0 => next instruction (no response), 
}
#endif