#include "..\defs.h"
#include "..\asgard\file.h"
#include "..\asgard\fs.h"
#include "..\midgard\midgard.h"
#include "..\liquid.h"
#include "..\framework\vas.h"
#include "..\framework\sms.h"
#include "..\framework\dcs.h"

#ifndef _APPLICATION__H

extern uint16 user_packet_decode(uchar * buffer, uchar * address, uchar length) _REENTRANT_ ;
extern uchar user_callback(uchar state, uchar tag, uchar size, uchar * buffer) _REENTRANT_ ;


#define _APPLICATION__H
#endif 