#include "file.h"
#include "defs.h"
#include "ioman.h"
#include "fs.h"
#include "mem.h"
#include "security.h"
#include "crc.h"


//global variable
//uint16 mf_ptr = 0x0000;		//diisi dengan cluster_no
uint16 cur_ptr = 0x0000;	//diisi dengan cluster_no dari ef/df yang diselect
uint16 efk_ptr = 0x0000;	//diisi dengan cluster_no dari ef_key active file
uint16 cur_rcd = 0x0000;	//direset ketika select


ef_header * file_get_header(uint16 cluster_no)
{
	ef_header *newfile = (ef_header *) malloc (sizeof(ef_header));
	char *str = (char *)newfile;
	uchar i;
	ioman_seek((cluster_no * CLUSTER_SIZE) + ALLOCATION_DATA_OFFSET);
	for(i=0;i<sizeof(ef_header);i++)
	{
		*(str++) = ioman_read();
	}
	if(newfile->crc != crcFast((char *)newfile, CRC_SIZE)) return NULL;
	return newfile;	//jangan dihapus karena akan dipakai
}

uint16 file_seek(uint16 file_id)
{
	uint16 cluster_no=0;			//pilih root, MF
	uint16 node_queue[QUEUE_SIZE];	//nodes queue, maksimum hanya menyimpan 5 child
	uchar queue_index_in=0;
	uchar queue_index_out=0;
	char i;
	ef_header *curfile;				//inisialisasi
	get_next_sibling:
	curfile = file_get_header(cluster_no);
	if(curfile->child != 0)
	{
		//enqueue child
		node_queue[queue_index_in++] = curfile->child;
	}
	//printf("%s, %i\n", curfile->id, queue_index);
	if(curfile->FID == file_id)
	{
		return cluster_no;
	}
	if(curfile->sibling != 0) {
		cluster_no = curfile->sibling;
		goto get_next_sibling;
	}
	if(queue_index_in==queue_index_out) { return FILE_NOT_FOUND; }
	//dequeue
	cluster_no = node_queue[queue_index_out++];
	goto get_next_sibling;
}

uint16 _select(uint16 fid)
{
	ef_header *newfile;
	uint16 cluster_no;
	if(fid == FID_MF)	//yang dipilih adalah MF
	{
		cur_ptr = 0x0000;
		return APDU_SUCCESS_RESPONSE | DF_RESPONSE_SIZE;
	} else {
		cluster_no = cur_ptr;
		newfile = (ef_header *)file_get_header(cluster_no);
		if(newfile->FID == fid) { return APDU_SUCCESS; }	//memilih file yang sama
		if(newfile->type == T_EF) { 
			cluster_no = newfile->parent;
			free(newfile);
			newfile = (ef_header *)file_get_header(cluster_no);	//ambil anak pertama
			if(newfile->FID == fid) {
				free(newfile);
				cur_ptr = cluster_no;
				return APDU_SUCCESS_RESPONSE | DF_RESPONSE_SIZE;
			}
			cluster_no = newfile->child;
			free(newfile);
			goto cek_sibling; 
		}		//kalau posisi sudah di EF cukup cek sibling saja
		//dilakukan jika current file bertipe DF
		//jika bukan EF
		cluster_no = newfile->child;
		free(newfile);
		//cek child
		cek_child:
		if(cluster_no == 0) {
			cluster_no = cur_ptr;
			newfile = (ef_header *)file_get_header(cluster_no);
			cluster_no = newfile->parent;
			free(newfile);
			newfile = (ef_header *)file_get_header(cluster_no);	//ambil anak pertama
			cluster_no = newfile->child;
			free(newfile);
			goto cek_sibling_df;
		}
		newfile = (ef_header *)file_get_header(cluster_no);
		if(newfile->FID==fid) {
			cur_ptr = cluster_no;
			if(newfile->type == T_EF)
			{
				//efk_ptr = newfile->ef_key;
				//send response here(cek type dulu)
				free(newfile);
				cur_rcd = 0;
				return APDU_SUCCESS_RESPONSE | EF_RESPONSE_SIZE;
			}
			else
			{
				free(newfile);
				return APDU_SUCCESS_RESPONSE | DF_RESPONSE_SIZE;
			}
		} else {
			cluster_no = newfile->sibling;
			free(newfile);
			goto cek_child;
		}

		cek_sibling_df:	//hanya mencari sibling yang bertipe DF
		if(cluster_no == 0) {
			return APDU_FILE_NOT_FOUND;
		}
		newfile = (ef_header *)file_get_header(cluster_no);
		if(newfile->FID==fid) {
			
			if(newfile->type == T_DF) {	//jika DF maka arahkan pointer
				//efk_ptr = newfile->ef_key;
				//send response here(cek type dulu)
				cur_ptr = cluster_no;
				free(newfile);
				cur_rcd = 0;
				return APDU_SUCCESS_RESPONSE | DF_RESPONSE_SIZE;
			} else {
				free(newfile);
				return APDU_FILE_NOT_FOUND;
			}
		} else {
			cluster_no = newfile->sibling;
			free(newfile);
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
			cur_ptr = cluster_no;
			if(newfile->type == T_EF) {
				//efk_ptr = newfile->ef_key;
				//send response here(cek type dulu)
				free(newfile);
				cur_rcd = 0;
				return APDU_SUCCESS_RESPONSE | EF_RESPONSE_SIZE;
			} else {
				free(newfile);
				return APDU_SUCCESS_RESPONSE | DF_RESPONSE_SIZE;
			}
		} else {
			cluster_no = newfile->sibling;
			free(newfile);
			goto cek_sibling;
		}
	}
}

uint16 _readbin(uint16 offset, uchar size, char *buffer)
{
	ef_header *header = (ef_header *)file_get_header(cur_ptr);
	uint16 c_size = header->size;
	uint16 cluster_no = header->next;
	uint16 _ptr = 0;
	uint16 i,j=0;
	uchar status;
	ef_body *newdata;
	//char *str = (char *)header;
	//cek current file dengan perintah sekarang
	if(header->type != T_EF){
		free(header);
		return APDU_NO_EF_SELECTED;
	}
	if(header->structure != EF_TRANSPARENT) {
		free(header);
		return APDU_COMMAND_INVALID;
	}
	//<TODO>
	//cek access control untuk pembacaan
	status = getCHVstatus(header->access_rw>>4);	//cek status chv, access denied jika chv tidak terpenuhi
	switch(status)
	{
		case CHV_NEVER:
		case CHV_ENABLE:
		case CHV_BLOCK:
			free(header);
			return APDU_ACCESS_DENIED;
			break;
		case CHV_DISABLE:
		case CHV_UNBLOCK:
		case CHV_VERIFIED:
		case CHV_ALWAYS:
		default:
			break;
	}

	if(header->status == STAT_INVALID)
	{
		free(header);
		return APDU_INVALID_STATUS;
	}

	if(header->type != T_EF) {
		free(header);
		return APDU_NO_EF_SELECTED;
	}
	if(offset>header->size) { //wrong offset
		free(header); 
		return APDU_OUT_OF_RANGE; 
	}
	if(offset+size>header->size) { //wrong offset
		free(header); 
		return APDU_OUT_OF_RANGE; 
	} else {
		free(header);
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
		free(newdata);
		c_size = offset - _ptr;
		goto get_next_cluster;
	}

	if((_ptr + DATA_SIZE - offset) < size) {
		//printf("size read = %i, total size = %i\n",DATA_SIZE - c_size, size);
		j += memcopy(buffer+j, newdata->data + c_size, 0, DATA_SIZE - c_size);		//22, 8 deteksi ukuran tersisa
		if(newdata->crc != crcFast(newdata->data, CRC_SIZE)) {
			free(newdata);
			return APDU_MEMORY_PROBLEM;
		}
		cluster_no = newdata->next;
		_ptr += DATA_SIZE;
		free(newdata);
		c_size = 0;
		goto get_next_cluster;
	} else {
		//printf("size read = %i, total size = %i\n",size-j, size);
		j += memcopy(buffer+j, newdata->data + c_size, 0, size-j);		//22, 8 deteksi ukuran tersisa
		if(newdata->crc != crcFast(newdata->data, CRC_SIZE)) {
			free(newdata);
			return APDU_MEMORY_PROBLEM;
		}
		cluster_no = newdata->next;
		free(newdata);
	}
	return APDU_SUCCESS;
}

uint16 _readrec(uint16 rec_no, char *buffer)
{
	ef_header *header = (ef_header *)file_get_header(cur_ptr);
	uint16 c_size = header->size;
	uchar size = header->record_size;
	uint16 offset = header->record_size * rec_no; 
	uint16 cluster_no = header->next;
	uint16 _ptr = 0;
	uint16 i,j=0;
	uchar status;
	ef_body *newdata;
	//char *str = (char *)header;
	//cek current file dengan perintah sekarang
	if(header->type != T_EF){
		free(header);
		return APDU_NO_EF_SELECTED;
	}
	if(header->structure != EF_LINIER && header->structure != EF_CYCLIC) {
		free(header);
		return APDU_COMMAND_INVALID;
	}
	//<TODO>
	//cek access control untuk pembacaan
	status = getCHVstatus(header->access_rw>>4);	//cek status chv, access denied jika chv tidak terpenuhi
	switch(status)
	{
		case CHV_NEVER:
		case CHV_ENABLE:
		case CHV_BLOCK:
			free(header);
			return APDU_ACCESS_DENIED;
			break;
		case CHV_DISABLE:
		case CHV_UNBLOCK:
		case CHV_VERIFIED:
		case CHV_ALWAYS:
		default:
			break;
	}

	if(header->status == STAT_INVALID)
	{
		free(header);
		return APDU_INVALID_STATUS;
	}

	if(header->type != T_EF) {
		free(header);
		return APDU_NO_EF_SELECTED;
	}
	if(offset>header->size) { //wrong offset
		free(header); 
		return APDU_OUT_OF_RANGE; 
	}
	if(offset+size>header->size) { //wrong offset
		free(header); 
		return APDU_OUT_OF_RANGE; 
	} else {
		free(header);
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
		free(newdata);
		c_size = offset - _ptr;
		goto get_next_cluster;
	}

	if((_ptr + DATA_SIZE - offset) < size) {
		//printf("size read = %i, total size = %i\n",DATA_SIZE - c_size, size);
		j += memcopy(buffer+j, newdata->data + c_size, 0, DATA_SIZE - c_size);		//22, 8 deteksi ukuran tersisa
		if(newdata->crc != crcFast(newdata->data, CRC_SIZE)) {
			free(newdata);
			return APDU_MEMORY_PROBLEM;
		}
		cluster_no = newdata->next;
		_ptr += DATA_SIZE;
		free(newdata);
		c_size = 0;
		goto get_next_cluster;
	} else {
		//printf("size read = %i, total size = %i\n",size-j, size);
		j += memcopy(buffer+j, newdata->data + c_size, 0, size-j);		//22, 8 deteksi ukuran tersisa
		if(newdata->crc != crcFast(newdata->data, CRC_SIZE)) {
			free(newdata);
			return APDU_MEMORY_PROBLEM;
		}
		cluster_no = newdata->next;
		free(newdata);
	}
	return APDU_SUCCESS;
}

uint16 _writebin(uint16 offset, uchar size, char *buffer)
{
	ef_header *header = (ef_header *)file_get_header(cur_ptr);
	uint16 c_size = header->size;
	uint16 cluster_no = header->next;
	uint16 _ptr = 0;
	uint16 i,j=0;
	uchar status;
	ef_body *newdata;
	char *str = (char *)header;	
	//cek current file dengan perintah sekarang
	if(header->type != T_EF) {
		free(header);
		return APDU_NO_EF_SELECTED;
	}
	if(header->structure != EF_TRANSPARENT) {
		free(header);
		return APDU_COMMAND_INVALID;
	}

	status = getCHVstatus(header->access_rw & 0x0f);	//cek status CHV, access denied jika chv tidak terpenuhi
	switch(status)
	{
		case CHV_NEVER:
		case CHV_ENABLE:
		case CHV_BLOCK:
			free(header);
			return APDU_ACCESS_DENIED;
			break;
		case CHV_DISABLE:
		case CHV_UNBLOCK:
		case CHV_VERIFIED:
		case CHV_ALWAYS:
		default:
			break;
	}
	
	if(header->status == STAT_INVALID)
	{
		free(header);
		return APDU_INVALID_STATUS;
	}

	if(offset>header->size) { //wrong offset
		free(header); 
		return APDU_OUT_OF_RANGE; 
	} else {
		if((size+offset)>header->size) {	
			header->size =  size+offset;
		}
		//write new header here
		//<TODO>
		//cek access control untuk penulisan
		if(header->next==0) {					
			header->next = fs_get_next_available_space();
			fs_write_available_space(header->next, TRUE);
			cluster_no = header->next;
		}
		
		newdata = (ef_body *)str;
		newdata->crc = crcFast(newdata->data, CRC_SIZE);
		ioman_seek(ALLOCATION_DATA_OFFSET + (cur_ptr * CLUSTER_SIZE));
		for(i=0;i<sizeof(ef_header);i++) {
			ioman_write(*(str++));
		}
		//file_write_cluster(header, cur_ptr);
		free(header);
	}
	
	if(cluster_no==0) {
		cluster_no = fs_get_next_available_space();	//create new cluster for data
		fs_write_available_space(cluster_no, TRUE);
	}
	c_size = offset - _ptr;
	get_next_cluster:
	newdata = (ef_body *)file_get_header(cluster_no);
	
	if(offset > (_ptr + DATA_SIZE)) {
		cluster_no = newdata->next;
		_ptr += DATA_SIZE;
		free(newdata);
		c_size = offset - _ptr;
		goto get_next_cluster;
	}
	
	
	//printf("current cluster = %i\n",(_ptr + DATA_SIZE - offset));
	if((_ptr + DATA_SIZE - offset) < size) {
		//printf("size wrote = %i, total size = %i\n",DATA_SIZE-c_size, size); 
		//str = (char *)newdata;
		j += memcopy(newdata->data+c_size, buffer+j, 0, DATA_SIZE-c_size);
		//newdata->crc = crcFast(newdata->data, DATA_SIZE);
		if(newdata->next==0){
			newdata->next = fs_get_next_available_space();	//create new cluster for data
			fs_write_available_space(newdata->next, TRUE);
		}
		newdata = (ef_body *)str;
		newdata->crc = crcFast(newdata->data, CRC_SIZE);
		ioman_seek(ALLOCATION_DATA_OFFSET + (cluster_no * CLUSTER_SIZE));
		for(i=0;i<sizeof(ef_header);i++)
		{
			ioman_write(*(str++));
		}
		//file_write_cluster(newdata, cluster_no);
		cluster_no = newdata->next;
		_ptr += DATA_SIZE;		
		free(newdata);
		c_size = 0;
		goto get_next_cluster;
	} else {
		//printf("size wrote = %i, total size = %i\n",size-j, size);
		str = (char *)newdata;	
		j += memcopy(newdata->data+c_size, buffer+j, 0, size-j);
		newdata->next = 0;
		newdata->crc = crcFast(newdata->data, DATA_SIZE);
		ioman_seek(ALLOCATION_DATA_OFFSET + (cluster_no * CLUSTER_SIZE));
		for(i=0;i<sizeof(ef_header);i++) {
			ioman_write(*(str++));
		}
		//file_write_cluster(newdata, cluster_no);
		free(newdata);
	}
	return APDU_SUCCESS;
}


uint16 _writerec(uint16 rec_no, uchar size, char *buffer)
{
	ef_header *header = (ef_header *)file_get_header(cur_ptr);
	uint16 c_size = header->size;
	uint16 offset = header->record_size * rec_no;
	uint16 cluster_no = header->next;
	uint16 _ptr = 0;
	uint16 i,j=0;
	uchar status;
	ef_body *newdata;
	//char *str = (char *)header;	
	//cek current file dengan perintah sekarang
	if(header->type != T_EF) {
		free(header);
		return APDU_NO_EF_SELECTED;
	}
	if(header->structure != EF_LINIER && header->structure != EF_CYCLIC ) {
		free(header);
		return APDU_COMMAND_INVALID;
	}

	status = getCHVstatus(header->access_rw & 0x0f);	//cek status CHV, access denied jika chv tidak terpenuhi
	switch(status)
	{
		case CHV_NEVER:
		case CHV_ENABLE:
		case CHV_BLOCK:
			free(header);
			return APDU_ACCESS_DENIED;
			break;
		case CHV_DISABLE:
		case CHV_UNBLOCK:
		case CHV_VERIFIED:
		case CHV_ALWAYS:
		default:
			break;
	}

	if(header->status == STAT_INVALID)
	{
		free(header);
		return APDU_INVALID_STATUS;
	}

	if(offset>header->size) { //wrong offset
		free(header); 
		return APDU_OUT_OF_RANGE; 
	} else {
		if((size+offset)>header->size) {	
			header->size =  size+offset;
		}
		//write new header here
		//<TODO>
		//cek access control untuk penulisan
		if(header->next==0) {					
			header->next = fs_get_next_available_space();
			fs_write_available_space(header->next, TRUE);
			cluster_no = header->next;
		}
		/*newdata = (ef_body *)str;
		newdata->crc = crcFast(newdata->data, CRC_SIZE);
		ioman_seek(ALLOCATION_DATA_OFFSET + (cur_ptr * CLUSTER_SIZE));
		for(i=0;i<sizeof(ef_header);i++) {
			ioman_write(*(str++));
		}*/
		file_write_cluster(header, cur_ptr);
		free(header);
	}
	
	if(cluster_no==0) {
		cluster_no = fs_get_next_available_space();	//create new cluster for data
		fs_write_available_space(cluster_no, TRUE);
	}
	c_size = offset - _ptr;
	get_next_cluster:
	newdata = (ef_body *)file_get_header(cluster_no);
	
	if(offset > (_ptr + DATA_SIZE)) {
		cluster_no = newdata->next;
		_ptr += DATA_SIZE;
		free(newdata);
		c_size = offset - _ptr;
		goto get_next_cluster;
	}
	
	
	//printf("current cluster = %i\n",(_ptr + DATA_SIZE - offset));
	if((_ptr + DATA_SIZE - offset) < size) {
		//printf("size wrote = %i, total size = %i\n",DATA_SIZE-c_size, size); 
		//str = (char *)newdata;
		j += memcopy(newdata->data+c_size, buffer+j, 0, DATA_SIZE-c_size);
		newdata->crc = crcFast(newdata->data, DATA_SIZE);
		if(newdata->next==0){
			newdata->next = fs_get_next_available_space();	//create new cluster for data
			fs_write_available_space(newdata->next, TRUE);
		}
		/*newdata = (ef_body *)str;
		newdata->crc = crcFast(newdata->data, CRC_SIZE);
		ioman_seek(ALLOCATION_DATA_OFFSET + (cluster_no * CLUSTER_SIZE));
		for(i=0;i<sizeof(ef_header);i++)
		{
			ioman_write(*(str++));
		}*/
		file_write_cluster(newdata, cluster_no);
		cluster_no = newdata->next;
		_ptr += DATA_SIZE;		
		free(newdata);
		c_size = 0;
		goto get_next_cluster;
	} else {
		//printf("size wrote = %i, total size = %i\n",size-j, size);
		//str = (char *)newdata;	
		j += memcopy(newdata->data+c_size, buffer+j, 0, size-j);
		newdata->next = 0;
		/*newdata->crc = crcFast(newdata->data, DATA_SIZE);
		newdata->next = 0;
		ioman_seek(ALLOCATION_DATA_OFFSET + (cluster_no * CLUSTER_SIZE));
		for(i=0;i<sizeof(ef_header);i++) {
			ioman_write(*(str++));
		}*/
		file_write_cluster(newdata, cluster_no);
		free(newdata);
	}
	return APDU_SUCCESS;
}

uint16 _createfilebin(
	uint16 fid, 
	uchar ACC_READ, 
	uchar ACC_WRT, 
	uchar ACC_INC, 
	uchar ACC_INV, 
	uchar ACC_RHB, 
	char *buffer, 
	uint16 size)
{
	ef_header *header = (ef_header *)file_get_header(cur_ptr);
	df_header *dfhead = (df_header *)header;
	ef_body *newdata = (ef_body *)header;
	uint16 cluster_no;
	uint16 parent_no = cur_ptr;
	uint16 sibling_no;
	uchar i;
	char *str;
	if(header->type == T_EF) {
		//cluster_no = header->parent;
		//free(header);
		//parent_no = cluster_no;
		//dfhead = (df_header *)file_get_header(cluster_no);
		free(header);
		return APDU_COMMAND_INVALID;
	}
	cluster_no = dfhead->child;
	dfhead->number_of_ef += 1;
	str = (char *)dfhead;		//update data parent
	newdata = (ef_body *)dfhead;
	dfhead->crc = crcFast(newdata->data, CRC_SIZE);
	ioman_seek(ALLOCATION_DATA_OFFSET + (parent_no * CLUSTER_SIZE));
	for(i=0;i<sizeof(ef_header);i++) {
		ioman_write(*(str++));
	}
	//file_write_cluster(dfhead, parent_no);
	if(cluster_no==0) {
		//create file here(buat child baru)
		dfhead->child = fs_get_next_available_space();
		fs_write_available_space(dfhead->child, TRUE);
		cluster_no = dfhead->child;
		cur_ptr = dfhead->child;		//select header baru
		dfhead->crc = crcFast(newdata->data, CRC_SIZE);
		//tulis stream ke eeprom   ////////////////////////////////////////////////////////////////
		str = (char *)dfhead;		//update data parent
		ioman_seek(ALLOCATION_DATA_OFFSET + (parent_no * CLUSTER_SIZE));
		for(i=0;i<sizeof(ef_header);i++)
		{
			ioman_write(*(str++));
		}
		//file_write_cluster(dfhead, parent_no);
		free(dfhead);
		//tulis header file yang baru dibuat
		header = (ef_header *) malloc (sizeof(ef_header));
		newdata = (ef_body *)header;
		header->type = T_EF;
		header->size = size;
		header->FID = fid;
		header->structure = EF_TRANSPARENT;
		header->status = 0x00;
		header->access_rw = (ACC_READ << 4) | (ACC_WRT & 0x0F);
		header->access_inc = (ACC_INC << 4);
		header->access_ri = (ACC_RHB << 4) | (ACC_INV & 0x0F);
		header->record_size = 0x00;
		header->parent = parent_no;
		header->sibling = 0x0000;
		header->child = 0x0000;
		header->next = 0;
		header->crc = crcFast(newdata->data, CRC_SIZE);
		str = (char *)header;
		ioman_seek(ALLOCATION_DATA_OFFSET + (cluster_no * CLUSTER_SIZE));
		for(i=0;i<sizeof(ef_header);i++) {
			ioman_write(*(str++));
		}
		//file_write_cluster(header, cluster_no);
		//tulis data dengan bantuan _writebin
		free(header);
		return _writebin(0, size, buffer);
	} else {
		//scan till last child
		get_last_child:
		if(cluster_no==0) {
			//create file here
			if(header->type == T_EF){
				header->sibling = fs_get_next_available_space();
				fs_write_available_space(header->sibling, TRUE);
				cluster_no = header->sibling;
				cur_ptr = header->sibling;		//select header baru
				//tulis stream ke eeprom   ////////////////////////////////////////////////////////////////
				str = (char *)header;			//update data last sibling
				newdata = (ef_body *)header;
				header->crc = crcFast(newdata->data, CRC_SIZE);
				ioman_seek(ALLOCATION_DATA_OFFSET + (sibling_no * CLUSTER_SIZE));
				for(i=0;i<sizeof(ef_header);i++)
				{
					ioman_write(*(str++));
				}
				file_write_cluster(header, sibling_no);
				free(header);
			}
			else {
				dfhead->sibling = fs_get_next_available_space();
				fs_write_available_space(dfhead->sibling, TRUE);
				cluster_no = dfhead->sibling;
				cur_ptr = dfhead->sibling;		//select header baru
				//tulis stream ke eeprom   ////////////////////////////////////////////////////////////////
				str = (char *)dfhead;			//update data last sibling
				newdata = (ef_body *)dfhead;
				dfhead->crc = crcFast(newdata->data, CRC_SIZE);
				ioman_seek(ALLOCATION_DATA_OFFSET + (sibling_no * CLUSTER_SIZE));
				for(i=0;i<sizeof(df_header);i++)
				{
					ioman_write(*(str++));
				}
				//file_write_cluster(dfhead, sibling_no);
				free(dfhead);
			}
			//tulis header file yang baru dibuat
			header = (ef_header *) malloc (sizeof(ef_header));
			newdata = (ef_body *)header;
			header->type = T_EF;
			header->size = size;
			header->FID = fid;
			header->structure = EF_TRANSPARENT;
			header->status = 0x00;
			header->access_rw = (ACC_READ << 4) | (ACC_WRT & 0x0F);
			header->access_inc = (ACC_INC << 4);
			header->access_ri = (ACC_RHB << 4) | (ACC_INV & 0x0F);
			header->record_size = 0x00;
			header->parent = parent_no;
			header->sibling = 0x0000;
			header->child = 0x0000;
			header->next = 0;
			header->crc = crcFast(newdata->data, CRC_SIZE);
			str = (char *)header;			//update data last sibling
			ioman_seek(ALLOCATION_DATA_OFFSET + (cluster_no * CLUSTER_SIZE));
			for(i=0;i<sizeof(ef_header);i++)
			{
				ioman_write(*(str++));
			}
			//file_write_cluster(header, cluster_no);
			//tulis data dengan bantuan _writebin
			free(header);
			return _writebin(0, size, buffer);
		}
		free(header);
		sibling_no = cluster_no;
		header = (ef_header *)file_get_header(cluster_no);
		dfhead = (df_header *)header;
		newdata = (ef_body *)header;
		if(header->FID == fid) {
			free(header);
			return APDU_FILE_NOT_FOUND;
		}
		cluster_no = header->sibling;
		goto get_last_child;
	}
}

uint16 _createfilerec(
	uint16 fid, 
	uchar ACC_READ, 
	uchar ACC_WRT, 
	uchar ACC_INC, 
	uchar ACC_INV, 
	uchar ACC_RHB, 
	uint16 total_rec, 
	uchar rec_size)
{
	ef_header *header = (ef_header *)file_get_header(cur_ptr);
	df_header *dfhead = (df_header *)header;
	ef_body *newdata = (ef_body *)header;
	uint16 cluster_no;
	uint16 parent_no = cur_ptr;
	uint16 sibling_no;
	uchar i;
	char *str;
	char *buffer = (char *)malloc(rec_size);	//isi dengan padding karakter
	uint16 status;
	for(i=0;i<rec_size;i++)
	{
		buffer[i] = 0xff;
	}
	if(header->type == T_EF) {
		//cluster_no = header->parent;
		//free(header);
		//parent_no = cluster_no;
		//dfhead = (df_header *)file_get_header(cluster_no);
		free(header);
		return APDU_COMMAND_INVALID;
	}
	cluster_no = dfhead->child;
	dfhead->number_of_ef += 1;
	str = (char *)dfhead;		//update data parent
	newdata = (ef_body *)dfhead;
	newdata->crc = crcFast(newdata->data, CRC_SIZE);
	ioman_seek(ALLOCATION_DATA_OFFSET + (parent_no * CLUSTER_SIZE));
	for(i=0;i<sizeof(ef_header);i++) {
		ioman_write(*(str++));
	}
	//file_write_cluster(dfhead, parent_no);
	if(cluster_no==0) {
		//create file here(buat child baru)
		dfhead->child = fs_get_next_available_space();
		fs_write_available_space(dfhead->child, TRUE);
		cluster_no = dfhead->child;
		cur_ptr = dfhead->child;		//select header baru
		//tulis stream ke eeprom   ////////////////////////////////////////////////////////////////
		str = (char *)dfhead;		//update data parent
		newdata = (ef_body *)dfhead;
		newdata->crc = crcFast(newdata->data, CRC_SIZE);
		ioman_seek(ALLOCATION_DATA_OFFSET + (parent_no * CLUSTER_SIZE));
		for(i=0;i<sizeof(ef_header);i++)
		{
			ioman_write(*(str++));
		}
		//file_write_cluster(dfhead, parent_no);
		free(dfhead);
		//tulis header file yang baru dibuat
		header = (ef_header *) malloc (sizeof(ef_header));
		header->type = T_EF;
		header->size = rec_size * total_rec;
		header->FID = fid;
		header->structure = EF_LINIER;
		header->status = 0x00;
		header->access_rw = (ACC_READ << 4) | (ACC_WRT & 0x0F);
		header->access_inc = (ACC_INC << 4);
		header->access_ri = (ACC_RHB << 4) | (ACC_INV & 0x0F);
		header->record_size = rec_size;
		header->parent = parent_no;
		header->sibling = 0x0000;
		header->child = 0x0000;
		header->next = 0;
		str = (char *)header;
		newdata = (ef_body *)header;
		newdata->crc = crcFast(newdata->data, CRC_SIZE);
		ioman_seek(ALLOCATION_DATA_OFFSET + (cluster_no * CLUSTER_SIZE));
		for(i=0;i<sizeof(ef_header);i++) {
			ioman_write(*(str++));
		}
		//file_write_cluster(header, cluster_no);
		//tulis data dengan bantuan _writebin
		free(header);
		for(i=0;i<total_rec;i++)
		{
			status = _writerec(i, rec_size, buffer);
			if(status != APDU_SUCCESS) {
				return status;
			}
		}
		return APDU_SUCCESS;
	} else {
		//scan till last child
		get_last_child:
		if(cluster_no==0) {
			//create file here
			if(header->type == T_EF){
				header->sibling = fs_get_next_available_space();
				fs_write_available_space(header->sibling, TRUE);
				cluster_no = header->sibling;
				cur_ptr = header->sibling;		//select header baru
				//tulis stream ke eeprom   ////////////////////////////////////////////////////////////////
				str = (char *)header;			//update data last sibling
				newdata = (ef_body *)header;
				newdata->crc = crcFast(newdata->data, CRC_SIZE);
				ioman_seek(ALLOCATION_DATA_OFFSET + (sibling_no * CLUSTER_SIZE));
				for(i=0;i<sizeof(ef_header);i++)
				{
					ioman_write(*(str++));
				}
				//file_write_cluster(header, sibling_no);
				free(header);
			}
			else {
				dfhead->sibling = fs_get_next_available_space();
				fs_write_available_space(dfhead->sibling, TRUE);
				cluster_no = dfhead->sibling;
				cur_ptr = dfhead->sibling;		//select header baru
				//tulis stream ke eeprom   ////////////////////////////////////////////////////////////////
				str = (char *)dfhead;			//update data last sibling
				newdata = (ef_body *)dfhead;
				newdata->crc = crcFast(newdata->data, CRC_SIZE);
				ioman_seek(ALLOCATION_DATA_OFFSET + (sibling_no * CLUSTER_SIZE));
				for(i=0;i<sizeof(df_header);i++)
				{
					ioman_write(*(str++));
				}
				//file_write_cluster(dfhead, sibling_no);
				free(dfhead);
			}
			//tulis header file yang baru dibuat
			header = (ef_header *) malloc (sizeof(ef_header));
			header->type = T_EF;
			header->size = rec_size * total_rec;
			header->FID = fid;
			header->structure = EF_LINIER;
			header->status = 0x00;
			header->access_rw = (ACC_READ << 4) | (ACC_WRT & 0x0F);
			header->access_inc = (ACC_INC << 4);
			header->access_ri = (ACC_RHB << 4) | (ACC_INV & 0x0F);
			header->record_size = rec_size;
			header->parent = parent_no;
			header->sibling = 0x0000;
			header->child = 0x0000;
			header->next = 0;
			newdata = (ef_body *)header;
			newdata->crc = crcFast(newdata->data, CRC_SIZE);
			str = (char *)header;			//update data last sibling
			ioman_seek(ALLOCATION_DATA_OFFSET + (cluster_no * CLUSTER_SIZE));
			for(i=0;i<sizeof(ef_header);i++)
			{
				ioman_write(*(str++));
			}
			//file_write_cluster(header, cluster_no);
			//tulis data dengan bantuan _writebin
			free(header);
			for(i=0;i<total_rec;i++)
			{
				status = _writerec(i, rec_size, buffer);
				if(status !=APDU_SUCCESS) {
					return status;
				}
			}
			return APDU_SUCCESS;
		}
		free(header);
		sibling_no = cluster_no;
		header = (ef_header *)file_get_header(cluster_no);
		dfhead = (df_header *)header;
		newdata = (ef_body *)header;
		if(header->FID == fid) {
			free(header);
			return APDU_FILE_NOT_FOUND;
		}
		cluster_no = header->sibling;
		goto get_last_child;
	}
}

uint16 _createdirectory(uint16 fid, char *dirname, uchar len_name)
{
	ef_header *header = (ef_header *)file_get_header(cur_ptr);
	df_header *dfhead = (df_header *)header;
	ef_body *newdata = (ef_body *)header;
	uint16 cluster_no;
	uchar i;
	uint16 parent_no = cur_ptr;
	uint16 sibling_no;
	char *str;
	if(header->type == T_EF) {
		//cluster_no = header->parent;
		//free(header);
		//parent_no = cluster_no;
		//dfhead = (df_header *)file_get_header(cluster_no);
		free(header);
		return APDU_COMMAND_INVALID;
	}
	cluster_no = dfhead->child;
	dfhead->number_of_df += 1;
	str = (char *)dfhead;		//update data parent
	newdata = (ef_body *)dfhead;
	newdata->crc = crcFast(newdata->data, CRC_SIZE);
	ioman_seek(ALLOCATION_DATA_OFFSET + (cur_ptr * CLUSTER_SIZE));
	for(i=0;i<sizeof(ef_header);i++) {
		ioman_write(*(str++));
	}
	if(cluster_no==0)
	{
		dfhead->child = fs_get_next_available_space();
		fs_write_available_space(dfhead->child, TRUE);
		cluster_no = dfhead->child;
		//printf("%i\n", cluster_no);
		cur_ptr = dfhead->child;		//select header baru
		//tulis stream ke eeprom   ////////////////////////////////////////////////////////////////
		str = (char *)dfhead;		//update data parent
		newdata = (ef_body *)dfhead;
		newdata->crc = crcFast(newdata->data, CRC_SIZE);
		ioman_seek(ALLOCATION_DATA_OFFSET + (parent_no * CLUSTER_SIZE));
		for(i=0;i<sizeof(ef_header);i++)
		{
			ioman_write(*(str++));
		}
		free(dfhead);
		dfhead = (df_header *) malloc (sizeof(df_header));
		dfhead->type = T_DF;
		dfhead->size = 0x0000;
		dfhead->FID = fid;
		memclear(dfhead->name, 14);
		memcopy(dfhead->name, dirname, 0, len_name);
		dfhead->name_length = len_name;
		dfhead->number_of_df = 0;
		dfhead->number_of_ef = 0;
		dfhead->parent = parent_no;
		dfhead->sibling = 0;
		dfhead->child = 0;
		str = (char *)dfhead;			//update data last sibling
		newdata = (ef_body *)dfhead;
		newdata->crc = crcFast(newdata->data, CRC_SIZE);
		ioman_seek(ALLOCATION_DATA_OFFSET + (cluster_no * CLUSTER_SIZE));
		for(i=0;i<sizeof(ef_header);i++)
		{
			ioman_write(*(str++));
		}
		free(dfhead);
		return APDU_SUCCESS;
	} else {
		get_last_child:
		if(cluster_no==0)
		{
			//create file here
			if(header->type == T_EF){
				header->sibling = fs_get_next_available_space();
				fs_write_available_space(header->sibling, TRUE);
				cluster_no = header->sibling;
				cur_ptr = header->sibling;		//select header baru
				//tulis stream ke eeprom   ////////////////////////////////////////////////////////////////
				str = (char *)header;			//update data last sibling
				newdata = (ef_body *)header;
				newdata->crc = crcFast(newdata->data, CRC_SIZE);
				ioman_seek(ALLOCATION_DATA_OFFSET + (sibling_no * CLUSTER_SIZE));
				for(i=0;i<sizeof(ef_header);i++)
				{
					ioman_write(*(str++));
				}
				free(header);
			}
			else {
				dfhead->sibling = fs_get_next_available_space();
				fs_write_available_space(dfhead->sibling, TRUE);
				cluster_no = dfhead->sibling;
				cur_ptr = dfhead->sibling;		//select header baru
				//tulis stream ke eeprom   ////////////////////////////////////////////////////////////////
				str = (char *)dfhead;			//update data last sibling
				newdata = (ef_body *)dfhead;
				newdata->crc = crcFast(newdata->data, CRC_SIZE);
				ioman_seek(ALLOCATION_DATA_OFFSET + (sibling_no * CLUSTER_SIZE));
				for(i=0;i<sizeof(df_header);i++)
				{
					ioman_write(*(str++));
				}
				free(dfhead);
			}
			dfhead = (df_header *) malloc (sizeof(df_header));
			dfhead->type = T_DF;
			dfhead->size = 0x0000;
			dfhead->FID = fid;
			memcopy(dfhead->name, dirname, 0, len_name);
			dfhead->name_length = len_name;
			dfhead->number_of_df = 0;
			dfhead->number_of_ef = 0;
			dfhead->parent = parent_no;
			dfhead->sibling = 0;
			dfhead->child = 0;
			str = (char *)dfhead;			//update data last sibling
			newdata = (ef_body *)dfhead;
			newdata->crc = crcFast(newdata->data, CRC_SIZE);
			ioman_seek(ALLOCATION_DATA_OFFSET + (cluster_no * CLUSTER_SIZE));
			for(i=0;i<sizeof(ef_header);i++)
			{
				ioman_write(*(str++));
			}
			free(dfhead);
			return APDU_SUCCESS;
		}
		free(header);
		sibling_no = cluster_no;
		//printf("sibling no : %i\n", sibling_no);
		header = (ef_header *)file_get_header(cluster_no);
		dfhead = (df_header *)header;
		newdata = (ef_body *)header;
		newdata->crc = crcFast(newdata->data, CRC_SIZE);
		if(header->FID == fid) {
			free(header);
			return APDU_FILE_NOT_FOUND;
		}
		cluster_no = header->sibling;
		goto get_last_child;
	}
}

void file_dirlist()
{
	uint16 cluster_no=0;			//pilih root, MF
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
	//printf
	for(k=0;k<j;k++) {
		if(k==(j-1)){
			putchar(0xc3);
		} else if((k%5)==4) {
			putchar(0xb3);
		} else {
			putchar(' ');
		}
	}
	//memcopy(buffer, curfile->id, 0, 4);
	printf("%x", curfile->FID);
	if(curfile->child!=0) {
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
	goto get_next_child;
}

uint16 _remove(uint16 file_id)	//belum dites
{
	uint16 cluster_no, c_pos, parent_pos = cur_ptr;
	uint16 temp, sibling;
	ef_header *newfile;
	df_header *df_head;
	ef_body *newdata;
	uchar i;
	uchar curfiletype;
	//char *str;
	if(_select(file_id) == APDU_FILE_NOT_FOUND) {	//dicoba diselect dulu, kalau gagal brarti file tidak ditemukan
		return APDU_FILE_NOT_FOUND;
	}
	c_pos = cur_ptr;
	cur_ptr = parent_pos;
	newfile = file_get_header(c_pos);
	curfiletype = newfile->type;
	temp = newfile->parent;		//inisialisasi mulai dari parent
	sibling = newfile->sibling;
	//printf("sibling dari file yang dihapus = %i\n", sibling);
	free(newfile);
	//if(c_pos==FILE_NOT_FOUND) { cur_ptr = parent_pos; return APDU_FILE_NOT_FOUND; }
	//cluster_no = cur_ptr;
	newfile =  file_get_header(cur_ptr);	//dapat header parent
	df_head = (df_header *)newfile;
	//str = (char *)newfile;
	if(curfiletype == T_EF) {
		df_head->number_of_ef -= 1;	//ubah nilai number of ef
	} else {
		df_head->number_of_df -= 1;	//ubah nilai number of df
	}
	//update parent header
	/*newdata = (ef_body *)str;
	newdata->crc = crcFast(newdata->data, CRC_SIZE);
	ioman_seek(ALLOCATION_DATA_OFFSET + (cur_ptr * CLUSTER_SIZE));
	for(i=0;i<sizeof(ef_header);i++)
	{
		ioman_write(*(str++));
	}*/
	file_write_cluster(newfile, cur_ptr);
	//temp = cur_ptr;
	//mulai melakukan searching child dari child yang paling awal
	temp = newfile->child;
	cluster_no = newfile->child;
	delete_cluster_chain:
	if(cluster_no==c_pos)	{	//adalah file yang akan dihapus
		//str = (char *)newfile;
		//curfiletype = newfile->type; 
		if(cluster_no==temp) {
			//jika ternyata adalah parent
			newfile->child = sibling;
			//operasi write header
			/*newdata = (ef_body *)str;
			newdata->crc = crcFast(newdata->data, CRC_SIZE);
			ioman_seek(ALLOCATION_DATA_OFFSET + (cur_ptr * CLUSTER_SIZE));
			for(i=0;i<sizeof(ef_header);i++) {
				ioman_write(*(str++));
			}*/
			file_write_cluster(newfile, cur_ptr);
			free(newfile);
		} else {	//jika bukan parent, brarti sibling
			newfile->sibling = sibling;
			//operasi write header
			/*newdata = (ef_body *)str;
			newdata->crc = crcFast(newdata->data, CRC_SIZE);
			ioman_seek(ALLOCATION_DATA_OFFSET + (temp * CLUSTER_SIZE));
			for(i=0;i<sizeof(ef_header);i++) {
				ioman_write(*(str++));
			}*/
			file_write_cluster(newfile, temp);
			free(newfile);
		}
	} else {	//bukan file yang akan dihapus
		free(newfile);
		temp = cluster_no;
		newfile = file_get_header(cluster_no);	
		//str = (char *)newfile;
		cluster_no = newfile->sibling;
		goto delete_cluster_chain;
	}
	
	cluster_no = c_pos;			///delete header yang sebenarnya
	delete_allocation_table:
	newfile = file_get_header(cluster_no);
	fs_write_available_space(cluster_no, FALSE);
	//next pointer harus diisi 0, jika tidak maka ketika header tersebut dipakai lagi maka dianggap masih ada pointer
	//ke data selanjutnya, padahal data tersebut seharusnya sudah terhapus. tidak usah urusin CRC karena data ini tidak valid
	ioman_seek(ALLOCATION_DATA_OFFSET + (cluster_no * CLUSTER_SIZE) + DATA_SIZE);
	for(i=0;i<sizeof(uint16);i++) {
		ioman_write(0);	//hapus pointer ke data selanjutnya, tanpa menghapus datanya
	}

	if(newfile->next!=0 && curfiletype == T_EF) {
		//yang dihapus hanya allocation table saja, datanya masih
		cluster_no = newfile->next;
		free(newfile);
		goto delete_allocation_table;
	}
	free(newfile); 
	return APDU_SUCCESS;
}

uint16 _invalidate()
{
	ef_header *header = (ef_header *)file_get_header(cur_ptr);
	ef_body *newdata = (ef_body *)header;
	//char *str = (char *)header;
	uchar status;
	uchar i;
	
	if(header->type != T_EF) {
		free(header);
		return APDU_NO_EF_SELECTED;
	}

	status = getCHVstatus(header->access_ri & 0x0f);	//cek status CHV, access denied jika chv tidak terpenuhi
	switch(status)
	{
		case CHV_NEVER:
		case CHV_ENABLE:
		case CHV_BLOCK:
			free(header);
			return APDU_ACCESS_DENIED;
			break;
		case CHV_DISABLE:
		case CHV_UNBLOCK:
		case CHV_VERIFIED:
		case CHV_ALWAYS:
		default:
			break;
	}
	if(header->status == STAT_INVALID) {	//status sudah invalid
		free(header);
		return APDU_INVALID_STATUS;
	}
	header->status = STAT_INVALID;
	//tulis header disini
	/*newdata = (ef_body *)str;
	newdata->crc = crcFast(newdata->data, CRC_SIZE);
	ioman_seek(ALLOCATION_DATA_OFFSET + (cur_ptr * CLUSTER_SIZE));
	for(i=0;i<sizeof(ef_header);i++) {
		ioman_write(*(str++));
	}*/
	file_write_cluster(header, cur_ptr);
	free(header);
	return APDU_SUCCESS;
}

uint16 _rehabilitate()
{
	ef_header *header = (ef_header *)file_get_header(cur_ptr);
	ef_body *newdata = (ef_body *)header;
	//char *str = (char *)header;
	uchar status;
	uchar i;
	
	if(header->type != T_EF) {
		free(header);
		return APDU_NO_EF_SELECTED;
	}

	status = getCHVstatus(header->access_ri >> 4);	//cek status CHV, access denied jika chv tidak terpenuhi
	switch(status)
	{
		case CHV_NEVER:
		case CHV_ENABLE:
		case CHV_BLOCK:
			free(header);
			return APDU_ACCESS_DENIED;
			break;
		case CHV_DISABLE:
		case CHV_UNBLOCK:
		case CHV_VERIFIED:
		case CHV_ALWAYS:
		default:
			break;
	}
	if(header->status == STAT_VALID) {	//status sudah valid
		free(header);
		return APDU_INVALID_STATUS;
	}
	header->status = STAT_VALID;
	//tulis header disini
	/*newdata = (ef_body *)str;
	newdata->crc = crcFast(newdata->data, CRC_SIZE);
	ioman_seek(ALLOCATION_DATA_OFFSET + (cur_ptr * CLUSTER_SIZE));
	for(i=0;i<sizeof(ef_header);i++) {
		ioman_write(*(str++));
	}*/
	file_write_cluster(header, cur_ptr);
	free(header);
	return APDU_SUCCESS;
}

































//unused functions
#if 0
uint16 file_get_last_child(uint16 parent)
{
	ef_header *newfile = file_get_header(parent);	
	uint16 cluster_no;
	if(newfile->child==0) { cluster_no = newfile->pos; free(newfile); return cluster_no; }
	newfile = file_get_header(newfile->child);
	while(newfile->sibling!=0)
	{
		newfile = file_get_header(newfile->sibling);
	}
	cluster_no = newfile->pos;
	free(newfile);	//hapus karena tidak dipakai
	return cluster_no;
}

void file_add(uint16 prvnode, uint16 sibling, uint16 parent)
{
	ef_header *newfile = file_get_header(prvnode);
	char *str = (char *)newfile;
	uchar i;
	//printf("prev node : %i, new file : %i, parent : %i\n", prvnode, sibling, parent);
	if(parent==prvnode){newfile->child = sibling;}
	else{
	newfile->sibling = sibling;}
	ioman_seek((newfile->pos * CLUSTER_SIZE)+ ALLOCATION_DATA_OFFSET);
	for(i=0;i<sizeof(ef_header);i++)
	{
		ioman_write(*(str++));
	}
	free(newfile);	//baru dihapus karena sudah dipakai
}



uint16 file_create(char *id, uchar type, uchar structure, uint16 parent)
{
	ef_header *newfile = (ef_header *) malloc (sizeof(ef_header));
	char *str = (char *)newfile;
	uchar i;
	uint16 pos = fs_get_next_available_space();	//cluster no
	//printf("new space : %i\n", pos);
	fs_write_available_space(pos, TRUE);
	//newfile->pos = pos;
	memcopy(newfile->id, id, 0, 4);
	newfile->type = type;
	newfile->structure = structure;
	newfile->size = 0;
	newfile->parent = parent;
	newfile->sibling = 0;
	newfile->child = 0;
	newfile->next = 0;
	//return value adalah cluster number	
	ioman_seek(ALLOCATION_DATA_OFFSET + (pos * CLUSTER_SIZE));
	for(i=0;i<sizeof(ef_header);i++)
	{
		ioman_write(*(str++));
	}
	free(newfile);	//hapus pointer karena tidak digunakan
	return pos;
}


uint16 file_write_bin(ef_header * header, uint16 offset, char *buffer, uint16 size)
{
	uint16 c_size = header->size;
	uint16 cluster_no = header->next;
	uint16 _ptr = 0;
	uint16 i,j=0;
	ef_body *newdata;
	char *str = (char *)header;
	if(offset>header->size) { //wrong offset
		free(header); 
		return 0; 
	}	else 	{
		if((size+offset)>header->size){	
			header->size =  size+offset;
		}
		//write new header here
		if(header->next==0){
			header->next = fs_get_next_available_space();
			fs_write_available_space(header->next, TRUE);
			cluster_no = header->next;
		}
		ioman_seek(ALLOCATION_DATA_OFFSET + (header->pos * CLUSTER_SIZE));
		for(i=0;i<sizeof(ef_header);i++)
		{
			ioman_write(*(str++));
		}
		free(header);
	}
	
	if(cluster_no==0)
	{
		cluster_no = fs_get_next_available_space();	//create new cluster for data
		fs_write_available_space(cluster_no, TRUE);
	}
	c_size = offset - _ptr;
	get_next_cluster:
	newdata = (ef_body *)file_get_header(cluster_no);
	
	if(offset > (_ptr + DATA_SIZE))
	{
		cluster_no = newdata->next;
		_ptr += DATA_SIZE;
		free(newdata);
		c_size = offset - _ptr;
		goto get_next_cluster;
	}
	
	
	//printf("current cluster = %i\n",(_ptr + DATA_SIZE - offset));
	if((_ptr + DATA_SIZE - offset) < size)
	{
		//printf("size wrote = %i, total size = %i\n",DATA_SIZE-c_size, size); 
		j += memcopy(newdata->data+c_size, buffer+j, 0, DATA_SIZE-c_size);
		str = (char *)newdata;
		
		if(newdata->next==0){
			newdata->next = fs_get_next_available_space();	//create new cluster for data
			fs_write_available_space(newdata->next, TRUE);
		}
		ioman_seek(ALLOCATION_DATA_OFFSET + (cluster_no * CLUSTER_SIZE));
		for(i=0;i<sizeof(ef_header);i++)
		{
			ioman_write(*(str++));
		}
		cluster_no = newdata->next;
		_ptr += DATA_SIZE;		
		free(newdata);
		c_size = 0;
		goto get_next_cluster;
	}
	else
	{
		//printf("size wrote = %i, total size = %i\n",size-j, size);
		j += memcopy(newdata->data+c_size, buffer+j, 0, size-j);
		str = (char *)newdata;
		 
		newdata->next = 0;
		ioman_seek(ALLOCATION_DATA_OFFSET + (cluster_no * CLUSTER_SIZE));
		for(i=0;i<sizeof(ef_header);i++)
		{
			ioman_write(*(str++));
		}
		free(newdata);
	}
	return j;
}

uint16 file_read_bin(ef_header * header, uint16 offset, char *buffer, uint16 size)
{
	uint16 c_size = header->size;
	uint16 cluster_no = header->next;
	uint16 _ptr = 0;
	uint16 i,j=0;
	ef_body *newdata;
	char *str = (char *)header;
	if(offset>header->size) { //wrong offset
		free(header); 
		return 0; 
	}
	if(offset+size>header->size) { //wrong offset
		free(header); 
		return 0; 
	}
	else 	{
		free(header);
	}
	
	if(cluster_no==0)
	{
		return 0;	//no data available
	}
	c_size = offset - _ptr;
	get_next_cluster:
	newdata = (ef_body *)file_get_header(cluster_no);
	
	if(offset > (_ptr + DATA_SIZE))
	{
		cluster_no = newdata->next;
		_ptr += DATA_SIZE;
		free(newdata);
		c_size = offset - _ptr;
		goto get_next_cluster;
	}


	if((_ptr + DATA_SIZE - offset) < size)
	{
		//printf("size read = %i, total size = %i\n",DATA_SIZE - c_size, size);
		j += memcopy(buffer+j, newdata->data + c_size, 0, DATA_SIZE - c_size);		//22, 8 deteksi ukuran tersisa
		cluster_no = newdata->next;
		_ptr += DATA_SIZE;
		free(newdata);
		c_size = 0;
		goto get_next_cluster;
	}
	else
	{
		//printf("size read = %i, total size = %i\n",size-j, size);
		j += memcopy(buffer+j, newdata->data + c_size, 0, size-j);		//22, 8 deteksi ukuran tersisa
		cluster_no = newdata->next;
		free(newdata);
	}
	return j;
}


//gunakan algoritma BFS untuk menelusuri semua header dan mencari file berdasarkan ID
//nilai kembalian berupa nomor cluster dari header file
uint16 file_seek(char *file_id)
{
	uint16 cluster_no=0;			//pilih root, MF
	uint16 node_queue[QUEUE_SIZE];	//nodes queue, maksimum hanya menyimpan 5 child
	uchar queue_index_in=0;
	uchar queue_index_out=0;
	char i;
	ef_header *curfile;				//inisialisasi
	get_next_sibling:
	curfile = file_get_header(cluster_no);
	if(curfile->child!=0)
	{
		//enqueue child
		node_queue[queue_index_in++] = curfile->child;
	}
	//printf("%s, %i\n", curfile->id, queue_index);
	if(memcompare(curfile->id, file_id, 0, 4)==SUCCESS)
	{
		return cluster_no;
	}
	if(curfile->sibling!=0) {
		cluster_no = curfile->sibling;
		goto get_next_sibling;
	}
	if(queue_index_in==queue_index_out) { return FILE_NOT_FOUND; }
	//dequeue
	cluster_no = node_queue[queue_index_out++];
	goto get_next_sibling;
}

uchar file_remove(char *file_id)	//belum dites
{
	uint16 cluster_no, c_pos= file_seek(file_id);
	uint16 temp, sibling;
	ef_header *newfile;
	uchar i;
	char *str;
	newfile = file_get_header(c_pos);
	temp = newfile->parent;		//inisialisasi mulai dari parent
	sibling = newfile->sibling;
	//printf("sibling dari file yang dihapus = %i\n", sibling);
	free(newfile);
	if(c_pos==FILE_NOT_FOUND) { return FAIL; }
	newfile =  file_get_header(temp);	//dapat header parent
	cluster_no = newfile->child;		//mulai melakukan searching child dari child yang paling awal
	delete_cluster_chain:
	if(cluster_no==c_pos)	{	//adalah file yang akan dihapus
		str = (char *)newfile; 
		
		if(newfile->pos==temp)
		{
			//jika ternyata adalah parent
			newfile->child = sibling;
		}
		else
		{	//jika bukan parent, brarti sibling
			newfile->sibling = sibling;
		}
		//operasi write header
		ioman_seek(ALLOCATION_DATA_OFFSET + (newfile->pos * CLUSTER_SIZE));
		for(i=0;i<sizeof(ef_header);i++)
		{
			ioman_write(*(str++));
		}
		free(newfile);
	} else {	//bukan file yang akan dihapus
		free(newfile);
		newfile = file_get_header(cluster_no);	
		str = (char *)newfile;
		cluster_no = newfile->sibling;
		goto delete_cluster_chain;
	}
	
	cluster_no = c_pos;			///delete header yang sebenarnya
	delete_allocation_table:
	newfile = file_get_header(cluster_no);
	fs_write_available_space(cluster_no, FALSE);
	ioman_seek(ALLOCATION_DATA_OFFSET + (cluster_no * CLUSTER_SIZE) + DATA_SIZE);
	for(i=0;i<sizeof(uint16);i++)
	{
		ioman_write(0);	//hapus pointer ke data selanjutnya, tanpa menghapus datanya
	}
	if(newfile->next!=0)
	{
		//yang dihapus hanya allocation table saja, datanya masih
		cluster_no = newfile->next;
		free(newfile);
		goto delete_allocation_table;
	}
	free(newfile);
	return SUCCESS;
}

//utk file_dirlist gunakan algoritma DFS
void file_dirlist()
{
	uint16 cluster_no=0;			//pilih root, MF
	uint16 node_stack[QUEUE_SIZE+10];	//nodes queue, maksimum hanya menyimpan 5 child
	uint16 space_stack[QUEUE_SIZE+10];
	uchar stack_index=1;
	uchar space_index=1;
	char i,j=0,k;
	ef_header *curfile;				//inisialisasi
	char buffer[5] = { 0,0,0,0,0 };
	get_next_child:
	curfile = file_get_header(cluster_no);
	if(curfile->sibling!=0)
	{
		//enqueue child
		space_stack[space_index++] = j;
		node_stack[stack_index++] = curfile->sibling;
	}
	//printf
	for(k=0;k<j;k++)
	{
		if(k==(j-1)){
			putchar(0xc3);
		} else if((k%5)==4) {
			putchar(0xb3);
		} else {
			putchar(' ');
		}
	}
	memcopy(buffer, curfile->id, 0, 4);
	printf("%s", buffer);
	if(curfile->child!=0) {
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
	goto get_next_child;
}
#endif


