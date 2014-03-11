#include "..\defs.h"
#include "..\ISO7816\ISO7816.h"
#include "..\NORFlash\NORFlash.h"
#ifndef _FLASH__H

typedef struct flash_mark {
 	uint16 num;
	uchar cntr;
	uchar tag;	
} flash_mark;

//maze configuration
#define FLASH_SECTOR_TAG			0xAA
#define FLASH_MAP_TAG				0x55
#define FLASH_PHYS_PAGE_SIZE_Y		10											//x^y								
#define FLASH_PHYS_PAGE_SIZE		512										 //physical page size
#define FLASH_LOG_PAGE_SIZE			(FLASH_PHYS_PAGE_SIZE - sizeof(flash_mark))
#define FLASH_NUM_OF_PAGE			128	
#define FLASH_PHYS_TOTAL_SIZE		(FLASH_NUM_OF_PAGE * FLASH_PHYS_PAGE_SIZE)	
#define FLASH_LOG_TOTAL_SIZE		(FLASH_NUM_OF_PAGE * FLASH_LOG_PAGE_SIZE)

typedef struct flash_sector {
	uchar buffer[FLASH_LOG_PAGE_SIZE];
	flash_mark mark;
} flash_sector;


void flash_read(uint16 pos, uchar * bytes, uint16 length) _REENTRANT_ ;
uint16 flash_write(uint16 pos, uchar * bytes) _REENTRANT_ ;	  		//always 1 page size
uint16 flash_erase(uint16 pos) _REENTRANT_ ;
#define _FLASH__H
#endif