#include "..\defs.h"
#ifndef _SYS_FILE_H
#include "crc.h"
#include "file.h"


uint16 sys_select(uint16 fid);
uint16 sys_createfile(
	uint16 fid,  
	char *buffer, 
	uint16 size) _REENTRANT_;
uint16 sys_read(uint16 offset, uint16 size, char *buffer) _REENTRANT_;
uint16 sys_write(uint16 offset, uint16 size, char *buffer) _REENTRANT_;

#define _SYS_FILE_H
#endif
