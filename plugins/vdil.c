#include "..\defs.h"
#include "..\midgard\midgard.h"
#include "..\asgard\file.h"
#include "..\asgard\security.h"
#include "..\liquid.h"
#include "..\framework\VAS.h"
#include "..\framework\SMS.h" 
#include "..\framework\dcs.h"
#include "dil.h"
#include <string.h>


#if VAS_VDIL_ALLOCATED

#define VDIL_STATE_IDLE		0
#define VDIL_STATE_RET_RECID_NEED_PIN		1
#define VDIL_STATE_RET_RECID_VERIFIED		2  
#define VDIL_STATE_RET_RECID_SELECT			3
#define VDIL_STATE_RET_NAME_NEED_PIN		9
#define VDIL_STATE_RET_NAME_VERIFIED		10  
#define VDIL_STATE_RET_NAME_SELECT			11
#define VDIL_STATE_RET_VAL_NEED_PIN			5
#define VDIL_STATE_RET_VAL_VERIFIED			6  
#define VDIL_STATE_RET_VAL_SELECT			7

static uchar vdil_state = VDIL_STATE_IDLE;

uint16 vdil_fetch(uchar * response, uint16 length) _REENTRANT_ {
	extern uchar * vas_cistr; 	 		//current input string
	extern uchar vas_cvid;				//current variable id	
	uchar i = 0;
	uchar tag;
	uchar size;
	uint16 status;
	if(SAT_init(response, length) == 0) return APDU_STK_OVERLOAD;
	while(i != length) {
		i += SAT_file_pop(i, &tag, &size, STK_buffer);
		tag |= 0x80; 
		switch(tag) {			//TERMINAL HANDLER	 
			case STK_TAG_RESULT:
				if((STK_buffer[0] & 0xF0) != 0) {
					vdil_state = VDIL_STATE_IDLE;
					VAS_exit_plugin();
					goto exit_fetch;
				}
				break;
			case STK_TAG_ITEM_ID:
				//set variable by vas_cvid to selected rec_id, name or value 
				//STK_buffer[0];
				switch(vdil_state) {
					case VDIL_STATE_RET_RECID_SELECT:
						VAS_set_variable(vas_cvid, 1, STK_buffer); 
						vdil_state = VDIL_STATE_IDLE;
						VAS_exit_plugin();						//exit plugin 
						goto exit_fetch;
					case VDIL_STATE_RET_NAME_SELECT:
					case VDIL_STATE_RET_VAL_SELECT:
						if(vas_cistr != NULL) {
							vas_cistr[1] &= 0xFE; 		//do not present item
							vas_cistr[4] = STK_buffer[0];		 //set current input string selected record
						}
						goto start_decode;
				}
				break;
			case STK_TAG_TEXT_STRING: 			//PIN verification
				STK_buffer[4] = 0xFF;
				STK_buffer[5] = 0xFF;
				STK_buffer[6] = 0xFF;
				STK_buffer[7] = 0xFF;
				status = chv_verify(1, STK_buffer);
				if(status == 0x9000) goto start_decode;		//PIN verification success
				if(status == APDU_ACCESS_DENIED) return SAT_file_flush(dil_get_pin(2));			//incorrect pin entry (try again)
				return SAT_file_flush(dil_get_pin(3));	 	//failed, turn of phone
				break;
			default: break;
		}
	}
	start_decode: 
	switch(vdil_state) {
		case VDIL_STATE_RET_RECID_NEED_PIN: vdil_state = VDIL_STATE_RET_RECID_VERIFIED; break;
		case VDIL_STATE_RET_NAME_NEED_PIN: vdil_state = VDIL_STATE_RET_NAME_VERIFIED; break;
		case VDIL_STATE_RET_VAL_NEED_PIN: vdil_state = VDIL_STATE_RET_VAL_VERIFIED; break;
	}
	i = vdil_decode();
	if(i != 0) {
		SAT_file_flush(i);
	}
	exit_fetch:
	return VAS_decode();
}

uchar vdil_decode() _REENTRANT_ {			//return response length  
	extern uchar * vas_cistr; 	 		//current input string
	extern uchar vas_cvid;				//current variable id
	fs_handle temp_fs;
	ef_header * curfile = NULL;
	uchar len;
	uchar i = 0;
	uchar j = 0;
	uchar mode;
	uint16 fid;
	uchar t_rec;	
	uchar params[4];
	if(vas_cistr == NULL) goto exit_plugin;		//no response, input string not exist
	len = vas_cistr[0];							//length of current input string
	mode = vas_cistr[1];
	fid = (uint16)vas_cistr[2];
	fid <<= 8;
	fid |= vas_cistr[3];
	
	//select the corresponding file id
	_select(&temp_fs, FID_MF);
	_select(&temp_fs, FID_WIB);
	if(_select(&temp_fs, fid) < 0x9F00) goto exit_plugin;		//fid not found

	switch(mode & 0x18) {
		case VDIL_OP_RET_RECID:
			//check access
			if(_check_access(&temp_fs, FILE_READ) != APDU_SUCCESS) {
				vdil_state = VDIL_STATE_RET_RECID_NEED_PIN;
				return dil_get_pin(1);		
			}
			curfile = file_get_current_header(&temp_fs);
			memset(STK_buffer, 0xFF, curfile->rec_size);
			t_rec = (curfile->size / curfile->rec_size);
			if(mode & 1) {    
				vdil_state = VDIL_STATE_RET_RECID_SELECT;
				goto list_item_by_name;
			} else {
				j = dil_display_text(12);
				m_free(curfile);
				goto exit_plugin;
			}

			list_item_by_name:
			//list all items by names
			j = 0;			
			params[0] = 0;
			params[1] = SELECT_ITEM;
			params[2] = 0x00;		//no selection preference
			j += SAT_file_push(j, STK_TAG_CMD_DETAIL, 3, params);
			params[0] = STK_DEV_SIM;
			params[1] = STK_DEV_ME;
			j += SAT_file_push(j, STK_TAG_DEV_ID, 2, params);
			//view item list name
			len = 0;
			for(i=0,j=0;i<t_rec;i++) {
		   		_readrec(&temp_fs, i, STK_buffer, curfile->rec_size);
				if(STK_buffer[0] == 0x04) {		//DCS = 4, 8bit default alphabet
					len = STK_buffer[1];	  			//length of current name parameter
					STK_buffer[1] = (i + 1);			//item id
					j += SAT_file_push(j, (STK_TAG_ITEM & 0x7F), (len + 1), STK_buffer + 1);	 	
				}	
			} 
			if(len == 0) { j = 0; }				//no item available, exit plugin without display any data 
			m_free(curfile);
			return j;

		case VDIL_OP_RET_NAME:
			//check access
			if(_check_access(&temp_fs, FILE_READ) != APDU_SUCCESS) {
				vdil_state = VDIL_STATE_RET_NAME_NEED_PIN;
				return dil_get_pin(1);		
			}
			curfile = file_get_current_header(&temp_fs);
			memset(STK_buffer, 0xFF, curfile->rec_size);
			t_rec = (curfile->size / curfile->rec_size);
			if(mode & 1) {    
				vdil_state = VDIL_STATE_RET_NAME_SELECT;
				goto list_item_by_name;
			} else {
				if(vas_cistr[4] > t_rec) { 			//check if record id is out of range
					j = dil_display_text(6);
				} else { 							//read name from selected record id and set variable
					_readrec(&temp_fs, vas_cistr[4], STK_buffer, curfile->rec_size);
					VAS_set_variable(vas_cvid, STK_buffer[1], STK_buffer + 2);
				}
			}
			m_free(curfile);
			break;
		case VDIL_OP_RET_VAL:
			//check access
			if(_check_access(&temp_fs, FILE_READ) != APDU_SUCCESS) {  
				vdil_state = VDIL_STATE_RET_VAL_NEED_PIN;
				return dil_get_pin(1);		
			}
			curfile = file_get_current_header(&temp_fs);
			memset(STK_buffer, 0xFF, curfile->rec_size);
			t_rec = (curfile->size / curfile->rec_size);
			if(mode & 1) {  
				vdil_state = VDIL_STATE_RET_VAL_SELECT;
				goto list_item_by_name;
			} else {
				if(vas_cistr[4] > t_rec) { 			//check if record id is out of range
					j = dil_display_text(6);
				} else {							//read value from selected record id and set variable 
					_readrec(&temp_fs, vas_cistr[4], STK_buffer, curfile->rec_size);
					VAS_set_variable(vas_cvid, STK_buffer[21], STK_buffer + 22);
				}
			}
			m_free(curfile);
			break;
	}


	exit_plugin:
	vdil_state = VDIL_STATE_IDLE;
	VAS_exit_plugin();
	return j;					//0 => next instruction (no response),  
}

#endif