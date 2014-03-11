/* A bottom layer of Asgard File System, which encapsulating internal
 * access to eeprom or flash memory of the file system itself
 *
 * Copyright 2010, Agus Purwanto.
 * All rights reserved.
 *
 * 
 */

#include "..\defs.h"
#include "fs.h"
#include "..\drivers\ioman.h"
//#include "file.h"
#include "security.h"
//#include "crc.h"
#include "..\misc\mem.h"
//#include "..\misc\barrenkalaheapcalc.h"
#include "..\midgard\midgard.h"
#include <string.h>
//#include "..\NORFlash\NORFlash.h"

//ef_table table;

//static uchar _fs_buffer[FS_BUFFER_SIZE];
#if ASGARD_VERSION==3
fs_handle * fs_init(hw_rca * rca)
{
	uchar fs_buffer[SECTOR_SIZE];								//we didn't know the actual sector size of the file system, use default (32, minimum)
	fs_handle * fs = malloc(sizeof(fs_handle));
	ioman_read(rca, 0, fs_buffer, SECTOR_SIZE);
	m_memcopy(&fs->fs_table, fs_buffer, sizeof(ef_table));		//initialize file system table
	fs->current_alloc_ptr = ALLOCATION_TABLE_OFFSET;			//set to start of alloc table
	if(fs->fs_table.fs_type[0] == 'A' 
		&& fs->fs_table.fs_type[1] == 'S' 
		&& fs->fs_table.fs_type[2] == 'G') {					//valid asgard system
		if(fs->fs_table.fd_ver == ASGARD_VERSION) {				//check for asgard 3.0 (compatible)
			fs->current_alloc_ptr = 0;
			return fs;
		}
	}
	free(fs);													//cannot initialize file system, return null
	return NULL;
}

uint16 fs_get_next_available_sector(fs_handle * fs)
{
	uint16 sector_size = (uint16)actual_sector_size(fs->fs_table.sector_size);	
	uint16 i, offset, sector_offset;
	uint16 pos;
	uint16 last_available_pos = fs->current_alloc_ptr + ALLOCATION_TABLE_OFFSET;	//start scanning position
	//calculate maximum alloc table size using partition size, not all alloc table is used
	uint16 alloc_data_offset = ALLOCATION_TABLE_OFFSET + ((fs->fs_table.fs_size / sector_size) * sizeof(uint16));		
	uchar * fs_buffer = (uchar *)malloc(sector_size);
	for(i = last_available_pos;i<alloc_data_offset;i+=sector_size) {				//scanning for free space
		ioman_read(fs->rca, i, fs_buffer, sector_size);
		for(offset=0, sector_offset=0;offset<sector_size;offset+=sizeof(uint16), sector_offset++) {
			pos = *(uint16 *)(fs_buffer + offset);
			if(pos == FS_VOID) {
				free(fs_buffer);													//don't forget to freed buffer before return
				//(sector_index * sector_size) + offset, update alloc_ptr
				fs->current_alloc_ptr = (i - ALLOCATION_TABLE_OFFSET);
				return (((i - ALLOCATION_TABLE_OFFSET) * sector_size) + sector_offset);
			}
		}
	}
	fs->current_alloc_ptr = 0;			
	alloc_data_offset = ALLOCATION_TABLE_OFFSET;									//set alloc_ptr to start and see if there's some free space
	for(i = alloc_data_offset;i<last_available_pos;i+=sector_size) {				//scanning for free space
		ioman_read(fs->rca, i, fs_buffer, sector_size);
		for(offset=0, sector_offset=0;offset<sector_size;offset+=sizeof(uint16), sector_offset++) {
			pos = *(uint16 *)(fs_buffer + offset);
			if(pos == FS_VOID) {
				free(fs_buffer);													//don't forget to freed buffer before return
				//(sector_index * sector_size) + offset, update alloc_ptr
				fs->current_alloc_ptr = (i - ALLOCATION_TABLE_OFFSET);
				return (((i - ALLOCATION_TABLE_OFFSET) * sector_size) + sector_offset);
			}
		}
	}
	fs->current_alloc_ptr = 0;									//i give up there's no available space
	free(fs_buffer);											//don't forget to freed buffer before return
	return FS_NO_AVAILABLE_SPACE;								//no available space (0xFFFF, reserved)
}

void fs_set_next_sector(fs_handle * fs, uint16 sector_index, uint16 next_sector)
{
	uint16 sector_size = (uint16)actual_sector_size(fs->fs_table.sector_size);
	uint16 actual_address = ALLOCATION_TABLE_OFFSET + (sector_index * sizeof(uint16));		//actual address in plain segment
	uint16 sector_address = actual_address / sector_size;							
	uint16 offset = actual_address % sector_size;
	uchar * fs_buffer = (uchar *)malloc(sector_size);
	ioman_read(fs->rca, sector_address, fs_buffer, sector_size);					//read a block of alloc table
	*(uint16 *)(fs_buffer + offset) = next_sector;									//set value
	ioman_write(fs->rca, sector_address, fs_buffer, sector_size);					//write the new block to alloc table
	//
	memset(fs_buffer, 0, sector_size);												//clear buffer
	//actual_address = ALLOCATION_DATA_OFFSET + (next_sector * sector_size);			//actual address in plain segment
	fs_write_sector(fs, next_sector, fs_buffer, sector_size);					//clear next sector contents, prevent any access to the previous data
	free(fs_buffer);																//just freed the buffer before return
}

uint16 fs_get_next_sector(fs_handle * fs, uint16 sector_index) {
	uint16 sector_size = actual_sector_size(fs->fs_table.sector_size);
	uint16 actual_address = ALLOCATION_TABLE_OFFSET + (sector_index * sizeof(uint16));		//actual address in plain segment
	uint16 sector_address = actual_address / sector_size;														
	uint16 offset = actual_address % sector_size;
	uchar * fs_buffer = (uchar *)malloc(sector_size);
	ioman_read(fs->rca, sector_address, fs_buffer, sector_size);					//read a block of alloc table
	sector_index = *(uint16 *)(fs_buffer + offset);
	free(fs_buffer);																//just freed the buffer before return
	return sector_index;															//biar ngirit stack, variable yang ndak dipake direuse
}

uint16 fs_read_sector(fs_handle * fs, uint16 sector_index, uchar * buffer, uint16 sector_size) {
	//uint16 sector_size = (uint16)actual_sector_size(fs->fs_table.sector_size);
	uint16 actual_address = ALLOCATION_DATA_OFFSET + (sector_index * sector_size);	//actual address in plain segment
	//uint16 r_size = (size > sector_size)?sector_size:size;
	return ioman_read(fs->rca, actual_address, buffer, sector_size);
	//return sector_size;		//return number of bytes readed
}

uint16 fs_write_sector(fs_handle * fs, uint16 sector_index, uchar * buffer, uint16 sector_size) {
	//uint16 sector_size = (uint16)actual_sector_size(fs->fs_table.sector_size);
	uint16 actual_address = ALLOCATION_DATA_OFFSET + (sector_index * sector_size);	//actual address in plain segment
	//uint16 w_size = (size > sector_size)?sector_size:size;
	//printf("address : %x, %x %x %x %x, %i\n", actual_address, buffer[0], buffer[1], buffer[2], buffer[3], sector_size); getch();
	return ioman_write(fs->rca, actual_address, buffer, sector_size);
	//return sector_size;		//return number of bytes wrote
}

fs_entry * fs_create_entry(fs_handle * fs, uchar type, uint16 fid, uint16 first_sector_index) {
	uint16 sector_size = (uint16)actual_sector_size(fs->fs_table.sector_size);
	uint16 sector_index = first_sector_index;
	uint16 next_sector, entry_sector;
	fs_entry * i_entry;
	fs_entry * entry = NULL;
	uchar * fs_buffer = (uchar *)malloc(sector_size);
	uint16 i;
	//printf("first sector index : %x\n", first_sector_index);
	while(1) {
		fs_read_sector(fs, sector_index, fs_buffer, sector_size);
		for(i=0;i<sector_size;i+=sizeof(fs_entry)) {
			i_entry = (fs_entry *)(fs_buffer + i);
			//printf("scanned entry : %x\n", i_entry->fid);
			if((i_entry->type & FS_TYPE_EXIST) == 0) {								//this entry is non existent, replace with the new entry and writeback
				i_entry->type = type | FS_TYPE_EXIST;
				i_entry->fid = fid;
				i_entry->size = 0;
				//printf("create new entry fid : %x at sector : %x\n", i_entry->fid, sector_index);getch();
				if(type & FS_TYPE_DIR) {
					entry_sector = fs_get_next_available_sector(fs);
					if(entry_sector == FS_NO_AVAILABLE_SPACE) {
						//printf("entry cannot be created\n");
						goto exit_fsc_entry;										//entry cannot be created
					} else {
						i_entry->ptr = entry_sector;
						fs_set_next_sector(fs, entry_sector, FS_EOS);
					}
				} else {
					i_entry->ptr = FS_EOS;											//didn't have any data sector
				}
				//printf("write new sector at sector index : %x, %x\n", sector_index, i_entry->fid); getch();
				//printf("sector_size : %x\n", sector_size);getch();
				fs_write_sector(fs, sector_index, fs_buffer, sector_size);			//write back to the specified sector
				entry = (fs_entry *)malloc(sizeof(fs_entry));
				m_memcopy(entry, i_entry, sizeof(fs_entry));
				goto exit_fsc_entry;
			}
		}
		next_sector = fs_get_next_sector(fs, sector_index);							//get next sector
		if(next_sector == FS_EOS) {													//check if next sector is unavailable
			next_sector = fs_get_next_available_sector(fs);							//try to look for free sector
			if(next_sector == FS_EOS) {												//no available space
				goto exit_fsc_entry;												//quit create entry operation
			}
			fs_set_next_sector(fs, next_sector, FS_EOS);
			fs_set_next_sector(fs, sector_index, next_sector);						//set the new sector as next sector
		}
		sector_index = next_sector;
	}
	exit_fsc_entry:
	free(fs_buffer);
	return entry;
}

void fs_update_entry(fs_handle * fs, fs_entry * entry, uint16 first_sector_index) {
	uint16 sector_size = (uint16)actual_sector_size(fs->fs_table.sector_size);
	uint16 sector_index = first_sector_index;
	uint16 next_sector;
	fs_entry * i_entry;
	uchar * fs_buffer = (uchar *)malloc(sector_size);
	uint16 i;
	while(1) {
		fs_read_sector(fs, sector_index, fs_buffer, sector_size);
		for(i=0;i<sector_size;i+=sizeof(fs_entry)) {
			i_entry = (fs_entry *)(fs_buffer + i);
			if(i_entry->fid == entry->fid) {									//entry found, update type, size and ptr
				i_entry->type = entry->type;
				i_entry->rsv = entry->rsv;
				i_entry->size = entry->size;
				i_entry->ptr = entry->ptr;										//didn't have any data sector
				fs_write_sector(fs, sector_index, fs_buffer, sector_size);		//write back to the specified sector
				goto exit_fsu_entry;
			}
		}
		next_sector = fs_get_next_sector(fs, sector_index);						//get next sector
		if(next_sector == FS_EOS) {												//no available space
			goto exit_fsu_entry;												//quit create entry operation
		}
		sector_index = next_sector;
	}
	exit_fsu_entry:
	free(fs_buffer);
}

void fs_delete_entry(fs_handle * fs, fs_entry * entry, uint16 first_sector_index) {
	entry->type &= ~FS_TYPE_EXIST;													//set type to non existent
	entry->size = 0;
	fs_unlink_sector_chain(fs, entry->ptr);											//unlink all sector chain (data chain)
	fs_update_entry(fs, entry, first_sector_index);									//update entry
}

void fs_unlink_sector_chain(fs_handle * fs, uint16 first_sector_index) {
	uint16 next_sector;
	uint16 sector_index = first_sector_index;										//set to first sector
	while(sector_index != FS_EOS) {													//check for end of sector
		next_sector = fs_get_next_sector(fs, sector_index);
		fs_set_next_sector(fs, sector_index, FS_VOID);								//set link to void(empty) and also clear it content
		sector_index = next_sector;
	}
}

fs_file * fs_root(fs_handle * fs) {													//automatically return handle to the root file system, useful for creating master file
	fs_file * file = (fs_file *)malloc(sizeof(fs_file));						
	file->fs = fs;
	file->entry.type = FS_TYPE_DIR | FS_TYPE_USER | FS_TYPE_SYS | FS_TYPE_EXIST;
	file->entry.fid = 0;
	file->entry.size = 0;
	file->entry.ptr = 0;															//always 0 for root file system
	file->sector_index = 0;
	file->data_ptr = 0;
	return file;
}

fs_file * fs_fopen_q(fs_handle * fs, uint16 * fids, uchar lsize, uchar mode) {
	uint16 sector_size = (uint16)actual_sector_size(fs->fs_table.sector_size);
	uint16 sector_index = 0;
	uint16 first_sector_index = sector_index;
	uint16 fid;
	uchar i;
	uint16 j;
	fs_entry * entry;
	fs_file * file = NULL;
	uchar * fs_buffer = (uchar *)malloc(sector_size);
	for(i=0;i<lsize;i++) {
		fid = *(uint16 *)(fids + (i * sizeof(uint16)));
		fs_read_sector(fs, sector_index, fs_buffer, sector_size);
		sector_index = fs_get_next_sector(fs, sector_index);						//get next sector (if available)
		for(j=0;j<sector_size;j+=sizeof(fs_entry)) {								//check for all entry list in the specified sector
			entry = (fs_entry *)(fs_buffer + j);
			if(entry->fid == fid) {													//check mode here
				if(entry->type & FS_TYPE_EXIST && (entry->type & mode) != 0) {		//file exist and accessible
					if(i<(lsize - 1)) {												//the selected file
						file = (fs_file *)malloc(sizeof(fs_file));					//the selected file, automatically create fs_file
						file->fs = fs;
						m_memcopy(&file->entry, entry, sizeof(fs_entry));
						file->sector_index = first_sector_index;
						file->data_ptr = 0;
						goto exit_fsopen;
					} else if(entry->type & FS_TYPE_DIR) {							//check if directory, then iterate through it
						sector_index = entry->ptr;
						first_sector_index = sector_index;							//change first_sector_index to the new directory
					}
				}
			}
		}
		if(sector_index == FS_EOS) {												//have no next sector
			goto exit_fsopen;
		}
	}
	exit_fsopen:
	free(fs_buffer);
	return file;
}

fs_file * fs_fopen(fs_file * parent, uint16 fid, uchar mode) {
	uint16 sector_size;
	uint16 sector_index;
	uint16 j;
	fs_entry * entry;
	fs_file * file = NULL;
	uchar * fs_buffer;
	sector_index = parent->entry.ptr;
	sector_size = (uint16)actual_sector_size(parent->fs->fs_table.sector_size);
	m_mem_dump();
	fs_buffer = (uchar *)malloc(sector_size);
	//printf("sector_size : %i\n", sector_size);
	if(parent == NULL) goto exit_fsopen;											//check for parent
	while(sector_index != FS_EOS) {
		//printf("parent : %x\n", parent);
		//m_mem_dump();
		fs_read_sector(parent->fs, sector_index, fs_buffer, sector_size);
		//printf("ggg fs.c line 291 %x %x\n", parent, fs_buffer);
		sector_index = fs_get_next_sector(parent->fs, sector_index);				//get next sector (if available)
		for(j=0;j<sector_size;j+=sizeof(fs_entry)) {								//check for all entry list in the specified sector
			entry = (fs_entry *)(fs_buffer + j);
			//printf("entry->fid : %x\n", entry->fid);
			if(entry->fid == fid) {													//fid found
				//printf("file mode : %x, current mode :%x\n\n\n\n", entry->type, mode);
				if(entry->type & FS_TYPE_EXIST && (entry->type & mode) != 0) {		//file exist and accessible												//the selected file
					file = (fs_file *)malloc(sizeof(fs_file));						//the selected file, automatically create fs_file
					file->fs = parent->fs;
					m_memcopy(&file->entry, entry, sizeof(fs_entry));
					file->sector_index = parent->sector_index;
					file->data_ptr = 0;
					goto exit_fsopen;
				}
			}
		}
		getch();
		if(sector_index == FS_EOS) {												//have no next sector
			goto exit_fsopen;
		}
	}
	exit_fsopen:
	free(fs_buffer);
	return file;
}

uint16 fs_fwrite(fs_file * file, uchar * buffer, uint16 size) {
	uint16 sector_size = (uint16)actual_sector_size(file->fs->fs_table.sector_size);
	uint16 sector_index = file->entry.ptr;								//first data sector
	uint16 elapsed_sector = file->data_ptr / sector_size;				//number of sector elapsed until data_ptr
	uint16 sector_offset = file->data_ptr % sector_size;				//offset of sector to the data at the specified data_ptr
	uint16 next_sector;													//next sector index
	uint16 w_size = 0;													//number of bytes wrote
	uint16 l_size;														//local size for memory copy read->copy->write
	uint16 i;															
	uchar * fs_buffer;
	if(file->entry.type & FS_TYPE_DIR) return 0;						//unable to write directory
	if(sector_index == FS_EOS) {
		next_sector = fs_get_next_available_sector(file->fs);			//try to look for free sector
		if(next_sector == FS_EOS) {										//no available space
			goto exit_write_sequence;									//quit write operation
		}
		fs_set_next_sector(file->fs, next_sector, FS_EOS);
		file->entry.ptr = next_sector;									//change entry data ptr
		sector_index = next_sector;					
		fs_update_entry(file->fs, &file->entry, file->sector_index);		//update entry
		file->data_ptr = 0;												//clear file->data_ptr if one already exist
		elapsed_sector = file->data_ptr / sector_size;					//re-calculate elapsed_sector
		sector_offset = file->data_ptr % sector_size;					//re-calculate sector_offset
	}
	fs_buffer = (uchar *)malloc(sector_size);							//allocate new buffer
	for(i=0;i<elapsed_sector;i++) {										//set sector_index
		sector_index = fs_get_next_sector(file->fs, sector_index);
	}
	//for(i=0;i<size;i+=sector_size) {
	while(size > 0) {
		fs_read_sector(file->fs, sector_index, fs_buffer, sector_size);
		l_size = (sector_size - sector_offset);
		l_size = (size > l_size)? l_size : size;
		size -= l_size;
		m_memcopy((uchar *)(fs_buffer + sector_offset), (uchar *)(buffer + w_size), l_size);
		w_size += l_size;
		fs_write_sector(file->fs, sector_index, fs_buffer, sector_size);
		next_sector = fs_get_next_sector(file->fs, sector_index);
		if((int32)size > 0 && next_sector == FS_EOS) {					//jika next sector tidak tersedia dan masih ada data yang tersisa, the use of uint16 might cause overflow so cast it to int32
			next_sector = fs_get_next_available_sector(file->fs);		//try to look for free sector
			if(next_sector == FS_EOS) {									//no available space
				goto exit_write_sequence;
			}
			fs_set_next_sector(file->fs, next_sector, FS_EOS);			//set next_sector->next = FS_EOS, the sector->next might be 0x0000, this prevent the concatenation with sector index 0
			fs_set_next_sector(file->fs, sector_index, next_sector);	//set the new sector as next sector
		}
		sector_index = next_sector;
		sector_offset = 0;														//clear offset
	}
	exit_write_sequence:
	if((file->data_ptr + w_size) > file->entry.size) {					//check if number of bytes wrote exceeds the file size
		file->entry.size = (file->data_ptr + w_size);					//change entry size
		fs_update_entry(file->fs, &file->entry, file->sector_index);		//update entry
	}
	file->data_ptr = (file->data_ptr + w_size);							//update file->data_ptr for next operation
	free(fs_buffer);
	return w_size;
}

uint16 fs_fread(fs_file * file, uchar * buffer, uint16 size) {
	uint16 sector_size = (uint16)actual_sector_size(file->fs->fs_table.sector_size);
	uint16 sector_index = file->entry.ptr;								//first data sector
	uint16 elapsed_sector = file->data_ptr / sector_size;				//number of sector elapsed until data_ptr
	uint16 sector_offset = file->data_ptr % sector_size;				//offset of sector to the data at the specified data_ptr
	uint16 next_sector;													//next sector index
	uint16 r_size = 0;													//number of bytes wrote
	uint16 l_size;														//local size for memory copy read->copy
	uint16 i;															
	uchar * fs_buffer;
	if(file->entry.type & FS_TYPE_DIR) return 0;						//unable to read directory
	if(sector_index == FS_EOS) {
		return 0;
	}
	fs_buffer = (uchar *)malloc(sector_size);							//allocate new buffer
	for(i=0;i<elapsed_sector;i++) {										//set sector_index
		sector_index = fs_get_next_sector(file->fs, sector_index);
	}
	//for(i=0;i<size;i+=sector_size) {
	while(size > 0) {
		fs_read_sector(file->fs, sector_index, fs_buffer, sector_size);
		l_size = (sector_size - sector_offset);
		l_size = (size > l_size)? l_size : size;
		size -= l_size;
		m_memcopy((uchar *)(buffer + r_size), (uchar *)(fs_buffer + sector_offset), l_size);
		r_size += l_size;
		next_sector = fs_get_next_sector(file->fs, sector_index);
		if(next_sector == FS_EOS) {										//jika next sector tidak tersedia
			goto exit_read_sequence;
		}
		sector_index = next_sector;
		sector_offset = 0;														//clear offset
	}
	exit_read_sequence:
	file->data_ptr = (file->data_ptr + r_size);							//update file->data_ptr for next operation
	free(fs_buffer);
	return r_size;
}


//format and initialize file system
fs_handle * fs_format(hw_rca * rca, uint32 partition_size, uchar requested_sector_size)
{
	uchar allocated_sector_size;
	ef_table * fs_table;
	//uint16 alloc_data_offset = ALLOCATION_TABLE_OFFSET + ((partition_size / sector_size) * sizeof(uint16));
	//check for partition_size = f(sector_size)
	uint32 maximum_partition;
	uchar * fs_buffer;
	uint16 sector_size;
	uint16 i;
	fs_handle * fs;
	fs_file * root;
	partition_size = partition_size - ALLOCATION_DATA_OFFSET;		//when user asked for 64K then the actual partition size is 64K-2K
	//the system automatically corrected the size of sector size according to the partition size requested, maximum sector size = 2048
	for(allocated_sector_size=requested_sector_size;allocated_sector_size < 128;allocated_sector_size<<=1) {
		sector_size = (uint16)actual_sector_size(allocated_sector_size);
		maximum_partition = (ALLOCATION_TABLE_SIZE / sizeof(uint16)) * sector_size;		//calculate maximum partition size
		if(maximum_partition > partition_size) break;									//check for partition size capability
	}
	fs_buffer = (uchar *)malloc(sector_size);
	memset(fs_buffer, 0, allocated_sector_size);										//clear buffer for alloc table and file system conf
	for(i=0;i<ALLOCATION_DATA_OFFSET;i+=sector_size) {
		ioman_write(rca, i, fs_buffer, sector_size);
	}
	fs_table = (ef_table *)malloc(sizeof(ef_table));
	fs_table->fs_type[0] = 'A';
	fs_table->fs_type[1] = 'S';
	fs_table->fs_type[2] = 'G';
	fs_table->fd_ver = ASGARD_VERSION;
	fs_table->fs_mod = 16;																//16 bit or 32 bit, reserved for future use (default 16)
	fs_table->sector_size = allocated_sector_size;
	fs_table->fs_size = partition_size;
	ioman_write(rca, 0, (uchar *)fs_table, sizeof(ef_table));
	free(fs_table);																	//freed fs_table, we didn't need this anymore
	free(fs_buffer);																	//freed fs_buffer, we didn't need this anymore
	fs =  fs_init(rca);																	//initialize file system
	//printf("next available sector : %x\n", fs_get_next_available_sector(fs));
	fs_set_next_sector(fs, 0, FS_EOS);													//allocate sector 0 for root file system
	//fs_entry * fs_create_entry(fs, FS_TYPE_USER | FS_TYPE_DIR | FS_TYPE_SYS | FS_TYPE_EXIST, 0x3f00, 0);
	root = fs_root(fs);
	free(fs_mkdir(root, 0x3f00, FS_TYPE_USER | FS_TYPE_DIR | FS_TYPE_SYS));
	free(root);
	return fs;
}

fs_file * fs_mkdir(fs_file * parent, uint16 fid, uchar mode) {
	fs_entry * entry;
	fs_file * file;
	if((parent->entry.type & FS_TYPE_DIR) == 0) return NULL;
	mode = mode | FS_TYPE_DIR | FS_TYPE_EXIST;
	file = (fs_file *)malloc(sizeof(fs_file));
	entry = fs_create_entry(parent->fs, mode, fid, parent->entry.ptr);
	file->fs = parent->fs;
	m_memcopy(&file->entry, entry, sizeof(fs_entry));
	file->sector_index = parent->entry.ptr;
	file->data_ptr = 0;
	free(entry);
	return file;
}

fs_file * fs_fcreate(fs_file * parent, uint16 fid, uchar mode) {
	fs_entry * entry;
	fs_file * file;
	if((parent->entry.type & FS_TYPE_DIR) == 0) return NULL;
	mode |= FS_TYPE_EXIST;
	mode &= ~FS_TYPE_DIR;
	file = (fs_file *)malloc(sizeof(fs_file));
	printf("parent->entry.ptr : %x\n", parent->entry.ptr);getch();
	entry = fs_create_entry(parent->fs, mode, fid, parent->entry.ptr);
	file->fs = parent->fs;
	m_memcopy(&file->entry, entry, sizeof(fs_entry));
	file->sector_index = parent->entry.ptr;
	file->data_ptr = 0;
	free(entry);
	return file;
}

void fs_rmfile(fs_file * file, uchar mode) {
	if(file->entry.type & FS_TYPE_EXIST && (file->entry.type & mode) != 0)
		fs_delete_entry(file->fs, &file->entry, file->sector_index);
}

uint16 fs_freespace()
{
	//ambil space bebas memanfaatkan allocation table
	//nilai yang direturn adalah jumlah cluster yang bebas
	return 0;
	
}

//reserve for future use, lebih efektif untuk media mekanik
void fs_defrag()
{
	//gunakan algoritma dfs pada file_dirlist untuk mencari file yang terfragmentasi
	//proses ini bisa lama
	/*uint16 cluster_no=0;			//pilih root, MF
	uint16 node_stack[QUEUE_SIZE];	//nodes queue, maksimum hanya menyimpan 5 child
	uint16 space_stack[QUEUE_SIZE];
	uchar stack_index=1;
	uchar space_index=1;
	char i,j=0,k;
	ef_header *curfile;				//inisialisasi
	char buffer[5] = { 0,0,0,0,0 };
	get_next_child:
	curfile = file_get_header(cluster_no);
	if(curfile->sibling!=0) {
		//enqueue child
		space_stack[space_index++] = j;
		node_stack[stack_index++] = curfile->sibling;
	}
	//printf	if(curfile->child!=0) {
		putchar(0xbf);
		putchar(0x0a);
		cluster_no = curfile->child;
		j=j+5;
		goto get_next_child;
	}
	putchar(0x0a);
	if(stack_index==1) { return; }
	//dequeue
	cluster_no = node_stack[--stack_index];
	j = space_stack[--space_index];	
	goto get_next_child;*/
}

void fs_dismount(fs_handle * fs)
{
	//ioman_close();
	hw_rca * rca = fs->rca;
	free(fs);
	ioman_close(rca);
}
#endif


#if ASGARD_VERSION == 4
//static fs_chain _chunkroot;
fs_table fs_info;

uchar fs_init(void) {
	//fs_table * table;
	//table = (fs_table *)m_alloc(sizeof(fs_table));
	ioman_read_buffer(0, &fs_info, sizeof(fs_table));		//load fs_info from memory
	if(fs_info.fs_type[0] == 'A' &&
		fs_info.fs_type[1] == 'S' &&
		fs_info.fs_type[2] == 'G') {
		if(fs_info.fs_ver != ASGARD_VERSION) { return FS_WRONG_VERSION; }
		//ioman_read_buffer(ALLOCATION_DATA_OFFSET, &_chunkroot, sizeof(fs_chain));
		//m_free(table);
		//initialize file system security
		chv_init();
		return FS_INITIALIZED;
	}
	//m_free(table);
	return FS_UNFORMATTED;
} 

uint32 fs_freespace(void) _REENTRANT_ {
	uint32 freespace;
	//fs_table * table;
	//table = (fs_table *)m_alloc(sizeof(fs_table));
	//ioman_read_buffer(0, (uchar *)table, sizeof(fs_table));
	freespace = (fs_info.fs_size - fs_get_allocated_space()) - ALLOCATION_DATA_OFFSET;
	//m_free(table);
	return freespace;
}

uchar code _fsh_mask[] = "";
uchar fs_generate_FSH(uchar * buffer) _REENTRANT_ {
	#define HASH_SIZE		0x40
	uchar temp[HASH_SIZE];
	//uint32 locator = 0x200;			//start of file system data
	uchar i = 0, j;
	memset(buffer, 0, HASH_SIZE);		//clear output buffer
	memset(temp, 0, HASH_SIZE);			//clear temp 
	ioman_read_buffer(ALLOCATION_DATA_OFFSET, temp, HASH_SIZE);
	//metadata hash
	while(((fs_chain *)temp)->next != 0) {
		ioman_read_buffer(((fs_chain *)temp)->next, temp, HASH_SIZE);
		for(i=0;i<HASH_SIZE;i++) {
			buffer[i] ^= temp[i];
		}
	}
	//chaining mechanism
	for(i=(HASH_SIZE - 8);i!=0;i-=8) {
		for(j=0;j<8;j++) {
			buffer[i] ^= buffer[i+8];
		}
	}
	//mask with program code
	for(i=0;i<HASH_SIZE;i++) {
		buffer[i] ^= _fsh_mask[i];
	}
	return HASH_SIZE;
}

#if FS_ADDRESS_WIDTH == AW_32BIT
uint32 fs_get_allocated_space(void) _REENTRANT_ { 
	uint32 allocated_space = 0;
	uint32 alloc_pos;
#else
uint16 fs_get_allocated_space(void) _REENTRANT_ {
	uint16 alloc_pos;		 
	uint16 allocated_space = 0;
#endif
	//fs_chain * alloc_ptr = (fs_chain *)m_alloc(sizeof(fs_chain));
	fs_chain fs_chdr;
	alloc_pos = ALLOCATION_DATA_OFFSET;
	//memcpy(&fs_chdr, &_chunkroot, sizeof(fs_chain));
	ioman_read_buffer(ALLOCATION_DATA_OFFSET, &fs_chdr, sizeof(fs_chain));
	get_next_chain:
	if(fs_chdr.next == 0) {		//allocate new chunk at the end of the heap
		allocated_space += fs_chdr.size + sizeof(fs_chain);
		//m_free(alloc_ptr);
		return allocated_space;
	}
	//temp = alloc_ptr->next;		//isi dengan pointer chunk sebelumnya
	//alloc_ptr_temp = alloc_ptr;
	//alloc_pos_temp = alloc_pos;
	//memcopy(alloc_ptr_temp, alloc_ptr, 0, sizeof(fs_chain));
	//alloc_ptr = alloc_ptr->next;
	allocated_space += fs_chdr.size + sizeof(fs_chain);
	alloc_pos = fs_chdr.next;
	ioman_read_buffer(alloc_pos, &fs_chdr, sizeof(fs_chain));
	goto get_next_chain;
}

extern BYTEX	FlashBuffer[512];
uint16 fs_format(uint32 size) _REENTRANT_ {
	//fs_table * table;
	register uint16 status;
	df_header * head;
	fs_chain _chunkroot;
	#if FS_ADDRESS_WIDTH == AW_32BIT
	uint32 address;
	#else
	uint16 address;
	#endif
	//table = (fs_table *)m_alloc(sizeof(fs_table));
	//initialize file system info
	ioman_erase_all();
	fs_info.fs_type[0] = 'A';  					//default tag
	fs_info.fs_type[1] = 'S';
	fs_info.fs_type[2] = 'G';
	fs_info.fs_ver = ASGARD_VERSION;				//4
	fs_info.fs_mod = FS_ADDRESS_WIDTH;			//16 bit or 32 bit, reserved for future use (default 16)
	fs_info.sector_size = 0;					//no sector available (only for AS30), not used
	fs_info.fs_size = size;						//file system limitation
	status = ioman_write_buffer(0, &fs_info, sizeof(fs_table));
	if(status != APDU_SUCCESS) goto exit_format;
	//initialize fs_alloc / linked list structure
	_chunkroot.size = 0;		//chunkroot tidak bisa di s_free
	_chunkroot.next = NULL;
	_chunkroot.prev = ALLOCATION_DATA_OFFSET;
	address = ALLOCATION_DATA_OFFSET + (sizeof(fs_chain) + sizeof(df_header) + (sizeof(df_header) % 4) );
	status = ioman_write_buffer(address, (uchar *)&_chunkroot, sizeof(fs_chain));
	if(status != APDU_SUCCESS) goto exit_format;
	_chunkroot.prev = 0;
	_chunkroot.size = sizeof(df_header);
	_chunkroot.next = address;
	status = ioman_write_buffer(ALLOCATION_DATA_OFFSET, (uchar *)&_chunkroot, sizeof(fs_chain));
	if(status != APDU_SUCCESS) goto exit_format;
	//initialize root file system / tree structure (Master File, 0x3F00)
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
	status = ioman_write_buffer(address, (uchar *)head, sizeof(df_header));		//write the specified allocated address
	if(status != APDU_SUCCESS) goto exit_format;
	
	//m_free(table);
	//#ifdef _X86_ARCH										//PC only simulation
	//initialize default file system security
	chv_create(1, "1234\xFF\xFF\xFF\xFF", "12345678", 3, 10);			//create chv 1	(default disable)
	chv_create(2, "1234\xFF\xFF\xFF\xFF", "12345678", 3, 10);			//create chv 2	(default disable)
	chv_create(4, "12345678", "12345678", 100, 100);			//create chv 4 (admin login)
	chv_create(6, "12345678", "12345678", 100, 100);			//create chv 6 (system login)
	chv_enable(2, "1234\xFF\xFF\xFF\xFF");				//automatically enable chv 2
	chv_enable(4, "12345678");							//automatically enable chv 4
	chv_enable(6, "12345678");							//automatically enable chv 6
	chv_init();
	exit_format:
	m_free(head);
	return status;
	//#endif
}

#if FS_ADDRESS_WIDTH == AW_32BIT
uint32 fs_alloc(uint32 size) _REENTRANT_ {
	uint32 alloc_pos;
	uint32 alloc_pos_temp;
	uint32 candidate_pos;
#else
uint16 fs_alloc(uint16 size) _REENTRANT_ {
	uint16 alloc_pos;
	uint16 alloc_pos_temp;
	uint16 candidate_pos;
#endif
	fs_chain * alloc_ptr = (fs_chain *)m_alloc(sizeof(fs_chain));
	fs_chain * alloc_ptr_temp = (fs_chain *)m_alloc(sizeof(fs_chain));
	fs_chain * candidate = (fs_chain *)m_alloc(sizeof(fs_chain));
	alloc_pos = ALLOCATION_DATA_OFFSET;
	//printf("aaa\n");
	//memcpy(alloc_ptr, &_chunkroot, sizeof(fs_chain));	 
	ioman_read_buffer(ALLOCATION_DATA_OFFSET, alloc_ptr, sizeof(fs_chain));
	//#ifdef OS_uCOS_II_H
	//OS_ENTER_CRITICAL();
	//#endif
	//printf("aa\n");
	//size = size + (4 - (size % 4));		//4 byte align
	get_next_chain:
	if(alloc_ptr->next == 0) {		//allocate new chunk at the end of the heap
		alloc_ptr->size = size;
		//check for file system limit
		if(((uint32)alloc_pos + sizeof(fs_chain) + size) >= (fs_info.fs_size - sizeof(fs_chain))) {
			m_free(candidate);
			m_free(alloc_ptr_temp);
			m_free(alloc_ptr);
			return 0;				//return 0 if there is no available space	
		}
		alloc_ptr->next = (alloc_pos + sizeof(fs_chain) + size);
		//disini cek STACK POINTER, dikhawatirkan akan bertabrakan dengan stack, sebaiknya antara stack pointer dan heap dibuat
		//GAP yang cukup lebar karena dikhawatirkan ukuran stack akan bertambah setelah keluar dari fungsi s_alloc karena automatic allocation
		alloc_ptr->prev = alloc_pos_temp;
		//alloc_ptr_temp = alloc_ptr->next;	//berubah fungsi untuk menjadi pointer chunk selanjutnya
		alloc_pos_temp = alloc_ptr->next;
		alloc_ptr_temp->prev = alloc_pos;
		alloc_ptr_temp->next = 0;
		alloc_ptr_temp->size = 0;
		ioman_write_buffer(alloc_pos_temp, alloc_ptr_temp, sizeof(fs_chain));
		ioman_write_buffer(alloc_pos, alloc_ptr, sizeof(fs_chain));
		//alloc_ptr = alloc_ptr_temp;
		//#ifdef MIDGARD_DEBUG_ACTIVATED
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
	memcpy(alloc_ptr_temp, alloc_ptr, sizeof(fs_chain));
	//alloc_ptr = alloc_ptr->next;
	alloc_pos = alloc_ptr->next;
	ioman_read_buffer(alloc_ptr->next, alloc_ptr, sizeof(fs_chain));
	if(alloc_pos >= (alloc_pos_temp + alloc_ptr_temp->size + sizeof(fs_chain) + sizeof(fs_chain) + size + (size % 4))) {
		//allocate new heap using FFA (First Fit Algorithm)
		//Uart_Printf("allocate previous heap\n");
		candidate_pos = (alloc_pos_temp + alloc_ptr_temp->size + sizeof(fs_chain));
		//ioman_read_buffer(candidate_pos, candidate, sizeof(fs_chain));
		//#ifdef MIDGARD_DEBUG_ACTIVATED
		//printf(" * creating new chunk with size %i\n", size);
		//#endif
		//candidate->size = size + (size % 4);
		candidate->size = size;
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
		m_free(candidate);
		m_free(alloc_ptr_temp);
		m_free(alloc_ptr);
		return candidate_pos + sizeof(fs_chain);
	}
	goto get_next_chain;
}

#if FS_ADDRESS_WIDTH == AW_32BIT
uint16 fs_dealloc(uint32 address) _REENTRANT_ {
	uint32 alloc_pos;
	uint32 alloc_prev_pos;
	uint32 alloc_next_pos;
#else
uint16 fs_dealloc(uint16 address) _REENTRANT_ {
	uint16 alloc_pos;
	uint16 alloc_prev_pos;
	uint16 alloc_next_pos;
#endif
	register uint16 status;
	//#ifdef OS_uCOS_II_H
	//#if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    //OS_CPU_SR  cpu_sr;
    //#endif
    //#endif
	fs_chain * alloc_ptr = (fs_chain *)m_alloc(sizeof(fs_chain));
	fs_chain * alloc_prev = (fs_chain *)m_alloc(sizeof(fs_chain));
	fs_chain * alloc_next = (fs_chain *)m_alloc(sizeof(fs_chain));
	alloc_pos = address - sizeof(fs_chain);
	ioman_read_buffer(alloc_pos, alloc_ptr, sizeof(fs_chain));
	alloc_prev_pos = alloc_ptr->prev;
	ioman_read_buffer(alloc_prev_pos, alloc_prev, sizeof(fs_chain));
	alloc_next_pos = alloc_ptr->next;
	ioman_read_buffer(alloc_next_pos, alloc_next, sizeof(fs_chain));

	if(address == 0) { status = APDU_MEMORY_PROBLEM; goto exit_dealloc; }
	if(alloc_prev == 0) { status = APDU_SUCCESS; goto exit_dealloc; }		//this memory is already freed
	if(alloc_ptr->size == 0) { status = APDU_SUCCESS; goto exit_dealloc; }		//this memory is already freed
	//if(alloc_next_pos == 0) {		//this is the last chunk
		//printf("this is the last chunk\n");	
	//}
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
	alloc_ptr->next = 0;
	alloc_ptr->prev = 0;
	alloc_ptr->size = 0;
	status = ioman_write_buffer(alloc_pos, alloc_ptr, sizeof(fs_chain));
	if(status != APDU_SUCCESS) goto exit_dealloc;
	status = ioman_write_buffer(alloc_prev_pos, alloc_prev, sizeof(fs_chain));
	if(status != APDU_SUCCESS) goto exit_dealloc;
	status = ioman_write_buffer(alloc_next_pos, alloc_next, sizeof(fs_chain));
	if(status != APDU_SUCCESS) goto exit_dealloc;

	exit_dealloc: 
	m_free(alloc_next); 
	m_free(alloc_prev); 
	m_free(alloc_ptr); 
	return status;
	//#ifdef OS_uCOS_II_H
	//OS_EXIT_CRITICAL();
	//#endif

}

#endif
