#ifndef _DEFS_DEFINED
#include "..\defs.h"
#endif
#include "..\asgard\file.h"
#ifndef _CONFIG__H
#include "..\config.h"
#endif
#ifndef _RFM__H
//#include "..\liquid.h"

/* Remote File Management Service */
uint16 RFM_decode(fs_handle * handle, uint16 address, uint16 length) _REENTRANT_ ;
uint16 RFM_refresh(void) _REENTRANT_ ;
#define _RFM__H
#endif