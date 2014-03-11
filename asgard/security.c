#include "security.h"
#include "..\defs.h"
#include "..\config.h"
#include "..\drivers\ioman.h"
#include "fs.h"
#include "..\auth\des.h"
#include "..\yggdrasil\yggdrasil.h"
#include "..\misc\barrenkalaheapcalc.h"
#include "..\midgard\midgard.h"
#include "..\misc\mem.h"
#include <string.h>

#if ASGARD_VERSION == 2
#if USE_SHARED_VAR
BYTEC padding[8] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
uint16 cluster_no;
uchar * str;
uint16 i;
chv_file *cf;
uchar buffer[8];
uchar status;
#endif

uint16 getCHVcluster(uchar no)
{
	return (((table.partition_size - 512)/CLUSTER_SIZE)) - no;  
}

uint16 createCHV(uchar no, char *pin, char *puk, uchar pin_max_retry, uchar puk_max_retry) _REENTRANT_
{
	#if !USE_SHARED_VAR
	char padding[8] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	uint16 cluster_no;
	char * str;
	uint16 i;
	chv_file *cf;
	char buffer[8];
	#endif
	cf = (chv_file *)m_alloc(sizeof(chv_file));
	#if _DMA_DEBUG
	barren_insert(cf, sizeof(chv_file));
	#endif
	cluster_no = getCHVcluster(no);
	//printf("cluster no for chv %i is %i\n", no, cluster_no);
	fs_write_available_space(cluster_no, TRUE);
	str = (char *) cf;
	cf->type = T_CHV;
	cf->chv_no = no & 0x0F;
	if(cf->chv_no == ACC_ALW) {
		m_free(cf);
		#if _DMA_DEBUG
		barren_eject(cf);
		#endif
		return FILE_CANNOT_BE_CREATED;
	}
	cf->status = CHV_DISABLE;
	memcopy(buffer, padding, 0, 8);				//dienkripsi dengan des
	memcopy(buffer, pin, 0, strlen(pin) );
	//auth_DEShash( cf->pin, CHV_KEY, buffer);
	/*decrypt_DEShash(buffer, CHV_KEY, cf->pin);
	for(i=0;i<8;i++)
	{
		printf("%x ", (uchar)buffer[i]);
	}
	printf("\n");*/
		
	memcopy(buffer, padding, 0, 8);				//dienkripsi dengan des
	memcopy(buffer, puk, 0, strlen(puk) );
	//auth_DEShash( cf->puk, CHV_KEY, buffer);
	memcopy(cf->pin, buffer, 0, 8);
	//memcopy(cf->puk, padding, 0, 8);
	cf->pin_attempts = 0;
	cf->puk_attempts = 0;
	cf->pin_max_attempts = pin_max_retry;
	cf->puk_max_attempts = puk_max_retry;
	
	/*ioman_seek((cluster_no * CLUSTER_SIZE) + ALLOCATION_DATA_OFFSET);
	for(i=0;i<sizeof(chv_file);i++)
	{
		ioman_write(*(str++));
	}*/
	ioman_write_buffer((cluster_no * CLUSTER_SIZE) + ALLOCATION_DATA_OFFSET, str, sizeof(chv_file));
	m_free(cf);
	#if _DMA_DEBUG
	barren_eject(cf);
	#endif
	return cluster_no;
}

chv_file * file_get_chv(uint16 cluster_no)
{
	chv_file *newfile;
	char *str;
	uchar i;
	newfile = (chv_file *) m_alloc (sizeof(chv_file));
	#if _DMA_DEBUG
	barren_insert(newfile, sizeof(chv_file));
	#endif
	str = (char *)newfile;
	/*ioman_seek((cluster_no * CLUSTER_SIZE) + ALLOCATION_DATA_OFFSET);
	for(i=0;i<sizeof(chv_file);i++)
	{
		*(str++) = ioman_read();
	}*/
	ioman_read_buffer((cluster_no * CLUSTER_SIZE) + ALLOCATION_DATA_OFFSET, str, sizeof(chv_file));
	return newfile;	//jangan dihapus karena akan dipakai
}

uchar getCHVstatus(uchar chv_no)
{
	#if !USE_SHARED_VAR
	uchar status;
	chv_file * cf;
	#endif
	chv_no &= 0x0f;
	#ifdef _YGGDRASIL_MICRO_KERNEL		//fungsi ini dijalankan didalam sistem operasi
	if(v_chv_status[chv_no] == CHV_VERIFIED) {
		return CHV_VERIFIED;
	}
	#endif
	cf = file_get_chv(getCHVcluster(chv_no));
	if(chv_no == ACC_ALW) {
		status = CHV_ALWAYS;
	} else if(chv_no == ACC_NVR) {
		status = CHV_NEVER;
	} else {
		status = cf->status;
	}
	m_free(cf);
	#if _DMA_DEBUG
	barren_eject(cf);
	#endif
	return status;
}

uchar verifyCHV(uchar chv_no, char *pin) _REENTRANT_
{
	#if !USE_SHARED_VAR
	uchar status;
	char * str;
	uint16 i;
	uint16 cluster_no;
	chv_file * cf;
	char buffer[8];
	#endif
	chv_no &= 0x0f;	
	if(chv_no == ACC_ALW) {
		return CHV_DISABLE;
	}
	cluster_no = getCHVcluster(chv_no);
	cf = file_get_chv(cluster_no);
	str = (char *) cf;
	if(cf->status == CHV_BLOCK) {
		status = cf->status;
		m_free(cf);
		#if _DMA_DEBUG
		barren_eject(cf);
		#endif
		return status;
	}
	//auth_DEShash(buffer, CHV_KEY, pin);
	//memcopy(pin, buffer, 0, 8);

	if(memcompare(cf->pin, pin, 0, 8)==TRUE) {
		cf->pin_attempts = 0;
		status = cf->status;
		/*ioman_seek((cluster_no * CLUSTER_SIZE) + ALLOCATION_DATA_OFFSET);
		for(i=0;i<sizeof(chv_file);i++) {
			ioman_write(*(str++));
		} */
		ioman_write_buffer((cluster_no * CLUSTER_SIZE) + ALLOCATION_DATA_OFFSET, str, sizeof(chv_file));
		///if(status != CHV_DISABLE) {
			#ifdef _YGGDRASIL_MICRO_KERNEL		//fungsi ini dijalankan didalam sistem operasi
			v_chv_status[chv_no] = CHV_VERIFIED; 	//set status chv didalam kernel menjadi CHV_VERIFIED
			#endif
			status = CHV_VERIFIED;
		//}
	} else {
		cf->pin_attempts += 1;
		status = cf->status;
		if(cf->pin_attempts >= cf->pin_max_attempts) {
			status = CHV_BLOCK;
		}
		cf->status = status;
		/*ioman_seek((cluster_no * CLUSTER_SIZE) + ALLOCATION_DATA_OFFSET);
		for(i=0;i<sizeof(chv_file);i++) {
			ioman_write(*(str++));
		}*/
		ioman_write_buffer((cluster_no * CLUSTER_SIZE) + ALLOCATION_DATA_OFFSET, str, sizeof(chv_file));
		status = CHV_PIN_FAILED;
	}
	m_free(cf);
	#if _DMA_DEBUG
	barren_eject(cf);
	#endif
	return status;
}

uchar enableCHV(uchar chv_no, char *pin) _REENTRANT_
{
	#if !USE_SHARED_VAR
	uchar status;
	char * str;
	uint16 i;
	uint16 cluster_no;
	chv_file * cf;
	#endif
	//char buffer[8];
	chv_no &= 0x0f;
	
	//auth_DEShash( buffer, CHV_KEY, pin);
	//memcopy(pin, buffer, 0, 8);
	if(chv_no == ACC_ALW) {
		return CHV_DISABLE;
	}
	cluster_no = getCHVcluster(chv_no);
	cf = file_get_chv(cluster_no);
	str = (char *) cf;
	if(chv_no != ACC_CHV1)	//selain chv1 yang lain tidak dapat didisable/enable
	{
		status = cf->status;
		m_free(cf);
		#if _DMA_DEBUG
		barren_eject(cf);
		#endif
		return status;
	}
	if(cf->status == CHV_BLOCK) {
		status = cf->status;
		m_free(cf);
		#if _DMA_DEBUG
		barren_eject(cf);
		#endif
		return status;
	}


	if(memcompare(cf->pin, pin, 0, 8)==TRUE) {
		cf->pin_attempts = 0;
		status = CHV_ENABLE;
		cf->status = status;
		/*ioman_seek((cluster_no * CLUSTER_SIZE) + ALLOCATION_DATA_OFFSET);
		for(i=0;i<sizeof(chv_file);i++) {
			ioman_write(*(str++));
		}*/
		ioman_write_buffer((cluster_no * CLUSTER_SIZE) + ALLOCATION_DATA_OFFSET, str, sizeof(chv_file));
	} else {
		cf->pin_attempts += 1;
		status = cf->status;
		if(cf->pin_attempts >= cf->pin_max_attempts) {
			status = CHV_BLOCK;
		}
		cf->status = status;
		/*ioman_seek((cluster_no * CLUSTER_SIZE) + ALLOCATION_DATA_OFFSET);
		for(i=0;i<sizeof(chv_file);i++) {
			ioman_write(*(str++));
		}*/
		ioman_write_buffer((cluster_no * CLUSTER_SIZE) + ALLOCATION_DATA_OFFSET, str, sizeof(chv_file));
		status = CHV_PIN_FAILED;
	}
	m_free(cf);
	#if _DMA_DEBUG
	barren_eject(cf);
	#endif
	return status;
}

uchar disableCHV(uchar chv_no, char *pin) _REENTRANT_
{
	#if !USE_SHARED_VAR
	uchar status;
	char * str;
	uint16 i;
	uint16 cluster_no;
	chv_file * cf;
	#endif
	//char buffer[8];
	chv_no &= 0x0f;
	
	//auth_DEShash( buffer, CHV_KEY, pin);
	//memcopy(pin, buffer, 0, 8);
	if(chv_no == ACC_ALW) {
		return CHV_DISABLE;
	}
	cluster_no = getCHVcluster(chv_no);
	cf = file_get_chv(cluster_no);
	str = (char *) cf;
	if(chv_no != ACC_CHV1)	//selain chv1 yang lain tidak dapat didisable/enable
	{
		status = cf->status;
		m_free(cf);
		#if _DMA_DEBUG
		barren_eject(cf);
		#endif
		return status;
	}
	if(cf->status == CHV_BLOCK) {
		status = cf->status;
		m_free(cf);
		#if _DMA_DEBUG
		barren_eject(cf);
		#endif
		return status;
	}

	if(memcompare(cf->pin, pin, 0, 8)==TRUE) {
		cf->pin_attempts = 0;
		status = CHV_DISABLE;
		cf->status = status;
		/*ioman_seek((cluster_no * CLUSTER_SIZE) + ALLOCATION_DATA_OFFSET);
		for(i=0;i<sizeof(chv_file);i++) {
			ioman_write(*(str++));
		}*/
		ioman_write_buffer((cluster_no * CLUSTER_SIZE) + ALLOCATION_DATA_OFFSET, str, sizeof(chv_file));
	} else {
		cf->pin_attempts += 1;
		status = cf->status;
		if(cf->pin_attempts >= cf->pin_max_attempts) {
			status = CHV_BLOCK;
		}
		cf->status = status;
		/*ioman_seek((cluster_no * CLUSTER_SIZE) + ALLOCATION_DATA_OFFSET);
		for(i=0;i<sizeof(chv_file);i++) {
			ioman_write(*(str++));
		}  */
		ioman_write_buffer((cluster_no * CLUSTER_SIZE) + ALLOCATION_DATA_OFFSET, str, sizeof(chv_file));
		status = CHV_PIN_FAILED;
	}
	m_free(cf);
	#if _DMA_DEBUG
	barren_eject(cf);
	#endif
	return status;
}

uchar unblockCHV(uchar chv_no, char *pin, char *puk) _REENTRANT_
{
	#if !USE_SHARED_VAR
	uchar status;
	char * str;
	uint16 i;
	uint16 cluster_no;
	chv_file * cf;
	char buffer[8];
	#endif
	chv_no &= 0x0f;
	//printf(puk)
	//auth_DEShash( buffer, CHV_KEY, pin);
	//memcopy(pin, buffer, 0, 8);
	//auth_DEShash( buffer, CHV_KEY, puk);
	//memcopy(puk, buffer, 0, 8);
	if(chv_no == ACC_ALW) {
		return CHV_DISABLE;
	}
	cluster_no = getCHVcluster(chv_no);
	cf = file_get_chv(cluster_no);
	str = (char *) cf;
	if(cf->status != CHV_BLOCK) {
		status = cf->status;
		m_free(cf);
		#if _DMA_DEBUG
		barren_eject(cf);
		#endif
		return status;
	}
	if(cf->puk_attempts < cf->puk_max_attempts) {
		if(memcompare(cf->puk, puk, 0, 8)==TRUE) {
			cf->pin_attempts = 0;
			cf->puk_attempts = 0;
			if(chv_no == ACC_CHV1) {
				status = CHV_ENABLE;
			} else {
				status = CHV_DISABLE;	//kondisi default semua chv kecuali chv1
			}
			memcopy(cf->pin, pin, 0, 8);	//new pin value
			cf->status = status;
			/*ioman_seek((cluster_no * CLUSTER_SIZE) + ALLOCATION_DATA_OFFSET);
			for(i=0;i<sizeof(chv_file);i++) {
				ioman_write(*(str++));
			}*/
			ioman_write_buffer((cluster_no * CLUSTER_SIZE) + ALLOCATION_DATA_OFFSET, str, sizeof(chv_file));
		} else {
			cf->puk_attempts += 1;
			/*ioman_seek((cluster_no * CLUSTER_SIZE) + ALLOCATION_DATA_OFFSET);
			for(i=0;i<sizeof(chv_file);i++) {
				ioman_write(*(str++));
			}*/
			ioman_write_buffer((cluster_no * CLUSTER_SIZE) + ALLOCATION_DATA_OFFSET, str, sizeof(chv_file));
			status = CHV_PUK_FAILED;
		}
	}
	else
	{
		status = CHV_PUK_FAILED;	//puk is blocked
	}
	m_free(cf);
	#if _DMA_DEBUG
	barren_eject(cf);
	#endif
	return status;
}

uchar changeCHV(uchar chv_no, char *pin1, char *pin2) _REENTRANT_
{
	#if !USE_SHARED_VAR
	uchar status;
	char * str;
	uint16 i;
	uint16 cluster_no;
	chv_file * cf;
	char buffer[8];
	#endif
	chv_no &= 0x0f;
	
	//auth_DEShash( buffer, CHV_KEY, pin1);
	//memcopy(pin1, buffer, 0, 8);
	//auth_DEShash( buffer, CHV_KEY, pin2);
	//memcopy(pin2, buffer, 0, 8);
	if(chv_no == ACC_ALW) {
		return CHV_DISABLE;
	}
	cluster_no = getCHVcluster(chv_no);
	cf = file_get_chv(cluster_no);
	str = (char *) cf;
	if(cf->status == CHV_BLOCK) {
		status = cf->status;
		m_free(cf);
		#if _DMA_DEBUG
		barren_eject(cf);
		#endif
		return status;
	}

	if(memcompare(cf->pin, pin1, 0, 8)==TRUE) {
		cf->pin_attempts = 0;
		status = cf->status;
		memcopy(cf->pin, pin2, 0, 8);
		cf->status = status;
		/*ioman_seek((cluster_no * CLUSTER_SIZE) + ALLOCATION_DATA_OFFSET);
		for(i=0;i<sizeof(chv_file);i++) {
			ioman_write(*(str++));
		}*/
		ioman_write_buffer((cluster_no * CLUSTER_SIZE) + ALLOCATION_DATA_OFFSET, str, sizeof(chv_file));
	} else {
		cf->pin_attempts += 1;
		status = cf->status;
		if(cf->pin_attempts >= cf->pin_max_attempts) {
			status = CHV_BLOCK;
		}
		cf->status = status;
		/*ioman_seek((cluster_no * CLUSTER_SIZE) + ALLOCATION_DATA_OFFSET);
		for(i=0;i<sizeof(chv_file);i++) {
			ioman_write(*(str++));
		}*/
		ioman_write_buffer((cluster_no * CLUSTER_SIZE) + ALLOCATION_DATA_OFFSET, str, sizeof(chv_file));
		status = CHV_PIN_FAILED;
	}
	m_free(cf);
	#if _DMA_DEBUG
	barren_eject(cf);
	#endif
	return status;
}

#endif
#if ASGARD_VERSION == 4
BYTEC chv_padding[8] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};		//padding char for chv
uchar _chv_status[MAX_CHV];		//maximum 6 chv		1~6

void chv_get_config(uchar chv_no, chv_file * buffer)  _REENTRANT_ {
	//chv_file * chv;
	chv_no = chv_no - 1;			//start from 0
	//chv = (chv_file *)m_alloc(sizeof(chv_file)); 
	ioman_read_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), buffer, sizeof(chv_file));
	//return chv;
}

void chv_init(void) _REENTRANT_ {
	register uchar i;
	//chv_file * chv;
	//chv = (chv_file *)m_alloc(sizeof(chv_file));
	uchar * chv_buffer = m_alloc(sizeof(chv_file) * MAX_CHV);
	chv_file * chv;
	chv = (chv_file *)chv_buffer;
	ioman_read_buffer(CHV_DATA_OFFSET + (i * sizeof(chv_file)), chv_buffer, sizeof(chv_file) * MAX_CHV);
	for(i=0;i<MAX_CHV;i++) {
		if(chv->tag == CHV_TAG) {			//valid chv file
			_chv_status[i] = chv->status;	//load any designated chv to the corresponding memory area
		} else {
			_chv_status[i] = CHV_UNINITIALIZED;
		}
		chv += sizeof(chv_file);
	}
	m_free(chv_buffer);
	//m_free(chv);							//free memory
}

void chv_create(uchar chv_no, uchar *pin, uchar *puk, uchar pin_max_retry, uchar puk_max_retry) _REENTRANT_ {
	//chv_file * chv;
	chv_file chv;
	chv_no = chv_no - 1;			//start from 0
	//chv = (chv_file *)m_alloc(sizeof(chv_file));
	chv.tag = CHV_TAG;
	chv.status = 0;	 								//default disabled
	_chv_status[chv_no] = 0;						//set current chv status
	memcpy(chv.pin, chv_padding, 8);				//
	memcpy(chv.pin, pin, strlen(pin) );
	memcpy(chv.puk, chv_padding, 8);				//
	memcpy(chv.puk, puk, strlen(pin) );
	chv.pin_attempts = 0;
	chv.pin_max_attempts = pin_max_retry;
	chv.puk_attempts = 0;
	chv.puk_max_attempts = puk_max_retry;
	ioman_write_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), &chv, sizeof(chv_file));
	//m_free(chv);
}

uint16 chv_verify(uchar chv_no, uchar *pin)  _REENTRANT_ {
	//chv_file chv[sizeof(chv_file)];
	chv_file chv;
	chv_no = chv_no - 1;			//start from 0
	//if(_chv_status[chv_no] & CHV_UNINITIALIZED) { return APDU_WRONG_PARAMETER; }				//this chv doesn't even exist
	//if(_chv_status[chv_no] & CHV_BLOCKED) { return APDU_INVALID_STATUS; }						//this chv is blocked

	//chv = (chv_file *)m_alloc(sizeof(chv_file));
	ioman_read_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), &chv, sizeof(chv_file));
	if(chv.tag != CHV_TAG) { /*m_free(chv);*/ return APDU_WRONG_PARAMETER; }
	if(chv.pin_attempts >= chv.pin_max_attempts) { /*m_free(chv);*/ return APDU_INVALID_STATUS; }				//this chv is blocked
	if(memcmp(chv.pin, pin, 8) == 0) {
		chv.pin_attempts = 0;
		ioman_write_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), &chv, sizeof(chv_file));			//
		_chv_status[chv_no] |= CHV_VERIFIED;																	//verification didn't change chv status on flash
	} else {
		chv.pin_attempts++;  
		if(chv.pin_attempts == (chv.pin_max_attempts - 1)) { 
			ioman_write_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), &chv, sizeof(chv_file));			//
			//m_free(chv);
			return APDU_CHV_LAST_ATTEMPT;
		}
		if(chv.pin_attempts >= chv.pin_max_attempts) { 
			chv.status |= CHV_BLOCKED;
			_chv_status[chv_no] = chv.status;
			ioman_write_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), &chv, sizeof(chv_file));			//
			//m_free(chv);
			return APDU_INVALID_STATUS;
		}
		ioman_write_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), &chv, sizeof(chv_file));			//
		//m_free(chv);
		return APDU_ACCESS_DENIED;
	}
	//m_free(chv);
	return APDU_SUCCESS;
}

uint16 chv_enable(uchar chv_no, uchar *pin) _REENTRANT_ {
	//chv_file chv[sizeof(chv_file)];
	chv_file chv;
	chv_no = chv_no - 1;			//start from 0
	//if(_chv_status[chv_no] & CHV_UNINITIALIZED) { return APDU_WRONG_PARAMETER; }				//this chv doesn't even exist
	//if(_chv_status[chv_no] & CHV_BLOCKED) { return APDU_INVALID_STATUS; }						//this chv is blocked

	//chv = (chv_file *)m_alloc(sizeof(chv_file));
	ioman_read_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), &chv, sizeof(chv_file));
	if(chv.tag != CHV_TAG) { /*m_free(chv);*/ return APDU_WRONG_PARAMETER; }
	if(chv.pin_attempts >= chv.pin_max_attempts) { /*m_free(chv);*/ return APDU_INVALID_STATUS; }				//this chv is blocked
	if(memcmp(chv.pin, pin, 8) == 0) {
		chv.pin_attempts = 0;
		_chv_status[chv_no] |= CHV_ENABLED;	
		_chv_status[chv_no] &= ~CHV_VERIFIED;																//
		chv.status = _chv_status[chv_no];
		ioman_write_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), &chv, sizeof(chv_file));
	} else {
		chv.pin_attempts++;
		if(chv.pin_attempts == (chv.pin_max_attempts - 1)) { 
			ioman_write_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), &chv, sizeof(chv_file));			//
			//m_free(chv);
			return APDU_CHV_LAST_ATTEMPT;
		}
		if(chv.pin_attempts >= chv.pin_max_attempts) { 
			chv.status |= CHV_BLOCKED;
			_chv_status[chv_no] = chv.status;
			ioman_write_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), &chv, sizeof(chv_file));			//
			//m_free(chv);
			return APDU_INVALID_STATUS;
		}
		ioman_write_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), &chv, sizeof(chv_file));			//
		//m_free(chv);
		return APDU_ACCESS_DENIED;
	}
	//m_free(chv);
	return APDU_SUCCESS;
}

uint16 chv_disable(uchar chv_no, uchar *pin) _REENTRANT_ {
	//chv_file chv[sizeof(chv_file)];
	chv_file chv;
	chv_no = chv_no - 1;			//start from 0
	//if(_chv_status[chv_no] & CHV_UNINITIALIZED) { return _chv_status[chv_no]; }				//this chv doesn't even exist
	//if(_chv_status[chv_no] & CHV_BLOCKED) { return _chv_status[chv_no]; }						//this chv is blocked

	//chv = (chv_file *)m_alloc(sizeof(chv_file));
	ioman_read_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), &chv, sizeof(chv_file));
	if(chv.tag != CHV_TAG) { /*m_free(chv);*/ return APDU_WRONG_PARAMETER; }
	if(chv.pin_attempts >= chv.pin_max_attempts) { /*m_free(chv);*/ return APDU_INVALID_STATUS; }				//this chv is blocked
	if(memcmp(chv.pin, pin, 8) == 0) {
		chv.pin_attempts = 0;
		_chv_status[chv_no] &= ~CHV_ENABLED;																	//
		chv.status = _chv_status[chv_no];
		ioman_write_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), &chv, sizeof(chv_file));
	} else {
		chv.pin_attempts++;  
		if(chv.pin_attempts == (chv.pin_max_attempts - 1)) { 
			ioman_write_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), &chv, sizeof(chv_file));			//
			//m_free(chv);
			return APDU_CHV_LAST_ATTEMPT;
		}
		if(chv.pin_attempts >= chv.pin_max_attempts) { 
			chv.status |= CHV_BLOCKED;
			_chv_status[chv_no] = chv.status;
			ioman_write_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), &chv, sizeof(chv_file));			//
			//m_free(chv);
			return APDU_INVALID_STATUS;
		}
		ioman_write_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), &chv, sizeof(chv_file));			//
		//m_free(chv);
		return APDU_ACCESS_DENIED;
	}
	//m_free(chv);
	return APDU_SUCCESS;
}

uint16 chv_unblock(uchar chv_no, uchar *pin, uchar *puk) _REENTRANT_ {
	//chv_file chv[sizeof(chv_file)];
	chv_file chv;
	chv_no = chv_no - 1;			//start from 0
	//if(_chv_status[chv_no] & CHV_UNINITIALIZED) { return _chv_status[chv_no]; }				//this chv doesn't even exist
	//if((_chv_status[chv_no] & CHV_BLOCKED) == 0) { return _chv_status[chv_no]; }			//this chv must be blocked

	//chv = (chv_file *)m_alloc(sizeof(chv_file));
	ioman_read_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), &chv, sizeof(chv_file));
	if(chv.tag != CHV_TAG) { /*m_free(chv);*/ return APDU_WRONG_PARAMETER; }
	//if(chv.status & CHV_UNINITIALIZED) { /*m_free(chv);*/ return APDU_WRONG_PARAMETER; }				//this chv doesn't even exist
	if(chv.puk_attempts >= chv.puk_max_attempts) { /*m_free(chv);*/ return APDU_INVALID_STATUS; }				//this chv is blocked
	if(memcmp(chv.puk, puk, 8) == 0) {
		chv.puk_attempts = 0;																	//
		chv.status &= ~CHV_BLOCKED; 
		_chv_status[chv_no] = chv.status;
		memcpy(chv.pin, pin, 8 );
		ioman_write_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), &chv, sizeof(chv_file));
	} else {
		chv.puk_attempts++;
		if(chv.puk_attempts == (chv.puk_max_attempts - 1)) { 
			ioman_write_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), &chv, sizeof(chv_file));			//
			//m_free(chv);
			return APDU_CHV_LAST_ATTEMPT;
		}
		ioman_write_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), &chv, sizeof(chv_file));
		return APDU_ACCESS_DENIED;
	}
	//m_free(chv);
	return APDU_SUCCESS;
}

uint16 chv_change(uchar chv_no, uchar *old, uchar *new) _REENTRANT_ {
	//chv_file chv[sizeof(chv_file)];
	chv_file chv;
	chv_no = chv_no - 1;			//start from 0
	//if(_chv_status[chv_no] & CHV_UNINITIALIZED) { return _chv_status[chv_no]; }				//this chv doesn't even exist
	//if(_chv_status[chv_no] & CHV_BLOCKED) { return _chv_status[chv_no]; }						//this chv is blocked

	//chv = (chv_file *)m_alloc(sizeof(chv_file));
	ioman_read_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), &chv, sizeof(chv_file));
	if(chv.tag != CHV_TAG) { /*m_free(chv);*/ return APDU_WRONG_PARAMETER; }
	if(chv.pin_attempts >= chv.pin_max_attempts) { /*m_free(chv);*/ return APDU_INVALID_STATUS; }				//this chv is blocked
	if(memcmp(chv.pin, old, 8) == 0) {
		chv.pin_attempts = 0;																	//
		chv.status = _chv_status[chv_no];
		memcpy(chv.pin, new, 8 );
		ioman_write_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), &chv, sizeof(chv_file));
	} else {
		chv.pin_attempts++; 
		if(chv.pin_attempts == (chv.pin_max_attempts - 1)) { 
			ioman_write_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), &chv, sizeof(chv_file));			//
			//m_free(chv);
			return APDU_CHV_LAST_ATTEMPT;
		}
		if(chv.pin_attempts >= chv.pin_max_attempts) { 
			chv.status |= CHV_BLOCKED;
			_chv_status[chv_no] = chv.status;
			ioman_write_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), &chv, sizeof(chv_file));			//
			///m_free(chv);
			return APDU_INVALID_STATUS;
		}
		ioman_write_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), &chv, sizeof(chv_file));			//
		//m_free(chv);
		return APDU_ACCESS_DENIED;
	}
	//m_free(chv);
	return APDU_SUCCESS;
}

uchar chv_get_status(uchar chv_no) _REENTRANT_ {
	chv_no = chv_no - 1;			//start from 0
	if(chv_no < MAX_CHV) {
		return _chv_status[chv_no];
	} else {
	 	return CHV_UNINITIALIZED;
	}
}

void chv_set_status(uchar chv_no, uchar status) {
	chv_no = chv_no - 1;			//start from 0 
	if(chv_no < MAX_CHV) {
		_chv_status[chv_no] = status;
	}
}
#endif
