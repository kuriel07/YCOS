#include "..\defs.h"
#include "..\midgard\midgard.h"
#include "..\asgard\file.h"
#include "..\liquid.h"
#include "..\framework\VAS.h"
#include "..\framework\SMS.h" 
#include "..\framework\dcs.h"

#ifndef _DIL__H  

#if VAS_MDIL_ALLOCATED || VAS_VDIL_ALLOCATED
uchar dil_get_pin(uchar id) _REENTRANT_ ;
uchar dil_display_text(uchar id) _REENTRANT_ ;
#endif

#if VAS_MDIL_ALLOCATED
#define MDIL_OP_DELETE					0
#define MDIL_OP_ADD_UPD_BOTH			0x18
#define MDIL_OP_UPD_NAME				0x08
#define MDIL_OP_UPD_VAL					0x10

uint16 mdil_fetch(uchar * response, uint16 length) _REENTRANT_;
uchar mdil_decode(void) _REENTRANT_ ;
#endif

#if VAS_VDIL_ALLOCATED	   
#define VDIL_OP_RET_RECID				0
#define VDIL_OP_RET_NAME				0x08
#define VDIL_OP_RET_VAL					0x10

uint16 vdil_fetch(uchar * response, uint16 length) _REENTRANT_ ;
uchar vdil_decode(void) _REENTRANT_ ;
#endif

#define _DIL__H
#endif