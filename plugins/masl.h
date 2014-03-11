#include "..\defs.h"
#include "..\midgard\midgard.h"
#include "..\asgard\file.h"
#include "..\liquid.h"
#include "..\framework\VAS.h"
#include "..\framework\SMS.h" 
#include "..\framework\dcs.h"

#ifndef _MASL__H

//uint16 masl_fetch(uchar * response, uint16 length) _REENTRANT_;
#define masl_fetch(response, length)
uchar masl_decode(void) _REENTRANT_ ;

#define _MASL__H
#endif