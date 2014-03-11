/* IO Man (IO Manager), provides access to the hardware such as EEPROM
 *
 * Copyright 2010, Agus Purwanto.
 * All rights reserved.
 *
 * 
 */

#include <stdio.h>
#include "ioman.h"
#include "..\defs.h"
#include "..\yggdrasil\yggdrasil.h"
#include "..\misc\hexstring.h"	 
#include "..\ISO7816\ISO7816.h"
#include "..\NORFlash\NORFlash.h"
#include "..\misc\mem.h"
#include "flash.h"
#include <string.h>
//#define _CRT_SECURE_NO_DEPRECATE 1
//<TO DO>
//ganti dengan macro biar pemakaian RAM lebih irit dan kecepatan akses lebih baik
//jika kompiler mendukung ketika diimplementasikan ke EEPROM, ganti dengan assembly

#ifdef _YGGDRASIL_MICRO_KERNEL
//apdu_command * iobuf = iso7816_buffer;		//buffer input/output
#endif

//<TO DO>
//ini untuk desktop saja, ketika implementasi ke eeprom tidak diperlukan
#ifdef _X86_ARCH
//static FILE *eeprom;
#endif
//uint16 xdata data_pointer;

void ioman_init() _REENTRANT_
{
	//init pertama mode baca dulu
	/*#ifdef _X86_ARCH
	eeprom = fopen("\\32K.eeprom", "r+b");
	if(eeprom==NULL) { fopen("\\32K.eeprom", "w+b"); 
	eeprom = fopen("\\32K.eeprom", "r+b");}
	#endif */
	//data_pointer = 0;
	rP1 = 0;
	rP3 = 1;
	MMU_SEL = 1;
	maze_init();
} 

/*void ioman_read_program(uint16 pos, ucharx * bytes, uint16 length) {
	uint16 ptr;
	uchar i;
	//ptr = pos + 0x8000;
	switch(pos >> 12) {
		case 0:
		case 1:
			rP3=6;	  //XDATA 8000-FFFFh --> 0000-1FFFh
			MMU_SEL = 0x01;
			ptr = pos + 0x8000;
			break;
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			MMU_SEL = 0x01;
			ptr = pos;
			break;
		case 8:
		case 9:
		case 0xA:
		case 0xB:
		case 0xC:
		case 0xD:
		case 0xE:
		case 0xF:
		default:
			rP3 = 0; 
			MMU_SEL = 0x01;
			ptr = pos;
			break;
	}
	ReadFlash(IOBuf, ptr, length);
	memcopy(bytes, IOBuf, 0, length);	
	MMU_SEL = 0x00;
}*/

//void ioman_read_buffer(uint16 pos, uchar * bytes, uint16 length) _REENTRANT_ {
				  

//uint16 maze_cache_log_address;
//uint16 maze_cache_phys_address; 
uint16 maze_cache_laddr = 0xFFFF;
uint16 maze_cache_paddr = 0xFFFF;
uint16 _maze_map_base_addr = 0xFFFF;				//maze table base address for sector mapping

void ioman_erase_all(void) {
	uint16 address;
 	for(address = FLASH_PHYS_PAGE_SIZE; address != 0; address += FLASH_PHYS_PAGE_SIZE) {
		ioman_erase_page(address);
	}
	ioman_erase_page(0);
	_maze_map_base_addr = 0xFFFF;			//during formating set map base address to 0xFFFF
}	 

void maze_init() _REENTRANT_ {
	uint16 sector_found = 0xFFFF;
	uint16 base_addr = 0;			//jangan diutak-atik
	uchar cur_cntr = 0x80;
	flash_mark mark;
	read_next_sector:
	flash_read(base_addr + FLASH_LOG_PAGE_SIZE, &mark, sizeof(flash_mark));
	if(mark.tag == FLASH_MAP_TAG) {	  		//valid sector tag
		if((cur_cntr & 0x80) && mark.cntr == 0) {  			//check counter
			sector_found = base_addr;
			cur_cntr = mark.cntr;	
		} else {
	   		if((cur_cntr & 0x7F) <= mark.cntr) { 
				sector_found = base_addr;
				cur_cntr = mark.cntr;
			}
		}
	}
	base_addr += FLASH_PHYS_PAGE_SIZE;
	if(base_addr != 0) goto read_next_sector;
	/*if(sector_found == 0xFFFF) {   				//no map found, create new map, clearing all table
		memset(FlashBuffer, 0xff, FLASH_PHYS_PAGE_SIZE);
		((flash_sector *)FlashBuffer)->mark.num = 0;
		((flash_sector *)FlashBuffer)->mark.cntr = 0x80;
		((flash_sector *)FlashBuffer)->mark.tag = FLASH_MAP_TAG;
		sector_found = maze_find_empty_sector(); 			//find empty sector
		flash_write(sector_found, FlashBuffer);	   				//write new sector
	} */
	_maze_map_base_addr = sector_found;
	//return sector_found;
}

uint16 maze_log2phys(uint16 laddr) _REENTRANT_ {		//convert logical ADDRESS to physical ADDRESS by iterating each sector, return -1 on not found
	#if 0
	uint16 sector_found = 0xFFFF;
	uint16 base_addr = 0;			//jangan diutak-atik
	uchar cur_cntr = 0x80;
	flash_mark mark;
	uint16 l_sector = laddr / FLASH_LOG_PAGE_SIZE;			//logical sector address
	if(maze_cache_laddr == l_sector) {		  				//already in cache
		//return ((maze_cache_paddr * FLASH_PHYS_PAGE_SIZE) + (laddr % FLASH_LOG_PAGE_SIZE));
	}
	read_next_sector:
	flash_read(base_addr + FLASH_LOG_PAGE_SIZE, &mark, sizeof(flash_mark));
	if(mark.tag == FLASH_SECTOR_TAG) {	  		//valid sector tag
		if(mark.num == l_sector) {				//sector.num equal logical id
			if((cur_cntr & 0x80) && mark.cntr == 0) {  			//check counter
				sector_found = base_addr + (laddr % FLASH_LOG_PAGE_SIZE);
				cur_cntr = mark.cntr;	
			} else {
		   		if((cur_cntr & 0x7F) <= mark.cntr) { 
					sector_found = base_addr + (laddr % FLASH_LOG_PAGE_SIZE);
					cur_cntr = mark.cntr;
				}
			}
		}
	}
	base_addr += FLASH_PHYS_PAGE_SIZE;
	if(base_addr != 0) goto read_next_sector;
	maze_cache_laddr = l_sector;
	maze_cache_paddr = (sector_found / FLASH_PHYS_PAGE_SIZE);
	return sector_found;	
	#endif
	uint16 l_sector = laddr / FLASH_LOG_PAGE_SIZE;			//logical sector address
	uint16 sector_found = 0xFFFF;
	if(_maze_map_base_addr != 0xFFFF) {
		flash_read(_maze_map_base_addr + (l_sector * sizeof(uint16)), &sector_found, sizeof(uint16));
	}
	return sector_found;
}

uint16 maze_find_empty_sector() _REENTRANT_ { 		//return empty sector physical base ADDRESS, can be used to allocate new sector
	//_maze_base_addr = 0;
	static uint16 _maze_base_addr = 0;
	uint16 base_addr = 0;
	flash_mark mark;
	read_next_sector:
	flash_read(_maze_base_addr + FLASH_LOG_PAGE_SIZE, &mark, sizeof(flash_mark));
	if(mark.tag != FLASH_SECTOR_TAG && mark.tag != FLASH_MAP_TAG) {	  		//invalid sector tag (empty sector)
		return _maze_base_addr;
	}
	base_addr += FLASH_PHYS_PAGE_SIZE;
	_maze_base_addr += FLASH_PHYS_PAGE_SIZE; 
	if(base_addr != 0) goto read_next_sector;
	return 0xFFFF;										//return -1 on not found
}

void ioman_read_buffer(uint16 pos, uchar * bytes, uint16 length) _REENTRANT_ {
	#if 0
	MMU_SEL=1;
	if((pos ^ (pos + length)) & 0x8000) { 
		rP3 = 1;									//update bank 1
		ReadFlash(IOBuf, (pos | 0x8000), 0x8000 - pos);
		rP3 = 2;									//update bank 2
		temp_pos = 0x8000 - pos;
		ReadFlash(IOBuf + temp_pos, 0x8000, (pos + length) - 0x8000);
		//memcpy(bytes, IOBuf, length);
	} else {
		if(pos & 0x8000) {
				rP3 = 2;
				//ptr = pos;
		} else {
				rP3 = 1;
				pos |= 0x8000;
		}
		ReadFlash(IOBuf, pos, length);
	}	
	memcpy(bytes, IOBuf, length);
	#endif
	uint16 paddr = 0;
	uint16 base_addr, bytes_read;
	uint16 offset = 0, p_offset = 0;
	p_offset = (pos % FLASH_LOG_PAGE_SIZE);
	read_next_sector:
	paddr = maze_log2phys(pos);
	base_addr = pos / FLASH_LOG_PAGE_SIZE;				//num of sector
	base_addr = base_addr * FLASH_LOG_PAGE_SIZE;		//base sector address for this address
	bytes_read = (base_addr + FLASH_LOG_PAGE_SIZE) - pos;			//number of bytes that can be read
	paddr = paddr + p_offset;
	if(bytes_read > length) {
		flash_read(paddr, bytes + offset, length);			//read phys address
	} else {
		flash_read(paddr, bytes + offset, bytes_read);			//read phys address
		p_offset = 0;
		pos += bytes_read;
		length = length - bytes_read;
		offset += bytes_read;
		goto read_next_sector;
	}
}

//extern BYTEC	SW[12];
//uint16 ioman_write_buffer(uint16 pos, uchar * bytes, uint16 length) _REENTRANT_ {
uint16 ioman_write_buffer(uint16 pos, uchar * bytes, uint16 length) _REENTRANT_ {
	#if 0
	MMU_SEL=1;
	if((pos ^ (pos + length)) & 0x8000) { 
		rP3 = 1;									//update bank 1
		if(UpdateFlash((pos | 0x8000), bytes, (0x8000 - pos)) != SUCCESS) return APDU_FLASH_WERROR;
		rP3 = 2;									//update bank 2
		temp_pos = 0x8000 - pos;
		//length = (pos + length) - 0x8000;
		if(UpdateFlash(0x8000, bytes + temp_pos, (pos + length) - 0x8000) != SUCCESS) return APDU_FLASH_WERROR;
	} else {
		if(pos & 0x8000) {
				rP3 = 2;
				//ptr = pos;
		} else {
				rP3 = 1;
				pos |= 0x8000;
		}
		//UpdateFlash(ptr, bytes, length);
		//memcopy(IOBuf, bytes, 0, length);
		if(UpdateFlash(pos, bytes, length) != SUCCESS) return APDU_FLASH_WERROR;
	}
	return APDU_SUCCESS;
	#endif
	uint16 status = APDU_SUCCESS;
	uint16 paddr = 0;
	uint16 base_laddr, bytes_write, base_paddr;
	uint16 new_base_paddr;
	uint16 l_offset = 0, p_offset = 0;
	write_next_sector:
	paddr = maze_log2phys(pos);
	base_laddr = pos / FLASH_LOG_PAGE_SIZE;				//num of sector
	base_laddr = base_laddr * FLASH_LOG_PAGE_SIZE;		//base sector address for this address 
	if(paddr == 0xFFFF) {	   							//physical address not found, initialize new flash_sector
		memset(FlashBuffer, 0xff, FLASH_PHYS_PAGE_SIZE);
		((flash_sector *)FlashBuffer)->mark.num = (pos / FLASH_LOG_PAGE_SIZE);
		((flash_sector *)FlashBuffer)->mark.cntr = 0x80;
		((flash_sector *)FlashBuffer)->mark.tag = FLASH_SECTOR_TAG;
		base_paddr = maze_find_empty_sector(); 			//find empty sector
	} else {											//physical address found, read sector physical address
		base_paddr = paddr / FLASH_PHYS_PAGE_SIZE;				//num of sector
		base_paddr = base_paddr * FLASH_PHYS_PAGE_SIZE;		//base sector address for this address
		flash_read(base_paddr, FlashBuffer, FLASH_PHYS_PAGE_SIZE);			//read phys address
	}
	bytes_write = (base_laddr + FLASH_LOG_PAGE_SIZE) - pos;
	new_base_paddr = maze_find_empty_sector();
	if(new_base_paddr == 0xFFFF) new_base_paddr = base_paddr;		//cannot find empty sector, use current sector
	if(new_base_paddr == 0xFFFF) return APDU_FLASH_WERROR;
	if(bytes_write > length) {
		p_offset = pos - base_laddr;
		memcpy(FlashBuffer + p_offset, bytes + l_offset, length);
		//increment counter to prevent redundant data and sequence validator
		if(((flash_sector *)FlashBuffer)->mark.cntr & 0x80) {
			((flash_sector *)FlashBuffer)->mark.cntr = 0;		//recycle counter
		} else {
		   	((flash_sector *)FlashBuffer)->mark.cntr++;
		}
		status = flash_write(new_base_paddr, FlashBuffer);	   				//write new sector
		if(status != APDU_SUCCESS) return status; 
		//maze_cache_laddr = (pos / FLASH_LOG_PAGE_SIZE);
		//maze_cache_paddr = (new_base_paddr / FLASH_PHYS_PAGE_SIZE);
		if(base_paddr != new_base_paddr) {									//check if new sector = old sector
			status = flash_erase(base_paddr);  						 			//erase old sector
			if(status != APDU_SUCCESS) return status;
		}

		/////////////////////    UPDATE MAZE MAP    /////////////////////	 
		p_offset = (pos / FLASH_LOG_PAGE_SIZE);
		if(_maze_map_base_addr == 0xFFFF) {				
			memset(FlashBuffer, 0xff, FLASH_PHYS_PAGE_SIZE);
			((flash_sector *)FlashBuffer)->mark.num = (pos / FLASH_LOG_PAGE_SIZE);
			((flash_sector *)FlashBuffer)->mark.cntr = 0x80;
			((flash_sector *)FlashBuffer)->mark.tag = FLASH_MAP_TAG;
			//_maze_map_base_addr = maze_find_empty_sector(); 			//find empty sector
		} else {
			flash_read(_maze_map_base_addr, FlashBuffer, FLASH_PHYS_PAGE_SIZE);			//read phys address
		}
		*((uint16 *)FlashBuffer + p_offset) = new_base_paddr;					//set new table physical address mapping
		new_base_paddr = maze_find_empty_sector(); 							//find empty sector	for table
		//increment counter to prevent redundant data and sequence validator
		if(((flash_sector *)FlashBuffer)->mark.cntr & 0x80) {
			((flash_sector *)FlashBuffer)->mark.cntr = 0;		//recycle counter
		} else {
		   	((flash_sector *)FlashBuffer)->mark.cntr++;
		}
		status = flash_write(new_base_paddr, FlashBuffer);	   				//write new sector
		if(status != APDU_SUCCESS) return status; 
		base_paddr = _maze_map_base_addr;
		_maze_map_base_addr = new_base_paddr;							//set maze map pointer to new table map
		if(new_base_paddr != base_paddr) {									//check if new sector = old sector
			status = flash_erase(base_paddr);  						//erase old sector (old map)
			if(status != APDU_SUCCESS) return status;
		}
		//////////////////    END OF UPDATE MAZE MAP    //////////////////

		p_offset = 0;
	} else {
		p_offset = pos - base_laddr;
		memcpy(FlashBuffer + p_offset, bytes + l_offset, bytes_write); 
		//increment counter to prevent redundant data and sequence validator
		if(((flash_sector *)FlashBuffer)->mark.cntr & 0x80) {
			((flash_sector *)FlashBuffer)->mark.cntr = 0;		//recycle counter
		} else {
		   	((flash_sector *)FlashBuffer)->mark.cntr++;
		}
		status = flash_write(new_base_paddr, FlashBuffer);		  			//write new sector	 
		if(status != APDU_SUCCESS) return status;
		status = flash_erase(base_paddr);  							   		//erase old sector
		if(status != APDU_SUCCESS) return status;

		/////////////////////    UPDATE MAZE MAP    /////////////////////		 
		p_offset = (pos / FLASH_LOG_PAGE_SIZE);
		if(_maze_map_base_addr == 0xFFFF) {				
			memset(FlashBuffer, 0xff, FLASH_PHYS_PAGE_SIZE);
			((flash_sector *)FlashBuffer)->mark.num = (pos / FLASH_LOG_PAGE_SIZE);
			((flash_sector *)FlashBuffer)->mark.cntr = 0x80;
			((flash_sector *)FlashBuffer)->mark.tag = FLASH_MAP_TAG;
			_maze_map_base_addr = maze_find_empty_sector(); 			//find empty sector
		} else {
			flash_read(_maze_map_base_addr, FlashBuffer, FLASH_PHYS_PAGE_SIZE);			//read phys address
		}
		*((uint16 *)FlashBuffer + p_offset) = new_base_paddr;					//set new table physical address mapping
		new_base_paddr = maze_find_empty_sector(); 							//find empty sector	for table
		//increment counter to prevent redundant data and sequence validator
		if(((flash_sector *)FlashBuffer)->mark.cntr & 0x80) {
			((flash_sector *)FlashBuffer)->mark.cntr = 0;		//recycle counter
		} else {
		   	((flash_sector *)FlashBuffer)->mark.cntr++;
		}
		status = flash_write(new_base_paddr, FlashBuffer);	   				//write new sector
		if(status != APDU_SUCCESS) return status; 	
		base_paddr = _maze_map_base_addr;
		_maze_map_base_addr = new_base_paddr;							//set maze map pointer to new table map
		if(new_base_paddr != base_paddr) {									//check if new sector = old sector
			status = flash_erase(base_paddr);  						//erase old sector (old map)
			if(status != APDU_SUCCESS) return status;
		}
		//////////////////    END OF UPDATE MAZE MAP    //////////////////

		p_offset = 0; 					//clear physical offset
		pos += bytes_write;
		length = length - bytes_write; 
		l_offset += bytes_write;
		goto write_next_sector;
	}

	return status;
} 

void ioman_erase_page(uint16 pos) {
	#if 0
	uint16 ptr = 0;	// = pos + 0x2000;
	MMU_SEL=1;
	if(pos & 0x8000) {
			rP3 = 2;
			ptr = pos;
	} else {
			rP3 = 1;
			ptr = pos | 0x8000;
	}
	Erase_Page((BYTEX *)(ptr & 0xFE00));
	#endif
	flash_erase(pos);
}

uint16 ioman_write_program(uint16 pos, ucharx * bytes, uint16 length) _REENTRANT_ {
	uint16 ptr;
	//uchar i;
	//ptr = pos + 0x8000;
	switch((uchar)(pos >> 12)) {
		case 0:
		case 1:
			rP3=6;	  //XDATA 8000-FFFFh --> 0000-1FFFh
			MMU_SEL = 0x01;
			ptr = pos + 0x8000;
			break;
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			MMU_SEL = 0x01;
			ptr = pos;
			break;
		case 8:
		case 9:
		case 0xA:
		case 0xB:
		case 0xC:
		case 0xD:
		case 0xE:
		case 0xF:
		default:
			rP3 = 0; 
			MMU_SEL = 0x01;
			ptr = pos;
			break;
	}
	if(UpdateFlash(ptr, bytes, length) != SUCCESS) return APDU_FLASH_WERROR;
	return APDU_SUCCESS;
}

uint16 ioman_program_copy(uint16 dest, uint16 src, uint16 length) _REENTRANT_ {
	dest = dest & 0xFE00;
	src = src & 0xFE00;
	length = length & 0xFE00;  
	rP3 = 0; 
	MMU_SEL = 0x01;
	for(length; length != 0; length -= 0x200) {
 		memcpy(FlashBuffer, (BYTEX *)((src + length) - 0x200), 0x200);
		if(Erase_Page((BYTEX *)((dest + length) - 0x200)) != SUCCESS) return APDU_FLASH_WERROR;
		if(Write_Bytes((BYTEX *)((dest + length) - 0x200), FlashBuffer, 0x200) != SUCCESS) return APDU_FLASH_WERROR;
	}
	return APDU_SUCCESS;
}

void ioman_set_to_bootloader(void) {
 	ReturnToBL();
}

void ioman_write_pos(uint16 pos, char byte)
{
	/*#ifdef _X86_ARCH
	fseek(eeprom, pos, SEEK_SET );	
	fputc(byte, eeprom);
	#endif */
	//UpdateFlash(pos,BYTE * RAMbuf,BYTE length)
}

#ifdef _YGGDRASIL_MICRO_KERNEL		//prosedur/fungsi berikut didalam sistem operasi


void ioman_transmit(uchar size, uchar ins, char * buffer, uint16 status) _REENTRANT_
{
	/*#ifdef _X86_ARCH
	uint16 i;
	for(i=0;i<size;i++)
	{
		printf("%02x ", (uchar)buffer[i]);
	}
	printf("SW1 : %02x, SW2 : %02x\n", status/256, status%256);
	#endif */
	//send_byte(ins);						//	Procedure byte in Case 2 APDU,followed with Le data
	if(size > 0) {
		send_byte(ins);
		Tx_n_Bytes(size,buffer);
	}
	Tx_Status(status);
}

#if 0
uchar ioman_receive(uchar * buffer)
{
	/*#ifdef _X86_ARCH
	uchar c;
	uchar i=0;
	ulang_get_key:
	//printf("%i\n", i);
		buffer[i] = 0x00;
		c = gethex();
		if(c==0x10) { goto exit_while; } else {
			buffer[i] = ((c & 0x0F) << 4); 
		}
		c = gethex();
		if(c==0x10) { goto exit_while; } else {
			buffer[i] |= (c & 0x0F); 
		}
		printf(" ");
		i++;
	goto ulang_get_key;
	exit_while:
	printf("\n");
	return i;
	#else
	return 0;
	#endif */
}
#endif
#endif
