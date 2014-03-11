#include "security.h"
#include "defs.h"
#include "..\drivers\ioman.h"
#include "fs.h"
//#include "..\auth\des.h"
//#include "..\yggdrasil.h"
//#include "..\misc\barrenkalaheapcalc.h"
#include "..\midgard\midgard.h"
#include "..\misc\mem.h"
#include <string.h>

//#if ASGARD_VERSION == 4
char chv_padding[8] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};		//padding char for chv
uchar _chv_status[MAX_CHV];		//maximum 6 chv		1~6

void chv_init(void) {
	register uchar i;
	chv_file * chv;
	chv = (chv_file *)m_alloc(sizeof(chv_file));
	for(i=0;i<MAX_CHV;i++) {
		ioman_read_buffer(CHV_DATA_OFFSET + (i * sizeof(chv_file)), chv, sizeof(chv_file));
		if(chv->tag == CHV_TAG) {			//valid chv file
			_chv_status[i] = chv->status;	//load any designated chv to the corresponding memory area
		} else {
			_chv_status[i] = CHV_UNINITIALIZED;
		}
	}
	m_free(chv);							//free memory
}

void chv_create(uchar chv_no, uchar *pin, uchar *puk, uchar pin_max_retry, uchar puk_max_retry) {
	chv_file * chv;
	chv_no = chv_no - 1;			//start from 0
	chv = (chv_file *)m_alloc(sizeof(chv_file));
	chv->tag = CHV_TAG;
	chv->status = 0;
	memcopy(chv->pin, chv_padding, 0, 8);				//
	memcopy(chv->pin, pin, 0, strlen(pin) );
	memcopy(chv->puk, chv_padding, 0, 8);				//
	memcopy(chv->puk, puk, 0, strlen(pin) );
	chv->pin_attempts = 0;
	chv->pin_max_attempts = pin_max_retry;
	chv->puk_attempts = 0;
	chv->puk_max_attempts = puk_max_retry;
	ioman_write_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), chv, sizeof(chv_file));
	m_free(chv);
}

uchar chv_verify(uchar chv_no, uchar *pin) {
	chv_file * chv;
	chv_no = chv_no - 1;			//start from 0
	if(_chv_status[chv_no] & CHV_UNINITIALIZED) { return _chv_status[chv_no]; }				//this chv doesn't even exist
	if(_chv_status[chv_no] & CHV_BLOCKED) { return _chv_status[chv_no]; }						//this chv is blocked

	chv = (chv_file *)m_alloc(sizeof(chv_file));
	ioman_read_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), chv, sizeof(chv_file));
	if(chv->tag != CHV_TAG) { m_free(chv); return CHV_UNINITIALIZED; }
	if(chv->pin_attempts >= chv->pin_max_attempts) { m_free(chv); return _chv_status[chv_no]; }				//this chv is blocked
	if(memcompare(chv->pin, pin, 0, 8) == TRUE) {
		chv->pin_attempts = 0;
		ioman_write_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), chv, sizeof(chv_file));			//
		_chv_status[chv_no] = CHV_VERIFIED;																	//verification didn't change chv status on flash
	} else {
		chv->pin_attempts++;
		if(chv->pin_attempts >= chv->pin_max_attempts) { 
			chv->status |= CHV_BLOCKED;
			_chv_status[chv_no] = chv->status;
			ioman_write_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), chv, sizeof(chv_file));			//
			m_free(chv);
			return _chv_status[chv_no];
		}
		ioman_write_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), chv, sizeof(chv_file));			//
		m_free(chv);
		return CHV_ENABLED;
	}
	m_free(chv);
	return _chv_status[chv_no];
}

uchar chv_enable(uchar chv_no, uchar *pin) {
	chv_file * chv;
	chv_no = chv_no - 1;			//start from 0
	if(_chv_status[chv_no] & CHV_UNINITIALIZED) { return _chv_status[chv_no]; }				//this chv doesn't even exist
	if(_chv_status[chv_no] & CHV_BLOCKED) { return _chv_status[chv_no]; }						//this chv is blocked

	chv = (chv_file *)m_alloc(sizeof(chv_file));
	ioman_read_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), chv, sizeof(chv_file));
	if(chv->tag != CHV_TAG) { m_free(chv); return CHV_UNINITIALIZED; }
	if(chv->pin_attempts >= chv->pin_max_attempts) { m_free(chv); return _chv_status[chv_no]; }				//this chv is blocked
	if(memcompare(chv->pin, pin, 0, 8) == TRUE) {
		chv->pin_attempts = 0;
		_chv_status[chv_no] |= CHV_ENABLED;																	//
		chv->status = _chv_status[chv_no];
		ioman_write_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), chv, sizeof(chv_file));
	} else {
		chv->pin_attempts++;
		if(chv->pin_attempts >= chv->pin_max_attempts) { 
			chv->status |= CHV_BLOCKED;
			_chv_status[chv_no] = chv->status;
			ioman_write_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), chv, sizeof(chv_file));			//
			m_free(chv);
			return _chv_status[chv_no];
		}
		ioman_write_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), chv, sizeof(chv_file));			//
		m_free(chv);
		return CHV_ENABLED;
	}
	m_free(chv);
	return _chv_status[chv_no];
}

uchar chv_disable(uchar chv_no, uchar *pin) {
	chv_file * chv;
	chv_no = chv_no - 1;			//start from 0
	if(_chv_status[chv_no] & CHV_UNINITIALIZED) { return _chv_status[chv_no]; }				//this chv doesn't even exist
	if(_chv_status[chv_no] & CHV_BLOCKED) { return _chv_status[chv_no]; }						//this chv is blocked

	chv = (chv_file *)m_alloc(sizeof(chv_file));
	ioman_read_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), chv, sizeof(chv_file));
	if(chv->tag != CHV_TAG) { m_free(chv); return CHV_UNINITIALIZED; }
	if(chv->pin_attempts >= chv->pin_max_attempts) { m_free(chv); return _chv_status[chv_no]; }				//this chv is blocked
	if(memcompare(chv->pin, pin, 0, 8) == TRUE) {
		chv->pin_attempts = 0;
		_chv_status[chv_no] &= ~CHV_ENABLED;																	//
		chv->status = _chv_status[chv_no];
		ioman_write_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), chv, sizeof(chv_file));
	} else {
		chv->pin_attempts++;
		if(chv->pin_attempts >= chv->pin_max_attempts) { 
			chv->status |= CHV_BLOCKED;
			_chv_status[chv_no] = chv->status;
			ioman_write_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), chv, sizeof(chv_file));			//
			m_free(chv);
			return _chv_status[chv_no];
		}
		ioman_write_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), chv, sizeof(chv_file));			//
		m_free(chv);
		return CHV_ENABLED;
	}
	m_free(chv);
	return _chv_status[chv_no];
}

uchar chv_unblock(uchar chv_no, uchar *pin, uchar *puk) {
	chv_file * chv;
	chv_no = chv_no - 1;			//start from 0
	if(_chv_status[chv_no] & CHV_UNINITIALIZED) { return _chv_status[chv_no]; }				//this chv doesn't even exist
	if((_chv_status[chv_no] & CHV_BLOCKED) == 0) { return _chv_status[chv_no]; }			//this chv must be blocked

	chv = (chv_file *)m_alloc(sizeof(chv_file));
	ioman_read_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), chv, sizeof(chv_file));
	if(chv->tag != CHV_TAG) { m_free(chv); return CHV_UNINITIALIZED; }
	if(_chv_status[chv_no] & CHV_UNINITIALIZED) { m_free(chv); return _chv_status[chv_no]; }				//this chv doesn't even exist
	if(chv->puk_attempts >= chv->puk_max_attempts) { m_free(chv); return _chv_status[chv_no]; }				//this chv is blocked
	if(memcompare(chv->puk, puk, 0, 8) == TRUE) {
		chv->puk_attempts = 0;
		_chv_status[chv_no] &= ~CHV_BLOCKED;																	//
		chv->status = _chv_status[chv_no];
		memcopy(chv->pin, pin, 0, 8 );
		ioman_write_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), chv, sizeof(chv_file));
	} else {
		chv->puk_attempts++;
		ioman_write_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), chv, sizeof(chv_file));
	}
	m_free(chv);
	return _chv_status[chv_no];
}

uchar chv_change(uchar chv_no, uchar *old, uchar *new) {
	chv_file * chv;
	chv_no = chv_no - 1;			//start from 0
	if(_chv_status[chv_no] & CHV_UNINITIALIZED) { return _chv_status[chv_no]; }				//this chv doesn't even exist
	if(_chv_status[chv_no] & CHV_BLOCKED) { return _chv_status[chv_no]; }						//this chv is blocked

	chv = (chv_file *)m_alloc(sizeof(chv_file));
	ioman_read_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), chv, sizeof(chv_file));
	if(chv->tag != CHV_TAG) { m_free(chv); return CHV_UNINITIALIZED; }
	if(chv->pin_attempts >= chv->pin_max_attempts) { m_free(chv); return _chv_status[chv_no]; }				//this chv is blocked
	if(memcompare(chv->pin, old, 0, 8) == TRUE) {
		chv->pin_attempts = 0;																	//
		chv->status = _chv_status[chv_no];
		memcopy(chv->pin, new, 0, 8 );
		ioman_write_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), chv, sizeof(chv_file));
	} else {
		chv->pin_attempts++;
		if(chv->pin_attempts >= chv->pin_max_attempts) { 
			chv->status |= CHV_BLOCKED;
			_chv_status[chv_no] = chv->status;
			ioman_write_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), chv, sizeof(chv_file));			//
			m_free(chv);
			return _chv_status[chv_no];
		}
		ioman_write_buffer(CHV_DATA_OFFSET + (chv_no * sizeof(chv_file)), chv, sizeof(chv_file));			//
		m_free(chv);
		return CHV_ENABLED;
	}
	m_free(chv);
	return _chv_status[chv_no];
}

uchar chv_get_status(uchar chv_no) {
	chv_no = chv_no - 1;			//start from 0
	return _chv_status[chv_no];
}
//#endif
