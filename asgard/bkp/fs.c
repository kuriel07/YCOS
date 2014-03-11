/* A bottom layer of Asgard File System, which encapsulating internal
 * access to eeprom or flash memory of the file system itself
 *
 * Copyright 2010, Agus Purwanto.
 * All rights reserved.
 *
 * 
 */

#include "..\defs.h"
#include "..\drivers\ioman.h"
//#include "file.h"
//#include "crc.h"
#include "..\misc\mem.h"
//#include "..\misc\barrenkalaheapcalc.h"
#include "..\midgard\midgard.h"
#include "fs.h"
#include "security.h"

//#if ASGARD_VERSION==4
static fs_chain _chunkroot;

uint16 fs_init(void) {
	fs_table * table;
	table = (fs_table *)m_alloc(sizeof(fs_table));
	ioman_read_buffer(0, (uchar *)table, sizeof(fs_table));
	if(table->fs_type[0] == 'A' &&
		table->fs_type[1] == 'S' &&
		table->fs_type[2] == 'G') {
		if(table->fd_ver != ASGARD_VERSION) { m_free(table); return FS_WRONG_VERSION; }
		ioman_read_buffer(ALLOCATION_DATA_OFFSET, &_chunkroot, sizeof(fs_chain));
		m_free(table);
		return FS_INITIALIZED;
	}
	m_free(table);
	return FS_UNFORMATTED;
}

void fs_format(uint32 size) {
	fs_table * table;
	df_header * head;
	//#if FS_ADDRESS_WIDTH == AW_32BIT
	//uint32 address;
	//#else
	uint16 address;
	//#endif
	table = (fs_table *)m_alloc(sizeof(fs_table));
	table->fs_type[0] = 'A';
	table->fs_type[1] = 'S';
	table->fs_type[2] = 'G';
	table->fd_ver = ASGARD_VERSION;
	table->fs_mod = 16;																//16 bit or 32 bit, reserved for future use (default 16)
	table->sector_size = 32;
	table->fs_size = size;
	ioman_write_buffer(0, (uchar *)table, sizeof(fs_table));
	_chunkroot.size = 0;		//chunkroot tidak bisa di s_free
	_chunkroot.next = NULL;
	_chunkroot.prev = ALLOCATION_DATA_OFFSET;
	address = ALLOCATION_DATA_OFFSET + (sizeof(fs_chain) + sizeof(df_header) + (sizeof(df_header) % 4) );
	ioman_write_buffer(address, (uchar *)&_chunkroot, sizeof(fs_chain));
	_chunkroot.prev = 0;
	_chunkroot.size = sizeof(df_header);
	_chunkroot.next = address;
	ioman_write_buffer(ALLOCATION_DATA_OFFSET, (uchar *)&_chunkroot, sizeof(fs_chain));
	address = ALLOCATION_DATA_OFFSET + sizeof(fs_chain);
	head = (df_header *)m_alloc(sizeof(df_header));
	head->parent = 0;
	head->sibling = 0;
	head->child = 0;											//not use, only for cyclic
	head->fid = FID_MF;
	head->type = T_MF;
	head->length = 9;											//length of the following data
	head->file_char = 1;										//not clarified
	head->num_of_df = 0;
	head->num_of_ef = 0;
	head->num_of_chv = 2;										//just set to 2
	ioman_write_buffer(address, (uchar *)head, sizeof(df_header));		//write the specified allocated address
	m_free(head);
	m_free(table);
	//#ifdef _X86_ARCH										//PC only simulation
	chv_create(1, "1234", "12345678", 3, 10);			//create chv 1
	chv_create(2, "1234", "12345678", 3, 10);			//create chv 2
	chv_create(4, "12345678", "12345678", 100, 100);			//create chv 4 (admin login)
	chv_create(6, "12345678", "12345678", 100, 100);			//create chv 6 (system login)
	//#endif
}

//#if FS_ADDRESS_WIDTH == AW_32BIT
///uint32 fs_alloc(uint32 size) {
//	uint32 alloc_pos;
//	uint32 alloc_pos_temp;
//	uint32 candidate_pos;
//#else
uint16 fs_alloc(uint16 size) {
	uint16 alloc_pos;
	uint16 alloc_pos_temp;
	uint16 candidate_pos;
//#endif
	fs_chain * alloc_ptr = (fs_chain *)m_alloc(sizeof(fs_chain));
	fs_chain * alloc_ptr_temp = (fs_chain *)m_alloc(sizeof(fs_chain));
	fs_chain * candidate = (fs_chain *)m_alloc(sizeof(fs_chain));
	alloc_pos = ALLOCATION_DATA_OFFSET;
	//printf("aaa\n");
	memcopy(alloc_ptr, &_chunkroot, 0, sizeof(fs_chain));
	//alloc_chain *temp;
	//#ifdef OS_uCOS_II_H
	//OS_ENTER_CRITICAL();
	//#endif
	//printf("aa\n");
	size = size + (4 - (size % 4));		//4 byte align
	get_next_chain:
	if(alloc_ptr->next == 0) {		//allocate new chunk at the end of the heap

	//printf("alloc_ptr=null\n");
		alloc_ptr->size = size;
		alloc_ptr->next = (alloc_pos + sizeof(alloc_chain) + size);
		//disini cek STACK POINTER, dikhawatirkan akan bertabrakan dengan stack, sebaiknya antara stack pointer dan heap dibuat
		//GAP yang cukup lebar karena dikhawatirkan ukuran stack akan bertambah setelah keluar dari fungsi s_alloc karena automatic allocation
		alloc_ptr->prev = alloc_pos_temp;
		//alloc_ptr_temp = alloc_ptr->next;	//berubah fungsi untuk menjadi pointer chunk selanjutnya
		alloc_pos_temp = alloc_ptr->next;
		alloc_ptr_temp->prev = alloc_pos;
		alloc_ptr_temp->next = NULL;
		alloc_ptr_temp->size = 0;
		ioman_write_buffer(alloc_pos_temp, alloc_ptr_temp, sizeof(fs_chain));
		ioman_write_buffer(alloc_pos, alloc_ptr, sizeof(fs_chain));
		//alloc_ptr = alloc_ptr_temp;
		//#ifdef MIDGARD_DEBUG_ACTIVATED
		//printf(" * address of new chunk at %ld\n", (alloc_chain *)((intptr)alloc_ptr));
		//#endif
		//Uart_Printf("%d\n", alloc_ptr);
		//#ifdef OS_uCOS_II_H
		//OS_EXIT_CRITICAL();
		//#endif
		//_total_heap += alloc_ptr->size;
		//if(_total_heap > _maximum_total_heap) {
		//	printf("Maximum total heap : %d\n", _total_heap);
		//	_maximum_total_heap = _total_heap;
		//}
		m_free(candidate);
		m_free(alloc_ptr_temp);
		m_free(alloc_ptr);
		return (alloc_pos + sizeof(fs_chain));	//return pointer sekarang + ukuran header karena *[header]+[body]
	}
	//temp = alloc_ptr->next;		//isi dengan pointer chunk sebelumnya
	//alloc_ptr_temp = alloc_ptr;
	alloc_pos_temp = alloc_pos;
	memcopy(alloc_ptr_temp, alloc_ptr, 0, sizeof(fs_chain));
	//alloc_ptr = alloc_ptr->next;
	alloc_pos = alloc_ptr->next;
	ioman_read_buffer(alloc_ptr->next, alloc_ptr, sizeof(fs_chain));
	if(alloc_pos >= (alloc_pos_temp + alloc_ptr_temp->size + sizeof(fs_chain) + sizeof(fs_chain) + size + (size % 4))) {
		//printf("allocate in previous memory : %x, %x\n", (uint32)alloc_ptr, ((uint32)alloc_ptr_temp + alloc_ptr_temp->size + sizeof(alloc_chain) + sizeof(alloc_chain) + size + (size % 4)));
		//printf("allocate in previous memory : %x, %x\n", (intptr)alloc_ptr, ((intptr)alloc_ptr_temp + alloc_ptr_temp->size + sizeof(alloc_chain)));
		//printf("allocate in previous memory : %x, %x\n", (uint32)alloc_ptr, sizeof(alloc_chain));
		//allocate new heap using FFA (First Fit Algorithm)
		//Uart_Printf("allocate previous heap\n");
		//candidate = (fs_chain *)((intptr)alloc_ptr_temp + alloc_ptr_temp->size + sizeof(alloc_chain));
		candidate_pos = (alloc_pos_temp + alloc_ptr_temp->size + sizeof(alloc_chain));
		//ioman_read_buffer(candidate_pos, candidate, sizeof(fs_chain));
		//#ifdef MIDGARD_DEBUG_ACTIVATED
		//printf(" * creating new chunk with size %i\n", size);
		//#endif
		candidate->size = size + (size % 4);
		alloc_ptr_temp->next = candidate_pos;
		candidate->prev = alloc_pos_temp;
		candidate->next = alloc_pos;
		alloc_ptr->prev = candidate_pos;
		ioman_write_buffer(candidate_pos, candidate, sizeof(fs_chain));
		ioman_write_buffer(alloc_pos_temp, alloc_ptr_temp, sizeof(fs_chain));
		ioman_write_buffer(alloc_pos, alloc_ptr, sizeof(fs_chain));
		//Uart_Printf("%d %d %d %d\n", candidate->prev, candidate, candidate->size, candidate->next);
		//#ifdef MIDGARD_DEBUG_ACTIVATED
		//printf(" * address of new chunk at %x\n", (uint32)candidate);
		//#endif
		//#ifdef OS_uCOS_II_H
		//Uart_Printf("exit\n");
		//OS_EXIT_CRITICAL();
		//#endif
		//_total_heap += candidate->size;
		//if(_total_heap > _maximum_total_heap) {
		//	printf("Maximum total heap : %d\n", _total_heap);
		//	_maximum_total_heap = _total_heap;
		//}
		//return (void *)((intptr)candidate + (intptr)sizeof(alloc_chain));
		m_free(candidate);
		m_free(alloc_ptr_temp);
		m_free(alloc_ptr);
		return candidate_pos + sizeof(fs_chain);
	}
	goto get_next_chain;
}

//#if FS_ADDRESS_WIDTH == AW_32BIT
//void fs_dealloc(uint32 address) {
//	uint32 alloc_pos;
//	uint32 alloc_prev_pos;
//	uint32 alloc_next_pos;
//#else
void fs_dealloc(uint16 address) {
	uint16 alloc_pos;
	uint16 alloc_prev_pos;
	uint16 alloc_next_pos;
//#endif
	//#ifdef OS_uCOS_II_H
	//#if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    //OS_CPU_SR  cpu_sr;
    //#endif
    //#endif
	fs_chain * alloc_ptr = (fs_chain *)m_alloc(sizeof(fs_chain));
	fs_chain * alloc_prev = (fs_chain *)m_alloc(sizeof(fs_chain));
	fs_chain * alloc_next = (fs_chain *)m_alloc(sizeof(fs_chain));
	//alloc_chain *alloc_ptr = (alloc_chain *)((intptr)ptr - (intptr)sizeof(alloc_chain));
	alloc_pos = address - sizeof(fs_chain);
	ioman_read_buffer(alloc_pos, alloc_ptr, sizeof(fs_chain));
	//alloc_chain *alloc_prev = alloc_ptr->prev;
	alloc_prev_pos = alloc_ptr->prev;
	ioman_read_buffer(alloc_prev_pos, alloc_prev, sizeof(fs_chain));
	//alloc_chain *alloc_next = alloc_ptr->next;
	alloc_next_pos = alloc_ptr->next;
	ioman_read_buffer(alloc_next_pos, alloc_next, sizeof(fs_chain));

	if(address == 0) return;
	if(alloc_prev == 0) return;		//this memory is already freed
	if(alloc_ptr->size == 0) return;		//this memory is already freed
	if(alloc_next_pos == 0) {		//this is the last chunk
		//printf("this is the last chunk\n");	
	}
	//#ifdef OS_uCOS_II_H
	//OS_ENTER_CRITICAL();
	//#endif
	//alloc_prev = alloc_ptr->prev;
	#ifdef MIDGARD_DEBUG_ACTIVATED
	//printf(" * delete chunk at %x\n", (intptr)alloc_ptr);
	//printf(" * chunk at %x, next chunk at ", (intptr)alloc_prev);
	#endif
	//_total_heap -= alloc_ptr->size;
	//alloc_ptr = alloc_ptr->next;
	if(alloc_pos == ALLOCATION_DATA_OFFSET) {
		//alloc_prev->next = alloc_ptr->next;
		//alloc_prev->next->prev = alloc_prev;
		#ifdef MIDGARD_DEBUG_ACTIVATED
		//printf("%x\n", (intptr)alloc_prev->next);
		#endif
	} else { 
		alloc_next->prev = alloc_prev_pos;
		alloc_prev->next = alloc_next_pos;
		//alloc_prev->next->prev = alloc_prev;
		#ifdef MIDGARD_DEBUG_ACTIVATED
		//printf("%x\n", (intptr)alloc_prev->next);
		#endif
		//s_gc();
		//getch();
	}
	alloc_ptr->next = NULL;
	alloc_ptr->prev = NULL;
	alloc_ptr->size = 0;
	ioman_write_buffer(alloc_pos, alloc_ptr, sizeof(fs_chain));
	ioman_write_buffer(alloc_prev_pos, alloc_prev, sizeof(fs_chain));
	ioman_write_buffer(alloc_next_pos, alloc_next, sizeof(fs_chain));
	//#ifdef OS_uCOS_II_H
	//OS_EXIT_CRITICAL();
	//#endif

}

//#endif
