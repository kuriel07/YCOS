#include "..\defs.h"
#include "..\midgard\midgard.h"
#include "..\asgard\file.h"
#include "..\liquid.h"
#include "..\framework\VAS.h"
#include "..\framework\SMS.h" 
#include "..\framework\dcs.h"

#ifndef _ICCID__H

//uint16 masl_fetch(uchar * response, uint16 length) _REENTRANT_;
#define iccid_fetch(response, length)
uchar iccid_decode(void) _REENTRANT_ ;

#define _ICCID__H
#endif