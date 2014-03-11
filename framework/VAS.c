//#include "..\..\defs.h"
#include "..\midgard\midgard.h"
#include "..\asgard\file.h"
#include "..\liquid.h"
//#include "SAT.h"
#include "VAS.h"
#include "SMS.h" 
#include "..\framework\dcs.h"
#include <string.h>	
	 
#if VAS_MDIL_ALLOCATED || VAS_VDIL_ALLOCATED
#include "..\plugins\dil.h"
#endif	 	   
#if VAS_DITR_ALLOCATED
#include "..\plugins\ditr.h"
#endif 
#if VAS_MASL_ALLOCATED
#include "..\plugins\masl.h"
#endif 
#if VAS_ENCR_ALLOCATED
#include "..\plugins\encr.h"
#endif		 
#if VAS_DECR_ALLOCATED
#include "..\plugins\decr.h"
#endif	 	 
#if VAS_ICCID_ALLOCATED
#include "..\plugins\iccid.h"
#endif

uchar VAS_activated(void) _REENTRANT_ {
	fs_handle cb_fs;
	_select(&cb_fs, FID_MF);
	if(_select(&cb_fs, FID_WIB) < 0x9F00) return FALSE;
	if(_select(&cb_fs, FID_WMENU) <= 0x9F00) return FALSE;
	if(_check_access(&cb_fs, FILE_REHABILITATE) == APDU_SUCCESS) {
		return FALSE;
	}
	return TRUE;
}

#if VAS_ALLOCATED
fs_handle _vas_fs;	   
uchar vas_state = VAS_STATE_NORMAL;
uchar vas_mode = VAS_MODE_SCRIPT;
uchar vas_cvid = 0;		//current variable id, shared between execute+decode   
uchar * vas_cistr = NULL; 	//plugin input string
vas_variable * _var_list = NULL;
vas_variable * _const_list = NULL;  
int16 vas_skip = 0;
uint16 vas_PL = 0;		//Program Length
uint16 vas_PC = 0;		//Program Counter
uint16 vas_PC_prev = 0;		//for skip -2
uchar vas_tar_mode = VAS_TAR_PULL;

//initialize VAS decoder to specified plugin address on EFbytecode
uchar VAS_plugin_init(uint16 address) _REENTRANT_ {
	uint16 length;
	_select(&_vas_fs, FID_MF);
	_select(&_vas_fs, FID_WIB);
	if(_select(&_vas_fs, FID_WBYTECODE) < 0x9F00) return FALSE;
	if(_readbin(&_vas_fs, address, &length, 2) == APDU_SUCCESS) {
		vas_PL = address + length;
		vas_PC = address + 2;
		vas_PC_prev = vas_PC;
		vas_skip = 0;
		return TRUE;
	}
	return FALSE;
}

void VAS_exit_plugin(void) {
	vas_mode = VAS_MODE_SCRIPT;
	m_free(vas_cistr);
	vas_cistr = NULL;
}

//initialize VAS to specified file
uchar VAS_init(fs_handle * handle, uint16 address, uint16 size) _REENTRANT_ {
	//file_select(&_vas_fs, FID_MF);
	//file_select(&_vas_fs, FID_LIQUID);
	//if(file_select(&_vas_fs, fid) < 0x9F00) return FALSE;
	//if(file_writebin(&_vas_fs, 0, buffer, size) == APDU_SUCCESS) {
	memcpy(&_vas_fs, handle, sizeof(fs_handle));
	vas_PL = address + size;
	vas_PC = address;
	vas_PC_prev = vas_PC;
	vas_skip = 0;
	return TRUE;
	//}
	//return FALSE;
}

uchar VAS_file_pop(uint16 i, uchar * tag, uchar * size, uchar * value) _REENTRANT_ {
	_readbin(&_vas_fs, i, tag, 1);
	_readbin(&_vas_fs, i+1, size, 1);
	_readbin(&_vas_fs, i+2, value, size[0]);
	return (size[0] + 2);
}

/*#if 0
uchar VAS_file_push(uint16 i, uchar tag, uchar length, uchar * value) _REENTRANT_ {
	//uchar buffer = (uchar *)m_alloc(length + 2);
	uchar buffer[2];
	buffer[0] = tag;
	buffer[1] = length;
	//memcpy(buffer + 2, value, length);
	file_writebin(&_vas_fs, i, buffer, 2);			//PUSH TL  (Tag-Length)
	file_writebin(&_vas_fs, i + 2, value, length);	  //PUSH V (Value)
	//m_free(buffer);
	return (length + 2);	
}
#endif*/
 
//set variable to the list
vas_variable * set_variable(vas_variable ** list, uchar id, uchar length, uchar * buffer) _REENTRANT_ {	
	vas_variable * iterator = *list;
	vas_variable * temp;
	if(length == 0) return NULL;
	if(*list == NULL) {
		create_new_list: 
		*list = m_alloc(length + sizeof(vas_variable_header)); 
		iterator = *list;
	} else {
		while(iterator->next != NULL) {
			temp = iterator->next;
			if(temp->id == id) {  		//same id not allowed, remove variable from memory
				iterator->next = temp->next;
				m_free(temp);
			}
			iterator = iterator->next;
		}
		temp = *list;
		if(temp->id == id) {		//the first variable on the list has the same id
			m_free(temp);
			goto create_new_list;	
		} else {
			iterator->next = m_alloc(length + sizeof(vas_variable_header));
			iterator = iterator->next;
		}
	}
	if(iterator != NULL) {		//assign newly created variable
		iterator->id = id;
		iterator->length = length;
		iterator->next = NULL;
		memcpy(iterator->buffer, buffer, length);
	}	
	return iterator;
}

//clear variable list
void clear_list(vas_variable ** list, uchar from, uchar to) _REENTRANT_ {
	vas_variable * iterator = *list;
	vas_variable * temp = iterator;
	vas_variable * last = NULL;
	while(iterator != NULL) {
		temp = iterator;
		iterator = iterator->next;
		if(temp->id >= from && temp->id <= to) {	//free this variable
			m_free(temp);
		} else {	//do not freed, reverse list
			if(last != NULL) {	
				temp->next = last;
				last = temp;
			} else {
				last = temp;
				last->next = NULL;
			}
		}
	}
	*list = last;
}

//get variable from list
vas_variable * get_variable(vas_variable ** list, uchar id) _REENTRANT_ {
	vas_variable * iterator = *list;
	while(iterator != NULL) {
		if(iterator->id == id) return iterator;
		iterator = iterator->next;
	}
	return NULL;
}

/*#if 0
void list_dump(vas_variable * list) _REENTRANT_ {
	vas_variable * iterator = list;
	uchar i = 0;
	while(iterator != NULL) {
		printf("id: %02x length: %02x data: ", iterator->id, iterator->length);
		for(i=0;i<iterator->length;i++) {
			printf("%c", iterator->buffer[i]);
		}
		printf("\n");
		iterator = iterator->next;
	}
	return NULL;
}
#endif*/

uint16 VAS_script_fetch(uchar * response, uint16 length) _REENTRANT_ {	  		//fetch on response	 +19~24 bytes
	uchar i = 0;
	uchar tag;
	uchar res;
	uchar size;
	//uchar cmd = 0;
	static uchar fetch_cntr = 0;			//retry counter
	vas_variable * selected;
	if((vas_state & 0x0F) == VAS_STATE_WAIT_RESPONSE) return APDU_SUCCESS;
	if(SAT_init(response, length) == 0) return APDU_STK_OVERLOAD;
	while(i != length) {
		i += SAT_file_pop(i, &tag, &size, STK_buffer);
		tag |= 0x80; 
		switch(tag) {			//TERMINAL HANDLER
			case STK_TAG_CMD_DETAIL:
				//cmd = STK_buffer[1];
				//break;
			case STK_TAG_DEV_ID: break;			//don't process this tag
			case STK_TAG_RESULT:   //cek result
				res = STK_buffer[0];
				switch(res >> 4) {
					case 0:	fetch_cntr = 0; break;	//success
					case 1:		//other operation, ex:backward
						fetch_cntr = 0; 
						switch(res) {
							case STK_RES_BACKWARD: //vas_skip = -2; break;
							clear_list(&_var_list, 0x01, 0xDF);		  		//clear variable
							goto exit_fetch; 
							default:
							case STK_RES_TRANSACTION_ABORT:	
							case STK_RES_NO_USER_RESPONSE:
							case STK_RES_TERMINATED: 
							terminate_vas:
							clear_list(&_var_list, 0x01, 0xFF);		 		//clear variable
							goto exit_fetch;
							//case STK_RES_TRANSACTION_ABORT: vas_skip = -1; break;
						}
						break;
					case 2:		//try 3 times before quit
						if(fetch_cntr++ < 3) {
							vas_skip = -2;								
							break;
						}
					case 3: fetch_cntr = 0; goto terminate_vas;
				}
				break;
			default: break;
			case STK_TAG_TIMER_IDENTIFIER:
				res = STK_buffer[0];
				break; 
			case STK_TAG_TIMER_VALUE:
				break;
			case STK_TAG_SMS_TPDU:
				break;
			case STK_TAG_ITEM_ID:
				if((vas_state & 0x0F) == VAS_STATE_MENU) {			//select item(script running), assign variable
					//select default pull TAR
					selected = m_alloc(sizeof(fs_handle));
					_select(selected, FID_MF);
					_select(selected, FID_WIB);
					if(_select(selected, FID_WTAR) >= 0x9F00) {
						res = 0;
						while(_readrec(selected, res++, STK_buffer + 1, 5) == APDU_SUCCESS) {
							if(STK_buffer[1] == 0) {
								p348_set_tar(STK_buffer + 2);
								break;
							}
						} 
					}
					m_free(selected);
					//prepare VAS to execute plugin and change state to normal execution
					VAS_invoke(VAS_SELECT_MENU, STK_buffer[0]);
					vas_state = (vas_state & 0xF0) | VAS_STATE_NORMAL;
				} else {									//setup menu selection(initial), use internal plugin
					if(vas_cvid != 0) {
						selected = get_variable(&_const_list, STK_buffer[0]);	//get selected variable from const_list
						vas_skip = (int16)((int8)selected->buffer[0]);		//get current skip value
						//printf("var len : %d, %s, %d\n", selected->length, selected->buffer + 1, vas_cvid);
						size = selected->length - 1;
						memcpy(STK_buffer, selected->buffer + 1, size);
						clear_list(&_const_list, 0, 0xFF);			//free all const_list variable					
						set_variable(&_var_list, vas_cvid, size, STK_buffer);		//create new variable with id=cvid
						//return 0x9C00 | vas_skip;
						//list_dump(&_var_list);
					}		
				}
				break;
			case STK_TAG_LOCATION_INFO:
				if(vas_state & VAS_STATE_WAIT_VARIABLE) {
					vas_state &= 0x0F;					//clear wait variable
					if(vas_cvid != 0) {
						set_variable(&_var_list, vas_cvid, size, STK_buffer);
						goto start_decode;
					}
				}
				break;
			case STK_TAG_TEXT_STRING:
				//if(vas_state != VAS_STATE_MENU) {		//prevent *attack* from external (check state)
				if(vas_state & VAS_STATE_WAIT_VARIABLE) {
					vas_state &= 0x0F;					//clear wait variable
					if(vas_cvid != 0) {
						if((STK_buffer[0] & 0x0F) == 0x00) {		//7 bit default alphabet
							decode_728(STK_buffer + 1, STK_buffer + 1, size - 1);
						} 
						set_variable(&_var_list, vas_cvid, size - 1, STK_buffer + 1);
						goto start_decode;
					}
				}
				//}
				break;
		}
	}
	//didn't get variable, exit fetch and wait for next terminal response
	if(vas_state & VAS_STATE_WAIT_VARIABLE) goto exit_fetch;
	start_decode:
	if((vas_state & 0x0F) != VAS_STATE_MENU) {				//use normal execution
		return VAS_decode();
	}
	exit_fetch:   
	return APDU_SUCCESS;
}

uint16 VAS_fetch(uchar * response, uint16 length) _REENTRANT_ {	  		//fetch on response	 +6~11 bytes
	if(vas_mode & VAS_MODE_PLUGIN) {
		//return VAS_plugin_fetch(response, length);
		switch(vas_mode & 0x7F) {
			default: break;
#if	VAS_MDIL_ALLOCATED
			case VAS_PLUGIN_MDIL: return mdil_fetch(response, length);
#endif	
#if	VAS_VDIL_ALLOCATED
			case VAS_PLUGIN_VDIL: return vdil_fetch(response, length);
#endif	 
#if	VAS_DITR_ALLOCATED
			case VAS_PLUGIN_DITR: return ditr_fetch(response, length);
#endif	
#if	VAS_MASL_ALLOCATED
			case VAS_PLUGIN_MASL: return masl_fetch(response, length);
#endif 
#if	VAS_ENCR_ALLOCATED
			case VAS_PLUGIN_ENCR: return encr_fetch(response, length);
#endif	
#if	VAS_DECR_ALLOCATED
			case VAS_PLUGIN_DECR: return decr_fetch(response, length);
#endif	   
#if	VAS_ICCID_ALLOCATED
			case VAS_PLUGIN_ICCID: return iccid_fetch(response, length);
#endif
		}
	} else { 
		//use local script
		return VAS_script_fetch(response, length);
	}
}
		
uint16 VAS_execute(uchar * response) _REENTRANT_ {			//execute on envelope	+17-22 bytes
	uchar dsize;
	uchar dtag; 
	uint16 status = APDU_SUCCESS;
	uchar tag, size, i = 0;
	SAT_pop(response, &dtag, &dsize, STK_buffer);
	memcpy(response, STK_buffer, dsize);
	//return VAS_fetch(response, size);			//use default execution
	switch(dtag) {
		case ENV_TAG_MENU:
			//cleanup
			clear_list(&_var_list, 0x01, 0xFF);		 		//clear variable
			clear_list(&_const_list, 0x01, 0xFF);		 		//clear constant
			vas_state = VAS_STATE_MENU;						//end of script, exit to menu
			status = VAS_fetch(response, dsize);			//use default execution	case ENV_TAG_CALL:	   //CALL CONTROL
			break;
		case ENV_TAG_SMS_PP:
		case ENV_TAG_SMS_CB: 
			//vas_state = VAS_STATE_NORMAL;
		case ENV_TAG_EVENT:
		case ENV_TAG_TIMER:
			if(SAT_init(response, dsize) == 0) return APDU_STK_OVERLOAD;
			while(i != dsize) {
				i += SAT_file_pop(i, &tag, &size, STK_buffer);
				tag |= 0x80; 
				switch(tag) {			//TERMINAL HANDLER
					case STK_TAG_CMD_DETAIL:
					case STK_TAG_DEV_ID: break;			//don't process this tag
				 	case STK_TAG_ITEM_ID:  
						break;
					case STK_TAG_RESULT:   //cek result
					default: break;
					case STK_TAG_TIMER_IDENTIFIER:
						//res = STK_buffer[0];
						break; 
					case STK_TAG_TIMER_VALUE:
						/*if(dtag == ENV_TAG_TIMER) {
							timer_callback(res, tag, size, STK_buffer);
						}*/
						break;
					case STK_TAG_SMS_TPDU:
						if(dtag == ENV_TAG_SMS_PP) {  
							status = decode_SMSTPDU(1, STK_buffer);
						}
						if(dtag == ENV_TAG_SMS_CB) {
							status = decode_SMSTPDU(0, STK_buffer);
						}
						break;
					case STK_TAG_EVENT_LIST:
						if(VAS_invoke(VAS_SELECT_EVENT, STK_buffer[0]) == TRUE) {
							status = VAS_decode();
						}
						break;
				}
			}
			break;
		default:
			status = APDU_SUCCESS; break;
	}
	return status;
}

uchar VAS_substitute(uchar lv_format, uchar * buffer_in, uchar len, uchar * buffer_out, uchar max_len) _REENTRANT_ {
	uchar i = 0, j = 0;
	uchar p;
	uchar d;
	vas_variable * selected;
	while(i < len) {
		p = buffer_in[i++];
		d = p;
		switch(p) {
			case 0x80:
			case 0x81:
			case 0x82:
				p = buffer_in[i++];
				switch(p) {
					//byte stuffing
					case 0x80:
					case 0x81:
					case 0x82:
						buffer_out[j++] = p;
						break;
					//variable reference	
					default:
						selected = get_variable(&_var_list, p);	//get selected variable from const_list
						if(selected == NULL) break;
						if(lv_format) {
							buffer_out[j++] = d;
							buffer_out[j++] = selected->length;
						}
						if((j + selected->length) < max_len) {
							memcpy(buffer_out + j, selected->buffer, selected->length);
							j += selected->length;
						}
						break;
				}
				break;
			default:
				buffer_out[j++] = p;
				break;
		}
	}
	//printf("length : %d\n", j);
	return j;
}

uchar VAS_push_header(uchar offset, uchar cmd, uchar qualifier, uchar target) _REENTRANT_ {
	uchar temp[3];
	temp[0] = 1;
	temp[1] = cmd;
	temp[2] = qualifier;
	offset = SAT_file_push(offset, STK_TAG_CMD_DETAIL, 3, temp);
	temp[0] = STK_DEV_SIM;
	temp[1] = target;
	offset += SAT_file_push(offset, STK_TAG_DEV_ID, 2, temp);
	return offset;
}

//WIB Instruction Decoder
uint16 VAS_decode(void) _REENTRANT_ {  		//+24~34 bytes
	//#define buffer	STK_buffer
	//uint16 i = 0;
	uint16 status = APDU_SUCCESS;
	uchar j, k;
	uchar tag;
	uchar size;
	//uchar skip = 0;
	//uchar id_gen = 0;
	uchar len;
	uchar temp[5];
	uchar attr;
	uchar * value;
	uchar * address;
	vas_variable * cvar;
	#if	VAS_MDIL_ALLOCATED || VAS_VDIL_ALLOCATED  
	fs_handle vas_fs;
	#endif
	//ignore wait response state
	next_instruction:
	while(vas_PC < vas_PL) {
		if(vas_skip == -2) { 		//backward operation
			vas_PC = vas_PC_prev;
			vas_skip = 0;
		}
		if(vas_skip == -1) goto finish_decode;
		vas_PC_prev = vas_PC;
		vas_PC += VAS_file_pop(vas_PC, &tag, &size, STK_buffer);
		//printf("T = %02x, L = %02x\n", tag, size);
		vas_cvid = 0;
		if(vas_skip == 0) {
			len = 0;
			j = 0;
			k = 0;
			attr = 0;
			switch(tag & 0x7F) {		//decode WIB (VAS)
#if IVAS_SUBMIT_EXTENDED_ALLOCATED
				case VAS_SUBMIT_EXTENDED:
					attr = STK_buffer[k++];
#endif
#if (IVAS_SUBMIT_ALLOCATED)
				case VAS_SUBMIT:
#endif
#if (IVAS_SUBMIT_ALLOCATED || IVAS_SUBMIT_EXTENDED_ALLOCATED)
					len = STK_buffer[k];		//output length
					value = m_alloc(len + 1);
					memcpy(value, STK_buffer + k + 1, len);
					//server inbound message
					len = VAS_substitute(TRUE, value, len, STK_buffer, 255);
					m_free(value);

					liquid_set_response_data(RESPONSE_PKT_POR_OK, STK_buffer, len);				//set EFres
					//m_free(value);
					j += VAS_push_header(j, (SEND_SHORT_MESSAGE & 0x7F), 0x00, STK_DEV_ME);		//packing not required
					
					//service center address (load from sms_header)
					address = m_alloc(sizeof(fs_handle));  		//--> address, temporary fs_handle
					_select(address, FID_MF);
					_select(address, FID_WIB);
					_select(address, FID_WTEXT);
					k = 0;
					//read alpha tag
					while(_readbin(address, k, STK_buffer, 2) == APDU_SUCCESS) {
						k += 2;
						if(STK_buffer[0] == 1) {
							len = STK_buffer[1];
							_readbin(address, k, STK_buffer, len);
							if(STK_buffer[0] == 0) {   			//alpha id tag
								len = STK_buffer[1]; 			//length of alpha identifier
								j += SAT_file_push(j, (STK_TAG_ALPHA & 0x7F), len, STK_buffer + 2);
								break;
							}
						} else {
							if(STK_buffer[0] == 0xFF) break;
						} 
						k += STK_buffer[1];
					}
					//address tag
					//m_free(address);
					len = p348_create_header(STK_buffer);											//create 03.48 header, configuration based on tar value
					len = p348_encode_command_packet(STK_buffer);								//encode response packet (combine 0348header with EFres content)
					
					send_short_message:
					value = m_alloc(len);
					memcpy(value, STK_buffer, len);		//copy response packet header and all it;s content to allocated memory
					//encode encrypted 03.48 response to SMS-TPDU packet
					_select(address, FID_MF);
					_select(address, FID_WIB);
					_select(address, FID_WSMSHEADER);
					_readrec(address, 0, STK_buffer + len + 1, 27);
					k = ((vas_sms_header *)(STK_buffer + len + 1))->sc_address[0];
					j += SAT_file_push(j, (STK_TAG_ADDRESS & 0x7F), k, ((vas_sms_header *)(STK_buffer + len + 1))->sc_address + 1);
					k = ((vas_sms_header *)(STK_buffer + len + 1))->da_address[0];
					m_free(address);

					address = m_alloc(k + 1);
					memcpy(address, ((vas_sms_header *)(STK_buffer + len + 1))->da_address, k + 1);
					k = ((vas_sms_header *)(STK_buffer + len + 1))->pid;
					attr = ((vas_sms_header *)(STK_buffer + len + 1))->dcs;
					len = encode_SMSTPDU(SMS_TYPE_SUBMIT, k, attr, len, address, (response_packet *)value, STK_buffer);	//SMS submit
					//STK_buffer[0] = len;		//encoded length
					m_free(value);
					m_free(address);
					j += SAT_file_push(j, (STK_TAG_SMS_TPDU & 0x7F), len, STK_buffer);
					//#else
					//len = encode_SMSTPDU(SMS_TYPE_SUBMIT, len, "\x00\x02\x45\xF3", (response_packet *)value, STK_buffer + 1);	//SMS submit
					//STK_buffer[0] = len;		//encoded length
					//m_free(value);
					//#endif
					if(vas_tar_mode == VAS_TAR_PULL) {
						vas_state = VAS_STATE_WAIT_RESPONSE;
					}
					//#if 0		//TODO:change to variable addressing mode for smsc
					//status = SAT_printf("cdam", SEND_SHORT_MESSAGE, STK_DEV_ME, smsc, STK_buffer);
					//#else		//use default address mode
					//status = SAT_printf("cdm", SEND_SHORT_MESSAGE, STK_DEV_ME, STK_buffer);
					//#endif
					goto exit_decode;
#endif
#if (IVAS_DISPLAY_TEXT_EXTENDED_ALLOCATED)
				case VAS_DISPLAY_TEXT_EXTENDED:
					if(STK_buffer[k] & 0x02) attr |= 0x80;	//wait for user to clear message
					if(STK_buffer[k] & 0x01) attr |= 0x01;	//high priority
					k++;
					goto show_display_text;
#endif
#if (IVAS_DISPLAY_TEXT_ALLOCATED)
				case VAS_DISPLAY_TEXT:
					attr |= 0x80;		//wait for user to clear message
#endif
#if	(IVAS_DISPLAY_TEXT_CLEAR_AFTER_DELAY_ALLOCATED)
				case VAS_DISPLAY_TEXT_CLEAR_AFTER_DELAY:
#endif
#if (IVAS_DISPLAY_TEXT_EXTENDED_ALLOCATED || IVAS_DISPLAY_TEXT_ALLOCATED || IVAS_DISPLAY_TEXT_CLEAR_AFTER_DELAY_ALLOCATED)
					show_display_text:	
					j += VAS_push_header(j, (DISPLAY_TEXT & 0x7F), attr, STK_DEV_DISPLAY);
					len = STK_buffer[k];
					//for text tag STK_buffer now is obsolote, can be used to store result
					value = m_alloc(len + 1);
					memcpy(value, STK_buffer + k + 1, len);
					//use no lv format (not a inbound message)
					#if UNICODE_SUPPORT 
					if(tag & 0x80) { 			//encode to 8 bit
						len = decode_ucs28(value, value, len);	
					}
					#endif
					len = VAS_substitute(FALSE, value, len, STK_buffer, 255);
					m_free(value);
					value = m_alloc(len + 1);
					//value[len] = 0;
					value[0] = 0x04;										//DCS
					memcpy(value + 1, STK_buffer, len);
					j += SAT_file_push(j, (STK_TAG_TEXT_STRING & 0x7F), len + 1, value);
					m_free(value);
					status = APDU_STK_RESPONSE | j;
					goto exit_decode;
#endif
#if (IVAS_GET_INPUT_ALLOCATED)
				case VAS_GET_INPUT:
					vas_cvid = STK_buffer[k++];	
					len = STK_buffer[k];	//k=1
					value = m_alloc(len + 1);
					memcpy(value, STK_buffer + k + 1, len);
					//use no lv format (not a inbound message)
					len = VAS_substitute(FALSE, value, len, STK_buffer + size, 255 - size);
					m_free(value);
					value = m_alloc(len + 2);
					value[0] = len + 1;
					value[1] = 0xF6;			//DCS
					memcpy(value + 2, STK_buffer + size, len);
					//value[len + 1] = 0;
					len = STK_buffer[k++];
					k += len;
					while(k < size) {
						len = STK_buffer[k++];
						switch(STK_buffer[k]) {
							case 0: temp[4] = STK_buffer[k+1]; break;
							case 1: attr = STK_buffer[k+1]; break;
							case 2: temp[3] = STK_buffer[k+1]; break;
						}
						k += len;
					}
					j += VAS_push_header(j, (GET_INPUT & 0x7F), attr, STK_DEV_ME);
					#if UNICODE_SUPPORT
					if(tag & 0x80) { 			//encode to 8 bit
						value[0] = decode_ucs28(value + 2, value + 2, value[0] - 1);
						value[0] += 1;		//for DCS	
					}
					#endif
					j += SAT_file_push(j, (STK_TAG_TEXT_STRING & 0x7F), value[0], value + 1);
					len = 2;
					j += SAT_file_push(j, (STK_TAG_RESPONSE_LENGTH & 0x7F), len, temp + 3);
					m_free(value);
					while(k < size) {
						len = STK_buffer[k++];
						if(STK_buffer[k] == 4) {
							j += SAT_file_push(j, (STK_TAG_DEFAULT_TEXT & 0x7F), STK_buffer[k+1], STK_buffer + k + 2);
						}
						k += len;
					}
					//list_dump(_const_list); 
					vas_state |= (VAS_STATE_WAIT_VARIABLE | VAS_STATE_VAR_TEXT);
					status = APDU_STK_RESPONSE | j;
					goto exit_decode;
#endif
#if (IVAS_SELECT_ITEM_ALLOCATED)
				case VAS_SELECT_ITEM:	  //show menu
					vas_cvid = STK_buffer[k++];
					j += VAS_push_header(j, (SELECT_ITEM & 0x7F), 0, STK_DEV_ME);
					len = STK_buffer[k++];
					temp[0] = len;
					//value = m_alloc(len + 1);
					//memcpy(value, STK_buffer + k, len);
					//use no lv format (not a inbound message) 
					#if UNICODE_SUPPORT
					if(tag & 0x80) { 			//encode to 8 bit
						len = decode_ucs28(STK_buffer + k, STK_buffer + k, len);	
					}
					#endif
					len = VAS_substitute(FALSE, STK_buffer + k, len, STK_buffer + size, 255 - size);
					//m_free(value);
					j += SAT_file_push(j, (STK_TAG_ALPHA & 0x7F), len, STK_buffer + size);
					k += temp[0];
					attr = 1;
					//clear_list(&_const_list, 0x01, 0xFF);		//clear all variables (freeup memory)
					while(k < size) {
						len = STK_buffer[k];
						temp[0] = len;			//backup length
						value = m_alloc(len + 2);
						//value[0] = attr;		//item id
						memcpy(value + 1, STK_buffer + k + 1, len);
						value[len + 1] = 0;
						#if UNICODE_SUPPORT
						if(tag & 0x80) { 			//encode to 8 bit
							len = decode_ucs28(value + 1, STK_buffer + k + 1, len);	
						}
						#endif
						STK_buffer[k] = attr;
						j += SAT_file_push(j, (STK_TAG_ITEM & 0x7F), len + 1, STK_buffer + k);
						len = temp[0];			//restore backup length			
						len += 1;
						k += len;
						len = STK_buffer[k];
						if(len == 0) {
							len = temp[0];		  //load from backup
							//k++;					//skip length
							value[0] = STK_buffer[k + 1];		//skip var
							set_variable(&_const_list, attr++, len + 1, value);
							m_free(value);
							k++;  			//skip byte
						} else {
							m_free(value);
							STK_buffer[k] = STK_buffer[k + 1 + len];		//skip var
							set_variable(&_const_list, attr++, len + 1, STK_buffer + k);
							len += 1;		//increment index by length
							k += len;	   	//skip byte
						}
						k++;
					}
					//list_dump(_const_list);
					status = APDU_STK_RESPONSE | j;
					goto exit_decode;
#endif
#if (IVAS_PROVIDE_LOCAL_INFORMATION_ALLOCATED)
				case VAS_PROVIDE_LOCAL_INFORMATION:
					attr = STK_buffer[k++];
					vas_cvid = STK_buffer[k];
					j += VAS_push_header(j, (PROVIDE_LOCAL_INFORMATION & 0x7F), attr, STK_DEV_ME);
					status = APDU_STK_RESPONSE | j;
					vas_state |= (VAS_STATE_WAIT_VARIABLE | VAS_STATE_VAR_LOCALINFO);
					goto exit_decode;
#endif
#if (IVAS_PLAY_TONE_ALLOCATED)
				case VAS_PLAY_TONE:
					j += VAS_push_header(j, (PLAY_TONE & 0x7F), attr, STK_DEV_ME);
					k = 3;
					len = STK_buffer[k++];
					temp[0] = len;
					if(tag & 0x80) { 			//encode to 8 bit
						len = decode_ucs28(STK_buffer + k, STK_buffer + k, len);	
					}
					j += SAT_file_push(j, (STK_TAG_ALPHA & 0x7F), len, STK_buffer + k);
					k += temp[0];
					j += SAT_file_push(j, (STK_TAG_TONE & 0x7F), 1, STK_buffer);
					j += SAT_file_push(j, (STK_TAG_DURATION & 0x7F), 1, STK_buffer + 1);
					status = APDU_STK_RESPONSE | j;
					goto exit_decode;
#endif
#if (IVAS_SETUP_IDLE_MODE_TEXT_EXTENDED_ALLOCATED)
				case VAS_SETUP_IDLE_MODE_TEXT_EXTENDED:
					temp[3] = STK_buffer[k++];
					goto show_idle_text;
#endif
#if	(IVAS_SETUP_IDLE_MODE_TEXT_ALLOCATED)
				case VAS_SETUP_IDLE_MODE_TEXT:
					temp[3] = size;
#endif
#if	(IVAS_SETUP_IDLE_MODE_TEXT_ALLOCATED || IVAS_SETUP_IDLE_MODE_TEXT_EXTENDED_ALLOCATED)
					show_idle_text:
					j += VAS_push_header(j, (SET_UP_IDLE_TEXT & 0x7F), 0, STK_DEV_ME);
					//variable substitution including reference
					k = 3;
					len = temp[k++];	//k=3
					value = m_alloc(len + 1);
					memcpy(value, STK_buffer + k, len);
					//use no lv format (not a inbound message)	 
					if(tag & 0x80) { 			//encode to 8 bit
						len = decode_ucs28(value, value, len);	
					}
					len = VAS_substitute(FALSE, value, len, STK_buffer + 1, 255);
					m_free(value);
					STK_buffer[0] = 0xF6;						//DCS
					j += SAT_file_push(j, (STK_TAG_TEXT_STRING & 0x7F), len + 1, STK_buffer);
					status = APDU_STK_RESPONSE | j;
					goto exit_decode;
#endif
#if (IVAS_REFRESH_ALLOCATED)
				case VAS_REFRESH:
					j += VAS_push_header(j, (REFRESH & 0x7F), STK_buffer[k++], STK_DEV_ME);
					len = STK_buffer[k++];
					j += SAT_file_push(j, (STK_TAG_FILE_LIST & 0x7F), len, STK_buffer + k);
					status = APDU_STK_RESPONSE | j;
					goto exit_decode;
#endif
#if (IVAS_SETUP_CALL_EXTENDED_ALLOCATED)
				case VAS_SETUP_CALL_EXTENDED:
					k = 1;
					len = STK_buffer[k++];
					k += len;
					len = STK_buffer[k++];
					k += len;
					temp[4] = STK_buffer[k++];		//address format specifier
					cvar = get_variable(&_var_list, STK_buffer[k++]);
					value = NULL;
					while(k < size) {
						len = STK_buffer[k++];
						if(STK_buffer[k] == 0) {
							value = m_alloc(len);
							memcpy(value, STK_buffer + k + 1, len - 1);
							temp[3] = len - 1;
						}
						k += len;
					}
					k = 1;
					goto show_setup_call;
#endif
#if (IVAS_SETUP_CALL_ALLOCATED)
				case VAS_SETUP_CALL:
					k = 1;
					cvar = NULL;
					len = STK_buffer[k++];
					value = m_alloc(len);
					memcpy(value, STK_buffer + k, len);
					temp[3] = len;
					k += len;
#endif
#if (IVAS_SETUP_CALL_ALLOCATED || IVAS_SETUP_CALL_EXTENDED_ALLOCATED)
					show_setup_call:
					attr = STK_buffer[0];
					j += VAS_push_header(j, (SET_UP_CALL & 0x7F), attr, STK_DEV_ME);
					if(value != NULL) {		//alpha identifier
						//can be outdone from setup_call or setup_call_extended, depends on value reference, use substitution
						//never change any STK_buffer value below command size, use STK_buffer+size as temporary
						temp[3] = VAS_substitute(FALSE, value, temp[3], STK_buffer + size, 255 - size);
						j += SAT_file_push(j, (STK_TAG_ALPHA & 0x7F), temp[3], STK_buffer + size);
						m_free(value);
					}
					if(cvar != NULL) {		//address for extended
						value = m_alloc(cvar->length + 2);
						value[0] = temp[4];
						value[1] = cvar->length;
						memcpy(value + 2, cvar->buffer, cvar->length);
						j += SAT_file_push(j, (STK_TAG_ADDRESS & 0x7F), cvar->length + 2, value);
						m_free(value);	
					}
					len = STK_buffer[k++];		//capability
					j += SAT_file_push(j, (STK_TAG_CAPABILITY & 0x7F), len, STK_buffer + k);
					k += len;
					len = STK_buffer[k++];		//duration
					j += SAT_file_push(j, (STK_TAG_DURATION & 0x7F), len, STK_buffer + k);	
					k += len;
					len = STK_buffer[k++];		//address for non extended
					if(cvar == NULL) {
						j += SAT_file_push(j, (STK_TAG_ADDRESS & 0x7F), len, STK_buffer + k);	
					}
					k += len;
					status = APDU_STK_RESPONSE | j;
					goto exit_decode;
#endif
#if (IVAS_SEND_USSD_ALLOCATED)
				case VAS_SEND_USSD:
					j += VAS_push_header(j, (SEND_USSD & 0x7F), 0, STK_DEV_NETWORK);
					len = STK_buffer[k++];
					value = m_alloc(len + 1);
					memcpy(value, STK_buffer + k + 1, len);
					//server inbound message  
					k += len;
					#if UNICODE_SUPPORT
					if(tag & 0x80) { 			//encode to 8 bit
						len = decode_ucs28(value, value, len);	
					}
					#endif
					len = VAS_substitute(FALSE, value, len, STK_buffer + size, 255 - size);
					m_free(value);
					value = m_alloc(len + 1);
					memcpy(value, STK_buffer + size, len);
					value[len] = 0;
					j += SAT_file_push(j, (STK_TAG_ALPHA & 0x7F), len, value);
					m_free(value);
					len = STK_buffer[k++];
					temp[0] = len;
					memcpy(STK_buffer + size, STK_buffer + k, len);
					#if UNICODE_SUPPORT
					if(tag & 0x80) { 			//encode to 8 bit
						len = encode_82ucs(STK_buffer + size, STK_buffer + size, len);	
					}
					#endif
					j += SAT_file_push(j, (STK_TAG_USSD_STRING & 0x7F), len, STK_buffer + size);
					k += temp[0];
					while(k < size) {
						len = STK_buffer[k++];
						if(STK_buffer[k] == 0) {
							vas_cvid = STK_buffer[k+1];
						}
						k += len;
					}
					status = APDU_STK_RESPONSE | j;
					goto exit_decode;
#endif
#if (IVAS_SEND_SM_EXTENDED_ALLOCATED)
				case VAS_SEND_SM_EXTENDED:
					///TODO : 	PID+DCS not implemented
					j += VAS_push_header(j, (SEND_SHORT_MESSAGE & 0x7F), 0, STK_DEV_NETWORK);
					len = STK_buffer[k++];
					value = m_alloc(len + 1);
					memcpy(value, STK_buffer + k + 1, len);
					k += len;
					len = VAS_substitute(FALSE, value, len, STK_buffer + size, 255 - size);
					m_free(value);
					value = m_alloc(len + 1);
					memcpy(value, STK_buffer + size, len);
					value[len] = 0;
					j += SAT_file_push(j, (STK_TAG_ALPHA & 0x7F), len, value);
					m_free(value);
					temp[0] = STK_buffer[k++];		//PID
					temp[1] = STK_buffer[k++];		//DCS
					temp[4] = STK_buffer[k++];		//address format specifier
					cvar = get_variable(&_var_list, STK_buffer[k++]);
					if(cvar != NULL) {		
						address = m_alloc(cvar->length + 2);
						address[0] = temp[4];
						address[1] = cvar->length;
						memcpy(address + 2, cvar->buffer, cvar->length);	
					} else {
						address = NULL;
					}
					temp[0] = STK_buffer[k++];
					temp[1] = k;
					k += len;
					while(k < size) {
						len = STK_buffer[k++];
						if(STK_buffer[k] == 0) {
							temp[3] = STK_buffer[k+1];		//address format specifier
							cvar = get_variable(&_var_list, STK_buffer[k+2]);
							if(cvar != NULL) {			//smsc address tag
							 	value = m_alloc(cvar->length + 2);
								value[0] = temp[3];	//address format specifier
								value[1] = cvar->length;
								memcpy(value + 2, cvar->buffer, cvar->length);
								j += SAT_file_push(j, (STK_TAG_ADDRESS & 0x7F), cvar->length + 2, value);
								m_free(value);
							}
						}
						k += len;
					}
					k = temp[1];
					len = temp[0];
					//submit SMS with variable reference
					//variable substitution
					value = m_alloc(len + 1);
					memcpy(value, STK_buffer + k, len);
					len = VAS_substitute(FALSE, value, len, STK_buffer, 255);
					m_free(value);
					value = m_alloc(len + 1);
					memcpy(value, STK_buffer, len);
					value[len] = 0;
					len = encode_SMSTPDU(SMS_TYPE_SUBMIT, 0x7F, 0xF6, len, address, value, STK_buffer);	//SMS submit
					j += SAT_file_push(j, (STK_TAG_SMS_TPDU & 0x7F), len, STK_buffer);
					m_free(address);
					status = APDU_STK_RESPONSE | j;
					goto exit_decode;
#endif
#if (IVAS_SEND_SM_ALLOCATED)
				case VAS_SEND_SM:
					///TODO : 	PID+DCS not implemented
					j += VAS_push_header(j, (SEND_SHORT_MESSAGE & 0x7F), 0, STK_DEV_NETWORK);
					temp[0] = STK_buffer[k++];		//PID
					temp[1] = STK_buffer[k++];		//DCS
					len = STK_buffer[k++];
					address = m_alloc(len);
					memcpy(address, STK_buffer + k, len);
					k += len;
					len = STK_buffer[k++];
					if(len != 0) {
						j += SAT_file_push(j, (STK_TAG_ADDRESS & 0x7F), len, STK_buffer + k);
						k += len;
					}
					len = STK_buffer[k++];
					//submit SMS with variable reference
					//variable substitution
					value = m_alloc(len + 1);
					memcpy(value, STK_buffer + k, len);
					len = VAS_substitute(FALSE, value, len, STK_buffer, 255);
					m_free(value);
					value = m_alloc(len + 1);
					memcpy(value, STK_buffer, len);
					value[len] = 0;
					len = encode_SMSTPDU(SMS_TYPE_SUBMIT, temp[0], temp[1], len, address, value, STK_buffer);	//SMS submit
					j += SAT_file_push(j, (STK_TAG_SMS_TPDU & 0x7F), len, STK_buffer);
					m_free(address);
					status = APDU_STK_RESPONSE | j;
					goto exit_decode;
#endif
#if (IVAS_TIMER_MANAGEMENT_ALLOCATED)
				case VAS_TIMER_MANAGEMENT:				//NOT IMPLEMENTED
					break;
#endif
#if (IVAS_LAUNCH_BROWSER_EXTENDED_ALLOCATED)
				case VAS_LAUNCH_BROWSER_EXTENDED:
#endif 
#if (IVAS_LAUNCH_BROWSER_ALLOCATED)
				case VAS_LAUNCH_BROWSER:				//NOT IMPLEMENTED
#endif
#if (IVAS_LAUNCH_BROWSER_EXTENDED_ALLOCATED || IVAS_LAUNCH_BROWSER_ALLOCATED)
					break;
#endif
#if (IVAS_SKIP_ALLOCATED)
				case VAS_SKIP:
					if(size == 1) { 
						vas_skip = (int16)((int8)STK_buffer[0]);
					}
					if(size == 2) {
						vas_skip = *(int16 *)STK_buffer;
					}
					//return 0x9C00 | vas_skip;
					goto next_instruction;		//next instruction
#endif
#if (IVAS_EXIT_ALLOCATED)
				case VAS_EXIT:
					finish_decode:
					vas_PC = 0;
					vas_PL = 0;
					clear_list(&_var_list, 0x01, 0xFF);		//clear all variables (freeup memory)
					status = APDU_SUCCESS;
					goto exit_decode;
#endif
#if (IVAS_NEW_CONTEXT_ALLOCATED)
				case VAS_NEW_CONTEXT:
					attr = STK_buffer[k++];
					switch(attr) {			   //start from 0x01, 0x00 is invalid variable
					 	case 0: clear_list(&_var_list, 0x01, 0xDF);	break;
						case 1: clear_list(&_var_list, 0xE0, 0xFF);	break;
						case 2: clear_list(&_var_list, 0x01, 0xFF);	break;
						default: break;
					} 
					goto next_instruction;
#endif
#if (IVAS_SET_EXTENDED_ALLOCATED)
				case VAS_SET_EXTENDED:
#endif	  
#if (IVAS_SET_ALLOCATED)
				case VAS_SET:
#endif
#if (IVAS_SET_ALLOCATED || IVAS_SET_EXTENDED_ALLOCATED)
					set_variable(&_var_list, STK_buffer[0], size - 1, STK_buffer + 1);
					goto next_instruction;
#endif
#if (IVAS_SET_RETURN_TAR_VALUE_ALLOCATED)
				case VAS_SET_RETURN_TAR_VALUE:			//TAR index 1-255
					value = m_alloc(sizeof(fs_handle));
					if(value != NULL) {
						_select(value, FID_MF);
						_select(value, FID_WIB);
						if(_select(value, FID_WTAR) >= 0x9F00) {
							k = STK_buffer[0] - 1;
							if(_readrec(value, k, STK_buffer, 5) == APDU_SUCCESS) {
								p348_set_tar(STK_buffer + 1);
							} 
						}
						m_free(value);
					}
					goto next_instruction;
#endif
#if (IVAS_BRANCH_ON_VAR_VALUE_ALLOCATED)
				case VAS_BRANCH_ON_VAR_VALUE:
					cvar = get_variable(&_var_list, STK_buffer[k++]);
					while(k < size) {
						len = STK_buffer[k++];
						if(memcmp(STK_buffer + k, cvar->buffer, len - 1) == 0) {
							vas_skip = (int16)((int8)STK_buffer[k + len - 1]);
							goto next_instruction;
						}
						k += len; 			
					}
					goto next_instruction;
#endif
#if (IVAS_CHECK_TERMINAL_PROFILE_ALLOCATED)
				case VAS_CHECK_TERMINAL_PROFILE:			//NOT IMPLEMENTED
					goto next_instruction;
#endif
#if (IVAS_SUBSTRING_ALLOCATED)
				case VAS_SUBSTRING:
					cvar = get_variable(&_var_list, STK_buffer[1]);
					value = m_alloc(STK_buffer[3]);
					memcpy(value, cvar->buffer + STK_buffer[2], STK_buffer[3]);
					set_variable(&_var_list, STK_buffer[0], STK_buffer[3], value);
					m_free(value);
					goto next_instruction;
#endif
#if (IVAS_EXECUTE_LOCAL_SCRIPT_ALLOCATED)
				case VAS_EXECUTE_LOCAL_SCRIPT:
					len = STK_buffer[0]; 
					value = m_alloc(len);
					if(len != 0) {
						memcpy(value, STK_buffer + 1, len);
						len = VAS_substitute(FALSE, value, len, STK_buffer + 1, 255);
						m_free(value);
						value = m_alloc(sizeof(fs_handle));
						_select(value, FID_MF);
						_select(value, FID_WIB);
						if(_select(value, FID_WSCRADDR) <= 0x9F00) goto exit_execute_local;
						k = 2;
						//use absolute addressing on EFscript_address
						while(1) {
							if(_readbin(value, k, STK_buffer + 5, 6)!= APDU_SUCCESS) goto exit_execute_local;
							if(memcmp(STK_buffer + 1, STK_buffer + 5, 4) == 0) goto start_execute_local;
							k += 6;
						}
						start_execute_local:
						VAS_plugin_init(*((uint16 *)(STK_buffer + 9)));			//change to plugin execution
					}
					exit_execute_local:
					m_free(value);
					goto next_instruction;
#endif
#if (IVAS_ADD_SUBSTRACT_ALLOCATED)
				case VAS_ADD_SUBSTRACT:
					//allocate source operand first
					if(size > 2) { 		//use source variable
						cvar = get_variable(&_var_list, STK_buffer[1]);		//get destination variable
						len = cvar->length;								//calculating temporary variable length based on destination variable
						value = m_alloc(cvar->length);
						memset(value, 0, cvar->length);					//zeroing variable for padding
						//source variable
						cvar = get_variable(&_var_list, STK_buffer[2]);		//get source variable
						j = cvar->length;								//calculating source variable size
						for(k = (len - 1); k != 0, j != 0; k--, j--) {	//copying original value (source)
							value[k] = cvar->buffer[k]; 		
						}
						//destination variable
						cvar = get_variable(&_var_list, STK_buffer[1]);
						k = len - 1;
					} else {
						cvar = get_variable(&_var_list, STK_buffer[1]);
						value = m_alloc(cvar->length);
						memset(value, 0, cvar->length);
						value[cvar->length - 1] = 1;
						len = cvar->length;
						k = len - 1;
					}
					//operation
					attr = 0;											//use attr as carry/borrow
					while(k != 0) {
						if(STK_buffer[0] & 0x01) {		//substraction
							status = cvar->buffer[k] - (value[k] + attr);	 	//use status as temporary variable
						} else {   					//addition
							status = cvar->buffer[k] + (value[k] + attr);		//use status as temporary variable
						}
						attr = 0;						//clear carry/borrow flag	
						if(status > 0xFF) {				//overflow to carry/borrow flag
							attr = 1;			   
						}
						cvar->buffer[k] = (uchar)status; 			
					}
					m_free(value);
					goto next_instruction;
#endif
#if (IVAS_CONVERT_VARIABLE_ALLOCATED)
				case VAS_CONVERT_VARIABLE:	  		//NOT IMPLEMENTED
					goto next_instruction;
#endif
#if (IVAS_GROUP_UNGROUP_VARIABLE_ALLOCATED)
				case VAS_GROUP_UNGROUP_VARIABLE:
					attr = STK_buffer[k++];
					j = STK_buffer[k++];								//j = variable id (grouped variable)
					if(attr & 1) {									//ungroup variable
						cvar = get_variable(&_var_list, j);			//get source variable
						j = 0; 										//j is unused anymore
						while(j < cvar->length && k < size) { 		//STK_buffer[k] = variable id
							len = cvar->buffer[j++];
							set_variable(&_var_list, STK_buffer[k++], len, cvar->buffer + j);
							j += len;
						}
					} else { 				//group variable
						len = 0;
						while(k < size) {
							cvar = get_variable(&_var_list, STK_buffer[k++]);		//get source variable
							STK_buffer[size + len++] = cvar->length;					//insert L to destination STK_buffer
							memcpy(STK_buffer + size + len, cvar->buffer, cvar->length);
							len += cvar->length;
						}
						//set new variable										 	
						set_variable(&_var_list, j, len, STK_buffer + size);
					}
					goto next_instruction;
#endif
#if (IVAS_SWAP_NIBBLES_ALLOCATED)
				case VAS_SWAP_NIBBLES:
					cvar = get_variable(&_var_list, STK_buffer[1]);
					for(len = 0; len < cvar->length; len++) {
						temp[0] = cvar->buffer[len];
						cvar->buffer[len] = ((cvar->buffer[len] >> 4) | (temp[0] << 4));
					}
					goto next_instruction;
#endif
#if (IVAS_PLUGIN_ALLOCATED)
				case VAS_PLUGIN:
					//VAS_decode_plugin(STK_buffer, size);
					vas_cvid = STK_buffer[k++];	 			//variable out
					len = STK_buffer[k++];				//length of plugin name
					
					//every terminal response should be route to activated plugin
					//enter plugin mode
					vas_mode = VAS_MODE_PLUGIN;
					
					k += len;	
					len = STK_buffer[k++]; 				//length of input string
					//allocate-copy, substitute variable reference, reallocate-copy
					vas_cistr = (uchar *)m_alloc(len);
					memcpy(vas_cistr, STK_buffer + k, len + 1);
					len = VAS_substitute(FALSE, vas_cistr, len, STK_buffer + k, 128);
					m_free(vas_cistr);
					vas_cistr = (uchar *)m_alloc(len + 1);
					vas_cistr[0] = len;
					memcpy(vas_cistr + 1, STK_buffer + k, len);

#if	VAS_MDIL_ALLOCATED
					if(VAS_strcmp(STK_buffer + 2, "MDIL", len) == TRUE) {
						vas_mode |= VAS_PLUGIN_MDIL; 
						j = mdil_decode();
						if(j == 0) goto next_instruction;
						goto exit_decode;
					}
#endif
				
#if VAS_VDIL_ALLOCATED
					if(VAS_strcmp(STK_buffer + 2, "VDIL", len) == TRUE) { 
						vas_mode |= VAS_PLUGIN_VDIL;
						j = vdil_decode();
						if(j == 0) goto next_instruction;
						goto exit_decode;
					}
#endif
				
#if VAS_DITR_ALLOCATED
					if(VAS_strcmp(STK_buffer + 2, "DITR", len) == TRUE) {
						vas_mode |= VAS_PLUGIN_DITR; 
						j = ditr_decode();
						if(j == 0) goto next_instruction;
						goto exit_decode;	
					}
#endif
				
#if VAS_MASL_ALLOCATED
					if(VAS_strcmp(STK_buffer + 2, "MASL", len) == TRUE) {
						vas_mode |= VAS_PLUGIN_MASL;
						j = masl_decode();
						if(j == 0) goto next_instruction;
						goto exit_decode;	
					}
#endif
				
#if VAS_DECR_ALLOCATED
					if(VAS_strcmp(STK_buffer + 2, "DECR", len) == TRUE) {
						vas_mode |= VAS_PLUGIN_DECR;
						j = decr_decode();
						if(j == 0) goto next_instruction;
						goto exit_decode;	
					}
#endif
				
#if VAS_ENCR_ALLOCATED
					if(VAS_strcmp(STK_buffer + 2, "ENCR", len) == TRUE) {
						vas_mode |= VAS_PLUGIN_ENCR; 
						j = encr_decode();
						if(j == 0) goto next_instruction;
						goto exit_decode;	
					}
#endif

#if	VAS_ICCID_ALLOCATED
					if(VAS_strcmp(STK_buffer + 2, "*ICCID", len) == TRUE) {
						vas_mode |= VAS_PLUGIN_ICCID; 
						j = iccid_decode();
						if(j == 0) goto next_instruction;
						goto exit_decode;	
					}
#endif
					//no plugin found, exit plugin mode, skip this instruction, release memory
					VAS_exit_plugin();
					break;
#endif
				/* Administrative Commands */
				case VAS_INSTALL_PLUGIN	:
				case VAS_REMOVE_PLUGIN:
				case VAS_SET_SCRIPT_TRIGGER_MODE:
				case VAS_GET_SCRIPT_TRIGGER_MODE:
				case VAS_GET_MENU:
				case VAS_SCRIPT_INFO:
				default:
					break;
			}
		} else {
			vas_skip--;
		}
	}
	exit_decode:
	/*if(vas_PC == vas_PL && vas_state != VAS_STATE_WAIT_RESPONSE) {
			
	}*/
	if(j != 0) {
		status = SAT_file_flush(j);
	}
	return status;
}

uchar VAS_strcmp(uchar * a, uchar * b, uchar len) _REENTRANT_ {
	uchar i = 0;
	uchar c;
	for(i=0;i<len;i++) {
		c = b[i] - 0x20; 			//to uppercase (capitalization)
		if(a[i] != b[i] && a[i] != c) return FALSE; 
	}
	return TRUE;	
}

#endif