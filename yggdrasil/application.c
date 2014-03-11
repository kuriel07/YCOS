#include "..\defs.h"
#include "..\asgard\file.h"
#include "..\asgard\fs.h"
#include "..\midgard\midgard.h"
#include "..\liquid.h"
#include "..\framework\vas.h"
#include "..\framework\sms.h"
#include "..\framework\dcs.h" 
#include "..\misc\mem.h"
#include "application.h"
#include <string.h>

#define TSEL_MENU_DAFTAR		1
#define TSEL_MENU_MUTASI		2
#define TSEL_MENU_STATUS		3
uchar tsel_menu = TSEL_MENU_DAFTAR;
#define TSEL_LANG_BAHASA		0x0B
#define TSEL_LANG_ENGLISH		0x0E
uchar tsel_lang	= TSEL_LANG_BAHASA;
#define TSEL_NSP_AKTIFASI		1
#define TSEL_NSP_KIRIM			2
#define TSEL_NSP_LANGGANAN		3
#define TSEL_NSP_PERPANJANGAN	4
#define TSEL_NSP_BANTUAN		5
#define TSEL_NSP_KIRIM_TUJUAN	6
uchar tsel_nsp = TSEL_NSP_AKTIFASI;
#define TSEL_PULSA_PHONEBOOK	1
#define TSEL_PULSA_NOMOR		2
uchar tsel_pulsa = TSEL_PULSA_PHONEBOOK;


#define TSEL_UDS_NAMA			20
#define TSEL_UDS_KLAHIR			15
#define TSEL_UDS_TLAHIR			10
#define TSEL_UDS_ALAMAT			50
#define TSEL_UDS_KOTA			15
#define TSEL_UDS_KODEPOS		5
#define TSEL_UDS_ID				16
struct tsel_user_data {
	uchar nama[TSEL_UDS_NAMA];
	uchar s0;
	uchar kota_lahir[TSEL_UDS_KLAHIR];
	uchar s1;
	uchar tgl_lahir[TSEL_UDS_TLAHIR];
	uchar s2;
	uchar alamat[TSEL_UDS_ALAMAT];
	uchar s3;
	uchar kota[TSEL_UDS_KOTA];
	uchar s4;
	uchar kode_pos[TSEL_UDS_KODEPOS];
	uchar s5;
	uchar tipe_id;
	uchar s6;
	uchar no_id[TSEL_UDS_ID];
	uchar s7;
	uchar sex;
	uchar s8;
	uchar agama;
	uchar s9;
	uchar hobi;
	uchar s10;
	uchar perokok;	
};

typedef struct tsel_user_data tsel_user_data;

uchar remove_null(uchar * buffer_in, uchar size, uchar * buffer_out) _REENTRANT_ {
	uchar i;
	uchar j = 0;
	for(i=0;i<size;i++) {
		if(buffer_in[i] != 0) {
			buffer_out[j++] = buffer_in[i];	
		}
	}
	return j;
}

#define USR_CONF_REG_FLAG		0
#define USR_CONF_SEND_REG		1
#define USR_CONF_CUR_LANG		2
#define USR_CONF_CHNG_LANG		3

extern uchar _sat_buffer[];
uint16 _default_menu_offset = 0;
uchar is_sending_reg = FALSE;	  
uchar * x_variable = NULL;
uchar * y_variable = NULL;
uint16 send_registration(uchar mode) _REENTRANT_ {
	fs_handle cb_fs;
	uchar * temp;
	uchar len;
	uchar res;
	uchar i = 0;
	file_select(&cb_fs, FID_MF);
	file_select(&cb_fs, FID_LIQUID);
	#if 1
	file_select(&cb_fs, 0x6F02);		//user data
	switch(mode) {
		case TSEL_MENU_DAFTAR:
			memcpy(STK_buffer, "STK ", 4);
			i = 4;
			goto format_registration;
		case TSEL_MENU_MUTASI:	
			memcpy(STK_buffer, "Mutasi ", 7);
			i = 7;
			format_registration:
			file_readbin(&cb_fs, 0, STK_buffer + i, sizeof(tsel_user_data));
			((tsel_user_data *)(STK_buffer + i))->s0 = '|';
			((tsel_user_data *)(STK_buffer + i))->s1 = '|';
			((tsel_user_data *)(STK_buffer + i))->s2 = '|';
			((tsel_user_data *)(STK_buffer + i))->s3 = '|';
			((tsel_user_data *)(STK_buffer + i))->s4 = '|';
			((tsel_user_data *)(STK_buffer + i))->s5 = '|';
			((tsel_user_data *)(STK_buffer + i))->s6 = '|';
			((tsel_user_data *)(STK_buffer + i))->s7 = '|';
			((tsel_user_data *)(STK_buffer + i))->s8 = '|';
			((tsel_user_data *)(STK_buffer + i))->s9 = '|';
			((tsel_user_data *)(STK_buffer + i))->s10 = '|';
			res = remove_null(STK_buffer, sizeof(tsel_user_data) + i, STK_buffer);
			temp = m_alloc(res);
			memcpy(temp, STK_buffer, res);
			len = encode_SMSTPDU(SMS_TYPE_NOHEADER | SMS_TYPE_SUBMIT, 0x7F, 0, res, "\x03\x81\x44\x44", temp, STK_buffer + 1);
			//len = encode_SMSTPDU(SMS_TYPE_NOHEADER | SMS_TYPE_PACKED7 | SMS_TYPE_SUBMIT, res, "\x03\x81\x44\x44", temp, STK_buffer + 1);	//SMS submit
			STK_buffer[0] = len;
			m_free(temp);
			break;
		case TSEL_MENU_STATUS:
			memcpy(STK_buffer, "Cek", 3);
			res = 3;
			temp = m_alloc(res);
			memcpy(temp, STK_buffer, res);
			len = encode_SMSTPDU(SMS_TYPE_NOHEADER | SMS_TYPE_SUBMIT, 0x7F, 0, res, "\x03\x81\x44\x44", temp, STK_buffer + 1);
			//len = encode_SMSTPDU(SMS_TYPE_NOHEADER | SMS_TYPE_PACKED7 | SMS_TYPE_SUBMIT, res, "\x03\x81\x44\x44", temp, STK_buffer + 1);	//SMS submit
			m_free(temp);
			STK_buffer[0] = len;
			break;
	}
	temp = m_alloc(9);
	if(tsel_lang == TSEL_LANG_BAHASA) {
		memcpy(temp + 1, "Mengirim", 8);
		temp[0] = 8;
	} else {	
		memcpy(temp + 1, "Sending", 7);
		temp[0] = 7;
		//SAT_printf("cdlm", (SEND_SHORT_MESSAGE & 0x7F), STK_DEV_ME, temp, STK_buffer);
	} 
	SAT_printf("cdlm", (SEND_SHORT_MESSAGE & 0x7F), STK_DEV_ME, temp, STK_buffer);
	m_free(temp);
	return SAT_status();
	//is_sending_reg = TRUE;
	//result = CALLBACK_TAMPER_RESULT | CALLBACK_STAY_NODE;
	#endif
}

uchar is_leap_year(uint16 year) _REENTRANT_ {
	if((year % 4) == 0) {
		if((year % 100) == 0) { 
			if((year % 400) == 0) {
				return TRUE;
			} else {
				return FALSE; 
			} 
		} else { 
			return TRUE; 
		}
	} else {
		return FALSE;		
	}
}

uchar is_date_valid(uchar * buffer) _REENTRANT_ {
	uchar tgl;
	uchar bln;
	uint16 thn;
	tgl = (buffer[1] & 0x0F);
	tgl += ((buffer[0] & 0x0F) * 10);
	bln = (buffer[3] & 0x0F);
	bln += ((buffer[2] & 0x0F) * 10);
	thn = (buffer[7] & 0x0F);  
	thn += ((buffer[6] & 0x0F) * 10);
	thn += ((buffer[5] & 0x0F) * 100);
	thn += ((buffer[4] & 0x0F) * 1000);
	if(tgl == 0) return 1;
	if(thn == 0) return 1;
	if(bln == 0) return 1;
	switch(bln) {
		case 1:
		case 3:
		case 5:
		case 7:
		case 8:
		case 10:
		case 12:		//31 hari
			if(tgl > 31) return 1;
			break;
		case 4:
		case 6:
		case 9:
		case 11:  		//30 hari
			if(tgl > 30) return 1;
			break;
		case 2:		   	//28/29 hari
			if(is_leap_year(thn)) {
				if(tgl > 29) return 1;
			} else {
			 	if(tgl > 28) return 1;
			}
			break;
	}
	return 0;
}

uint16 user_packet_decode(uchar * buffer, uchar * address, uchar length) _REENTRANT_  {
	if(memcmp(buffer, "$REG$", 5) == 0) {
		return send_registration(1);
	}
	return APDU_SUCCESS;
}

uchar user_callback(uchar state, uchar tag, uchar size, uchar * buffer) _REENTRANT_ {
	uchar result = CALLBACK_SUCCESS;

	uchar len;
	//uchar res[51];		//50 bytes
	//uchar * address;
	//uchar * smsc;
	uint16 i;
	uchar * temp;
	uchar * sex;
	uchar * agama;
	uchar * hobi;
	uchar * nama;
	uchar * klahir;
	uchar * tlahir;
	fs_handle cb_fs;
//	uchar i;
	uchar res;
	uchar dcs;
	extern uint16 _stk_menu_offset;
	extern uint16 _stk_menu_current;
	tag |= 0x80; 
	//#define CALLBACK_STAY_NODE		(1<<5)
	//#define CALLBACK_TAMPER_RESULT	(1<<6)
	//#define CALLBACK_FORCE_END		(1<<7)
	//result = (CALLBACK_TAMPER_RESULT);
	switch(tag) {
		case STK_TAG_ITEM_ID:
			file_select(&cb_fs, FID_MF);	
			file_select(&cb_fs, FID_LIQUID);
			file_select(&cb_fs, 0x6F02);		//user data
			switch(state) {
				case 0x80: 			  //select registrasi
					tsel_menu = buffer[0];
					if(tsel_menu == TSEL_MENU_STATUS) {	
						//send sms
						send_registration(TSEL_MENU_STATUS);
						tsel_menu = TSEL_MENU_DAFTAR;	 
						result = (CALLBACK_TAMPER_RESULT | CALLBACK_FORCE_END);
						//break;
					}// else {
					
					_default_menu_offset = _stk_menu_offset;
					break;
				case 0x11:
				case 0x01: 	 		//tipe identitas
					res = buffer[0];
					file_readbin(&cb_fs, 0, STK_buffer, sizeof(tsel_user_data));
					((tsel_user_data *)STK_buffer)->tipe_id = (0x30 | res);
					file_writebin(&cb_fs, 0, STK_buffer, sizeof(tsel_user_data));
					break;
				case 0x19:
				case 0x09:		   		//sex
					res = buffer[0];
					file_readbin(&cb_fs, 0, STK_buffer, sizeof(tsel_user_data));
					((tsel_user_data *)STK_buffer)->sex = (0x30 | res);
					file_writebin(&cb_fs, 0, STK_buffer, sizeof(tsel_user_data));
					break;
				case 0x1A:
				case 0x0A:			  //agama
					res = buffer[0];
					file_readbin(&cb_fs, 0, STK_buffer, sizeof(tsel_user_data));
					((tsel_user_data *)STK_buffer)->agama = (0x30 | res);
					file_writebin(&cb_fs, 0, STK_buffer, sizeof(tsel_user_data));
					break;
				case 0x1B:
				case 0x0B:				//hobi
					res = buffer[0];
					file_readbin(&cb_fs, 0, STK_buffer, sizeof(tsel_user_data));
					((tsel_user_data *)STK_buffer)->hobi = (0x30 | res);
					file_writebin(&cb_fs, 0, STK_buffer, sizeof(tsel_user_data));
					break;
				case 0x1C:
				case 0x0C:			//perokok
					res = buffer[0];
					file_readbin(&cb_fs, 0, STK_buffer, sizeof(tsel_user_data));
					((tsel_user_data *)STK_buffer)->perokok = (0x30 | res);
					file_writebin(&cb_fs, 0, STK_buffer, sizeof(tsel_user_data));
					break;
				case 0x20:			  //NSP1212
					tsel_nsp = buffer[0];
					if(tsel_nsp == TSEL_NSP_BANTUAN) {		//send ussd immedietly
						memcpy(STK_buffer + 2, "*121#", 5);
						res = 6;
						STK_buffer[0] = res;
						STK_buffer[1] = 4;	   
						temp = m_alloc(9);
						if(tsel_lang == TSEL_LANG_BAHASA) {
							memcpy(temp + 1, "Mengirim", 8);
							temp[0] = 8;
						} else {	
							memcpy(temp + 1, "Sending", 7);
							temp[0] = 7;
						}  
						STK_buffer[1] = 0;
						STK_buffer[0] = encode_827(STK_buffer + 2, STK_buffer + 2, res, 0) + 1;
						SAT_printf("cdlu", (SEND_USSD & 0x7F), STK_DEV_NETWORK, temp, STK_buffer);
						m_free(temp);
						result = (CALLBACK_TAMPER_RESULT | CALLBACK_FORCE_END);
						//break;	
					}
					break;
				case 0x40:	   		//Transfer pulsa
					tsel_pulsa = buffer[0];	
					if(tsel_pulsa == TSEL_PULSA_NOMOR) {
						temp = m_alloc(9);
						if(tsel_lang == TSEL_LANG_BAHASA) {
							memcpy(temp + 1, "\x04Nomor", 6);
							temp[0] = 6;
						} else {	
							memcpy(temp + 1, "\x04Number", 7);
							temp[0] = 7;
						} 
						SAT_printf("cdx", (GET_INPUT & 0x7F), STK_DEV_ME, temp);
						m_free(temp);
						result = (CALLBACK_TAMPER_RESULT);	
					}
					break;
				case 0xE0:						//change language (popup)
					res = buffer[0];
					file_select(&cb_fs, FID_MF);	
					file_select(&cb_fs, FID_LIQUID);
					if(res == 1) {
						file_select(&cb_fs, 0x6F2B);
					} else { 
						file_select(&cb_fs, 0x6F2E);
					}
					temp = m_alloc(sizeof(fs_handle)); 
					file_select(temp, FID_MF);	
					file_select(temp, FID_LIQUID);
					file_select(temp, FID_STKMENU);
					nama = file_get_current_header(temp);
					if(((ef_header *)nama)->size > 0x80) {
						for(i=0;i<((ef_header *)nama)->size - 0x80;i+=0x80) {
							file_readbin(&cb_fs, i, STK_buffer, 0x80);
							file_writebin(temp, i, STK_buffer, 0x80);
						}
					}
					file_readbin(&cb_fs, i, STK_buffer, ((ef_header *)nama)->size - i);
					file_writebin(temp, i, STK_buffer, ((ef_header *)nama)->size - i);
					file_readbin(temp, 0, STK_buffer, sizeof(stk_config));
					m_free(nama);
					m_free(temp);
					//reset engine 
					_stk_menu_offset = ((stk_config *)STK_buffer)->sibling;
					_stk_menu_current = 0;	 		
					break;
				case 0xE1: 						//change language menu 
					res = STK_buffer[0];
					file_select(&cb_fs, 0x6F01);		//user config 	
					file_readbin(&cb_fs, 0, STK_buffer, 0x0C);
					if(res == 1) {
						STK_buffer[USR_CONF_CHNG_LANG] = TSEL_LANG_BAHASA;			
					} else { 
						STK_buffer[USR_CONF_CHNG_LANG] = TSEL_LANG_ENGLISH;
					}  	
					file_writebin(&cb_fs, 0, STK_buffer, 0x0C);
					break;
				default: break;
			}

			//display summaries
			if((state & 0xF0) == 0x10) {   		//display user data
				//if((state & 0x0F) != 0) {
				//tamper result, display user information, next node  
				file_readbin(&cb_fs, 0, STK_buffer, sizeof(tsel_user_data));
				nama = m_alloc(TSEL_UDS_NAMA);
				klahir = m_alloc(TSEL_UDS_KLAHIR);
				tlahir = m_alloc(TSEL_UDS_TLAHIR + 3);
				sex = m_alloc(10);
				agama = m_alloc(10);
				hobi = m_alloc(10);
				memset(nama, 0, TSEL_UDS_NAMA);
				memset(klahir, 0, TSEL_UDS_KLAHIR);
				memset(tlahir, 0, TSEL_UDS_TLAHIR);
				memset(sex, 0, 10);
				memset(agama, 0, 10);
				memset(hobi, 0, 10); 
				memcpy(nama, ((tsel_user_data *)STK_buffer)->nama, TSEL_UDS_NAMA); 
				memcpy(klahir, ((tsel_user_data *)STK_buffer)->kota_lahir, TSEL_UDS_KLAHIR);
				memcpy(tlahir+2, ((tsel_user_data *)STK_buffer)->tgl_lahir, TSEL_UDS_TLAHIR);
				tlahir[0] = tlahir[2];
				tlahir[1] = tlahir[3];
				tlahir[2] = '-';
				tlahir[3] = tlahir[4];
				tlahir[4] = tlahir[5];
				tlahir[5] = '-';
				tlahir[TSEL_UDS_TLAHIR + 2] = 0;
				if(tsel_lang == TSEL_LANG_BAHASA) {
					file_select(&cb_fs, 0x6FB9);
					file_readrec(&cb_fs, (((tsel_user_data *)STK_buffer)->sex -1) & 0x0F, sex, 10);   
					file_select(&cb_fs, 0x6FBA);
					file_readrec(&cb_fs, (((tsel_user_data *)STK_buffer)->agama -1) & 0x0F, agama, 10);
					if(((tsel_user_data *)STK_buffer)->hobi != 0xFF) {
						file_select(&cb_fs, 0x6FBB); 
						file_readrec(&cb_fs, (((tsel_user_data *)STK_buffer)->hobi -1) & 0x0F, hobi, 10);
						ls_printf(STK_buffer + 2, "Data yang Anda masukan adalah:\r\nNama Lengkap:%s\r\nJenis Kelamin:%s\r\nTempat Lahir:%s\r\nTgl. Lahir:%s\r\nAgama:%s\r\nHobi:%s\r\nApakah data-data tersebut benar?", nama, sex, klahir, tlahir, agama, hobi);
					} else {
						ls_printf(STK_buffer + 2, "Data yang Anda masukan adalah:\r\nNama Lengkap:%s\r\nJenis Kelamin:%s\r\nTempat Lahir:%s\r\nTgl. Lahir:%s\r\nAgama:%s\r\nApakah data-data tersebut benar?", nama, sex, klahir, tlahir, agama);
					}
				} else { 
					file_select(&cb_fs, 0x6FE9);
					file_readrec(&cb_fs, (((tsel_user_data *)STK_buffer)->sex -1) & 0x0F, sex, 10);   
					file_select(&cb_fs, 0x6FEA);
					file_readrec(&cb_fs, (((tsel_user_data *)STK_buffer)->agama -1) & 0x0F, agama, 10);
					if(((tsel_user_data *)STK_buffer)->hobi != 0xFF) {
						file_select(&cb_fs, 0x6FEB); 
						file_readrec(&cb_fs, (((tsel_user_data *)STK_buffer)->hobi -1) & 0x0F, hobi, 10);
						ls_printf(STK_buffer + 2, "Your data summaries are:\r\nFull Name:%s\r\nSex:%s\r\nPlace of birth:%s\r\nDate of Birth:%s\r\nReligion:%s\r\nHobby:%s\r\nare the data correct?", nama, sex, klahir, tlahir, agama, hobi);
					} else {
						ls_printf(STK_buffer + 2, "Your data summaries are:\r\nFull Name:%s\r\nSex:%s\r\nPlace of birth:%s\r\nDate of Birth:%s\r\nReligion:%s\r\nare the data correct?", nama, sex, klahir, tlahir, agama);
					}
				}
				file_select(&cb_fs, 0x6F02);
				m_free(hobi); 
				m_free(agama);
				m_free(sex);
				m_free(tlahir);
				m_free(klahir);
				m_free(nama);
				//strcpy(STK_buffer + 2, "result tamper data\x0"); 
				res = strlen(STK_buffer + 2);
				///file_writebin(&cb_fs, 0, STK_buffer + 2, res); 
				STK_buffer[0] = res + 1;
				STK_buffer[1] = 0x04;
				i = 0;
				//i += SAT_command(_sat_buffer + i, 0x01, (GET_INKEY & 0x7F), 4);
				//i += SAT_device(_sat_buffer + i, STK_DEV_SIM, STK_DEV_ME);
				//i += SAT_file_push(i, (STK_TAG_TEXT_STRING & 0x7F), res + 1, STK_buffer);
				//SAT_file_flush(i);
				SAT_printf("cdx", (DISPLAY_TEXT & 0x7F), STK_DEV_ME, STK_buffer);
				result = CALLBACK_TAMPER_RESULT;
				//}	
			}
			break;
		case STK_TAG_TEXT_STRING:
			size = size - 1;  
			dcs = buffer[0];
			if(dcs == 0) {
				size = decode_728(buffer + 1, buffer, size);
				//buffer[0] = size;
			} else {
			   	memcpy(buffer, buffer + 1, size);
			}
			file_select(&cb_fs, FID_MF);	
			file_select(&cb_fs, FID_LIQUID);
			file_select(&cb_fs, 0x6F02);		//user data
			switch(state) {
				case 0x12:		  //no identitas
				case 0x02:
					temp = m_alloc(size + 1);
					memcpy(temp, buffer, size);
					temp[size] = 0;	
					file_readbin(&cb_fs, 0, STK_buffer, sizeof(tsel_user_data));
					memset(((tsel_user_data *)STK_buffer)->no_id, 0, TSEL_UDS_ID);
					memcpy(((tsel_user_data *)STK_buffer)->no_id, temp, size + 1);
					file_writebin(&cb_fs, 0, STK_buffer, sizeof(tsel_user_data)); 
					m_free(temp);
					break;
				case 0x13:
				case 0x03:	 		//nama
					temp = m_alloc(size + 1);
					memcpy(temp, buffer, size);
					temp[size] = 0;
					file_readbin(&cb_fs, 0, STK_buffer, sizeof(tsel_user_data));
					memset(((tsel_user_data *)STK_buffer)->nama, 0, TSEL_UDS_NAMA);
					memcpy(((tsel_user_data *)STK_buffer)->nama, temp, size + 1);
					file_writebin(&cb_fs, 0, STK_buffer, sizeof(tsel_user_data));
					m_free(temp);
					break;
				case 0x14:
				case 0x04:	  		//kota lahir
					temp = m_alloc(size + 1);
					memcpy(temp, buffer, size);
					temp[size] = 0;
					file_readbin(&cb_fs, 0, STK_buffer, sizeof(tsel_user_data));
					memset(((tsel_user_data *)STK_buffer)->kota_lahir, 0, TSEL_UDS_KLAHIR);
					memcpy(((tsel_user_data *)STK_buffer)->kota_lahir, temp, size + 1);
					file_writebin(&cb_fs, 0, STK_buffer, sizeof(tsel_user_data));
					m_free(temp);
					break;
				case 0x15:
				case 0x05:
					if(is_date_valid(buffer) == 0) {
						temp = m_alloc(size + 1);
						memcpy(temp, buffer, size);
						temp[size] = 0;
						file_readbin(&cb_fs, 0, STK_buffer, sizeof(tsel_user_data));
						memset(((tsel_user_data *)STK_buffer)->tgl_lahir, 0, TSEL_UDS_TLAHIR);
						memcpy(((tsel_user_data *)STK_buffer)->tgl_lahir, temp, size + 1);
						file_writebin(&cb_fs, 0, STK_buffer, sizeof(tsel_user_data));
						m_free(temp);
					} else {
						result = (CALLBACK_STAY_NODE | 1);
					}
					break;
				case 0x16:
				case 0x06:	   			//alamat
					temp = m_alloc(size + 1);
					memcpy(temp, buffer, size);
					temp[size] = 0;
					file_readbin(&cb_fs, 0, STK_buffer, sizeof(tsel_user_data));  
					memset(((tsel_user_data *)STK_buffer)->alamat, 0, TSEL_UDS_ALAMAT);
					memcpy(((tsel_user_data *)STK_buffer)->alamat, temp, size + 1);
					file_writebin(&cb_fs, 0, STK_buffer, sizeof(tsel_user_data));
					m_free(temp);
					break;
				case 0x17:
				case 0x07:				//kota
					temp = m_alloc(size + 1);
					memcpy(temp, buffer, size);
					temp[size] = 0;
					file_readbin(&cb_fs, 0, STK_buffer, sizeof(tsel_user_data));
					memset(((tsel_user_data *)STK_buffer)->kota, 0, TSEL_UDS_KOTA);
					memcpy(((tsel_user_data *)STK_buffer)->kota, temp, size + 1);
					file_writebin(&cb_fs, 0, STK_buffer, sizeof(tsel_user_data));
					m_free(temp);
					break;
				case 0x18:
				case 0x08:		 		//kode pos
					temp = m_alloc(size + 1);
					memcpy(temp, buffer, size);
					temp[size] = 0;
					file_readbin(&cb_fs, 0, STK_buffer, sizeof(tsel_user_data));
					memset(((tsel_user_data *)STK_buffer)->kode_pos, 0, TSEL_UDS_KODEPOS);
					memcpy(((tsel_user_data *)STK_buffer)->kode_pos, temp, size + 1);
					file_writebin(&cb_fs, 0, STK_buffer, sizeof(tsel_user_data));
					m_free(temp);
					break;
				case 0x21:				//NSP (input) kode nada
					switch(tsel_nsp) {
						case TSEL_NSP_KIRIM:
							if(x_variable != NULL) {
								m_free(x_variable);
								x_variable = NULL;
							}
							x_variable = m_alloc(size + 1);
							x_variable[0] = size;
							memcpy(x_variable + 1, buffer, size);
							temp = m_alloc(21);
							if(tsel_lang == TSEL_LANG_BAHASA) {
								memcpy(temp + 1, "\x04Nomor Tujuan", 13);
								temp[0] = 13;
							} else {	
								memcpy(temp + 1, "\x04Destination Number", 19);
								temp[0] = 19;
							} 
							i = 0;
							i += SAT_command(STK_buffer + i, 1, (GET_INPUT & 0x7F), 0);
							i += SAT_device(STK_buffer + i, STK_DEV_SIM, STK_DEV_ME);
							i += SAT_push(STK_buffer + i, (STK_TAG_TEXT_STRING & 0x7F), temp[0], temp + 1);
							i += SAT_push(STK_buffer + i, (STK_TAG_RESPONSE_LENGTH & 0x7F), 2, "\x01\x0C");
							//SAT_printf("cdx", (GET_INPUT & 0x7F), STK_DEV_ME, temp);
							SAT_response(STK_buffer, i);
							m_free(temp);
							result = (CALLBACK_TAMPER_RESULT | CALLBACK_STAY_NODE);
							tsel_nsp = TSEL_NSP_KIRIM_TUJUAN;
							break;
						case TSEL_NSP_KIRIM_TUJUAN:
							if(y_variable != NULL) {
								m_free(y_variable);
								y_variable = NULL;
							}
							y_variable = m_alloc(size + 1);
							y_variable[0] = size;
							memcpy(y_variable + 1, buffer, size);
							buffer[1] = 0x04;
							memcpy(buffer + 2, "*121*", 5);	 				//*121*
							res = x_variable[0];
							memcpy(buffer + 7, x_variable + 1, res);	   //*121*XXXXXX
							buffer[7 + res] = '*';						   //*121*XXXXXX*
							memcpy(buffer + 8 + res, y_variable + 1, y_variable[0]); //*121*XXXXXX*YYYYYY
							res = 8 + res + y_variable[0];			 
							buffer[res++] = '#'; 					 //*121*XXXXXX*YYYYYY#
							buffer[res] = 0;
							buffer[0] = res;  
							temp = m_alloc(10);
							if(tsel_lang == TSEL_LANG_BAHASA) {
								memcpy(temp + 1, "Mengirim", 8);
								temp[0] = 8;
							} else {	
								memcpy(temp + 1, "Sending", 7);
								temp[0] = 7;
							}
							buffer[1] = 0;
							buffer[0] = encode_827(buffer + 2, buffer + 2, res, 0);
							SAT_printf("cdlu", (SEND_USSD & 0x7F), STK_DEV_NETWORK, temp, buffer);
							m_free(temp);
							m_free(y_variable); y_variable = NULL;
							m_free(x_variable);	x_variable = NULL;
							result = CALLBACK_TAMPER_RESULT | CALLBACK_FORCE_END;
							break;
						default:
							if(x_variable != NULL) {
								m_free(x_variable);
								x_variable = NULL;
							}
							x_variable = m_alloc(size + 1);
							x_variable[0] = size;
							memcpy(x_variable + 1, buffer, size);
							buffer[1] = 0x04;
							memcpy(buffer + 2, "*121*", 5);	 				//*121*
							res = x_variable[0];
							memcpy(buffer + 7, x_variable + 1, res);	   //*121*XXXXXX
							res = 7 + res;			 
							buffer[res++] = '#'; 					 //*121*XXXXXX#	  
							buffer[res] = 0;
							buffer[0] = res;  
							temp = m_alloc(10);
							if(tsel_lang == TSEL_LANG_BAHASA) {
								memcpy(temp + 1, "Mengirim", 8);
								temp[0] = 8;
							} else {	
								memcpy(temp + 1, "Sending", 7);
								temp[0] = 7;
							}
							buffer[1] = 0;
							buffer[0] = encode_827(buffer + 2, buffer + 2, res, 0);
							SAT_printf("cdlu", (SEND_USSD & 0x7F), STK_DEV_NETWORK, temp, buffer);
							m_free(temp);
							m_free(x_variable);	x_variable = NULL;
							result = CALLBACK_TAMPER_RESULT | CALLBACK_FORCE_END;
							break;
					}
					break;
				case 0x41:				//transfer pulsa, input nomor tujuan/nama
					file_select(&cb_fs, FID_MF);	  
					file_select(&cb_fs, 0x7F10);
					file_select(&cb_fs, 0x6F3A);
					if(tsel_pulsa == TSEL_PULSA_PHONEBOOK) { 				//input nama phonebook
						i = file_seek(&cb_fs, 1, buffer, buffer, size);							//offset 14	
						if((i & 0xFF00) == APDU_SUCCESS_RESPONSE) {
							temp = file_get_current_header(&cb_fs);
							file_readrec(&cb_fs, (i & 0x00FF), STK_buffer, ((ef_header *)temp)->rec_size);
							m_free(temp);
							if(x_variable != NULL) {
								m_free(x_variable);
								x_variable = NULL;
							}
							x_variable = m_alloc((STK_buffer[14] * 2) + 1);
							i = 1;
							for(res = 15, i = 0;res < (15 + STK_buffer[14]);res++) {
								dcs = (STK_buffer[res] & 0x0F) | 0x30;
								if(dcs != 0x3F) {
									x_variable[i++] = dcs;
								}
								dcs = ((STK_buffer[res] & 0xF0) >> 4) | 0x30;
								if(dcs != 0x3F) {
									x_variable[i++] = dcs;
								}		
							}
							x_variable[0] = i -1;
						} else {		   								//not found, force end
							result = CALLBACK_FORCE_END;					
						}
					} else {	   										//input number
						if(x_variable != NULL) {
							m_free(x_variable);
							x_variable = NULL;
						}
						x_variable = m_alloc(size + 1);
						memcpy(x_variable + 1, buffer, size);
						x_variable[0] = size;
					}
					break;
				case 0x42:	 			//transfer pulsa, nominal
					if(y_variable != NULL) {
						m_free(y_variable);
						y_variable = NULL;
					}
					y_variable = m_alloc(size + 1);
					memcpy(y_variable + 1, buffer, size);
					y_variable[0] = size;
					buffer[1] = 0x04;
					memcpy(buffer + 2, "*858*", 5);	 				//*121*
					res = x_variable[0];
					memcpy(buffer + 7, x_variable + 1, res);	   //*121*XXXXXX
					buffer[7 + res] = '*';						   //*121*XXXXXX*
					memcpy(buffer + 8 + res, y_variable + 1, y_variable[0]); //*121*XXXXXX*YYYYYY
					res = 8 + res + y_variable[0];			 
					buffer[res++] = '#'; 					 //*121*XXXXXX*YYYYYY#
					buffer[res] = 0;
					buffer[0] = res;  
					temp = m_alloc(10);
					if(tsel_lang == TSEL_LANG_BAHASA) {
						memcpy(temp + 1, "Mengirim", 8);
						temp[0] = 8;
					} else {	
						memcpy(temp + 1, "Sending", 7);
						temp[0] = 7;
					}
					buffer[1] = 0;
					buffer[0] = encode_827(buffer + 2, buffer + 2, res, 0);
					SAT_printf("cdlu", (SEND_USSD & 0x7F), STK_DEV_NETWORK, temp, buffer);
					m_free(temp);
					m_free(y_variable); y_variable = NULL;
					m_free(x_variable);	x_variable = NULL;
					result = CALLBACK_TAMPER_RESULT | CALLBACK_FORCE_END;
					break;
				default: break;
			} 
			break;
		case STK_TAG_RESULT:  
			file_select(&cb_fs, FID_MF);	
			file_select(&cb_fs, FID_LIQUID);
			file_select(&cb_fs, 0x6F02);		//user data
			switch(state) {
				case 0x90:			//setup sequence   
					if(buffer[0] == STK_RES_SUCCESS) { 	   
						file_select(&cb_fs, 0x6F01);		//user config				
						file_readbin(&cb_fs, 0, STK_buffer, 0x0C);
						tsel_lang = STK_buffer[USR_CONF_CUR_LANG];
						tsel_menu = STK_buffer[USR_CONF_REG_FLAG];
						if(is_sending_reg == TRUE) {		   											//reg sms sending successfull
							is_sending_reg = FALSE;
							STK_buffer[USR_CONF_SEND_REG] = 0xFF;	
						}
						if(tsel_menu == 0xFF) tsel_menu = TSEL_MENU_DAFTAR; 
						file_writebin(&cb_fs, 0, STK_buffer, 0x0C);

						if(STK_buffer[USR_CONF_SEND_REG] == 0x00) {									//send registration message
							send_registration(STK_buffer[USR_CONF_REG_FLAG]);
							/*#if 1
							file_select(&cb_fs, 0x6F02);		//user data
							file_readbin(&cb_fs, 0, STK_buffer, sizeof(tsel_user_data));
							((tsel_user_data *)STK_buffer)->s0 = '|';
							((tsel_user_data *)STK_buffer)->s1 = '|';
							((tsel_user_data *)STK_buffer)->s2 = '|';
							((tsel_user_data *)STK_buffer)->s3 = '|';
							((tsel_user_data *)STK_buffer)->s4 = '|';
							((tsel_user_data *)STK_buffer)->s5 = '|';
							((tsel_user_data *)STK_buffer)->s6 = '|';
							((tsel_user_data *)STK_buffer)->s7 = '|';
							((tsel_user_data *)STK_buffer)->s8 = '|';
							((tsel_user_data *)STK_buffer)->s9 = '|';
							((tsel_user_data *)STK_buffer)->s10 = '|';
							res = remove_null(STK_buffer, sizeof(tsel_user_data), STK_buffer);
							temp = m_alloc(res);
							memcpy(temp, STK_buffer, res);
							len = encode_SMSTPDU(SMS_TYPE_NOHEADER | SMS_TYPE_PACKED7 | SMS_TYPE_SUBMIT, res, "\x03\x81\x44\x44", temp, STK_buffer + 1);	//SMS submit
							STK_buffer[0] = len;
							m_free(temp);
							temp = m_alloc(9);
							if(tsel_lang == TSEL_LANG_BAHASA) {
								memcpy(temp + 1, "Mengirim", 8);
								temp[0] = 8;
							} else {	
								memcpy(temp + 1, "Sending", 7);
								temp[0] = 7;
								//SAT_printf("cdlm", (SEND_SHORT_MESSAGE & 0x7F), STK_DEV_ME, temp, STK_buffer);
							} 
							SAT_printf("cdlm", (SEND_SHORT_MESSAGE & 0x7F), STK_DEV_ME, temp, STK_buffer);
							m_free(temp);
							#endif */
							is_sending_reg = TRUE;
							result = CALLBACK_TAMPER_RESULT | CALLBACK_STAY_NODE;
						} else if(STK_buffer[USR_CONF_REG_FLAG] != 0xFF) {									//registration done, no need for popup
						  	result = CALLBACK_FORCE_END;
						}
					}
					break;
				case 0xE2: 
					if(buffer[0] == STK_RES_SUCCESS) {
						file_select(&cb_fs, 0x6F01);		//user config				
						file_readbin(&cb_fs, 0, STK_buffer, 0x0C);
						if(STK_buffer[USR_CONF_CHNG_LANG] != 0xFF) { 									//change language
							file_select(&cb_fs, FID_MF);	
							file_select(&cb_fs, FID_LIQUID);
							if(STK_buffer[USR_CONF_CHNG_LANG] == TSEL_LANG_BAHASA) {
								tsel_lang = TSEL_LANG_BAHASA;
								file_select(&cb_fs, 0x6F2B);
							} else if(STK_buffer[USR_CONF_CHNG_LANG] == TSEL_LANG_ENGLISH) { 
								tsel_lang = TSEL_LANG_ENGLISH;
								file_select(&cb_fs, 0x6F2E);
							}
							temp = m_alloc(sizeof(fs_handle)); 
							file_select(temp, FID_MF);	
							file_select(temp, FID_LIQUID);
							file_select(temp, FID_STKMENU);
							nama = file_get_current_header(temp);
							if(((ef_header *)nama)->size > 0x80) {
								for(i=0;i<((ef_header *)nama)->size - 0x80;i+=0x80) {
									file_readbin(&cb_fs, i, STK_buffer, 0x80);
									file_writebin(temp, i, STK_buffer, 0x80);
								}
							}
							file_readbin(&cb_fs, i, STK_buffer, ((ef_header *)nama)->size - i);
							file_writebin(temp, i, STK_buffer, ((ef_header *)nama)->size - i);
							file_readbin(temp, 0, STK_buffer, sizeof(stk_config));
							m_free(nama);
							m_free(temp);
							//reset engine 
							_stk_menu_offset = ((stk_config *)STK_buffer)->sibling;
							_stk_menu_current = 0; 
							//disable language changing
							file_select(&cb_fs, 0x6F01);		//user config				
							file_readbin(&cb_fs, 0, STK_buffer, 0x0C);
							STK_buffer[USR_CONF_CUR_LANG] = tsel_lang;
							STK_buffer[USR_CONF_CHNG_LANG] = 0xFF;				
							file_writebin(&cb_fs, 0, STK_buffer, 0x0C);	 
							result = CALLBACK_STAY_NODE;
						}
					}
					break;
				case 0x0F:			//(registration success
					if(buffer[0] == STK_RES_SUCCESS) { 
						file_select(&cb_fs, 0x6F01);		//user config
						//#define USR_CONF_REG_FLAG		0
						//#define USR_CONF_SEND_REG		1
						//#define USR_CONF_CUR_LANG		2
						//#define USR_CONF_CHNG_LANG	3						
						file_readbin(&cb_fs, 0, STK_buffer, 0x0C);
						STK_buffer[USR_CONF_REG_FLAG] = tsel_menu;
						STK_buffer[USR_CONF_SEND_REG] = 0;				
						file_writebin(&cb_fs, 0, STK_buffer, 0x0C);
						//rehabilitate ADN+SMSP
						file_select(&cb_fs, FID_MF);
						file_select(&cb_fs, FID_TELECOM); 
						file_select(&cb_fs, 0x6F3A);
						file_rehabilitate(&cb_fs);
						file_select(&cb_fs, FID_TELECOM); 
						file_select(&cb_fs, 0x6F42);
						file_rehabilitate(&cb_fs);
					} else {
						_stk_menu_offset = _default_menu_offset;
					}
					break;
				case 0x0D:			//display user data (part 2)
					//tamper result, display user information, next node
					if(buffer[0] == STK_RES_SUCCESS) {  
						file_readbin(&cb_fs, 0, STK_buffer, sizeof(tsel_user_data));
						nama = m_alloc(TSEL_UDS_ALAMAT);			//alamat
						klahir = m_alloc(TSEL_UDS_KOTA);			//kota
						tlahir = m_alloc(TSEL_UDS_KODEPOS + 1);			//kodepos
						sex = m_alloc(10);							//tipe id
						agama = m_alloc(TSEL_UDS_ID);				//no identitas
						memset(nama, 0, TSEL_UDS_ALAMAT);
						memset(klahir, 0, TSEL_UDS_KOTA);
						memset(tlahir, 0, TSEL_UDS_KODEPOS);
						memset(sex, 0, 10);
						memset(agama, 0, TSEL_UDS_ID);	
						memcpy(nama, ((tsel_user_data *)STK_buffer)->alamat, TSEL_UDS_ALAMAT); 
						memcpy(klahir, ((tsel_user_data *)STK_buffer)->kota, TSEL_UDS_KOTA);
						memcpy(tlahir, ((tsel_user_data *)STK_buffer)->kode_pos, TSEL_UDS_KODEPOS);
						tlahir[TSEL_UDS_KODEPOS] = 0;  
						memcpy(agama, ((tsel_user_data *)STK_buffer)->no_id, TSEL_UDS_ID);
						if(tsel_lang == TSEL_LANG_BAHASA) {
							file_select(&cb_fs, 0x6FB2);
							file_readrec(&cb_fs, (((tsel_user_data *)STK_buffer)->tipe_id -1) & 0x0F, sex, 10);
							switch(((tsel_user_data *)STK_buffer)->perokok == 0xFF) {
								default:
								case 0xFF:
									ls_printf(STK_buffer + 2, "Data yang Anda masukan adalah:\r\nAlamat Rumah:%s\r\nKota:%s\r\nKode Pos:%s\r\nTipe Identitas:%s\r\nNo. Identitas:%s\r\nApakah data-data tersebut benar?", nama, klahir, tlahir, sex, agama);
									break;
								case 0x01:
									ls_printf(STK_buffer + 2, "Data yang Anda masukan adalah:\r\nAlamat Rumah:%s\r\nKota:%s\r\nKode Pos:%s\r\nTipe Identitas:%s\r\nNo. Identitas:%s\r\nPerokok:Ya\r\nApakah data-data tersebut benar?", nama, klahir, tlahir, sex, agama);
									break;
								case 0x02:
									ls_printf(STK_buffer + 2, "Data yang Anda masukan adalah:\r\nAlamat Rumah:%s\r\nKota:%s\r\nKode Pos:%s\r\nTipe Identitas:%s\r\nNo. Identitas:%s\r\nPerokok:Bukan\r\nApakah data-data tersebut benar?", nama, klahir, tlahir, sex, agama);
									break;
							}
						} else { 
							file_select(&cb_fs, 0x6FE2);
							file_readrec(&cb_fs, (((tsel_user_data *)STK_buffer)->tipe_id -1) & 0x0F, sex, 10);   
							ls_printf(STK_buffer + 2, "Your data summaries are:\r\nHome Address:%s\r\nCity:%s\r\nZIP code:%s\r\nID:%s\r\nID. Number:%s\r\nare the data correct?", nama, klahir, tlahir, sex, agama);  
						}
						file_select(&cb_fs, 0x6F02);
						m_free(agama);
						m_free(sex);
						m_free(tlahir);
						m_free(klahir);
						m_free(nama);
						//strcpy(STK_buffer + 2, "result tamper data\x0"); 
						res = strlen(STK_buffer + 2);
						///file_writebin(&cb_fs, 0, STK_buffer + 2, res); 
						STK_buffer[0] = res + 1;
						STK_buffer[1] = 0x04;
						i = 0;
						//i += SAT_command(_sat_buffer + i, 0x01, (GET_INKEY & 0x7F), 4);
						//i += SAT_device(_sat_buffer + i, STK_DEV_SIM, STK_DEV_ME);
						//i += SAT_file_push(i, (STK_TAG_TEXT_STRING & 0x7F), res + 1, STK_buffer);
						//SAT_file_flush(i);
						SAT_printf("cdx", (DISPLAY_TEXT & 0x7F), STK_DEV_ME, STK_buffer);
						result = CALLBACK_TAMPER_RESULT;
					} else {	   
						_stk_menu_offset = _default_menu_offset;
					}
					break;
				case 0x60:
					if(buffer[0] == STK_RES_SUCCESS) {
						
					} else {
						//invalidate EFADN & EFSMSP
						file_select(&cb_fs, FID_MF);
						file_select(&cb_fs, FID_TELECOM); 
						file_select(&cb_fs, 0x6F3A);
						file_invalidate(&cb_fs);
						file_select(&cb_fs, FID_TELECOM); 
						file_select(&cb_fs, 0x6F42);
						file_invalidate(&cb_fs); 
						/*if(tsel_lang == TSEL_LANG_BAHASA) {   
							ls_printf(STK_buffer + 2, "Data yang Anda masukan adalah:\r\nAlamat Rumah:%s\r\nKota:%s\r\nKode Pos:%s\r\nTipe Identitas:%s\r\nNo. Identitas:%s\r\nApakah data-data tersebut benar?", nama, klahir, tlahir, sex, agama);
						} else {    
							ls_printf(STK_buffer + 2, "Data yang Anda masukan adalah:\r\nAlamat Rumah:%s\r\nKota:%s\r\nKode Pos:%s\r\nTipe Identitas:%s\r\nNo. Identitas:%s\r\nApakah data-data tersebut benar?", nama, klahir, tlahir, sex, agama);  
						}
						SAT_printf("cdx", (DISPLAY_TEXT & 0x7F), STK_DEV_ME, STK_buffer); */
						result = (CALLBACK_FORCE_END | 1);
					}
					break;
				default:
					
					break;
			}
			break;
	} 
	

	return result;
}