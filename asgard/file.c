/* An upper layer of Asgard File System, which provide API(s)
 * for user application
 *
 * Copyright 2010, Agus Purwanto.
 * All rights reserved.
 *
 * 
 */


#include "file.h"
#include "..\defs.h"
#include "fs.h"
#include "..\midgard\midgard.h"
#include "security.h"
#include "..\misc\algorithm.h"
#include "..\misc\mem.h"	
#include <string.h>

#if  ASGARD_VERSION == 3
#if USE_ENTRY_STACK
/* automatically push fs_file handle onto asgard_handle stack, use on file selection */
void as_push(asgard_handle * handle, fs_file * file) {
	if(handle->stack != MAX_STACK_HANDLE)
		handle->stack[handle->stack_index++] = file;
}

/* return parent fs_file handle and freed the previous handle */
fs_file * as_pop(asgard_handle * handle) {
	if(handle->stack_index != 0) {
		free(handle->stack[handle->stack_index]);
		handle->stack[handle->stack_index--] = NULL;				//clear handle
		return handle->stack[handle->stack];
	} 
	return NULL;													//stack cannot be popped
}

/* peek asgard_handle current stack */
fs_file * as_peek(asgard_handle * handle) {
	if(handle->stack_index > 1 && handle->stack_index <=8)
		return handle->stack[handle->stack_index -1];
	return NULL;
}

/* peek asgard_handle current stack */
fs_file * as_peek_parent(asgard_handle * handle) {
	if(handle->stack_index > 2 && handle->stack_index <=8)
		return handle->stack[handle->stack_index -2];
	return NULL;
}

/* flush asgard_handle stack from memory, freed all references and nullifies stack_index */
void as_flush(asgard_handle * handle) {
	uchar i;
	for(i=0;i<handle->stack_index;i++) {
		free(handle->stack[i]);
	}
	handle->stack_index = 0;
}

#endif

fs_file * as_get_current_directory(asgard_handle * handle) {
#if USE_ENTRY_STACK
	return as_peek(handle);											//temporaly use as_peek
#else
	//printf("current directory : %x\n", handle->cur_dir->entry.fid);
	return handle->cur_dir;
#endif
}

void as_set_current_file(asgard_handle * handle, fs_file * file) {
#if USE_ENTRY_STACK
	as_push(handle, file);											//temporaly use as_push
#else
	if(handle->cur_file != NULL) free(handle->cur_file);
	if(file->entry.type & FS_TYPE_DIR) handle->cur_dir = file;		//check if currently selected file is directory then set handle->cur_dir
	handle->cur_file = file;
#endif
}

fs_file * as_get_current_file(asgard_handle * handle) {
#if USE_ENTRY_STACK
	return as_peek(handle);											//temporaly use as_peek
#else
	return handle->cur_file;
#endif
}

asgard_handle * as_create_handle(fs_handle * fs, uchar mode) {
	asgard_handle * handle = NULL;
	if(fs != NULL) {
		handle = (asgard_handle *)malloc(sizeof(asgard_handle));
		handle->fs = fs;												//set current file system of asgard handle
#if USE_ENTRY_STACK
		uchar i = 0;
		for(i=0;i<MAX_STACK_HANDLE;i++) {								//clear stack pointer
			handle->stack[i] = NULL;
		}		
		handle->stack_index = 0;										//set stack index to 0
#else
		handle->cur_dir = NULL;
		handle->cur_file = NULL;
#endif
		handle->cur_rec_index = 0;										//current record index = 0
		handle->mode = mode;											//set mode user/sys
		handle->num_of_df = 0;											//
		handle->num_of_ef = 0;											//
	}
	return handle;
}

uint16 as_select(asgard_handle * handle, uint16 fid) {
	fs_file * cur_file = NULL;
	fs_file * root = NULL;
#if USE_ENTRY_STACK
	if(handle->stack_index > 2 && handle->stack_index <=8) {
		cur_file = handle->stack[handle->stack_index -2];				//check for parent
		if(cur_file != NULL) {
			if(cur_file->entry.fid == fid) {
				as_pop(handle);											//freed current file
				goto as_select_finished;
			}
		}
	}
	cur_file = fs_fopen(as_peek(handle), fid, handle->mode);			//select child
	as_select_finished:
	if(cur_file != NULL) {
		as_push(handle, cur_file);
		return APDU_SUCCESS_RESPONSE;
	}
	cur_file = as_peek(handle);
	if(cur_file != NULL && (file->entry.type & FS_TYPE_DIR)) {			//entry exist and type directory
		as_push(handle, NULL);											//push no selected file
	}
#else
	//printf("select\n");
	if(fid == FID_MF) {
		root = fs_root(handle->fs);
		if(root == NULL) return APDU_FILE_NOT_FOUND;
		//printf("root : %x\n", root);
		cur_file = fs_fopen(root, FID_MF, handle->mode);
		if(handle->cur_dir != NULL) { free(handle->cur_dir); handle->cur_dir = NULL; }
		if(handle->cur_file != NULL && handle->cur_dir != handle->cur_file) { free(handle->cur_file); handle->cur_file = NULL; }
		handle->cur_dir = cur_file;
		handle->cur_file = cur_file;
		free(root);													//prevent memory leakage
		if(cur_file != NULL) return APDU_SUCCESS_RESPONSE;				//selection successful
	}
	if(handle->cur_dir != NULL && fid == handle->cur_dir->entry.fid) return APDU_SUCCESS_RESPONSE;
	if(handle->cur_file != NULL && fid == handle->cur_file->entry.fid) return APDU_SUCCESS_RESPONSE;
	//m_mem_dump();
	//printf("select : %x\n", handle->cur_dir->entry.fid);
	//printf("handle->cur_dir : %x\n", handle->cur_dir);
	cur_file = fs_fopen(handle->cur_dir, fid, handle->mode);
	//if(cur_file == NULL) printf("file not found\n");
	//getch();
	if(cur_file == NULL) return APDU_FILE_NOT_FOUND;
	if(cur_file->entry.type & FS_TYPE_DIR) {						//type is directory
		//printf("handle is directory\n");
		if(handle->cur_dir != NULL) { free(handle->cur_dir); handle->cur_dir = NULL; }
		if(handle->cur_file != NULL && handle->cur_dir != handle->cur_file) { free(handle->cur_file); handle->cur_file = NULL; }
		handle->cur_dir = cur_file;
		handle->cur_file = cur_file;
		//getch();
		if(cur_file != NULL) return APDU_SUCCESS_RESPONSE;			//selection successful
	} else {														//type is file
		//if(handle->cur_dir != NULL) { free(handle->cur_dir); handle->cur_dir = NULL; }
		if(handle->cur_file != NULL && handle->cur_dir != handle->cur_file) { free(handle->cur_file); handle->cur_file = NULL; }
		handle->cur_file = cur_file;
		//getch();
		if(cur_file != NULL) return APDU_SUCCESS_RESPONSE;			//selection successful
	}
#endif
	return APDU_FILE_NOT_FOUND;
}

uint16 as_read_binary(asgard_handle * handle, uint16 offset, uchar * buffer, uchar size) {
	ef_bin_header header;
	fs_file * file = as_get_current_file(handle);
	if(file == NULL) return APDU_NO_EF_SELECTED;
	if(file->entry.type & FS_TYPE_DIR) return APDU_COMMAND_INVALID;
	fs_fseek(file, 0);
	fs_fread(file, &header, sizeof(ef_bin_header));					//getting ef information from ef header first
	if(header.structure == EF_TRANSPARENT) {
		return APDU_INSTRUCTION_INVALID;							//wrong command
	}
	if(header.status != EF_VALID) {
		return APDU_INVALID_STATUS;
	}
	if(offset + size <= (header.size + sizeof(ef_bin_header))) {
		fs_fseek(file, offset + sizeof(ef_bin_header));
		fs_fread(file, buffer, size);
		return APDU_SUCCESS;										//read success
	}
	return APDU_OUT_OF_RANGE;										//wrong offset and length
}

uint16 as_write_binary(asgard_handle * handle, uint16 offset, uchar * buffer, uchar size) {
	ef_bin_header header;
	fs_file * file = as_get_current_file(handle);
	if(file == NULL) return APDU_NO_EF_SELECTED;
	if(file->entry.type & FS_TYPE_DIR) return APDU_COMMAND_INVALID;
	fs_fseek(file, 0);
	fs_fread(file, &header, sizeof(ef_bin_header));					//getting ef information from ef header first
	if(header.structure == EF_TRANSPARENT) {
		return APDU_INSTRUCTION_INVALID;							//wrong command
	}
	if(header.status != EF_VALID) {
		return APDU_INVALID_STATUS;
	}
	if(offset + size <= (header.size + sizeof(ef_bin_header))) {
		fs_fseek(file, offset + sizeof(ef_bin_header));
		fs_fwrite(file, buffer, size);
		return APDU_SUCCESS;										//read success
	}
	return APDU_OUT_OF_RANGE;										//wrong offset and length	
}

uint16 as_create_binary(asgard_handle * handle, uint16 fid, uchar ACC_READ, uchar ACC_WRT, uchar ACC_INC, uchar ACC_INV, uchar ACC_RHB, uchar * buffer, uint16 size) {
	ef_bin_header header;
	fs_file * file = as_get_current_directory(handle);				//get currently selected directory
	if(file == NULL) return APDU_NO_EF_SELECTED;
	if((file->entry.type & FS_TYPE_DIR) == 0) return APDU_COMMAND_INVALID;
	header.structure = EF_TRANSPARENT;
	header.size = size;
	header.status = EF_VALID;
	header.access_rw = ACC_READ << 4 | ACC_WRT;
	header.access_inc = ACC_INC;
	header.access_ri = ACC_RHB << 4 | ACC_INV;
	file = fs_fcreate(file, fid, handle->mode);
	fs_fwrite(file, &header, sizeof(ef_bin_header));				//write header first
	fs_fwrite(file, buffer, size);									//write content (should i write content first to default??)
	as_set_current_file(handle, file);								//automatically select new file
	return APDU_SUCCESS;
}

//read record for cyclic and linier fixed
uint16 as_read_record(asgard_handle * handle, uint16 rec_no, uchar * buffer, uchar size) {
	ef_rec_header header;
	fs_file * file = as_get_current_file(handle);
	uchar bytes_readed;
	uint16 offset;
	//uchar actual_index;
	if(file == NULL) return APDU_NO_EF_SELECTED;
	if(file->entry.type & FS_TYPE_DIR) return APDU_COMMAND_INVALID;
	fs_fseek(file, 0);
	fs_fread(file, (uchar *)&header, sizeof(ef_rec_header));							//getting ef information from ef header first
	if((header.structure == EF_LINFIX) || (header.structure == EF_CYCLIC)) {
		return APDU_INSTRUCTION_INVALID;									//wrong command
	}
	if(header.status != EF_VALID) {
		return APDU_INVALID_STATUS;
	}
	if(size != header.rec_size) return APDU_OUT_OF_RANGE;					//check for record size
	if(header.structure == EF_LINFIX) {										//read operation for linier fixed
		offset = sizeof(ef_rec_header) + (header.rec_size * rec_no);		//calculate offset for linier fixed
		handle->cur_rec_index = rec_no;										//update handle current record
		fs_fseek(file, offset);
		bytes_readed = fs_fread(file, buffer, header.rec_size);
		if(bytes_readed == header.rec_size) return APDU_SUCCESS;
		//return APDU_FATAL_ERROR;
	} else if(header.structure == EF_CYCLIC) {								//read operation for cyclic
		if(rec_no == 0xffff) rec_no = header.num_of_records - 1;			//check if rec_no is negative
		rec_no = ((rec_no + header.first) % header.num_of_records);			//calculate modded record number
		handle->cur_rec_index = rec_no;										//update handle current record
		offset = sizeof(ef_rec_header) + (header.rec_size * rec_no);		//calculate offset for cyclic
		fs_fseek(file, offset);
		bytes_readed = fs_fread(file, buffer, header.rec_size);
		if(bytes_readed == header.rec_size) return APDU_SUCCESS;
		//return APDU_FATAL_ERROR;
	}
	return APDU_FATAL_ERROR;
}

uint16 as_read_record_next(asgard_handle * handle, uchar *buffer, uchar size) {
	return as_read_record(handle, ++handle->cur_rec_index, buffer, size);
}

uint16 as_read_record_prev(asgard_handle * handle, uchar *buffer, uchar size) {
	return as_read_record(handle, --handle->cur_rec_index, buffer, size);
}

//uint16 _write_new_rec(uint16 rec_no, uchar size, uchar *buffer);
uint16 as_write_record(asgard_handle * handle, uint16 rec_no, uchar *buffer, uchar size) {
	ef_rec_header header;
	fs_file * file = as_get_current_file(handle);
	uchar bytes_wrote;
	uint16 offset;
	//uchar actual_index;
	if(file == NULL) return APDU_NO_EF_SELECTED;
	if(file->entry.type & FS_TYPE_DIR) return APDU_COMMAND_INVALID;
	fs_fseek(file, 0);
	fs_fread(file, &header, sizeof(ef_rec_header));							//getting ef information from ef header first
	if(header.structure == EF_LINFIX || header.structure == EF_CYCLIC) {
		return APDU_INSTRUCTION_INVALID;									//wrong command
	}
	if(header.status != EF_VALID) {
		return APDU_INVALID_STATUS;
	}
	if(size != header.rec_size) return APDU_OUT_OF_RANGE;					//check for record size
	if(header.structure == EF_LINFIX) {										//read operation for linier fixed
		offset = sizeof(ef_rec_header) + (header.rec_size * rec_no);		//calculate offset for linier fixed
		handle->cur_rec_index = rec_no;										//update handle current record
		fs_fseek(file, offset);
		bytes_wrote = fs_fwrite(file, buffer, header.rec_size);
		if(bytes_wrote == header.rec_size) return APDU_SUCCESS;
		//return APDU_FATAL_ERROR;
	} else if(header.structure == EF_CYCLIC) {								//read operation for cyclic
		if(rec_no == 0xffff) rec_no = header.num_of_records - 1;			//check if rec_no is negative
		rec_no = ((rec_no + header.first) % header.num_of_records);			//calculate modded record number
		handle->cur_rec_index = rec_no;										//update handle current record
		offset = sizeof(ef_rec_header) + (header.rec_size * rec_no);		//calculate offset for cyclic
		fs_fseek(file, offset);
		bytes_wrote = fs_fwrite(file, buffer, header.rec_size);
		if(bytes_wrote == header.rec_size) { 
			header.first = rec_no;
			fs_fseek(file, 0);
			fs_fwrite(file, &header, sizeof(ef_rec_header));				//set new ef information
			//handle->cur_rec_index = 0;									//should current record index set to 0 for relative positioning
			return APDU_SUCCESS;
		}
		//return APDU_FATAL_ERROR;
	}
	return APDU_FATAL_ERROR;
}

uint16 as_write_record_next(asgard_handle * handle, uchar *buffer, uchar size) {
	return as_write_record(handle, ++handle->cur_rec_index, buffer, size);
}

uint16 as_write_record_prev(asgard_handle * handle, uchar *buffer, uchar size) {
	return as_write_record(handle, --handle->cur_rec_index, buffer, size);
}

uint16 as_create_record(asgard_handle * handle, uint16 fid, uchar ACC_READ, uchar ACC_WRT, uchar ACC_INC, uchar ACC_INV, uchar ACC_RHB, uchar total_rec, uchar rec_size) {
	ef_rec_header header;
	uchar * buffer;
	uchar i;
	fs_file * file = as_get_current_directory(handle);				//get currently selected directory
	if(file == NULL) return APDU_NO_EF_SELECTED;
	if((file->entry.type & FS_TYPE_DIR) == 0) return APDU_COMMAND_INVALID;
	header.structure = EF_LINFIX;
	header.status = EF_VALID;
	header.access_rw = ACC_READ << 4 | ACC_WRT;
	header.access_inc = ACC_INC;
	header.access_ri = ACC_RHB << 4 | ACC_INV;
	header.num_of_records = total_rec;
	header.rec_size = rec_size;
	header.first = 0;
	file = fs_fcreate(file, fid, handle->mode);
	fs_fwrite(file, &header, sizeof(ef_rec_header));				//write header first
	buffer = (uchar *)malloc(rec_size);
	memset(buffer, 0xff, rec_size);
	for(i=0;i<total_rec;i++) {
		fs_fwrite(file, buffer, rec_size);
	}
	free(buffer);
	as_set_current_file(handle, file);								//automatically select new file
	return APDU_SUCCESS;
}

uint16 as_create_cyclic(asgard_handle * handle, uint16 fid, uchar ACC_READ, uchar ACC_WRT, uchar ACC_INC, uchar ACC_INV, uchar ACC_RHB, uchar total_rec, uchar rec_size) {
	ef_rec_header header;
	uchar * buffer;
	uchar i;
	fs_file * file = as_get_current_directory(handle);				//get currently selected directory
	if(file == NULL) return APDU_NO_EF_SELECTED;
	if((file->entry.type & FS_TYPE_DIR) == 0) return APDU_COMMAND_INVALID;
	header.structure = EF_CYCLIC;
	header.status = EF_VALID;
	header.access_rw = ACC_READ << 4 | ACC_WRT;
	header.access_inc = ACC_INC;
	header.access_ri = ACC_RHB << 4 | ACC_INV;
	header.num_of_records = total_rec;
	header.rec_size = rec_size;
	header.first = 0;
	file = fs_fcreate(file, fid, handle->mode);
	fs_fwrite(file, &header, sizeof(ef_rec_header));				//write header first
	buffer = (uchar *)malloc(rec_size);
	memset(buffer, 0xff, rec_size);
	for(i=0;i<total_rec;i++) {
		fs_fwrite(file, buffer, rec_size);
	}
	free(buffer);
	as_set_current_file(handle, file);								//automatically select new file
	return APDU_SUCCESS;
}

uint16 as_create_directory(asgard_handle * handle, uint16 fid) {
	fs_file * file = as_get_current_directory(handle);				//get currently selected directory (for use as parent directory)
	if(file == NULL) return APDU_NO_EF_SELECTED;
	if((file->entry.type & FS_TYPE_DIR) == 0) return APDU_COMMAND_INVALID;
	//printf("parent directory : %x > %x\n", file->entry.fid, fid);
	//getch();
	file = fs_mkdir(file, fid, handle->mode);
	as_set_current_file(handle, file);								//also automatically set current directory
	return APDU_SUCCESS;
}

/* remove currently selected file */
uint16 as_remove(asgard_handle * handle) {
	fs_file * file = as_get_current_file(handle);
	fs_rmfile(file, handle->mode);
	file = as_get_current_directory(handle);						//get current directory
	as_set_current_file(handle, file);								//automatically select parent directory
	return APDU_SUCCESS;
}

/* invalidate currently selected file */
uint16 as_invalidate(asgard_handle * handle) {
	ef_bin_header header;														//standard ef header
	uchar bytes_wrote = 0;
	fs_file * file = as_get_current_file(handle);
	if(file == NULL) return APDU_NO_EF_SELECTED;
	fs_fseek(file, 0);
	fs_fread(file, &header, sizeof(ef_bin_header));								//getting ef information from ef header first
	if(header.status != EF_VALID) {
		return APDU_INVALID_STATUS;
	}
	header.status &= ~EF_VALID;
	fs_fseek(file, 0);
	bytes_wrote = fs_fwrite(file, &header, sizeof(ef_bin_header));				//set new ef information
	if(bytes_wrote != sizeof(ef_bin_header)) return APDU_FATAL_ERROR;
	return APDU_SUCCESS;
}

/* rehabilitate currently selected file */
uint16 as_rehabilitate(asgard_handle * handle) {
	ef_bin_header header;														//standard ef header
	uchar bytes_wrote = 0;
	fs_file * file = as_get_current_file(handle);
	if(file == NULL) return APDU_NO_EF_SELECTED;
	fs_fseek(file, 0);
	fs_fread(file, &header, sizeof(ef_bin_header));								//getting ef information from ef header first
	if((header.status & EF_VALID) == 0) {
		return APDU_INVALID_STATUS;
	}
	header.status |= EF_VALID;
	fs_fseek(file, 0);
	bytes_wrote = fs_fwrite(file, &header, sizeof(ef_bin_header));				//set new ef information
	if(bytes_wrote != sizeof(ef_bin_header)) return APDU_FATAL_ERROR;
	return APDU_SUCCESS;
}
#endif

#if ASGARD_VERSION == 4
#include "..\drivers\ioman.h"

/*#if FS_ADDRESS_WIDTH == AW_32BIT
uint32 _cur_ptr = ALLOCATION_DATA_OFFSET + sizeof(fs_chain);
uint32 _cur_dir = ALLOCATION_DATA_OFFSET + sizeof(fs_chain);
int16 _cur_wr_rec;
int16 _cur_rd_rec;
#else
uint16 _cur_ptr = ALLOCATION_DATA_OFFSET + sizeof(fs_chain);
uint16 _cur_dir = ALLOCATION_DATA_OFFSET + sizeof(fs_chain);
int16 _cur_wr_rec;
int16 _cur_rd_rec;
#endif*/

//fungsi file_get_header paling sering diakses
#if FS_ADDRESS_WIDTH == AW_32BIT
f_header * file_get_header(uint32 address) _REENTRANT_ {
#else
f_header * file_get_header(uint16 address) _REENTRANT_ {
#endif
	f_header *newfile;
	newfile = (f_header *) m_alloc (sizeof(f_header));
	ioman_read_buffer(address, newfile, sizeof(f_header));
	return newfile;	//jangan dihapus karena akan dipakai
}

/*#if FS_ADDRESS_WIDTH == AW_32BIT
void file_get_header_buffer(uint32 address, f_header * header) _REENTRANT_ {
#else
void file_get_header_buffer(uint16 address, f_header * header) _REENTRANT_ {
#endif
	ioman_read_buffer(address, newfile, sizeof(f_header));	
}*/

#if FS_ADDRESS_WIDTH == AW_32BIT
df_header * file_get_dfheader(uint32 address) _REENTRANT_ {
#else
df_header * file_get_dfheader(uint16 address) _REENTRANT_ {
#endif
	df_header *newfile;
	newfile = (df_header *) m_alloc (sizeof(df_header));
	ioman_read_buffer(address, newfile, sizeof(df_header));
	return newfile;	//jangan dihapus karena akan dipakai
}

#if FS_ADDRESS_WIDTH == AW_32BIT
ef_header * file_get_efheader(uint32 address) _REENTRANT_ {
#else
ef_header * file_get_efheader(uint16 address) _REENTRANT_ {
#endif
	ef_header *newfile;
	newfile = (ef_header *) m_alloc (sizeof(ef_header));
	ioman_read_buffer(address, newfile, sizeof(ef_header));
	return newfile;	//jangan dihapus karena akan dipakai
} 

void * file_get_current_header(fs_handle * handle) _REENTRANT_ {
	if(handle->cur_ptr == handle->cur_dir) {
	 	return (void *)file_get_dfheader(handle->cur_ptr); 
	}
	return (void *)file_get_efheader(handle->cur_ptr);
}

#if FS_ADDRESS_WIDTH == AW_32BIT
uint16 file_add_entry(fs_handle * handle, uint32 address) _REENTRANT_ {
	uint32 ptr;
#else
uint16 file_add_entry(fs_handle * handle, uint16 address) _REENTRANT_ {
	uint16 ptr;
#endif	
	uint16 status = APDU_SUCCESS;
	f_header * head;
	f_header * temp;
	temp = file_get_header(address);
	head = file_get_header(handle->cur_dir);
	if(head->child == 0) { 
		//printf("add entry to child\n");
		head->child = address; 
		status = ioman_write_buffer(handle->cur_dir, head, sizeof(f_header)); 
		m_free(head); 
		m_free(temp);
		return status; 
	}
	ptr = head->child;
	while(ptr != 0) {
		m_free(head);
		head = file_get_header(ptr);
		if(head->sibling == 0) {
			//printf("add entry as sibling\n");
			head->sibling = address;
			status = ioman_write_buffer(ptr, head, sizeof(f_header)); 
			m_free(head); 
			m_free(temp);
			return status;
		}
		ptr = head->sibling;
	}		
	m_free(head);
	m_free(temp);
	return status;
}


#if FS_ADDRESS_WIDTH == AW_32BIT
uint16 file_remove_entry(fs_handle * handle, uint32 address) _REENTRANT_ {
	uint32 ptr;
#else
uint16 file_remove_entry(fs_handle * handle, uint16 address) _REENTRANT_ {
	uint16 ptr;
#endif	
	register uint16 status = APDU_SUCCESS;
	f_header * head;
	f_header * temp;
	temp = file_get_header(address);
	head = file_get_header(handle->cur_dir);
	if(head->child == address) { 
		head->child = temp->sibling;
		status = ioman_write_buffer(handle->cur_dir, head, sizeof(f_header)); 
		m_free(head); 
		m_free(temp); 
		return status; 
	}
	ptr = head->child;
	while(ptr != 0) {
		m_free(head);
		head = file_get_header(ptr);
		if(head->sibling == address) {
			head->sibling = temp->sibling; 
			status = ioman_write_buffer(ptr, head, sizeof(f_header)); 
			m_free(head); 
			m_free(temp); 
			return status;
		}
		ptr = head->sibling;
	}		
	m_free(head);
	m_free(temp); 
	return status;
}

#if FS_ADDRESS_WIDTH == AW_32BIT
uint32 file_find_entry(fs_handle * handle, uint16 fid) _REENTRANT_ {
	uint32 ptr;
#else
uint16 file_find_entry(fs_handle * handle, uint16 fid) _REENTRANT_ {
	uint16 ptr;
#endif
	f_header * head;
	head = file_get_header(handle->cur_dir);
	ptr = head->child;
	while(ptr != 0) {
		m_free(head);
		head = file_get_header(ptr);
		if(head->fid == fid) {
			m_free(head);
			return ptr;
		}
		ptr = head->sibling;
	}		
	m_free(head);
	return 0;
}

//recursively deallocating all childs at specified header address, added : may 01 2013
void file_dealloc_all_child(uint16 address) _REENTRANT_ {
	uint16 temp_address;
	#if USE_FAST_SELECT	
	f_header temp;
	#else
	f_header * temp;
	#endif
	//f_header * head = (f_header *)file_get_header(address);
	#if USE_FAST_SELECT
	ioman_read_buffer(address, &temp, sizeof(f_header));
	temp_address = temp.child;
	#else
	temp = (f_header *)file_get_header(address);
	temp_address = temp->child;
	#endif
	//if(temp_address != NULL) {
	while(temp_address != 0) {
		#if USE_FAST_SELECT
		ioman_read_buffer(temp_address, &temp, sizeof(f_header)); 
		if(temp.child != 0) {
		#else
		temp = (f_header *)file_get_header(temp_address);
		if(temp->child != 0) {
		#endif
			file_dealloc_all_child(temp_address);
		}
		fs_dealloc(temp_address);
		#if USE_FAST_SELECT	
		temp_address = temp.sibling;
		#else
		temp_address = temp->sibling;
		m_free(temp);
		#endif
	}	
	//}
}

uint16 file_check_status(ef_header * head, uchar op) _REENTRANT_ {
	uchar chv_no;
	if(head->type != T_EF) return APDU_NO_EF_SELECTED;
	switch(op) {				//get chv no
		case FILE_READ:
			chv_no = ((head->acc_rw >> 4) & 0x0f);
			if(head->status == STAT_INVALID) return APDU_INVALID_STATUS;
			break;
		case FILE_WRITE:
			chv_no = (head->acc_rw & 0x0f);
			if(head->status == STAT_INVALID) return APDU_INVALID_STATUS;
			break;
		case FILE_INCREASE:
			chv_no = (head->acc_inc & 0x0f);
			if(head->inc == 0) return APDU_FILE_INCONSISTENT;			//check for increase access
			if(head->status == STAT_INVALID) return APDU_INVALID_STATUS;
			break;
		case FILE_INVALIDATE:
			if(head->status == STAT_INVALID) return APDU_INVALID_STATUS;
			chv_no = (head->acc_ri & 0x0f);
			break;
		case FILE_REHABILITATE:
			chv_no = ((head->acc_ri >> 4) & 0x0f);
			if(head->status == STAT_VALID) return APDU_INVALID_STATUS;
			break;
	}
	switch(chv_no) {
		case 0: break;		//always
		case 0xf: return APDU_INCONTRADICTION_W_CHV;		//never
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
			if(_chv_status[chv_no - 1] & CHV_BLOCKED) return APDU_INVALID_STATUS;		//status is invalid
			if(_chv_status[chv_no - 1] & CHV_ENABLED && (_chv_status[chv_no - 1] & CHV_VERIFIED) == 0) return APDU_INCONTRADICTION_W_CHV;		//status is invalid
			break;
		default: return APDU_INCONTRADICTION_W_CHV;		//never
	}
	return APDU_SUCCESS;
}

/* added, fast select mode may 01 2013 */
#define USE_FAST_SELECT			1
uint16 _select(fs_handle * handle, uint16 fid) _REENTRANT_ {
	#if USE_FAST_SELECT
	f_header head;
	#else
	f_header * head;
	#endif
	#if FS_ADDRESS_WIDTH == AW_32BIT
	uint32 ptr;
	uint32 cptr;
	uint32 cparent;
	#else
	uint16 ptr;
	uint16 cptr;
	uint16 cparent;
	#endif
	#define SELECT_TYPE_ALL			3
	#define SELECT_TYPE_DF			2
	#define SELECT_TYPE_EF			1
	uchar select_type = SELECT_TYPE_ALL;
	handle->cur_rd_rec = -1;
	handle->cur_wr_rec = 0;
	if(fid == FID_MF) {
		handle->cur_ptr = ALLOCATION_DATA_OFFSET + sizeof(fs_chain);
		handle->cur_dir = ALLOCATION_DATA_OFFSET + sizeof(fs_chain);
		return APDU_SUCCESS_RESPONSE | 0x17;
	}
	cptr = handle->cur_dir;
	re_select:
	#if USE_FAST_SELECT
	ioman_read_buffer(cptr, &head, sizeof(f_header));
	cparent = head.parent;
	if(head.fid == fid) { handle->cur_ptr = cptr; return APDU_SUCCESS_RESPONSE | 0x17; }
	ptr = head.child;
	#else
	head = (f_header *)file_get_header(cptr);
	cparent = head->parent;
	if(head->fid == fid) { m_free(head); handle->cur_ptr = cptr; return APDU_SUCCESS_RESPONSE | 0x17; }
	ptr = head->child;
	#endif
	/*if(head->parent != 0) {
		//m_free(head);
		head = file_get_header(head->parent);
		if(head->fid = fid) { m_free(head); return APDU_SUCCESS_RESPONSE | 0x17; }
	}*/
	while(ptr != 0) {
		#if USE_FAST_SELECT
		ioman_read_buffer(ptr, &head, sizeof(f_header));
		#else
		m_free(head);
		head = file_get_header(ptr);
		#endif

		#if USE_FAST_SELECT
		if(head.fid == fid) {
		#else
		if(head->fid == fid) {
		#endif
			//printf("fid %x ketemu\n", fid);
			#if USE_FAST_SELECT	
			//switch(head.type) {
			if(head.type == T_EF) {
			#else
			if(head->type == T_EF) {
			//switch(head->type) {
			#endif 
				//case T_EF: 
					#if !USE_FAST_SELECT
					m_free(head);
					#endif
					if(select_type & SELECT_TYPE_EF) {
						handle->cur_ptr = ptr;
						return APDU_SUCCESS_RESPONSE | 0x0F;
					} else return APDU_FILE_NOT_FOUND; 
					//break;
			#if USE_FAST_SELECT	
			} else if(head.type == T_DF) {
			#else
			} else if(head->type == T_DF) {
			#endif
				//case T_MF:
				//case T_DF: 
					#if !USE_FAST_SELECT	
					m_free(head);
					#endif
					if(select_type & SELECT_TYPE_DF) {
						handle->cur_ptr = ptr;	
						handle->cur_dir = ptr;
						return APDU_SUCCESS_RESPONSE | 0x17;
					} else return APDU_FILE_NOT_FOUND;
					//break;
			} else {
				//case T_CHV:
				//case T_RFU:
				//default:
					#if !USE_FAST_SELECT
					m_free(head);
					#endif
					return APDU_FILE_NOT_FOUND; 		//the file is invalid
			}
		}
		#if USE_FAST_SELECT 
		ptr = head.sibling;
		#else
		ptr = head->sibling;
		#endif
	}
	/* fixed bug, on file select, selecting sibling directory january,23,2013 */
	/* fixed bug, on file select, selecting sibling directory only may 02 2013 */
	if(handle->cur_dir == cptr && handle->cur_dir != (ALLOCATION_DATA_OFFSET + sizeof(fs_chain))) {		//file not found and current file is directory
		cptr = cparent;
		select_type = SELECT_TYPE_DF;
		#if !USE_FAST_SELECT
		m_free(head);
		#endif
		goto re_select;				
	}
	#if !USE_FAST_SELECT		
	m_free(head);
	#endif

	return APDU_FILE_NOT_FOUND; 
} 

uint16 _check_access(fs_handle * handle, uchar mode) _REENTRANT_ {
	ef_header * head;
	uint16 status = APDU_SUCCESS;
	head = (ef_header *)file_get_efheader(handle->cur_ptr);	
	status = file_check_status(head, mode);
	m_free(head);
	return status;
}

uint16 _readbin(fs_handle * handle, uint16 offset, uchar *buffer, uint16 size) _REENTRANT_ {
	register uint16 status = APDU_SUCCESS;
	ef_header * head;
	head = (ef_header *)file_get_efheader(handle->cur_ptr);
	//status = file_check_status(head, FILE_READ);
	//if(status != APDU_SUCCESS) { m_free(head); return status; }
	if((offset + size) > head->size) { m_free(head); return APDU_WRONG_LENGTH; }
	ioman_read_buffer(handle->cur_ptr + offset + sizeof(ef_header), buffer, size);
	m_free(head);
	return status;
}

uint16 _readrec(fs_handle * handle, uint16 rec_no, uchar *buffer, uchar size) _REENTRANT_ {
	register uint16 status = APDU_SUCCESS;
	uint16 total_rec;
	ef_header * head;
	head = (ef_header *)file_get_efheader(handle->cur_ptr);
	//status = file_check_status(head, FILE_READ);
	//if(status != APDU_SUCCESS) { m_free(head); return status; }
	if(size != head->rec_size) { m_free(head); status = APDU_WRONG_LENGTH; return status; }
	total_rec = (head->size/head->rec_size);
	switch(head->structure) {
		case EF_LINIER:
			if(rec_no < total_rec) {
				rec_no = ((rec_no + head->child) % total_rec);
				ioman_read_buffer(handle->cur_ptr + (head->rec_size * rec_no ) + sizeof(ef_header), buffer, size);
				handle->cur_rd_rec = rec_no;
				handle->cur_wr_rec = rec_no;
				break;
			}
			status = APDU_OUT_OF_RANGE;
			break;
		case EF_CYCLIC:	
			//_cur_rd_rec = rec_no;	
			handle->cur_rd_rec = ((rec_no + total_rec) % total_rec);
			rec_no = ((handle->cur_rd_rec + head->child) % total_rec);
			ioman_read_buffer(handle->cur_ptr + (head->rec_size * rec_no ) + sizeof(ef_header), buffer, size);
			//_cur_rd_rec = rec_no;
			break;
		default:
			status = APDU_FILE_INCONSISTENT;
			break;
	}
	m_free(head);
	return status;
}

uint16 _readrec_next(fs_handle * handle, uchar *buffer, uchar size) _REENTRANT_ {
	int16 rec_no = handle->cur_rd_rec;	
	rec_no += 1;
	return _readrec(handle, rec_no, buffer, size);
}

uint16 _readrec_prev(fs_handle * handle, uchar *buffer, uchar size) _REENTRANT_ {
	int16 rec_no = handle->cur_rd_rec;	
	rec_no -= 1;
	return _readrec(handle, rec_no, buffer, size);
}

uint16 _writebin(fs_handle * handle, uint16 offset, uchar *buffer, uint16 size) _REENTRANT_ {
	register uint16 status = APDU_SUCCESS;
	ef_header * head;
	head = (ef_header *)file_get_efheader(handle->cur_ptr);
	//status = file_check_status(head, FILE_WRITE);
	//if(status != APDU_SUCCESS) { m_free(head); return status; }
	if((offset + size) > head->size) { m_free(head); return APDU_WRONG_LENGTH; }
	status = ioman_write_buffer(handle->cur_ptr + offset + sizeof(ef_header), buffer, size);
	m_free(head);
	return status;
}

uint16 _writerec(fs_handle * handle, uint16 rec_no, uchar *buffer, uchar size) _REENTRANT_ {
	register uint16 status = APDU_SUCCESS;
	uint16 total_rec;
	ef_header * head;
	head = (ef_header *)file_get_efheader(handle->cur_ptr);
	//status = file_check_status(head, FILE_WRITE);
	//if(status != APDU_SUCCESS) { m_free(head); return status; }
	if(size != head->rec_size) { m_free(head); return APDU_WRONG_LENGTH; }
	switch(head->structure) {
		case EF_LINIER:
			total_rec = (head->size/head->rec_size);
			if(rec_no < total_rec) {
				status = ioman_write_buffer(handle->cur_ptr + (head->rec_size * rec_no) + sizeof(ef_header), buffer, size);
				handle->cur_rd_rec = rec_no;
				handle->cur_wr_rec = rec_no;
			} else {
				status = APDU_OUT_OF_RANGE;
			}
			break;
		case EF_CYCLIC:
			total_rec = (head->size/head->rec_size);
			rec_no = (rec_no + head->child + total_rec) % total_rec;
			if(rec_no == ((total_rec + head->child - 1) % total_rec)) {
				status = ioman_write_buffer(handle->cur_ptr + (head->rec_size * rec_no) + sizeof(ef_header), buffer, size);
				if(status == APDU_SUCCESS) {
				head->child = rec_no;													//update first record index
					handle->cur_wr_rec = 0;
					handle->cur_rd_rec = 0;
					status = ioman_write_buffer(handle->cur_ptr, head, sizeof(ef_header));
				}
			} else {	
				status = APDU_WRONG_PARAMETER;
			}
			break;
		default: 
			status = APDU_FILE_INCONSISTENT;
			break;
	}
	m_free(head);
	return status;
}

uint16 _writerec_next(fs_handle * handle, uchar *buffer, uchar size) _REENTRANT_ {
	int16 rec_no = handle->cur_wr_rec;	
	rec_no += 1;
	return _writerec(handle, rec_no, buffer, size);
}

uint16 _writerec_prev(fs_handle * handle, uchar *buffer, uchar size) _REENTRANT_ {
	int16 rec_no = handle->cur_wr_rec;	
	rec_no -= 1;
	return _writerec(handle, rec_no, buffer, size);
}

uint16 _seek(fs_handle * handle, uchar type, uchar * buffer, uchar * pattern, uchar size) _REENTRANT_ {
	register uint16 status = APDU_SUCCESS;
	register uint16 total_rec;
	static uint16 rec_no = 0;
	int16 * kmpstates = NULL;
	uchar * temp = NULL;
	//int16 index;
	ef_header * head;
	head = (ef_header *)file_get_efheader(handle->cur_ptr);
	//status = file_check_status(head, FILE_READ);
	//if(status != APDU_SUCCESS) { m_free(head); return status; }
	if(head->structure != EF_LINIER && head->structure != EF_CYCLIC) { m_free(head); return APDU_FILE_INCONSISTENT; }
	if(size >= head->rec_size) { m_free(head); status = APDU_WRONG_LENGTH; return status; }
	total_rec = (head->size/head->rec_size);						
	kmpstates = (int16 *)m_alloc((size + 1) * sizeof(int16));	 //temporary buffer for kmpstates				 
	if(kmpstates == NULL) { m_free(head); return APDU_MEMORY_PROBLEM; }			//check for allocated heap
	temp = (uchar *)m_alloc(size);	 	//temporary buffer for pattern
	if(temp == NULL) { m_free(kmpstates); m_free(head); return APDU_MEMORY_PROBLEM; }	//check for allocated heap
	memcpy(temp, pattern, size);
	KMP_preprocess(pattern, size, kmpstates);		//preprocess KMP
	switch((type & 0x0f)) {
		case 0:		//begining forward
			rec_no = 0;
			while(rec_no < total_rec) {
				ioman_read_buffer(handle->cur_ptr + (head->rec_size * ((rec_no + head->child) % total_rec) ) + sizeof(ef_header), buffer, head->rec_size);
				//index = KMP_search(temp, kmpstates, size, buffer, head->rec_size);
				if(KMP_search(temp, kmpstates, size, buffer, head->rec_size) != -1) { status = (APDU_SUCCESS_RESPONSE | (uchar)(rec_no+1)); goto seek_exit; }
				rec_no++;
			}
			rec_no = 0;
			status = APDU_FILE_NOT_FOUND;
			break;
		case 1:		//end backward
			rec_no = total_rec - 1;
			while(rec_no != 0xFFFF ) {
				ioman_read_buffer(handle->cur_ptr + (head->rec_size * ((rec_no + head->child) % total_rec) ) + sizeof(ef_header), buffer, head->rec_size);
				if(KMP_search(temp, kmpstates, size, buffer, head->rec_size) != -1) { status = (APDU_SUCCESS_RESPONSE | (uchar)(rec_no+1)); goto seek_exit; }
				rec_no--;
			}
			rec_no = total_rec - 1;
			status = APDU_FILE_NOT_FOUND;
			break;
		case 2:	   	//next location backward
			rec_no++;
			while(rec_no < total_rec) {
				ioman_read_buffer(handle->cur_ptr + (head->rec_size * ((rec_no + head->child) % total_rec) ) + sizeof(ef_header), buffer, head->rec_size);
				if(KMP_search(temp, kmpstates, size, buffer, head->rec_size) != -1) { status = (APDU_SUCCESS_RESPONSE | (uchar)(rec_no+1)); goto seek_exit; }
				rec_no++;
			}
			rec_no = 0;
			status = APDU_FILE_NOT_FOUND;
			break;
		case 3:		//previous location backward
			rec_no--;
			while(rec_no != 0xFFFF ) {
				ioman_read_buffer(handle->cur_ptr + (head->rec_size * ((rec_no + head->child) % total_rec) ) + sizeof(ef_header), buffer, head->rec_size);
				if(KMP_search(temp, kmpstates, size, buffer, head->rec_size) != -1) { status = (APDU_SUCCESS_RESPONSE | (uchar)(rec_no+1)); goto seek_exit; }
				rec_no--;
			}
			rec_no = total_rec - 1;
			status = APDU_FILE_NOT_FOUND;
			break;
		default: 
			status = APDU_WRONG_PARAMETER;
			break;
	}
	/*if(rec_no < total_rec) {
		ioman_read_buffer(_cur_ptr + (head->rec_size * ((rec_no + head->child) % total_rec) ) + sizeof(ef_header), buffer, size);
		_cur_rec = rec_no;
		m_free(head);
		return status;
	} */
	seek_exit:
	m_free(temp);
	m_free(kmpstates);
	m_free(head);
	//status = APDU_OUT_OF_RANGE;
	return status;
}

uint16 _increase(fs_handle * handle, uint32 ival, uchar * res, uchar size) _REENTRANT_ {
	register uint16 status = APDU_SUCCESS;
	//register uint16 total_rec;
	uint32 value = 0;
	//uchar buffer[3];
	//uchar * buffer = &value;
	//int16 index;
	ef_header * head;
	value = 0;
	head = (ef_header *)file_get_efheader(handle->cur_ptr);
	//status = file_check_status(head, FILE_READ);
	//if(status != APDU_SUCCESS) { m_free(head); return status; }
	if(head->structure != EF_CYCLIC) { m_free(head); return APDU_FILE_INCONSISTENT; }
	if(size != 3) { m_free(head); status = APDU_WRONG_LENGTH; return status; }
	if(head->inc == 0 || head->rec_size != 3) { m_free(head); return APDU_ACCESS_DENIED; }
	//total_rec = (head->size/head->rec_size);
	if((status = _readrec(handle, 0, res+3, 3)) != APDU_SUCCESS) { m_free(head); return status; }
	value = ((uint32)res[3] << 16) + ((uint32)res[4] << 8) + res[5];
	value = value + ival;
	if(value >= 0x1000000) { m_free(head); return APDU_MAX_VALUE_REACHED; }
	res[0] = (uchar)(value >> 16) ;
	res[1] = (uchar)(value >> 8) ;
	res[2] = (uchar)(value) ;
	if((status = _writerec_prev(handle, res, 3)) != APDU_SUCCESS) { m_free(head); return status; } 
	m_free(head);
	//status = APDU_OUT_OF_RANGE;
	return APDU_SUCCESS_RESPONSE | 6;
}

uint16 _createfilebin(fs_handle * handle, uint16 fid, uchar acc_rw, uchar acc_io, acc_ri, uint16 size) _REENTRANT_ {
	uint16 status;
	ef_header * head;
	df_header * dir;
	#if FS_ADDRESS_WIDTH == AW_32BIT
	uint32 address;
	#else
	uint16 address;
	#endif
	address = fs_alloc(size + sizeof(ef_header));				//allocate new fs linked list
	if(address == 0) return APDU_NO_AVAILABLE_SPACE;	 		//exceed file system limitation
	dir = (df_header *)file_get_dfheader(handle->cur_dir);
	head = (ef_header *)m_alloc(sizeof(ef_header));
	head->parent = handle->cur_dir;
	head->sibling = 0;
	head->child = 0;											//not use, only for cyclic
	head->fid = fid;
	head->type = T_EF;
	head->size = size;
	head->inc = 0;
	head->status = STAT_VALID;
	head->acc_rw = acc_rw;
	head->acc_inc = acc_io;
	head->acc_ri = acc_ri;
	head->length = 2;											//length of the following data
	head->structure = EF_TRANSPARENT;
	head->rec_size = 0x00;
	status = ioman_write_buffer(address, head, sizeof(ef_header));		//write the specified allocated address
	if(status != APDU_SUCCESS) goto exit_create;
	dir->num_of_ef += 1;
	status = ioman_write_buffer(handle->cur_dir, dir, sizeof(df_header));		//
	if(status != APDU_SUCCESS) goto exit_create; 
	status = file_add_entry(handle, address);									//create new file entry for current directory
	exit_create: 
	m_free(head);												//freed all temporary variable
	m_free(dir);
	return status;
}

uint16 _createfilerec(fs_handle * handle, uint16 fid, uchar acc_rw, uchar acc_io, uchar acc_ri, uchar total_rec, uchar rec_size) _REENTRANT_ {
	uint16 status;
	ef_header * head;
	df_header * dir;
	#if FS_ADDRESS_WIDTH == AW_32BIT
	uint32 address;
	#else
	uint16 address;
	#endif
	address = fs_alloc((total_rec * rec_size) + sizeof(ef_header));				//allocate new fs linked list
	if(address == 0) return APDU_NO_AVAILABLE_SPACE;	 		//exceed file system limitation
	dir = (df_header *)file_get_dfheader(handle->cur_dir);
	head = (ef_header *)m_alloc(sizeof(ef_header));
	head->parent = handle->cur_dir;
	head->sibling = 0;
	head->child = 0;											//not use, only for cyclic
	head->fid = fid;
	head->type = T_EF;
	head->size = (total_rec * rec_size);
	head->inc = 0;												//no increase allowed
	head->acc_rw = acc_rw;
	head->acc_inc = acc_io;
	head->acc_ri = acc_ri;
	head->status = STAT_VALID;
	head->length = 2;			//length of the following data
	head->structure = EF_LINIER;
	head->rec_size = rec_size;
	status = ioman_write_buffer(address, head, sizeof(ef_header));		//write the specified allocated address
	if(status != APDU_SUCCESS) goto exit_create;
	dir->num_of_ef += 1;
	status = ioman_write_buffer(handle->cur_dir, dir, sizeof(df_header));		//
	if(status != APDU_SUCCESS) goto exit_create;
	status = file_add_entry(handle, address);									//create new file entry for current directory
	exit_create:
	m_free(head);												//freed all temporary variable
	m_free(dir);												//freed all temporary variable
	return status;
}

uint16 _createfilecyclic(fs_handle * handle, uint16 fid, uchar acc_rw, uchar acc_io, uchar acc_ri, uchar total_rec, uchar rec_size) _REENTRANT_ {
	uint16 status;
	ef_header * head;
	df_header * dir;
	#if FS_ADDRESS_WIDTH == AW_32BIT
	uint32 address;
	#else
	uint16 address;
	#endif
	address = fs_alloc((total_rec * rec_size) + sizeof(ef_header));				//allocate new fs linked list
	if(address == 0) return APDU_NO_AVAILABLE_SPACE;	 		//exceed file system limitation
	dir = (df_header *)file_get_dfheader(handle->cur_dir);
	head = (ef_header *)m_alloc(sizeof(ef_header));
	head->parent = handle->cur_dir;
	head->sibling = 0;
	head->child = 0;											//not use, only for cyclic
	head->fid = fid;
	head->type = T_EF;
	head->size = total_rec * rec_size;
	head->inc = 1;												//no increase allowed
	head->status = STAT_VALID;
	head->acc_rw = acc_rw;
	head->acc_inc = acc_io;
	head->acc_ri = acc_ri;
	head->length = 2;			//length of the following data
	head->structure = EF_CYCLIC;
	head->rec_size = rec_size;
	status = ioman_write_buffer(address, head, sizeof(ef_header));		//write the specified allocated address
	if(status != APDU_SUCCESS) goto exit_create;
	dir->num_of_ef += 1;
	status = ioman_write_buffer(handle->cur_dir, dir, sizeof(df_header));		//
	if(status != APDU_SUCCESS) goto exit_create;
	status = file_add_entry(handle, address);									//create new file entry for current directory
	exit_create:
	m_free(head);												//freed all temporary variable
	m_free(dir);												//freed all temporary variable
	return status;
}

uint16 _createdirectory(fs_handle * handle, uint16 fid, uchar filechar) _REENTRANT_ {
	register uint16 status;
	df_header * head;
	df_header * dir;
	#if FS_ADDRESS_WIDTH == AW_32BIT
	uint32 address;
	#else
	uint16 address;
	#endif
	address = fs_alloc(sizeof(df_header));						//allocate new fs linked list
	if(address == 0) return APDU_NO_AVAILABLE_SPACE;	 		//exceed file system limitation
	dir = (df_header *)file_get_dfheader(handle->cur_dir);	
	head = (df_header *)m_alloc(sizeof(df_header));
	head->parent = handle->cur_dir;
	head->sibling = 0;
	head->child = 0;											//not use, only for cyclic
	head->fid = fid;
	head->type = T_DF;
	head->length = 9;											//length of the following data
	head->file_char = filechar;										//not clarified
	head->num_of_df = 0;
	head->num_of_ef = 0;
	head->num_of_chv = 2;										//just set to 2
	status = ioman_write_buffer(address, head, sizeof(df_header));		//write the specified allocated address
	if(status != APDU_SUCCESS) goto exit_create;
	dir->num_of_df += 1;
	status = ioman_write_buffer(handle->cur_dir, dir, sizeof(df_header));		//
	if(status != APDU_SUCCESS) goto exit_create;
	status = file_add_entry(handle, address);									//create new file entry for current directory
	exit_create:	
	m_free(head);												//freed all temporary variable
	m_free(dir);
	return status;
}

uint16 _remove(fs_handle * handle, uint16 file_id) _REENTRANT_ {
	register uint16 status;
	f_header * head;
	df_header * dir;
	#if FS_ADDRESS_WIDTH == AW_32BIT
	uint32 address;
	#else
	uint16 address;
	#endif
	if(file_id == FID_MF) return APDU_FUNCTION_INVALID;		//check if MF
	head = (f_header *)file_get_header(handle->cur_ptr);	//delete current selected
	if(head->fid == file_id) {
		handle->cur_dir = head->parent;
		m_free(head);
	}
	handle->cur_ptr = handle->cur_dir;
	dir = (df_header *)file_get_dfheader(handle->cur_dir);
	address = file_find_entry(handle, file_id);
	if(address == 0) { m_free(dir); return APDU_FILE_NOT_FOUND; } 
	head = (f_header *)file_get_header(address);
	switch(head->type) {
		case T_DF:
			dir->num_of_df -= 1;
			//deallocating all childs
			/* fixed bug when deleting all childs on the DF */
			/* added childs deallocator may 01 2013 */
			file_dealloc_all_child(address);
			break;
		case T_EF:
			dir->num_of_ef -= 1;
			break;
		default:
			break;
	}
	if(head->parent != 0) handle->cur_dir = head->parent;		//set current directory to the specified entry parent
	status = ioman_write_buffer(handle->cur_dir, dir, sizeof(df_header));		//
	if(status != APDU_SUCCESS) goto exit_remove;
	status = file_remove_entry(handle, address);						//remove entry from file tree
	if(status != APDU_SUCCESS) goto exit_remove;
	status = fs_dealloc(address);							//dealloc entry from fs linked list
	exit_remove:
	m_free(head);												//freed all temporary variable
	m_free(dir);
	return status;
}

uint16 _invalidate(fs_handle * handle) _REENTRANT_ {
	register uint16 status = APDU_SUCCESS;
	ef_header * head;
	head = (ef_header *)file_get_efheader(handle->cur_ptr);
	//status = file_check_status(head, FILE_INVALIDATE);
	//if(status != APDU_SUCCESS) { m_free(head); return status; }
	head->status = STAT_INVALID;
	status = ioman_write_buffer(handle->cur_ptr, head, sizeof(ef_header));
	m_free(head);												//freed all temporary variable
	return status;
}

uint16 _rehabilitate(fs_handle * handle) _REENTRANT_ {
	register uint16 status = APDU_SUCCESS;
	ef_header * head;
	head = (ef_header *)file_get_efheader(handle->cur_ptr);
	//status = file_check_status(head, FILE_REHABILITATE);
	//if(status != APDU_SUCCESS) { m_free(head); return status; }
	head->status = STAT_VALID;
	status = ioman_write_buffer(handle->cur_ptr, head, sizeof(ef_header));
	m_free(head);												//freed all temporary variable
	return status;
}
#endif







