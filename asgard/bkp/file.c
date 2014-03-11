/* An upper layer of Asgard File System, which provide API(s)
 * for user application
 *
 * Copyright 2010, Agus Purwanto.
 * All rights reserved.
 *
 * 
 */

//#include "file.h"
#include "..\defs.h"
#include "..\midgard\midgard.h"	
#include "..\misc\mem.h"
//#if ASGARD_VERSION == 4
#include "..\drivers\ioman.h"
#include "fs.h"
#include "security.h"
#include "file.h"

//#if FS_ADDRESS_WIDTH == AW_32BIT
//uint32 _cur_ptr = ALLOCATION_DATA_OFFSET + sizeof(fs_chain);
//uint32 _cur_dir = ALLOCATION_DATA_OFFSET + sizeof(fs_chain);
//uint32 _cur_rec;
//#else
uint16 _cur_ptr = ALLOCATION_DATA_OFFSET + sizeof(fs_chain);
uint16 _cur_dir = ALLOCATION_DATA_OFFSET + sizeof(fs_chain);
uint16 _cur_rec;
//#endif
//fungsi file_get_header paling sering diakses
//#if FS_ADDRESS_WIDTH == AW_32BIT
//f_header * file_get_header(uint32 address)
//#else
f_header * file_get_header(uint16 address)
//#endif
{
	f_header *newfile;
	newfile = (f_header *) m_alloc (sizeof(f_header));
	ioman_read_buffer(address, newfile, sizeof(f_header));
	return newfile;	//jangan dihapus karena akan dipakai
}

//#if FS_ADDRESS_WIDTH == AW_32BIT
//df_header * file_get_dfheader(uint32 address)
//#else
df_header * file_get_dfheader(uint16 address)
//#endif
{
	df_header *newfile;
	newfile = (df_header *) m_alloc (sizeof(df_header));
	ioman_read_buffer(address, newfile, sizeof(df_header));
	return newfile;	//jangan dihapus karena akan dipakai
}

//#if FS_ADDRESS_WIDTH == AW_32BIT
//ef_header * file_get_efheader(uint32 address)
//#else
ef_header * file_get_efheader(uint16 address)
//#endif
{
	ef_header *newfile;
	newfile = (ef_header *) m_alloc (sizeof(ef_header));
	ioman_read_buffer(address, newfile, sizeof(ef_header));
	return newfile;	//jangan dihapus karena akan dipakai
}

void * file_get_current_header(void) {
	if(_cur_ptr == _cur_dir) {
	 	return (void *)file_get_dfheader(_cur_ptr); 
	}
	return (void *)file_get_efheader(_cur_ptr);
}

//#if FS_ADDRESS_WIDTH == AW_32BIT
//void file_add_entry(uint32 address)	{
//	uint32 ptr;
//#else
void file_add_entry(uint16 address) {
	uint16 ptr;
//#endif	
	f_header * head;
	f_header * temp;
	temp = file_get_header(address);
	head = file_get_header(_cur_dir);
	if(head->child == 0) { 
		//printf("add entry to child\n");
		head->child = address; 
		ioman_write_buffer(_cur_dir, head, sizeof(f_header)); 
		m_free(head); 
		m_free(temp);
		return; 
	}
	ptr = head->child;
	while(ptr != 0) {
		m_free(head);
		head = file_get_header(ptr);
		if(head->sibling == 0) {
			//printf("add entry as sibling\n");
			head->sibling = address;
			ioman_write_buffer(ptr, head, sizeof(f_header)); 
			m_free(head); 
			m_free(temp);
			return;
		}
		ptr = head->sibling;
	}		
	m_free(head);
	m_free(temp);
	return;
}


//#if FS_ADDRESS_WIDTH == AW_32BIT
//void file_remove_entry(uint32 address) {
//	uint32 ptr;
//#else
void file_remove_entry(uint16 address) {
	uint16 ptr;
//#endif	
	f_header * head;
	f_header * temp;
	temp = file_get_header(address);
	head = file_get_header(_cur_dir);
	if(head->child == address) { 
		head->child = temp->sibling;
		ioman_write_buffer(_cur_dir, head, sizeof(f_header)); 
		m_free(head); 
		m_free(temp); 
		return; 
	}
	ptr = head->child;
	while(ptr != 0) {
		m_free(head);
		head = file_get_header(ptr);
		if(head->sibling == address) {
			head->sibling = temp->sibling; 
			ioman_write_buffer(ptr, head, sizeof(f_header)); 
			m_free(head); 
			m_free(temp); 
			return;
		}
		ptr = head->sibling;
	}		
	m_free(head);
	return;
}

//#if FS_ADDRESS_WIDTH == AW_32BIT
//uint32 file_find_entry(uint16 fid) {
//	uint32 ptr;
//#else
uint16 file_find_entry(uint16 fid) {
	uint16 ptr;
//#endif
	f_header * head;
	head = file_get_header(_cur_dir);
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

uint16 file_check_status(ef_header * head, uchar op) {
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
			if(_chv_status[chv_no - 1] != CHV_VERIFIED) return APDU_INCONTRADICTION_W_CHV;		//status is invalid
			break;
		default: return APDU_INCONTRADICTION_W_CHV;		//never
	}
	return APDU_SUCCESS;
}

uint16 _select(uint16 fid) {
	f_header * head;
	//#if FS_ADDRESS_WIDTH == AW_32BIT
	//uint32 ptr;
	//#else
	uint16 ptr;
	//#endif
	if(fid == FID_MF) {
		_cur_ptr = ALLOCATION_DATA_OFFSET + sizeof(fs_chain);
		_cur_dir = ALLOCATION_DATA_OFFSET + sizeof(fs_chain);
		return APDU_SUCCESS_RESPONSE | 0x17;
	}
	head = (f_header *)file_get_header(_cur_dir);
	if(head->fid == fid) { m_free(head); return APDU_SUCCESS_RESPONSE | (sizeof(df_header) - sizeof(f_header) + 3); }
	ptr = head->child;
	if(head->parent != 0) {
		head = file_get_header(head->parent);
		if(head->fid = fid) { m_free(head); return APDU_SUCCESS_RESPONSE | (sizeof(df_header) - sizeof(f_header) + 3); }
	}
	//printf("ptr : %x\n", ptr);
	while(ptr != 0) {
		m_free(head);
		head = file_get_header(ptr);
		if(head->fid == fid) {
			//printf("fid %x ketemu\n", fid);
			_cur_ptr = ptr;
			switch(head->type) {
				case T_MF:
				case T_DF:	
					_cur_dir = ptr;	
					m_free(head);
					return APDU_SUCCESS_RESPONSE | 0x17;
				case T_EF:
					m_free(head);
					return APDU_SUCCESS_RESPONSE | 0x0F;
				case T_CHV:
				case T_RFU:
				default:
					m_free(head);
					return APDU_FILE_NOT_FOUND; 		//the file is invalid
			}
		}
		ptr = head->sibling;
	}		
	m_free(head);
	return APDU_FILE_NOT_FOUND; 
}

uint16 _readbin(uint16 offset, uchar *buffer, uint16 size) {
	register uint16 status = APDU_SUCCESS;
	ef_header * head;
	head = (ef_header *)file_get_efheader(_cur_ptr);
	status = file_check_status(head, FILE_READ);
	if(status != APDU_SUCCESS) { m_free(head); return status; }
	if((offset + size) > head->size) { m_free(head); return APDU_WRONG_LENGTH; }
	ioman_read_buffer(_cur_ptr + offset + sizeof(ef_header), buffer, size);
	m_free(head);
	return status;
}

uint16 _readrec(uint16 rec_no, uchar *buffer, uchar size) {
	register uint16 status = APDU_SUCCESS;
	register uint16 total_rec;
	ef_header * head;
	head = (ef_header *)file_get_efheader(_cur_ptr);
	status = file_check_status(head, FILE_READ);
	if(status != APDU_SUCCESS) { m_free(head); return status; }
	if(size != head->rec_size) { m_free(head); status = APDU_WRONG_LENGTH; return status; }
	total_rec = (head->size/head->rec_size);
	if(rec_no < total_rec) {
		ioman_read_buffer(_cur_ptr + (head->rec_size * ((rec_no + head->child) % total_rec) ) + sizeof(ef_header), buffer, size);
		_cur_rec = rec_no;
		m_free(head);
		return status;
	}
	m_free(head);
	status = APDU_OUT_OF_RANGE;
	return status;
}

uint16 _readrec_next(uchar *buffer, uchar size) {
	uint16 rec_no = _cur_rec + 1;
	return _readrec(rec_no, buffer, size);
}

uint16 _readrec_prev(uchar *buffer, uchar size) {	
	uint16 rec_no = _cur_rec - 1;
	return _readrec(rec_no, buffer, size);
}

uint16 _writebin(uint16 offset, uchar *buffer, uint16 size) {
	register uint16 status = APDU_SUCCESS;
	ef_header * head;
	head = (ef_header *)file_get_efheader(_cur_ptr);
	status = file_check_status(head, FILE_WRITE);
	if(status != APDU_SUCCESS) { m_free(head); return status; }
	if((offset + size) > head->size) { m_free(head); return APDU_WRONG_LENGTH; }
	ioman_write_buffer(_cur_ptr + offset + sizeof(ef_header), buffer, size);
	m_free(head);
	return status;
}

uint16 _write_new_rec(uint16 rec_no, uchar size, uchar *buffer) {

}

uint16 _writerec(uint16 rec_no, uchar *buffer, uchar size) {
	register uint16 status = APDU_SUCCESS;
	uint16 total_rec;
	ef_header * head;
	head = (ef_header *)file_get_efheader(_cur_ptr);
	status = file_check_status(head, FILE_WRITE);
	if(status != APDU_SUCCESS) { m_free(head); return status; }
	if(size != head->rec_size) { m_free(head); return APDU_WRONG_LENGTH; }
	switch(head->structure) {
		case EF_LINIER:
			total_rec = (head->size/head->rec_size);
			if(rec_no < total_rec) {
				ioman_write_buffer(_cur_ptr + (head->rec_size * rec_no) + sizeof(ef_header), buffer, size);
				_cur_rec = rec_no;
				m_free(head);
				return status;
			}
			m_free(head);
			status = APDU_OUT_OF_RANGE;
			return status;
		case EF_CYCLIC:
			total_rec = (head->size/head->rec_size);
			rec_no = rec_no % total_rec;
			if(rec_no == ((total_rec + head->child - 1) % total_rec)) {
				ioman_write_buffer(_cur_ptr + (head->rec_size * rec_no) + sizeof(ef_header), buffer, size);
				head->child = rec_no;													//update first record index
				_cur_rec = rec_no;
				ioman_write_buffer(_cur_ptr, head, sizeof(ef_header));
				m_free(head);
				return status;
			}
			m_free(head);
			status = APDU_WRONG_PARAMETER;
			return status;
		default:
			m_free(head);
			status = APDU_FILE_INCONSISTENT;
			return status;
	}
	m_free(head);
	status = APDU_OUT_OF_RANGE;
	return status;
}

uint16 _writerec_next(uchar *buffer, uchar size) {
	uint16 rec_no = _cur_rec + 1;
	return _writerec(rec_no, buffer, size);
}

uint16 _writerec_prev(uchar *buffer, uchar size) {
	uint16 rec_no = _cur_rec - 1;
	return _writerec(rec_no, buffer, size);
}

uint16 _createfilebin(uint16 fid, uchar ACC_READ, uchar ACC_WRT, uchar ACC_INC, uchar ACC_INV, uchar ACC_RHB, uchar *buffer, uint16 size) {
	ef_header * head;
	df_header * dir;
	#if FS_ADDRESS_WIDTH == AW_32BIT
	uint32 address;
	#else
	uint16 address;
	#endif
	dir = (df_header *)file_get_dfheader(_cur_dir);
	address = fs_alloc(size + sizeof(ef_header));				//allocate new fs linked list
	head = (ef_header *)m_alloc(sizeof(ef_header));
	head->parent = _cur_dir;
	head->sibling = 0;
	head->child = 0;											//not use, only for cyclic
	head->fid = fid;
	head->type = T_EF;
	head->size = size;
	head->inc = 0;
	head->status = STAT_VALID;
	head->acc_rw = (ACC_READ << 4) | (ACC_WRT & 0x0F);
	head->acc_inc = (ACC_INC << 4);
	head->acc_ri = (ACC_RHB << 4) | (ACC_INV & 0x0F);
	head->length = 2;											//length of the following data
	head->structure = EF_TRANSPARENT;
	head->rec_size = 0x00;
	ioman_write_buffer(address, head, sizeof(ef_header));		//write the specified allocated address
	dir->num_of_ef += 1;
	ioman_write_buffer(_cur_dir, dir, sizeof(df_header));		//
	m_free(head);												//freed all temporary variable
	file_add_entry(address);									//create new file entry for current directory
	m_free(dir);
	return APDU_SUCCESS;
}

uint16 _createfilerec(uint16 fid, uchar ACC_READ, uchar ACC_WRT, uchar ACC_INC, uchar ACC_INV, uchar ACC_RHB, uchar total_rec, uchar rec_size) {
	ef_header * head;
	df_header * dir;
	#if FS_ADDRESS_WIDTH == AW_32BIT
	uint32 address;
	#else
	uint16 address;
	#endif
	dir = (df_header *)file_get_dfheader(_cur_dir);
	address = fs_alloc((total_rec * rec_size) + sizeof(ef_header));				//allocate new fs linked list
	head = (ef_header *)m_alloc(sizeof(ef_header));
	head->parent = _cur_dir;
	head->sibling = 0;
	head->child = 0;											//not use, only for cyclic
	head->fid = fid;
	head->type = T_EF;
	head->size = total_rec * rec_size;
	head->inc = 0;												//no increase allowed
	head->status = STAT_VALID;
	head->acc_rw = (ACC_READ << 4) | (ACC_WRT & 0x0F);
	head->acc_inc = (ACC_INC << 4);
	head->acc_ri = (ACC_RHB << 4) | (ACC_INV & 0x0F);
	head->length = 2;			//length of the following data
	head->structure = EF_LINIER;
	head->rec_size = rec_size;
	ioman_write_buffer(address, head, sizeof(ef_header));		//write the specified allocated address
	dir->num_of_ef += 1;
	ioman_write_buffer(_cur_dir, dir, sizeof(df_header));		//
	file_add_entry(address);									//create new file entry for current directory
	m_free(head);												//freed all temporary variable
	m_free(dir);												//freed all temporary variable
	return APDU_SUCCESS;
}

uint16 _createfilecyclic(uint16 fid, uchar ACC_READ, uchar ACC_WRT, uchar ACC_INC, uchar ACC_INV, uchar ACC_RHB, uchar total_rec, uchar rec_size) {
	ef_header * head;
	df_header * dir;
	#if FS_ADDRESS_WIDTH == AW_32BIT
	uint32 address;
	#else
	uint16 address;
	#endif
	dir = (df_header *)file_get_dfheader(_cur_dir);
	address = fs_alloc((total_rec * rec_size) + sizeof(ef_header));				//allocate new fs linked list
	head = (ef_header *)m_alloc(sizeof(ef_header));
	head->parent = _cur_dir;
	head->sibling = 0;
	head->child = 0;											//not use, only for cyclic
	head->fid = fid;
	head->type = T_EF;
	head->size = total_rec * rec_size;
	head->inc = 1;												//no increase allowed
	head->status = STAT_VALID;
	head->acc_rw = (ACC_READ << 4) | (ACC_WRT & 0x0F);
	head->acc_inc = (ACC_INC << 4);
	head->acc_ri = (ACC_RHB << 4) | (ACC_INV & 0x0F);
	head->length = 2;			//length of the following data
	head->structure = EF_CYCLIC;
	head->rec_size = rec_size;
	ioman_write_buffer(address, head, sizeof(ef_header));		//write the specified allocated address
	dir->num_of_ef += 1;
	ioman_write_buffer(_cur_dir, dir, sizeof(df_header));		//
	file_add_entry(address);									//create new file entry for current directory
	m_free(head);												//freed all temporary variable
	m_free(dir);												//freed all temporary variable
	return APDU_SUCCESS;
}

uint16 _createdirectory(uint16 fid, uchar *dirname, uchar len_name) {
	df_header * head;
	df_header * dir;
	#if FS_ADDRESS_WIDTH == AW_32BIT
	uint32 address;
	#else
	uint16 address;
	#endif
	dir = (df_header *)file_get_dfheader(_cur_dir);
	address = fs_alloc(sizeof(df_header));				//allocate new fs linked list
	head = (df_header *)m_alloc(sizeof(df_header));
	head->parent = _cur_dir;
	head->sibling = 0;
	head->child = 0;											//not use, only for cyclic
	head->fid = fid;
	head->type = T_DF;
	head->length = 9;											//length of the following data
	head->file_char = 1;										//not clarified
	head->num_of_df = 0;
	head->num_of_ef = 0;
	head->num_of_chv = 2;										//just set to 2
	ioman_write_buffer(address, head, sizeof(df_header));		//write the specified allocated address
	dir->num_of_df += 1;
	ioman_write_buffer(_cur_dir, dir, sizeof(df_header));		//
	file_add_entry(address);									//create new file entry for current directory	
	m_free(head);												//freed all temporary variable
	m_free(dir);
	return APDU_SUCCESS;
}

uint16 _remove(uint16 file_id) {
	f_header * head;
	df_header * dir;
	#if FS_ADDRESS_WIDTH == AW_32BIT
	uint32 address;
	#else
	uint16 address;
	#endif
	dir = (df_header *)file_get_dfheader(_cur_dir);
	switch(head->type) {
		case T_DF:
			dir->num_of_df -= 1;
			break;
		case T_EF:
			dir->num_of_ef -= 1;
			break;
		default:
			break;
	}
	address = file_find_entry(file_id);
	head = (f_header *)file_get_header(address);
	if(head->parent != 0) _cur_dir = head->parent;		//set current directory to the specified entry parent
	ioman_write_buffer(_cur_dir, dir, sizeof(df_header));		//
	file_remove_entry(address);						//remove entry from file tree
	fs_dealloc(address);							//dealloc entry from fs linked list
	m_free(head);												//freed all temporary variable
	m_free(dir);
	return APDU_SUCCESS;
}

uint16 _invalidate() {
	register uint16 status = APDU_SUCCESS;
	ef_header * head;
	head = (ef_header *)file_get_efheader(_cur_ptr);
	status = file_check_status(head, FILE_INVALIDATE);
	if(status != APDU_SUCCESS) { m_free(head); return status; }
	head->status = STAT_INVALID;
	ioman_write_buffer(_cur_ptr, head, sizeof(ef_header));
	m_free(head);												//freed all temporary variable
	return status;
}

uint16 _rehabilitate() {
	register uint16 status = APDU_SUCCESS;
	ef_header * head;
	head = (ef_header *)file_get_efheader(_cur_ptr);
	status = file_check_status(head, FILE_REHABILITATE);
	if(status != APDU_SUCCESS) { m_free(head); return status; }
	head->status = STAT_VALID;
	ioman_write_buffer(_cur_ptr, head, sizeof(ef_header));
	m_free(head);												//freed all temporary variable
	return status;
}
//#endif







