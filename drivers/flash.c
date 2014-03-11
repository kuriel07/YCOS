#include "flash.h"	 
#include <string.h>

//#define	FlashStart	0x00000
//#define	FlashLimit	0x10000

uint16 temp_pos;
void flash_read(uint16 pos, uchar * bytes, uint16 length) _REENTRANT_ {
	//uint16 len;
	MMU_SEL=1;
	/*if((pos ^ (pos + length)) & 0x8000) { 
		rP3 = 1;									//update bank 1
		//ReadFlash(IOBuf, (pos | 0x8000), 0x8000 - pos);
		temp_pos = 0x8000 - pos;
		if(temp_pos == 0) {
			memcpy(bytes, (BYTEX *)(pos | 0x8000), FLASH_PHYS_PAGE_SIZE);
		} else {
			memcpy(bytes, (BYTEX *)(pos | 0x8000), 0x8000 - pos);
		}
		//if((FlashAddr >= FlashStart) && (FlashAddr < FlashLimit)) memcpy(ramAddr,(BYTEX *)FlashAddr,length);
		rP3 = 2;									//update bank 2
		temp_pos = 0x8000 - pos;
		//ReadFlash(IOBuf + temp_pos, 0x8000, (pos + length) - 0x8000);
		memcpy(bytes, (BYTEX *)(pos | 0x8000), (pos + length) - 0x8000);
	} else {	 */
	if(pos & 0x8000) {
		rP3 = 2;
			//ptr = pos;
	} else {
		rP3 = 1;
			//pos |= 0x8000;
	}
	//ReadFlash(IOBuf, pos, length);
	memcpy(bytes, (BYTEX *)(pos | 0x8000), length);
	//}	
	//memcpy(bytes, IOBuf, length);
}
//BYTE	Write_Bytes(BYTEX * pDest, BYTEX * pSrc, HALFW len)
uint16 flash_write(uint16 pos, uchar * bytes) _REENTRANT_ {
	uchar cntr=0;
	MMU_SEL=1;
	if(pos & 0x8000) {
		rP3 = 2;
		pos |= 0x8000;
	} else {
		rP3 = 1;
		pos |= 0x8000;
	}
	restart_flash_write:
	if(Write_Bytes((BYTEX *)pos, bytes, FLASH_PHYS_PAGE_SIZE) != SUCCESS) {
	 	if(++cntr > 3) return APDU_FLASH_WERROR;
		goto restart_flash_write; 
	}
	return APDU_SUCCESS;
}

uint16 flash_erase(uint16 pos) _REENTRANT_ {
	//uint16 ptr = 0;	// = pos + 0x2000;
	uchar cntr = 0;
 	MMU_SEL=1;
	if(pos & 0x8000) {
		rP3 = 2;
		pos = pos;
	} else {
		rP3 = 1;
		pos = pos | 0x8000;
	}
	restart_flash_erase:
	if(Erase_Page((BYTEX *)(pos & 0xFE00)) != SUCCESS) {
	 	if(++cntr > 3) return APDU_FLASH_WERROR;
		goto restart_flash_erase; 
	}
	return APDU_SUCCESS;
}