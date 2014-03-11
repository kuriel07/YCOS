#include "crc.h"  
//#include "..\..\defs.h"
//#include "..\..\YGGSYS\yggapis.h"
//#include "..\liquid.h"
#include <stdlib.h>

static uint32 lGenCRC32(uint32 lOldCRC, uchar ByteVal) _REENTRANT_ {
	uint32 TabVal;
	uchar j;
	TabVal = ((lOldCRC) ^ ByteVal) & 0xFF;
	for(j=8; j>0; j--) {
		if(TabVal & 1) {
		 	TabVal = (TabVal >> 1) ^ 0xEDB88320;
		} else {
			TabVal >>= 1;
		}
	}
	return TabVal ^ ((lOldCRC >> 8) & 0x00FFFFFF);
}

#if CRC32_PROCESS_MEMORY
uint32 CalCRC32(uchar * SrcAddr, uint16 length) _REENTRANT_ {
	uint32 crc32;
	uint16 ii;
	crc32 = 0xFFFFFFFF;		//pre-conditioning
	for(ii = 0; ii < length; ii++)
		crc32 = lGenCRC32(crc32, SrcAddr[ii]);
	crc32 = ~crc32;				//post-conditioning
	return crc32;
}
#endif

#if CRC32_PROCESS_FILE
uint32 FileCRC32(fs_handle * handle, uint16 length, uint16 offset) _REENTRANT_ {
	uint32 crc32;
	uint16 ii;
	uchar jj;
	uchar buffer[16];
	crc32 = 0xFFFFFFFF;		//pre-conditioning
	for(ii = 0; ii < length; ii+=16) {
		_readbin(handle, offset+ii, buffer, 16);
		for(jj = 0 ;jj < 16 && (ii+jj) < length; jj++) {
			crc32 = lGenCRC32(crc32, buffer[jj]);
		}
	}
	crc32 = ~crc32;				//post-conditioning
	return crc32;
}
#endif

 