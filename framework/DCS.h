#ifndef _DEFS_DEFINED
#include "..\defs.h"
#endif
#ifndef _CONFIG__H
#include "..\config.h"
#endif
#ifndef _DCS__H
//#include "..\..\defs.h"
#if COMPACT_7BIT_SUPPORT 
uchar decode_728(uchar * buffer_in, uchar * buffer_out, uchar size) _REENTRANT_ ;
uchar encode_827(uchar * buffer_in, uchar * buffer_out, uchar size, uchar offset)_REENTRANT_  ;
#endif

#if UNICODE_SUPPORT
uchar decode_ucs28(uint16 * buffer_in, uchar * buffer_out, uchar size) _REENTRANT_ ;
uchar encode_82ucs(uchar * buffer_in, uint16 * buffer_out, uchar size) _REENTRANT_ ;
#endif

#define _DCS__H
#endif