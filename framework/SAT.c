//#include "..\..\defs.h"
//#include "allocator\allocator.h"
#include "..\liquid.h"
#include "..\config.h"
#include "..\midgard\midgard.h"
#include "..\asgard\file.h"
#include <string.h>
#include <stdarg.h>

//fs_handle stk_fs _at_ 0x5f8;

uchar fetch_len = 0;

#if SAT_USE_TEMP_FILE == 0
uchar * _sat_buffer_in;
uchar _sat_buffer[256];
//uchar * _sat_temp = _sat_buffer;
#else 
fs_handle _sat_fs;
#endif 

//fs_handle stk_fs;

uchar SAT_push(uchar * buffer, uchar tag, uchar length, uchar * value) _REENTRANT_ {
	uchar i = 0, j;
	buffer[i++] = tag;
	if(length > 0x7F) {
	 	buffer[i++] = 0x81;
		buffer[i++] = length;
	} else {
		buffer[i++] = length;
	}
	for(j=0;j<length;j++) {
		buffer[i++] = value[j];
	}
	return i;	
}

uchar SAT_pop(uchar * buffer, uchar * tag, uchar * size, uchar * value) _REENTRANT_ {
	uchar i = 0, j;
	tag[0] = buffer[i++];
	if(buffer[i] == 0x81) {
		i++;
	}
	size[0] = buffer[i++];
	for(j=0;j<size[0];j++) {
		value[j] = buffer[i++];
	}
	return i;
}

uchar SAT_command(uchar * buffer, uchar number, uchar type, uchar qualifier) _REENTRANT_ { 
	buffer[2] = number;
	buffer[3] = type;
	buffer[4] = qualifier;
	return SAT_push(buffer, STK_TAG_CMD_DETAIL, 3, buffer+2);	
}

uchar SAT_device(uchar * buffer, uchar src, uchar dst) _REENTRANT_ {
	buffer[2] = src;
	buffer[3] = dst;
	return SAT_push(buffer, STK_TAG_DEV_ID, 2, buffer+2);	
}

uchar SAT_init(uchar * buffer, uint16 size) _REENTRANT_ {
	#if SAT_USE_TEMP_FILE
	_select(&_sat_fs, FID_MF);
	_select(&_sat_fs, FID_LIQUID);
	if(_select(&_sat_fs, FID_SAT) < 0x9F00) return 0;
	if(size == 0) return 1;
	if(_writebin(&_sat_fs, 0, buffer, size) == APDU_SUCCESS) return 1; 
	return 0;
	#else
	//if(_sat_temp != NULL) { m_free(_sat_temp); _sat_temp = NULL; }
	//_sat_temp = m_alloc(SAT_MAX_RESPONSE_SIZE);
	//if(_sat_temp != NULL) return 0;				//SAT overload
	//if(_sat_temp == NULL) _sat_temp = m_alloc(SAT_MAX_RESPONSE_SIZE);
	//_sat_temp = _sat_buffer;
	//memcpy((uchar *)_sat_buffer, buffer, size); 
	_sat_buffer_in = buffer;
	return 1;
	#endif
}

uchar SAT_file_pop(uint16 i, uchar * tag, uchar * size, uchar * value) _REENTRANT_ {
	#if SAT_USE_TEMP_FILE
	uchar j = 0;
	_readbin(&_sat_fs, i + j, tag, 1); j++;
	_readbin(&_sat_fs, i + j, size, 1); j++;
	if(size[0] == 0x81) {		  
		_readbin(&_sat_fs, i + j, size, 1); j++;
	}
	_readbin(&_sat_fs, i + j, value, size[0]);
	return (j + size[0]);
	#else
	/*if(_sat_temp != NULL) return */ return SAT_pop(_sat_buffer_in + i, tag, size, value);
	return 0;
	#endif
}  

uchar SAT_read(uchar * buffer, uint16 size) _REENTRANT_ {			//fetch purpose
	#if SAT_USE_TEMP_FILE
	_select(&_sat_fs, FID_MF);
	_select(&_sat_fs, FID_LIQUID);
	if(_select(&_sat_fs, FID_SAT) < 0x9F00) return 0;
	if(_readbin(&_sat_fs, 0, buffer, size) == APDU_SUCCESS) return 1;
	#else
	//if(_sat_temp != NULL) { memcpy(buffer, _sat_temp, size); return 1; }
	memcpy(buffer, (uchar *)_sat_buffer, size);
	#endif
	return 0;
}

void SAT_cleanup(void) _REENTRANT_ {
	#if SAT_USE_TEMP_FILE == 0
	//if(_sat_temp != NULL) { m_free(_sat_temp); _sat_temp = NULL; }
	#endif
	fetch_len = 0;
}

uint16 SAT_status(void) _REENTRANT_ {
	return (fetch_len)?(APDU_STK_RESPONSE | fetch_len):APDU_SUCCESS;
}

uint16 SAT_response(uchar * buffer, uchar len) _REENTRANT_ {
	fetch_len = SAT_file_push(0, FETCH_TAG_PROSIM, len, buffer);
	return SAT_status(); 
}

uint16 SAT_file_flush(uchar length) _REENTRANT_ {	
	SAT_read(STK_buffer, length);
	return SAT_response(STK_buffer, length);
}

uchar SAT_file_push(uint16 i, uchar tag, uchar length, uchar * value) _REENTRANT_ {
	#if SAT_USE_TEMP_FILE
	uchar j = 0;
	//uchar * buffer = (uchar *)m_alloc(length + 3);
	uchar buffer[3];
	_select(&_sat_fs, FID_MF);
	_select(&_sat_fs, FID_LIQUID);
	if(_select(&_sat_fs, FID_SAT) < 0x9F00) return 0;
	buffer[j++] = tag;
	if(length > 0x7F) {
	 	buffer[j++] = 0x81;
	}
	buffer[j++] = length;
	//memcpy(buffer + j, value, length);
	_writebin(&_sat_fs, i, buffer, j);
	_writebin(&_sat_fs, i + j, value, length);
	//m_free(buffer);
	return (j + length);
	#else
	/*if(_sat_temp != NULL)*/ 
	return SAT_push(_sat_buffer + i, tag, length, value);
	//return 0;
	#endif	
}

#if 0
uint16 SAT_file_write(uchar * fmt, ...) _REENTRANT_ {
#else
uint16 SAT_printf(uchar * fmt, ...) _REENTRANT_ {
#endif
	va_list arg_ptr;
	uchar len;
	uint16 i = 0;
	uchar j;
	uchar * ptr;
	uchar buffer[5];
	va_start(arg_ptr, fmt);
	len = strlen(fmt);
	//allocate new temporary response for SAT
	//if(_sat_temp != NULL) { m_free(_sat_temp); _sat_temp = NULL; }
	//_sat_temp = m_alloc(SAT_MAX_RESPONSE_SIZE);
	#if SAT_USE_TEMP_FILE == 0
	//_sat_temp = _sat_buffer;
	//if(_sat_temp == NULL) _sat_temp = m_alloc(SAT_MAX_RESPONSE_SIZE);
	#endif

	for(j=0;j<len;j++) {
		switch(fmt[j]) {
			case 'c':		//COMMAND TAG
			case 'C':
				buffer[0] = 0;								//command number
				buffer[1] = va_arg(arg_ptr, uchar);			//proactive sim command
				buffer[2] = 0;						   		//command qualifier
				i += SAT_file_push(i, STK_TAG_CMD_DETAIL, 3, buffer);
				break;	
			case 'd':		 //DEVICE TAG
			case 'D':
				buffer[0] = 0x81;
				buffer[1] = va_arg(arg_ptr, uchar);
				i += SAT_file_push(i, STK_TAG_DEV_ID, 2, buffer);
				break; 
			case 'a':		 //ADDRESS TAG
			case 'A':
				ptr = va_arg(arg_ptr, uchar *);
				i += SAT_file_push(i, STK_TAG_ADDRESS & 0x7F, ptr[0], ptr + 1);
				break;
			case 'l':
			case 'L':
				ptr = va_arg(arg_ptr, uchar *);
				i += SAT_file_push(i, STK_TAG_ALPHA & 0x7F, ptr[0], ptr + 1);
				break;
			case 'm':		 //MESSAGE TAG (SMS-TPDU)
			case 'M':
				ptr = va_arg(arg_ptr, uchar *);
				i += SAT_file_push(i, STK_TAG_SMS_TPDU & 0x7F, ptr[0], ptr + 1);
				break;
			case 't':
			case 'T':
				buffer[0] = va_arg(arg_ptr, uchar);
				i += SAT_file_push(i, STK_TAG_TIMER_IDENTIFIER & 0x7F, 1, buffer);
				break;
			case 'u':
				ptr = va_arg(arg_ptr, uchar *);
				i += SAT_file_push(i, STK_TAG_USSD_STRING & 0x7F, ptr[0], ptr + 1);
				break;
			case 'v':
			case 'V':
				ptr = va_arg(arg_ptr, uchar *);
				i += SAT_file_push(i, STK_TAG_TIMER_VALUE & 0x7F, ptr[0], ptr + 1);
				break;
			case 'x':
				ptr = va_arg(arg_ptr, uchar *);
				i += SAT_file_push(i, STK_TAG_TEXT_STRING & 0x7F, ptr[0], ptr + 1);
				break;
		}
	} 
	va_end(arg_ptr);
	SAT_read(STK_buffer, i);
	return SAT_response(STK_buffer, i);
}