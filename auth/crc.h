#ifndef _CRC__H
#ifndef _DEFS_DEFINED
#include "..\defs.h"
#endif
#ifndef _CONFIG__H
#include "..\config.h"
#endif
#include "..\asgard\file.h"

#if CRC32_PROCESS_MEMORY
uint32 CalCRC32(uchar * SrcAddr, uint16 length) _REENTRANT_ ;
#endif
#if CRC32_PROCESS_FILE
uint32 FileCRC32(fs_handle * handle, uint16 length, uint16 offset) _REENTRANT_ ;
#endif

#define _CRC__H
#endif