#ifndef _DEFS_DEFINED
#include "..\defs.h"
#endif
#ifndef _CONFIG__H
#include "..\config.h"
#endif
#ifndef _DES__H
//#include "..\..\defs.h"

#define DES_MODE_3KEYS		8		//default 2Keys
#define DES_MODE_TDES		4		//default DES
#define DES_MODE_CBC		2		//default EBC
#define DES_MODE_ENCRYPT	1		//default decrypt
#define DES_MODE_DECRYPT	0

#define des_decode

#if DES_OPERATION_MEMORY
void DES_MemOperation(uint16 length, uchar mode, uchar * key, uchar * inbuf, uchar * outbuf) _REENTRANT_ ;
#endif
#if DES_OPERATION_FILE
void DES_FileOperation(uint16 length, uint16 offset, uchar mode, uchar * key, fs_handle * handle) _REENTRANT_ ;
#endif

#define _DES__H
#endif