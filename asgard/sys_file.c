#include "sys_file.h"
#include "file.h"
#include "..\defs.h"
#include "..\drivers\ioman.h"
#include "fs.h"
#include "..\misc\mem.h"
#include "..\misc\barrenkalaheapcalc.h"
#include "security.h"
#include "crc.h"
#include "..\midgard\midgard.h"

//jadikan static biar tidak bisa diakses modul lainnya
uint16 xdata sys_ptr = 0x0000;	//diisi dengan cluster_no dari ef/df yang diselect
//static uint16 efk_ptr = 0x0000;	//diisi dengan cluster_no dari ef_key active file
uint16 xdata sys_dir = 0x0000;	//pointer yang menunjukkan current directory

uint16 sys_select(uint16 fid)	//system select, menselect file tanpa memperhatikan attribut
{
	ef_header *newfile;		//
	uint16 cluster_no;	//
	uint16 cluster_parent;	//
	if(fid == FID_MF)	//
	{
		sys_ptr = 0x0000;
		sys_dir = 0x0000;
		return APDU_SUCCESS_RESPONSE | DF_RESPONSE_SIZE;
	} else {
		cluster_no = sys_ptr;
		newfile = (ef_header *)file_get_header(cluster_no);
		if(newfile->FID == fid) {
			if(newfile->type == T_EF) {
				m_free(newfile);
				#if _DMA_DEBUG
				barren_eject(newfile);
				#endif 
				return APDU_SUCCESS_RESPONSE | EF_RESPONSE_SIZE; 
			}
			else if(newfile->type == T_DF) {
				m_free(newfile);
				#if _DMA_DEBUG
				barren_eject(newfile);
				#endif 
				return APDU_SUCCESS_RESPONSE | DF_RESPONSE_SIZE; 
			}
		}	//memilih file yang sama
		if(newfile->type == T_EF) { 
			cluster_no = newfile->parent;
			m_free(newfile);
			#if _DMA_DEBUG
			barren_eject(newfile);
			#endif
			newfile = (ef_header *)file_get_header(cluster_no);	//ambil anak pertama
			if(newfile->FID == fid) {
				m_free(newfile);
				#if _DMA_DEBUG
				barren_eject(newfile);
				#endif
				sys_ptr = cluster_no;
				sys_dir = cluster_no;
				return APDU_SUCCESS_RESPONSE | DF_RESPONSE_SIZE;
			}
			cluster_no = newfile->child;
			m_free(newfile);
			#if _DMA_DEBUG
			barren_eject(newfile);
			#endif
			goto cek_sibling; 
		}		//kalau posisi sudah di EF cukup cek sibling saja
		//dilakukan jika current file bertipe DF
		//jika bukan EF
		cluster_no = newfile->child;
		cluster_parent = newfile->parent;
		m_free(newfile);
		#if _DMA_DEBUG
		barren_eject(newfile);
		#endif
		newfile = (ef_header *)file_get_header(cluster_parent);	//ambil anak pertama
		if(newfile->FID == fid) {
			m_free(newfile);
			#if _DMA_DEBUG
			barren_eject(newfile);
			#endif
			sys_ptr = cluster_no;
			sys_dir = cluster_no;
			return APDU_SUCCESS_RESPONSE | DF_RESPONSE_SIZE;
		}
		m_free(newfile);
		#if _DMA_DEBUG
		barren_eject(newfile);
		#endif
		//cek child
		cek_child:
		if(cluster_no == 0) {
			cluster_no = sys_ptr;
			newfile = (ef_header *)file_get_header(cluster_no);
			cluster_no = newfile->parent;
			m_free(newfile);
			#if _DMA_DEBUG
			barren_eject(newfile);
			#endif
			newfile = (ef_header *)file_get_header(cluster_no);	//ambil anak pertama
			cluster_no = newfile->child;
			m_free(newfile);
			#if _DMA_DEBUG
			barren_eject(newfile);
			#endif
			goto cek_sibling_df;
		}
		newfile = (ef_header *)file_get_header(cluster_no);
		if(newfile->FID == fid) {
			sys_ptr = cluster_no;
			if(newfile->type == T_EF)
			{
				//efk_ptr = newfile->ef_key;
				//send response here(cek type dulu)
				m_free(newfile);
				#if _DMA_DEBUG
				barren_eject(newfile);
				#endif
				return APDU_SUCCESS_RESPONSE | EF_RESPONSE_SIZE;
			}
			else
			{
				sys_dir = cluster_no;
				m_free(newfile);
				#if _DMA_DEBUG
				barren_eject(newfile);
				#endif
				return APDU_SUCCESS_RESPONSE | DF_RESPONSE_SIZE;
			}
		} else {
			cluster_no = newfile->sibling;
			m_free(newfile);
			#if _DMA_DEBUG
			barren_eject(newfile);
			#endif
			goto cek_child;
		}

		cek_sibling_df:	//hanya mencari sibling yang bertipe DF
		if(cluster_no == 0) {
			return APDU_FILE_NOT_FOUND;
		}
		newfile = (ef_header *)file_get_header(cluster_no);
		if(newfile->FID==fid) {
			if(sys_ptr == 0) {	//jika DF maka arahkan pointer
				//efk_ptr = newfile->ef_key;
				//send response here(cek type dulu)
				sys_ptr = cluster_no;
				sys_dir = cluster_no;
				m_free(newfile);
				#if _DMA_DEBUG
				barren_eject(newfile);
				#endif
				return APDU_SUCCESS_RESPONSE | DF_RESPONSE_SIZE;
			}

			if(newfile->type == T_DF) {	//jika DF maka arahkan pointer
				//efk_ptr = newfile->ef_key;
				//send response here(cek type dulu)
				sys_ptr = cluster_no;
				sys_dir = cluster_no;
				m_free(newfile);
				#if _DMA_DEBUG
				barren_eject(newfile);
				#endif
				return APDU_SUCCESS_RESPONSE | DF_RESPONSE_SIZE;
			} else {
				m_free(newfile);
				#if _DMA_DEBUG
				barren_eject(newfile);
				#endif
				return APDU_FILE_NOT_FOUND;
			}
		} else {
			cluster_no = newfile->sibling;
			m_free(newfile);
			#if _DMA_DEBUG
			barren_eject(newfile);
			#endif
			goto cek_sibling_df;
		}

		//dilakukan jika current file bertipe EF
		//cek sibling, mencari semua sibling dengan tipe apa saja
		cek_sibling:
		if(cluster_no == 0) {
			return APDU_FILE_NOT_FOUND;
		}
		newfile = (ef_header *)file_get_header(cluster_no);
		if(newfile->FID==fid) {
			sys_ptr = cluster_no;
			if(newfile->type == T_EF) {
				//efk_ptr = newfile->ef_key;
				//send response here(cek type dulu)
				m_free(newfile);
				#if _DMA_DEBUG
				barren_eject(newfile);
				#endif
				return APDU_SUCCESS_RESPONSE | EF_RESPONSE_SIZE;
			} else {
				sys_dir = cluster_no;
				m_free(newfile);
				#if _DMA_DEBUG
				barren_eject(newfile);
				#endif
				return APDU_SUCCESS_RESPONSE | DF_RESPONSE_SIZE;
			}
		} else {
			cluster_no = newfile->sibling;
			m_free(newfile);
			#if _DMA_DEBUG
			barren_eject(newfile);
			#endif
			goto cek_sibling;
		}
	}
}

uint16 sys_createfile(
	uint16 fid,  
	char *buffer, 
	uint16 size) _REENTRANT_
{
	ef_header *header = (ef_header *)file_get_header(sys_dir);		//buat file pada current directory
	df_header *dfhead = (df_header *)header;
	ef_body *newdata = (ef_body *)header;
	uint16 cluster_no;
	uint16 parent_no = sys_dir;
	uint16 sibling_no;
	uchar i;
	char *str;
	check_crc(header);
	if(header->type == T_EF) {
		//cluster_no = header->parent;
		//m_free(header);
		//parent_no = cluster_no;
		//dfhead = (df_header *)file_get_header(cluster_no);
		m_free(header);
		#if _DMA_DEBUG
		barren_eject(header);
		#endif
		return APDU_COMMAND_INVALID;
	}
	cluster_no = dfhead->child;
	dfhead->number_of_ef += 1;
	str = (char *)dfhead;		//update data parent
	newdata = (ef_body *)dfhead;
	dfhead->crc = crcFast(newdata->bytes, CRC_SIZE);
	/*ioman_seek(ALLOCATION_DATA_OFFSET + (parent_no * CLUSTER_SIZE));
	for(i=0;i<sizeof(ef_header);i++) {
		ioman_write(*(str++));
	}*/
	ioman_write_buffer(ALLOCATION_DATA_OFFSET + (parent_no * CLUSTER_SIZE), str, sizeof(ef_header));
	if(cluster_no==0) {
		//create file here(buat child baru)
		dfhead->child = fs_get_next_available_space();
		if(dfhead->child == FS_NO_AVAILABLE_SPACE) {
			return APDU_NO_AVAILABLE_SPACE;
		}
		fs_write_available_space(dfhead->child, TRUE);
		cluster_no = dfhead->child;
		sys_ptr = dfhead->child;		//select header baru
		dfhead->crc = crcFast(newdata->bytes, CRC_SIZE);
		//tulis stream ke eeprom   ////////////////////////////////////////////////////////////////
		str = (char *)dfhead;		//update data parent
		/*ioman_seek(ALLOCATION_DATA_OFFSET + (parent_no * CLUSTER_SIZE));
		for(i=0;i<sizeof(ef_header);i++)
		{
			ioman_write(*(str++));
		}*/
		ioman_write_buffer(ALLOCATION_DATA_OFFSET + (parent_no * CLUSTER_SIZE), str, sizeof(ef_header));
		m_free(dfhead);
		#if _DMA_DEBUG
		barren_eject(dfhead);
		#endif
		//tulis header file yang baru dibuat
		header = (ef_header *) m_alloc (sizeof(ef_header));
		#if _DMA_DEBUG
		barren_insert(header,sizeof(ef_header));
		#endif
		newdata = (ef_body *)header;
		header->type = T_EF;
		header->size = size;
		header->FID = fid;
		header->structure = EF_INTERNAL;
		header->status = STAT_VALID;	//status harus selalu valid untuk system file
		header->access_rw = 0x00;		//always read and write
		header->access_inc = 0x00;
		header->access_ri = 0xFF;		//tidak bisa direhab dan invalidate
		header->record_size = 0x00;
		header->parent = parent_no;
		header->sibling = 0x0000;
		header->child = 0x0000;
		header->next = 0;
		header->crc = crcFast(newdata->bytes, CRC_SIZE);
		str = (char *)header;
		/*ioman_seek(ALLOCATION_DATA_OFFSET + (cluster_no * CLUSTER_SIZE));
		for(i=0;i<sizeof(ef_header);i++) {
			ioman_write(*(str++));
		}*/
		ioman_write_buffer(ALLOCATION_DATA_OFFSET + (cluster_no * CLUSTER_SIZE), str, sizeof(ef_header));
		//tulis data dengan bantuan sys_write
		m_free(header);
		#if _DMA_DEBUG
		barren_eject(header);
		#endif
		return sys_write(0, size, buffer);
	} else {
		//scan till last child
		get_last_child:
		if(cluster_no==0) {
			//create file here
			if(header->type == T_EF){
				header->sibling = fs_get_next_available_space();
				if(header->sibling == FS_NO_AVAILABLE_SPACE) {
					return APDU_NO_AVAILABLE_SPACE;
				}
				fs_write_available_space(header->sibling, TRUE);
				cluster_no = header->sibling;
				sys_ptr = header->sibling;		//select header baru
				//tulis stream ke eeprom   ////////////////////////////////////////////////////////////////
				str = (char *)header;			//update data last sibling
				newdata = (ef_body *)header;
				header->crc = crcFast(newdata->bytes, CRC_SIZE);
				/*ioman_seek(ALLOCATION_DATA_OFFSET + (sibling_no * CLUSTER_SIZE));
				for(i=0;i<sizeof(ef_header);i++)
				{
					ioman_write(*(str++));
				} */
				ioman_write_buffer(ALLOCATION_DATA_OFFSET + (sibling_no * CLUSTER_SIZE), str, sizeof(ef_header));
				m_free(header);
				#if _DMA_DEBUG
				barren_eject(header);
				#endif
			}
			else {
				dfhead->sibling = fs_get_next_available_space();
				if(dfhead->sibling == FS_NO_AVAILABLE_SPACE) {
					return APDU_NO_AVAILABLE_SPACE;
				}
				fs_write_available_space(dfhead->sibling, TRUE);
				cluster_no = dfhead->sibling;
				sys_ptr = dfhead->sibling;		//select header baru
				//tulis stream ke eeprom   ////////////////////////////////////////////////////////////////
				str = (char *)dfhead;			//update data last sibling
				newdata = (ef_body *)dfhead;
				dfhead->crc = crcFast(newdata->bytes, CRC_SIZE);
				/*ioman_seek(ALLOCATION_DATA_OFFSET + (sibling_no * CLUSTER_SIZE));
				for(i=0;i<sizeof(df_header);i++)
				{
					ioman_write(*(str++));
				}*/
				ioman_write_buffer(ALLOCATION_DATA_OFFSET + (sibling_no * CLUSTER_SIZE), str, sizeof(df_header));
				m_free(dfhead);
				#if _DMA_DEBUG
				barren_eject(dfhead);
				#endif
			}
			//tulis header file yang baru dibuat
			header = (ef_header *) m_alloc (sizeof(ef_header));
			#if _DMA_DEBUG
			barren_insert(header,sizeof(ef_header));
			#endif
			newdata = (ef_body *)header;
			header->type = T_EF;
			header->size = size;
			header->FID = fid;
			header->structure = EF_INTERNAL;
			header->status = STAT_VALID;	//status harus selalu valid untuk system file
			header->access_rw = 0x00;		//always read and write
			header->access_inc = 0x00;
			header->access_ri = 0xFF;		//tidak bisa direhab dan invalidate
			header->record_size = 0x00;
			header->parent = parent_no;
			header->sibling = 0x0000;
			header->child = 0x0000;
			header->next = 0;
			header->crc = crcFast(newdata->bytes, CRC_SIZE);
			str = (char *)header;			//update data last sibling
			/*ioman_seek(ALLOCATION_DATA_OFFSET + (cluster_no * CLUSTER_SIZE));
			for(i=0;i<sizeof(ef_header);i++)
			{
				ioman_write(*(str++));
			} */
			ioman_write_buffer(ALLOCATION_DATA_OFFSET + (cluster_no * CLUSTER_SIZE), str, sizeof(ef_header));
			//tulis data dengan bantuan sys_write
			m_free(header);
			#if _DMA_DEBUG
			barren_eject(header);
			#endif
			return sys_write(0, size, buffer);
		}
		m_free(header);
		#if _DMA_DEBUG
		barren_eject(header);
		#endif
		sibling_no = cluster_no;
		header = (ef_header *)file_get_header(cluster_no);
		dfhead = (df_header *)header;
		newdata = (ef_body *)header;
		if(header->FID == fid) {
			m_free(header);
			#if _DMA_DEBUG
			barren_eject(header);
			#endif
			return APDU_FILE_NOT_FOUND;
		}
		cluster_no = header->sibling;
		goto get_last_child;
	}
}

uint16 sys_read(uint16 offset, uint16 size, char *buffer) _REENTRANT_
{
	ef_header *header = (ef_header *)file_get_header(sys_ptr);
	uint16 cluster_no = header->next;
	uint16 c_size = header->size;
	uint16 _ptr = 0;
	uchar i,j=0;
	uchar status;
	ef_body *newdata;
	char *str = (char *)header;
	check_crc(header);
	//cek current file dengan perintah sekarang
	if(header->type != T_EF){
		m_free(header);
		#if _DMA_DEBUG
		barren_eject(header);
		#endif
		return APDU_NO_EF_SELECTED;
	}
	if(header->structure != EF_INTERNAL) {
		m_free(header);
		#if _DMA_DEBUG
		barren_eject(header);
		#endif
		return APDU_FILE_INCONSISTENT;
	}
	//TIDAK MEMPEDULIKAN ACCESS CONTROL DARI FILE
	/*
	i = header->access_rw >> 4;
	#ifdef _YGGDRASIL_MICRO_KERNEL
	if(v_chv_status[i] != CHV_VERIFIED) {	//CHV belum diverifikasi, cek chv status dulu
	#endif
		status = getCHVstatus(i);	//cek status chv, access denied jika chv tidak terpenuhi
		switch(status) {
			case CHV_NEVER:
			case CHV_ENABLE:
			case CHV_BLOCK:
				m_free(header);
				#if _DMA_DEBUG
				barren_eject(header);
				#endif
				return APDU_ACCESS_DENIED;
				break;
			case CHV_DISABLE:
			case CHV_UNBLOCK:
			case CHV_ALWAYS:
			default:
				break;
		}
	#ifdef _YGGDRASIL_MICRO_KERNEL
	}
	#endif
	*/

	//TIDAK MEMPERDULIKAN STATUS DARI FILE
	/*
	if(header->status == STAT_INVALID) {
		m_free(header);
		#if _DMA_DEBUG
		barren_eject(header);
		#endif
		return APDU_INVALID_STATUS;
	}
	*/

	if(header->type != T_EF) {
		m_free(header);
		#if _DMA_DEBUG
		barren_eject(header);
		#endif
		return APDU_NO_EF_SELECTED;
	}
	if(offset>header->size) { //wrong offset
		m_free(header);
		#if _DMA_DEBUG
		barren_eject(header);
		#endif 
		return APDU_WRONG_PARAMETER; 
	}
	if(offset+size>header->size) { //wrong offset
		m_free(header); 
		#if _DMA_DEBUG
		barren_eject(header);
		#endif
		return APDU_WRONG_LENGTH; 
	} else {
		m_free(header);
		#if _DMA_DEBUG
		barren_eject(header);
		#endif
	}
	
	if(cluster_no==0) {
		return APDU_OUT_OF_RANGE;	//no data available
	}
	c_size = offset - _ptr;
	get_next_cluster:
	newdata = (ef_body *)file_get_header(cluster_no);
	
	if(offset > (_ptr + DATA_SIZE)) {
		cluster_no = newdata->next;
		_ptr += DATA_SIZE;
		m_free(newdata);
		#if _DMA_DEBUG
		barren_eject(newdata);
		#endif
		c_size = offset - _ptr;
		goto get_next_cluster;
	}

	if((_ptr + DATA_SIZE - offset) < size) {
		//printf("size read = %i, total size = %i\n",DATA_SIZE - c_size, size);
		j += memcopy(buffer+j, newdata->bytes + c_size, 0, DATA_SIZE - c_size);		//22, 8 deteksi ukuran tersisa
		if(newdata->crc != crcFast(newdata->bytes, CRC_SIZE)) {
			m_free(newdata);
			#if _DMA_DEBUG
			barren_eject(newdata);
			#endif
			return APDU_MEMORY_PROBLEM;
		}
		cluster_no = newdata->next;
		_ptr += DATA_SIZE;
		m_free(newdata);
		#if _DMA_DEBUG
		barren_eject(newdata);
		#endif
		c_size = 0;
		goto get_next_cluster;
	} else {
		//printf("size read = %i, total size = %i\n",size-j, size);
		j += memcopy(buffer+j, newdata->bytes + c_size, 0, size-j);		//22, 8 deteksi ukuran tersisa
		if(newdata->crc != crcFast(newdata->bytes, CRC_SIZE)) {
			m_free(newdata);
			#if _DMA_DEBUG
			barren_eject(newdata);
			#endif
			return APDU_MEMORY_PROBLEM;
		}
		cluster_no = newdata->next;
		m_free(newdata);
		#if _DMA_DEBUG
		barren_eject(newdata);
		#endif
	}
	return APDU_SUCCESS;
}

uint16 sys_write(uint16 offset, uint16 size, char *buffer) _REENTRANT_
{
	ef_header *header = (ef_header *)file_get_header(sys_ptr);
	uint16 c_size = header->size;
	uint16 filesize = c_size;
	uint16 cluster_no = header->next;
	uint16 _ptr = 0;
	uchar i,j=0;
	uchar status;
	ef_body *newdata;
	char *str = (char *)header;	
	check_crc(header);
	//cek current file dengan perintah sekarang
	if(header->type != T_EF) {
		m_free(header);
		#if _DMA_DEBUG
		barren_eject(header);
		#endif
		return APDU_NO_EF_SELECTED;
	}
	if(header->structure != EF_INTERNAL) {
		m_free(header);
		#if _DMA_DEBUG
		barren_eject(header);
		#endif
		return APDU_FILE_INCONSISTENT;
	}

	//TIDAK MEMPEDULIKAN ACCESS CONTROL DARI FILE
	/*
	i = header->access_rw & 0x0f;
	#ifdef _YGGDRASIL_MICRO_KERNEL
	if(v_chv_status[i] != CHV_VERIFIED) {	//CHV belum diverifikasi, cek chv status dulu
	#endif
		status = getCHVstatus(i);	//cek status chv, access denied jika chv tidak terpenuhi
		switch(status) {
			case CHV_NEVER:
			case CHV_ENABLE:
			case CHV_BLOCK:
				m_free(header);
				#if _DMA_DEBUG
				barren_eject(header);
				#endif
				return APDU_ACCESS_DENIED;
				break;
			case CHV_DISABLE:
			case CHV_UNBLOCK:
			case CHV_ALWAYS:
			default:
				break;
		}
	#ifdef _YGGDRASIL_MICRO_KERNEL
	}
	#endif
	*/
	
	//TIDAK MEMPEDULIKAN STATUS DARI FILE
	/*
	if(header->status == STAT_INVALID) {
		m_free(header);
		#if _DMA_DEBUG
		barren_eject(header);
		#endif
		return APDU_INVALID_STATUS;
	}
	*/

	/*if(offset>header->size) { //wrong offset
		m_free(header);
		#if _DMA_DEBUG
		barren_eject(header);
		#endif 
		return APDU_OUT_OF_RANGE; 
	} else {*/
		//if((header->access_inc & 0xf0) != 0xf0) {	//dapat melakukan increase tidak NEVER
	if((size+offset)>header->size) {	//	SECARA OTOMATIS DAPAT MELAKUKAN INCREASE
		header->size =  size+offset;
	}
		//} else {
			//if((size+offset)>header->size) {
				//m_free(header);
				///#if _DMA_DEBUG
				///barren_eject(header);
				//#endif
				///return APDU_WRONG_LENGTH;
			//}
		//}		//write new header here
		//<TODO>
		//cek access control untuk penulisan
	if(header->next==0) {					
		header->next = fs_get_next_available_space();
		if(header->next == FS_NO_AVAILABLE_SPACE) {
			return APDU_NO_AVAILABLE_SPACE;
		}
		fs_write_available_space(header->next, TRUE);
		cluster_no = header->next;
	}
	newdata = (ef_body *)str;
	newdata->crc = crcFast(newdata->bytes, CRC_SIZE);
	/*ioman_seek(ALLOCATION_DATA_OFFSET + (sys_ptr * CLUSTER_SIZE));
	for(i=0;i<sizeof(ef_header);i++) {
		ioman_write(*(str++));
	}*/
	ioman_write_buffer(ALLOCATION_DATA_OFFSET + (sys_ptr * CLUSTER_SIZE), str, sizeof(ef_header));
	m_free(header);
	#if _DMA_DEBUG
	barren_eject(header);
	#endif
	//}
	
	if(cluster_no==0) {
		cluster_no = fs_get_next_available_space();	//create new cluster for data
		if(cluster_no == FS_NO_AVAILABLE_SPACE) {
				return APDU_NO_AVAILABLE_SPACE;
		}
		fs_write_available_space(cluster_no, TRUE);
	}
	c_size = offset - _ptr;
	get_next_cluster:
	newdata = (ef_body *)file_get_header(cluster_no);
	
	if(offset > (_ptr + DATA_SIZE)) {
		cluster_no = newdata->next;
		_ptr += DATA_SIZE;
		m_free(newdata);
		#if _DMA_DEBUG
		barren_eject(newdata);
		#endif
		c_size = offset - _ptr;
		goto get_next_cluster;
	}
	//printf("current cluster = %i\n",(_ptr + DATA_SIZE - offset));
	if((_ptr + DATA_SIZE - offset) < size) {
		//printf("size wrote = %i, total size = %i\n",DATA_SIZE-c_size, size); 
		str = (char *)newdata;
		j += memcopy(newdata->bytes+c_size, buffer+j, 0, DATA_SIZE-c_size);
		//newdata->crc = crcFast(newdata->data, DATA_SIZE);
		if(newdata->next==0){
			newdata->next = fs_get_next_available_space();	//create new cluster for data
			if(newdata->next == FS_NO_AVAILABLE_SPACE) {
				return APDU_NO_AVAILABLE_SPACE;
			}
			fs_write_available_space(newdata->next, TRUE);
		}
		newdata = (ef_body *)str;
		newdata->crc = crcFast(newdata->bytes, CRC_SIZE);
		/*ioman_seek(ALLOCATION_DATA_OFFSET + (cluster_no * CLUSTER_SIZE));
		for(i=0;i<sizeof(ef_header);i++)
		{
			ioman_write(*(str++));
		}*/
		ioman_write_buffer(ALLOCATION_DATA_OFFSET + (cluster_no * CLUSTER_SIZE), str, sizeof(ef_header));
		cluster_no = newdata->next;
		_ptr += DATA_SIZE;		
		m_free(newdata);
		#if _DMA_DEBUG
		barren_eject(newdata);
		#endif
		c_size = 0;
		goto get_next_cluster;
	} else {
		//printf("size wrote = %i, total size = %i\n",size-j, size);
		str = (char *)newdata;	
		j += memcopy(newdata->bytes+c_size, buffer+j, 0, size-j);
		if((offset + size) > filesize) {
			newdata->next = 0;
		}
		newdata->crc = crcFast(newdata->bytes, CRC_SIZE);
		/*ioman_seek(ALLOCATION_DATA_OFFSET + (cluster_no * CLUSTER_SIZE));
		for(i=0;i<sizeof(ef_header);i++) {
			ioman_write(*(str++));
		}*/
		ioman_write_buffer(ALLOCATION_DATA_OFFSET + (cluster_no * CLUSTER_SIZE), str, sizeof(ef_header));
		m_free(newdata);
		#if _DMA_DEBUG
		barren_eject(newdata);
		#endif
	}
	return APDU_SUCCESS;
}
