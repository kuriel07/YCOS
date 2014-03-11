//#include "..\..\defs.h"
#include "..\midgard\midgard.h"
#include "..\asgard\file.h"
#include "..\liquid.h"
//#include "SAT.h"
#include "VAS.h"
#include "SMS.h"
#include <string.h>

//#define FID_WIB				0x2700
//#define FID_WTAR				0x6F1A
//#define FID_WERRORTEXT		0x6F02
//#define FID_WBYTECODE			0x6F03
//#define FID_WSMSHEADER		0x6F04
//#define FID_WSC				0x6F1B
//#define FID_W0348CNTR			0x6F06
//#define FID_WVERSION			0x6F07
//#define FID_WIBCONF			0x6F08
//#define FID_WEVTCONF			0x6F0B
//#define FID_WTEXT				0x6F1C
//#define FID_WMENU				0x6F18
//#define FID_WSCRADDR			0x6F1D
//#define FID_WMENUTITLE		0x6F1E

#if VAS_ALLOCATED
extern uchar STK_buffer[];

//load default menu
uint16 VAS_menu(void) _REENTRANT_ {
	uint16 status = APDU_SUCCESS;
	uchar j = 0, k = 0;
	uchar i = 0;
	uchar len;
	uchar total_rec = 0;
	fs_handle temp_fs;
	ef_header * file;
	//select EFmenutitle
	_select(&temp_fs, FID_MF);
	_select(&temp_fs, FID_WIB);	
	if(_select(&temp_fs, FID_WMENUTITLE) < 0x9F00) goto exit_menu;		//EFsc not found
	j += VAS_push_header(j, SET_UP_MENU, 0, STK_DEV_ME);
	file = (ef_header *)file_get_current_header(&temp_fs);
	_readrec(&temp_fs, 0, STK_buffer, file->rec_size);			//read record #1 (EFmenutitle) 
	m_free(file);
	if(STK_buffer[2] == 0xFF) goto exit_menu;					//check menutitle
 	len = STK_buffer[2];
	j += SAT_file_push(j, (STK_TAG_ALPHA & 0x7F), len, STK_buffer + 3);		//menutitle
	//select EFmenu
	_select(&temp_fs, FID_WIB);	
	if(_select(&temp_fs, FID_WMENU) < 0x9F00) goto exit_menu;		//EFsc not found
	file = (ef_header *)file_get_current_header(&temp_fs);
	total_rec = (file->size / file->rec_size);
	k = 0;
	_readrec(&temp_fs, k++, STK_buffer, file->rec_size);		//first record (ignore)
	while(k < total_rec) {
		_readrec(&temp_fs, k++, STK_buffer, file->rec_size);
		if(STK_buffer[0] == 0x02) {						//item status (02 = show, 00 = hidden)
			len = STK_buffer[5];						//item menutext length
			memcpy(STK_buffer+2, STK_buffer+6, len);	//[menu identifier->1][menutext->N], slide menutext to menu ordinal
			j += SAT_file_push(j, (STK_TAG_ITEM & 0x7F), len+1, STK_buffer+1);		//menutitle
		}	
	}
	//vas_state = VAS_STATE_MENU;							//change VAS state to menu
	//status = APDU_STK_RESPONSE | j;
	status = SAT_file_flush(j);
	m_free(file);
	exit_menu:
	return status;
}

//set up event list
//extern uchar sat_init_state;
uint16 VAS_event(void) _REENTRANT_ {
	uint16 status = APDU_SUCCESS;
	uchar j = 0, k = 0;
	uchar i = 0;
	uchar len;
	uchar total_rec = 0;
	fs_handle temp_fs;
	//ef_header * file;
	//select EFmenutitle
	_select(&temp_fs, FID_MF);
	_select(&temp_fs, FID_WIB);	
	if(_select(&temp_fs, FID_WEVTCONF) < 0x9F00) goto exit_event;		//EFsc not found
	j += VAS_push_header(j, SET_UP_EVENT_LIST, 0, STK_DEV_ME);
	k = 3;
	i = 0;
	while(1) {
		if(_readbin(&temp_fs, i, STK_buffer, 3) != APDU_SUCCESS) goto flush_event;
		i += 3;
		switch(STK_buffer[0]) {
			case EVENT_MT_CALL:
			case EVENT_CALL_CONNECTED:
			case EVENT_CALL_DISCONNECTED:
			case EVENT_LOCATION_STATUS:
			case EVENT_USER_ACTIVITY:
			case EVENT_IDLE_SCREEN_AVAIL:
			case EVENT_CARD_READER_STAT:
			case EVENT_LANGUAGE_SELECT:
			case EVENT_BROWSER_TERMINATION:
			case EVENT_DATA_AVAILABLE:
			case EVENT_CHANNEL_STATUS:
				STK_buffer[k++] = STK_buffer[0];
				break;
			case VAS_STARTUP_EVENT:
				//sat_init_state = SAT_STARTUP_INIT;
				break;
			default:
				if(i == 0) goto exit_event;
				goto flush_event;
		}
	 	len = STK_buffer[2];
		i += len;
	}
	flush_event:
	j += SAT_file_push(j, (STK_TAG_EVENT_LIST & 0x7F), k - 3, STK_buffer + 3);
	//vas_state = VAS_STATE_MENU;							//change VAS state to menu
	//status = APDU_STK_RESPONSE | j;
	status = SAT_file_flush(j);
	//m_free(file);
	exit_event:
	return status;
}

//set up and execute startup script
uint16 VAS_startup(void) _REENTRANT_ {
	uint16 status = APDU_SUCCESS;
	if(VAS_invoke(VAS_SELECT_EVENT, VAS_STARTUP_EVENT) == TRUE) {
		status = VAS_decode();
	}
	return status;
}

uchar VAS_invoke(uchar mode, uchar id) _REENTRANT_ {
	fs_handle temp_fs;
	ef_header * file;
	uchar rec_size = 0;
	uchar sid = 0, k;							//script identifier, selalu mulai dari 1
	//uchar ord = 0;
	uint16 offset = 2, length;
	uchar fmask = 0;
	_select(&temp_fs, FID_MF);
	_select(&temp_fs, FID_WIB);
	if(mode == VAS_SELECT_MENU) {
		if(_select(&temp_fs, FID_WMENU) <= 0x9F00) return FALSE;
		file = (ef_header *)file_get_current_header(&temp_fs);
		rec_size = file->rec_size; 				//record size
		m_free(file);							//freeup memory
		if(_readrec(&temp_fs, id, STK_buffer, rec_size) != APDU_SUCCESS) return FALSE;
		if(STK_buffer[0] == 0x02) {						//item status (02 = show, 00 = hidden)
			sid = STK_buffer[1];						//item menutext length
			//ord = STK_buffer[2];						//item ordinal (relative)
		}
		fmask = 0x02;			//break;
	} else if(mode == VAS_SELECT_EVENT) {
		if(_select(&temp_fs, FID_WEVTCONF) < 0x9F00) return FALSE;		//EFsc not found
		k = TRUE;
		offset = 0;
		while(k) {
			if(_readbin(&temp_fs, offset, STK_buffer, 3) != APDU_SUCCESS) return FALSE;
			offset += 3;
			switch(STK_buffer[0]) {
				case EVENT_MT_CALL:
				case EVENT_CALL_CONNECTED:
				case EVENT_CALL_DISCONNECTED:
				case EVENT_LOCATION_STATUS:
				case EVENT_USER_ACTIVITY:
				case EVENT_IDLE_SCREEN_AVAIL:
				case EVENT_CARD_READER_STAT:
				case EVENT_LANGUAGE_SELECT:
				case EVENT_BROWSER_TERMINATION:
				case EVENT_DATA_AVAILABLE:
				case EVENT_CHANNEL_STATUS:	
				case VAS_STARTUP_EVENT:
					if(STK_buffer[0] == id) {
						sid = STK_buffer[1];
						k = FALSE;
					}
					break;
				default:
					return FALSE;
			}
 			//len = STK_buffer[2];
			offset += STK_buffer[2];
		}		//break;//default: return FALSE;
	} else {
		return FALSE;
	}
	offset = 2;
	if(sid != 0) {			   	  				//menu script
		_select(&temp_fs, FID_WIB);
		if(_select(&temp_fs, FID_WSCRADDR) <= 0x9F00) return FALSE;
		_readbin(&temp_fs, 0, STK_buffer, 2);
		if(STK_buffer[0] & 0x02) {		//ABSOLUTE ADDRESSING
			offset += (6 * (sid - 1)); 		//asumsi "Length of script address" selalu 2 bytes
			_readbin(&temp_fs, offset, STK_buffer + 2, 6);
			offset = *((uint16 *)(STK_buffer + 2 + 4));
		} else {						//RELATIVE ADDRESSING
			offset = 0;
			_select(&temp_fs, FID_WIB);
			if(_select(&temp_fs, FID_WBYTECODE) <= 0x9F00) return FALSE;
			length = 0;
			for(k=1;k<sid;k++) { 
				_readbin(&temp_fs, offset, &length, 2);
				offset += (length + 2);
			}
		}
		//got the script address of EFbytecode
		return VAS_plugin_init(offset);
	}
	return FALSE;
	//VAS_SELECT_MENU
}

uchar VAS_set_tar(uchar * tar) _REENTRANT_ {
	uchar temp_fs;
	TARconfig tar_config;
	uchar i = 0;
	//temp_fs = m_alloc(sizeof(fs_handle));
	//if(temp_fs == NULL) return FALSE;
	_select(&temp_fs, FID_MF);
	_select(&temp_fs, FID_WIB);
	if(_select(&temp_fs, FID_WTAR) < 0x9F00) goto exit_get_config; 		//EFtar not found
	//status = file_readrec(&_liquid_fs, i++, &tar_config, 5);
	while(_readrec(&temp_fs, i++, &tar_config, 5) == APDU_SUCCESS) {
		if(memcmp(tar_config.tar, tar, 3) == 0) {
			vas_tar_mode = tar_config.type;
			return TRUE;	 	
		}
		//status = file_readrec(&_liquid_fs, i++, &tar_config, 5);
	}
	exit_get_config:
	return FALSE;
}

uchar VAS_get_security_config(uchar * tar, uchar mode, uchar index, uchar * buffer_out) _REENTRANT_ {
	uchar * temp_fs;
	TARconfig tar_config;
	uint16 i = 0;
	uchar j = 0, k = 0;
	uchar buffer[2];
	uchar status = FALSE;
	temp_fs = m_alloc(sizeof(fs_handle));
	if(temp_fs == NULL) return FALSE;
	_select(temp_fs, FID_MF);
	_select(temp_fs, FID_WIB);
	if(_select(temp_fs, FID_WTAR) < 0x9F00) goto exit_get_config; 		//EFtar not found
	//status = file_readrec(&_liquid_fs, i++, &tar_config, 5);
	while(_readrec(temp_fs, i++, &tar_config, 5) == APDU_SUCCESS) {
		if(memcmp(tar_config.tar, tar, 3) == 0) goto start_get_config;
		//status = file_readrec(&_liquid_fs, i++, &tar_config, 5);
	}
	goto exit_get_config;
	start_get_config:		//TAR checking success
	//#define OFFSCINDEX		4;
	_select(temp_fs, FID_WIB);
	if(_select(temp_fs, FID_WSC) < 0x9F00) goto exit_get_config;		//EFsc not found
	////////////EF security configuration mechanism////////////////////////////	
	i = 0;		 													//i = offset for current SC
	for(j = 0; j != (tar_config.sc_index - 1); j++) {				//get TARsc index , j = index for current SC
		_readbin(temp_fs, i, buffer, 2);
		if(buffer[0] != 0x30) goto exit_get_config;			//invalid security configuration tag
		i += buffer[1];			//actual SC length
	}
	_readbin(temp_fs, i, buffer, 2);					//i = offset of current SC on TAR
	if(buffer[0] != 0x30) goto exit_get_config;					//invalid security configuration tag
	switch(mode) {
		case VAS_SC_OCNTR:
			k = 0;
			goto cntr_load;
		case VAS_SC_ICNTR: 
			k = 1;
			cntr_load:
			_readbin(temp_fs, i + 4, buffer, 2);					//i = offset of current SC on TAR
			_select(temp_fs, FID_WIB);
			if(_select(temp_fs, FID_W0348CNTR) < 0x9F00) goto exit_get_config; 		//EFtar not found
			_readrec(temp_fs, buffer[k], buffer_out, 5);
			break;
		case VAS_SC_OSPI:
			_readbin(temp_fs, i + 8, buffer_out, 4);
			break;
		case VAS_SC_ISPI:
		case VAS_SC_KID:
		case VAS_SC_KIC:
			i += 12;
			while(_readbin(temp_fs, i, buffer, 2) == APDU_SUCCESS) {
				i += 2;
				switch(buffer[0]) {
					case 0x82:		//SPI+KIc+KID list
						if(mode != VAS_SC_ISPI) break;
						k = index * 4;
						if(k >= buffer[1] || buffer[1] == 0) goto exit_get_config;
						_readbin(temp_fs, i + k, buffer_out, 4);
						status = TRUE;
						goto exit_get_config;
					case 0x83:		//KIc list 
						if(mode != VAS_SC_KIC) break;
						k = index * 2;	
						if(k >= buffer[1] || buffer[1] == 0) goto exit_get_config;
						goto key_load;
					case 0x84:	
						if(mode != VAS_SC_KID) break;
						k = index * 2;	
						if(k >= buffer[1] || buffer[1] == 0) goto exit_get_config;
						key_load:
						j = buffer[1];		//length
						for(k = 0; k < j; k += 2) {
							_readbin(temp_fs, i + k, buffer, 2);
							if(buffer[0] == index) {   
								_select(temp_fs, FID_MF);
								_select(temp_fs, FID_LIQUID);
								if(_select(temp_fs, FID_0348_KEY) < 0x9F00) goto exit_get_config; 		//EFtar not found
								_readrec(temp_fs, buffer[1], buffer_out, 16);
								status = TRUE;
								goto exit_get_config;	
							}	
						}
						break;
				}
				i += buffer[1];		//offset+length
			}
			break;
	}
	status = TRUE;
	exit_get_config:
	m_free(temp_fs);
	return status;
}

uchar VAS_replay_check(uchar mode, uchar * dcntr, uchar * scntr) _REENTRANT_ {
	uint32 dctr = *((uint32 *)(dcntr + 1));
	uint32 sctr = *((uint32 *)(scntr + 1));
	if(memcmp(scntr, "\xFF\xFF\xFF\xFF\xFF", 5) == 0) return TRUE;		//counter is blocked
	if(mode == VAS_CNTR_X_HIGHER) {
		if(scntr[0] > dcntr[0]) return FALSE;
		if(scntr[0] < dcntr[0]) return TRUE;
		return dctr > sctr;
	}
	if(mode == VAS_CNTR_1_HIGHER) {
		if(scntr[0] > dcntr[0]) return FALSE;
		if(dctr == 0 && (dcntr[0] == (scntr[0] + 1)) && sctr == 0xFFFFFFFFUL) return TRUE;			//kasus khusus
		if(scntr[0] < dcntr[0]) return FALSE;
		return dctr == (sctr + 1);
	}
	return FALSE; 
}

/*#if 0
uchar VAS_loadkey(vas_config * config, uchar mode, uchar index, uchar * buffer) _REENTRANT_ {
	fs_handle temp_fs;
	uchar k = 0;
	uchar j = 0;
	file_select(&temp_fs, FID_MF);
	file_select(&temp_fs, FID_WIB);	
	if(file_select(&temp_fs, FID_WSC) < 0x9F00) return FALSE;		//EFsc not found
	file_readbin(&temp_fs, config->sc_offset + 12, buffer, 2);		//ignore spi+kic+kid list
	k += buffer[1];		//k += length of spi+kic+kid list
	file_readbin(&temp_fs, config->sc_offset + 12 + k, buffer, 2);		//kic keyset  
	if(buffer[0] != 0x83) return FALSE;								//invalid kic keyset
	if(mode == VAS_LOAD_KID) {
		k += buffer[1];	 
		file_readbin(&temp_fs, config->sc_offset + 12 + k, buffer, 2);		//kid keyset
		if(buffer[0] != 0x84) return FALSE;							//invalid kid keyset
	}
	for(j = 0; j < buffer[1]; j += 2) {
		file_readbin(&temp_fs, config->sc_offset + 12 + k + j, buffer + 2, 2);		//kid keyset
		if(buffer[2] == index) { index = buffer[3]; goto start_load; }		//load key index
	}
	return FALSE;
	start_load:	  
	//kic/kid index + ref pair start from 0-15
	file_select(&temp_fs, FID_MF);
	file_select(&temp_fs, FID_LIQUID);
	if(file_select(&temp_fs, FID_0348_KEY) < APDU_SUCCESS_RESPONSE) return FALSE; 		//EFtar not found
	if(file_readrec(&temp_fs, index, buffer, 0x10) == APDU_SUCCESS) return TRUE;
	return FALSE; 
}

uchar VAS_loadcntr(vas_config * config, uchar index, uchar * buffer) _REENTRANT_ {	
	fs_handle temp_fs;
	file_select(&temp_fs, FID_MF);
	file_select(&temp_fs, FID_WIB);
	if(file_select(&temp_fs, FID_W0348CNTR) < APDU_SUCCESS_RESPONSE) return FALSE; 		//EFtar not found
	if(file_readrec(&temp_fs, index - 1, buffer, 5) == APDU_SUCCESS) return TRUE;
	return FALSE; 
}
#endif*/

vas_config * VAS_preprocess(command_packet * cmpkt) _REENTRANT_ { 		//for incoming message
	fs_handle temp_fs;
	TARconfig tar_config;
	uchar buffer[12];
	uint16 i = 0;
	uchar j = 0, k = 0;
	vas_config * vc = NULL;
	uint16 status = APDU_SUCCESS;
	_select(&temp_fs, FID_MF);
	_select(&temp_fs, FID_WIB);
	if(_select(&temp_fs, FID_WTAR) < 0x9F00) goto exit_preprocess; 		//EFtar not found
	status = _readrec(&temp_fs, i++, &tar_config, 5);
	while(status == APDU_SUCCESS) {
		if(memcmp(tar_config.tar, cmpkt->tar, 3) == 0) goto start_preprocess;
		status = _readrec(&temp_fs, i++, &tar_config, 5);
	}
	goto exit_preprocess;
	start_preprocess:		//TAR checking success
	#define OFFSCINDEX		4;
	_select(&temp_fs, FID_WIB);
	if(_select(&temp_fs, FID_WSC) < 0x9F00) goto exit_preprocess;		//EFsc not found
	////////////EF security configuration mechanism////////////////////////////	
	i = 0;		 													//i = offset for current SC
	for(j = 0; j != (tar_config.sc_index - 1); j++) {				//get TARsc index , j = index for current SC
		_readbin(&temp_fs, i, buffer, 12);
		if(buffer[0] != 0x30) goto exit_preprocess;			//invalid security configuration tag
		i += buffer[1];			//actual SC length
	}
	_readbin(&temp_fs, i, buffer, 12);					//i = offset of current SC on TAR
	if(buffer[0] != 0x30) goto exit_preprocess;			//invalid security configuration tag
	//temporary buffer format
	//[0] = outgoing cntr index
	buffer[0] = buffer[4];	//outgoing cntr index
	//[1] = incoming cntr index
	buffer[1] = buffer[5];	//incoming cntr index
	_readbin(&temp_fs, i + 12, buffer + 2, 2);			//read SPI+KIc+KID list tag+length
	//[2] = spi list tag
	if(buffer[2] != 0x82) goto exit_preprocess;			//invalid SPI+KIc+KID list tag
	//[3] = spi list length
	for(k = 0; k < buffer[3]; k += 4) {								 //k = offset for current operation from i
		_readbin(&temp_fs, i + 14 + k, buffer + 4, 4);			//iterate through all list, check for matched SC list
		//[4-7] = SPI+KIc+KID
		if(memcmp(buffer+4, cmpkt->spi, 4) == 0) goto start_preprocess_1;
	}
	goto exit_preprocess;
	start_preprocess_1:
	vc = m_alloc(sizeof(vas_config));
	vas_tar_mode = tar_config.type;			//set current operation mode  (PULL, PUSH, ADMIN)
	vc->type = tar_config.type;
	vc->sc_offset = i;
	vc->ocntr_index = buffer[0];		//outgoing cntr index
	vc->icntr_index = buffer[1];		//incoming cntr index
	exit_preprocess:
	return vc;		
}

uchar VAS_load_text(uint16 fid, uchar id, uchar * buffer, uchar max_len) _REENTRANT_ {
	fs_handle temp_fs;
	uchar temp[2];
	uint16 i = 0;
	_select(&temp_fs, FID_MF);
	_select(&temp_fs, FID_WIB);
	if(_select(&temp_fs, fid) < 0x9F00) return 0;
	while(_readbin(&temp_fs, i, temp, 2) == APDU_SUCCESS) {
		if(temp[0] == 0xFF) return 0;
		if(temp[0] == id) {
			if(temp[1] < max_len) {
				max_len = temp[1];
			}
			_readbin(&temp_fs, i + 2, buffer, max_len);	
			return max_len;
		}
		i += (temp[1] + 2);
	}
	return 0;
}
#endif