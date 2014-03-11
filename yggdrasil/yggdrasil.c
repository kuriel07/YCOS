/* Yggdrasil micro kernel provide services between smart card and the ME 
 *
 * Copyright 2010, Agus Purwanto.
 * All rights reserved.
 *
 * 
 */

#include "yggdrasil.h"
#include "application.h"
#include "..\defs.h"
#include "..\drivers\ioman.h"
#include "..\misc\mem.h"
#include "..\asgard\file.h"
#include "..\asgard\fs.h"
#include "..\asgard\security.h"
#include "..\auth\A3A8.h"
#include "..\midgard\midgard.h"
#include "..\asgard\sys_file.h"
#include "..\ISO7816\ISO7816.h"
#include "..\NORFlash\NORFlash.h"
#include "..\liquid.h"
#include "..\framework\vas.h"
#include "..\framework\sms.h"
#include "..\framework\des.h"
#include <string.h>

//extern apdu_command *iobuf;
extern fs_table fs_info;
//BYTEX v_chv_status[0x10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };	//jangan diinisialisasi, biar ketika reset nilainya tetap

//HALFWX free_space = 0;
uchar get_resp_length;
uchar response_length; 
uchar * response_data = NULL;
//uchar response[0x20];
os_config _os_config;  
//uchar xdata yggdrasil_state;
#if 0
fs_handle _ygg_fs _at_ 0x4F8;	
#else
fs_handle _ygg_fs;
#endif   

BYTEC appkey[] = "H3imdall";
BYTEC tokenkey[] = "R46N4RoK"; 
BYTEC auth_mask[] = { 0x59, 0x36, 0x36, 0x64, 0x72, 0x40, 0x73, 0x69 };

extern BYTEX 	iso7816_buffer[263];
//extern BYTEX	IOBuf[];
#define ResponseBuf 	(iso7816_buffer) 
#define apdu_le_value 	*(iso7816_buffer+262)

#define HALFWSWAP(x) (((x>>8)&0xff) | ((x<<8)&0xff00)) 
uchar * auth_token = NULL;			//software authentication

//0 = comand only no response
//1 = command only with response
//2 = command with data no response
//3 = command with data with response
/*BYTEC gsm_class_case[] = {
//	0,1,2,3,4,5,6,7,8,9,A,B,C,D,E,F
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //1
	2,0,0,0,2,0,2,0,2,0,0,0,2,0,0,0, //2
	0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0, //3
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //4
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //5
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //6
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //7
	0,0,0,0,0,0,0,0,2,0,0,0,0,0,0,0, //8
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //9
	0,0,2,0,2,0,0,0,0,0,0,0,0,0,0,0, //A
	1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0, //B
	1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //C
	0,0,0,0,0,0,2,0,0,0,0,0,2,0,0,0, //D
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //E
	0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0, //F
};

BYTEC yggdrasil_class_case[] = {
//	0,1,2,3,4,5,6,7,8,9,A,B,C,D,E,F
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //0
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //1
	2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,1, //2
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //3
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //4
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //5
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //6
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //7
	0,2,1,0,0,0,0,0,0,0,2,0,0,0,0,0, //8
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //9
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //A
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //B
	2,2,2,2,0,0,0,0,0,0,0,0,0,2,0,0, //C
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //D
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, //E
	0,0,0,0,1,0,2,0,0,0,0,0,0,0,0,0, //F
};	 */

void Initialize_Hardware() _REENTRANT_
{
	//inisialisasi eeprom
	//inisialisasi interrupt(jika ada)
	//inisialisasi watchdog timer(self recovery system)

	//inisialisasi driver2
	//printf(" * initializing all drivers\n");
	ioman_init();
}

void Initialize_Operating_System() _REENTRANT_
{
	register uint16 sw;
	//printf(" * initializing midgard memory manager\n");
	m_init_alloc();			//init memori manager
	//ioman_send_atr();		//send answer to reset, operating system response
	//iobuf = (apdu_command *)m_alloc(sizeof(apdu_command));		//untuk memori buffer digunakan memori statis
	//printf(" * initializing asgard file system\n");
	_os_config.os_state = 0;
	_os_config.os_state |= (YGG_ST_NO_INIT | YGG_ST_ACTIVATED);	   //no file system available, YGG_NO_INIT, default state
	//sprintf(iso7816_buffer, "");
	if(fs_init() == FS_UNFORMATTED)
	{
		//_os_config.os_state |= YGG_ST_NO_INIT;	   //no file system available, YGG_NO_INIT
		//Save_State();
	}
	else
	{
		//printf(" * mounting existing file system\n");
		Load_State(); 
		if((_os_config.os_state & YGG_ST_ACTIVATED) == 0) {
			//initialize default file system (shared with local application) 
			_select(&_ygg_fs, FID_MF); 						//auto select FID MF
			#if 0
			Initialize_User_App();
			#endif
		}
		//_os_config.os_state |= YGG_ST_NO_INIT;	   //no file system available, YGG_NO_INIT
	}
}

//Send_Status digunakan untuk mengambil attribut2 dari file/direktori yang sedang aktif
//merupakan fungsi internal dari kernel, tidak untuk dishare
// SEND_STATUS
#define STATUS_DIRECTORY 		1
#define STATUS_FILE				2
#define STATUS_ALL				3
uchar Send_Status(uchar mode, uchar * ResponseBuf) _REENTRANT_ {
	ef_header * curfile = NULL;
	fs_handle temp_fs;
	chv_file cf;
	memcpy(&temp_fs, &_ygg_fs, sizeof(fs_handle));
	switch(mode) {
		case STATUS_DIRECTORY:
			temp_fs.cur_ptr = temp_fs.cur_dir;
			curfile = file_get_current_header(&temp_fs);
			break;
		case STATUS_ALL:
			curfile = file_get_current_header(&temp_fs);
			break;		
	}
	//df_header * curdir = (df_header *)curfile;
	//siap mengisi iobuf
	//response_df * df_res = ((response_df *)ResponseBuf);
	//response_ef * ef_res = ((response_ef *)ResponseBuf);
	
	if(curfile->type == T_EF)	//response untuk EF
	{	  
		//memset(ResponseBuf, 0, 7);
		((response_ef *)ResponseBuf)->rfu_1 = 0;
		((response_ef *)ResponseBuf)->file_size = curfile->size;
		((response_ef *)ResponseBuf)->fid = curfile->fid;
		//ef_res->fid2 = curfile->FID & 0x00ff;
		((response_ef *)ResponseBuf)->type = curfile->type;
		//memcpy((uchar *)(ResponseBuf + 7), (uchar *)(curfile + 11), 8);
		if(curfile->type != EF_CYCLIC) {
			//((response_ef *)ResponseBuf)->increase_allowed = curfile->inc;
			((response_ef *)ResponseBuf)->increase_allowed = 0x00;
		} else {
			if((curfile->acc_inc & 0x0F) != ACC_NVR) {
				((response_ef *)ResponseBuf)->increase_allowed = 0x40;
			} else {
				((response_ef *)ResponseBuf)->increase_allowed = 0x00;
			}
		}
		((response_ef *)ResponseBuf)->acc_rw = curfile->acc_rw;
		((response_ef *)ResponseBuf)->acc_inc = (curfile->acc_inc & 0x0F); 		//do not show OTA access
		((response_ef *)ResponseBuf)->acc_ri = curfile->acc_ri;
		((response_ef *)ResponseBuf)->status = curfile->status;
		((response_ef *)ResponseBuf)->next_length = 2;
		((response_ef *)ResponseBuf)->structure = curfile->structure;
		((response_ef *)ResponseBuf)->rec_length = curfile->rec_size;
		m_free(curfile);
		#if _DMA_DEBUG
		//barren_eject(curfile);
		#endif
		return EF_RESPONSE_SIZE;
	} else {
		//if(((df_header *)curfile)->file_char != 0) { m_free(curfile); return 0; }
		//memset(ResponseBuf, 0, 0x17);					//response untuk DF/MF
		((response_df *)ResponseBuf)->rfu_1 = 0;
		((response_df *)ResponseBuf)->total_memory = fs_freespace();	//total memori bebas
		//df_res->total_memory = 0;	//total memori bebas
		((response_df *)ResponseBuf)->fid = ((df_header *)curfile)->fid;
		//df_res->fid2 = curdir->FID & 0x00ff;
		((response_df *)ResponseBuf)->type = ((df_header *)curfile)->type;
		((response_df *)ResponseBuf)->rfu_2 = 0;
		((response_df *)ResponseBuf)->rfu_3 = 0;
		((response_df *)ResponseBuf)->rfu_4 = 0;
		((response_df *)ResponseBuf)->rfu_5 = 0;
		((response_df *)ResponseBuf)->rfu_6 = 0;
		((response_df *)ResponseBuf)->next_length = 0x0A;	//next data length(gsm specific)
		chv_get_config(ACC_CHV1, &cf);
		((response_df *)ResponseBuf)->gsm.file_char = 0x11;		//0x1B (previous value)
		if((cf.status & CHV_ENABLED) == 0) {
			((response_df *)ResponseBuf)->gsm.file_char |= 0x80;
		}
		/*switch(cf->status) {			//file characteristics
			case CHV_UNBLOCK:
			case CHV_ENABLE:
				df_res->gsm.file_char = 0x13;	//angka 0x13 cuman mengikuti keluaran kartu flexi		0x13
				break;
			default:
			case CHV_BLOCK:
			case CHV_DISABLE:
				df_res->gsm.file_char = 0x13 | 0x80;	//angka 0x13 cuman mengikuti keluaran kartu flexi (karena tergantung hw)	0x93
				break;
		}*/
		//printf("file char = %x\n", chv->status);
		((response_df *)ResponseBuf)->gsm.number_of_df = ((df_header *)curfile)->num_of_df;
		((response_df *)ResponseBuf)->gsm.number_of_ef = ((df_header *)curfile)->num_of_ef;
		((response_df *)ResponseBuf)->gsm.number_of_chv = 0x04;	//cuman mengikuti keluaran kartu flexi
		//((response_df *)ResponseBuf)->gsm.rfu_1 = 0;
		//if(cf->pin_attempts <= cf->pin_max_attempts) {
		((response_df *)ResponseBuf)->gsm.chv1 = (0x80 + (cf.pin_max_attempts - cf.pin_attempts)); 
		//} else {
			//((response_df *)ResponseBuf)->gsm.chv1 = 0x80;
		//}
		//if(cf->puk_attempts <= cf->puk_max_attempts) {
		((response_df *)ResponseBuf)->gsm.unblock_chv1 = (0x80 + (cf.puk_max_attempts - cf.puk_attempts));
		//} else {
			//((response_df *)ResponseBuf)->gsm.unblock_chv1 = 0x80;
		//}
		//m_free(cf);
		#if _DMA_DEBUG
		//barren_eject(cf);
		#endif
		chv_get_config(ACC_CHV2, &cf);
		//if(cf->pin_attempts <= cf->pin_max_attempts) {
		((response_df *)ResponseBuf)->gsm.chv2 = (0x80 + (cf.pin_max_attempts - cf.pin_attempts)); 
		//} else {
			//((response_df *)ResponseBuf)->gsm.chv2 = 0x80;
		//}
		//if(cf->puk_attempts <= cf->puk_max_attempts) {
		((response_df *)ResponseBuf)->gsm.unblock_chv2 = (0x80 + (cf.puk_max_attempts - cf.puk_attempts));
		//} else {
			//((response_df *)ResponseBuf)->gsm.unblock_chv2 = 0x80;
		//}
		//m_free(cf);
		#if _DMA_DEBUG
		//barren_eject(cf);
		#endif
		//((response_df *)ResponseBuf)->gsm.rfu_2 = 0;
		m_free(curfile);
		#if _DMA_DEBUG
		//barren_eject(curdir);
		#endif
		return DF_RESPONSE_SIZE;
	}
}

void gsm_response(uchar * buffer, uchar length) _REENTRANT_ {
	if(response_data != NULL) {
		m_free(response_data);
		response_data = NULL;
	}
	response_data = (uchar *)m_alloc(length + 1);
	((response_buffer *)response_data)->length = length;
	memcpy(((response_buffer *)response_data)->buffer, buffer, length); 			
} 

#if (LIQUID_ALLOCATED)
fs_handle stk_fs; 
uchar _tres_cntr = 0;
extern uint16 _stk_menu_offset; 
extern uint16 _stk_menu_current;
extern uint16 _stk_menu_anchor;
#endif	

#if (VAS_ALLOCATED)
uchar sat_init_state = SAT_MENU_INIT;
#endif

uint16 Yggdrasil_Decode(apdu_command * command) //hindari pemakaian memori dinamis pada kernel, kemungkinan leak menjadi lebih besar
{
	uint16 status;
	uchar len;
	uint16 offset;
#if LIQUID_ALLOCATED
	uchar i, tag, state, size, res, dtag;
	stk_config stknode;
	uchar callback_result;
	uchar terminal_result;
#endif
	response_length = 0;
	//OPERATING SYSTEM INTERNAL CLASS APDU
	//case CLA_YGGDRASIL:
	if(command->CLA == CLA_YGGDRASIL) {
		switch(command->INS)
		{  
			case INS_SELECT	:		//select file/direktori
				ygg_ins_select:
				Get_Data();
				len = command->P3;	//simpan dulu P3 ke len sehingga jika ada perubahan pd buffer nilainya masih terselamatkan
				if((command->P1|command->P2) == 0) {
					if(len != 0x02) {
						response_length = 0;
						return (APDU_WRONG_LENGTH | 0x02); //wrong length .OR. requested length
					}
					//memcopy(buf, command->bytes, 0, len);
					/*status = _select(&_ygg_fs, *((uint16 *)command->bytes));
					len = Send_Status(command->bytes);
					status & 0xFF00
					get_resp_length = len;
					response_length = len;	*/  
					status = _select(&_ygg_fs, *((uint16 *)command->bytes));
					response_length = 0;
					get_resp_length = Send_Status(STATUS_ALL, command->bytes);
					gsm_response(command->bytes, get_resp_length);
					//system file (abort select)
					//if(response_length == 0) { _select(FID_MF); return APDU_SUCCESS; }
					//gsm_response(command->bytes, response_length); 
					//status &= 0xFF00;
					return status;
				} else {
					return APDU_WRONG_PARAMETER;	//wrong p1,p2
				}
				//response disini
				//tidak ada response(cuman sw1 dan sw2)
				//cu_rcd = 0;	//reset no record untuk operasi next dan prev
				return status;
						
			case INS_READ_BINARY :	//read binary	
				ygg_ins_readbin:
				len =  command->P3;
				offset = ((uint16)((command->P1<<8) | command->P2));
				//printf("offset : %i\n", (uint16)((command->P1<<8) | command->P2));   
				response_length = 0;
				if((status = _check_access(&_ygg_fs, FILE_READ)) == APDU_SUCCESS) {
					status = _readbin(&_ygg_fs, offset, command, len);
					response_length = len;
				}
				if(status != APDU_SUCCESS) {
					//memclear(command, len);
					memset(command, 0, len);
				}
				return status;

			case INS_READ_RECORD :	//read record  
				ygg_ins_readrec:
				len = command->P3;
				
				response_length = 0;
				if((status = _check_access(&_ygg_fs, FILE_READ)) == APDU_SUCCESS) {
					switch(command->P2) {
						case P_NEXT_REC :
								status = _readrec_next(&_ygg_fs, (uchar *)command, len);
							break;
						case P_PREV_REC :
								status = _readrec_prev(&_ygg_fs, (uchar *)command, len);
							break;
						case P_ABS_REC :
								//currcd = command->P1;
								//record dimulai dari 1, padahal fungsi _readrec dari 0
								status = _readrec(&_ygg_fs, command->P1 - 1, (uchar *)command, len);
							break;
						default :
							response_length = 0;
							return APDU_WRONG_PARAMETER;
							break;
					}
					response_length = len;
					if(status != APDU_SUCCESS) {
						//memclear((uchar *)command, len);
						memset((uchar *)command, 0, len);
					}
				}
				return status;
							
			case INS_UPDATE_BINARY :	//update binary	  
				ygg_ins_writebin:
				Get_Data();
				len =  command->P3;
				offset = ((uint16)((command->P1<<8) | command->P2));
				response_length = 0;
				if((status = _check_access(&_ygg_fs, FILE_WRITE)) == APDU_SUCCESS) {
					status = _writebin(&_ygg_fs, offset, command->bytes, len);
					response_length = 0;
					if(status != APDU_SUCCESS) {
						memset((uchar *)command, 0, len);
					}
				}
				return status;
						
			case INS_UPDATE_RECORD :	//update record	  
				ygg_ins_writerec:
				Get_Data();
				len = command->P3; 
				if((status = _check_access(&_ygg_fs, FILE_WRITE)) == APDU_SUCCESS) {
					switch(command->P2) {
						case P_NEXT_REC :
								status = _writerec_next(&_ygg_fs, command->bytes, len);
							break;
						case P_PREV_REC :
								status = _writerec_prev(&_ygg_fs, command->bytes, len);
							break;
						case P_ABS_REC :
								//currcd = command->P1;
								//record dimulai dari 1, padahal fungsi _writerec dari 0
								status = _writerec(&_ygg_fs, command->P1 - 1, command->bytes, len);
							break;
						default :
							response_length = 0;
							return APDU_WRONG_PARAMETER;
							break;
					}
					response_length = 0;
				}
				return status;

			case INS_REHABILITATE :		//rehabilitate	
				ygg_ins_rehab:
				len = command->P3;
				if((command->P1|command->P2) == 0) {
					if(len != 0x00) {
						response_length = 0;
						return (APDU_WRONG_LENGTH | 0x00); //wrong length .OR. requested length
					}
					if((status = _check_access(&_ygg_fs, FILE_REHABILITATE)) == APDU_SUCCESS) {
						status = _rehabilitate(&_ygg_fs);
						response_length = 0;
					}
				} else {
					return APDU_WRONG_PARAMETER;	//wrong p1,p2
				}
				return status;
							
			case INS_INVALIDATE :		//invalidate
				ygg_ins_invalid:
				len = command->P3;
				if((command->P1|command->P2) == 0) {
					if(len != 0x00) {
						response_length = 0;
						return (APDU_WRONG_LENGTH | 0x00); //wrong length .OR. requested length
					} 
					if((status = _check_access(&_ygg_fs, FILE_INVALIDATE)) == APDU_SUCCESS) {
						status = _invalidate(&_ygg_fs);
						response_length = 0;
					}
				} else {
					return APDU_WRONG_PARAMETER;	//wrong p1,p2
				}
				return status;

			case INS_VERIFY_CHV:		//admin or system login		  
				ygg_ins_verify_chv:
				Get_Data();
				response_length = 0;
				//if(_os_config.os_state & YGG_ST_NO_INIT) return APDU_MEMORY_PROBLEM;
				if(command->P3 == 0x08) {
					if(command->P1 == 0x00) {
						switch(command->P2) {
							case ACC_ADM:
								status = verifyCHV(ACC_ADM, command->bytes);
								break;
							case ACC_SYS:
								status = getCHVstatus(ACC_ADM);	  		//check admin login
								if((status & CHV_VERIFIED) == 0) return APDU_SECURITY_STATE_ERROR;
								status = verifyCHV(ACC_SYS, command->bytes);
								break;
							default:
								return APDU_WRONG_PARAMETER;
								break;
						} 
						//if(status & CHV_BLOCKED) return APDU_AUTH_BLOCKED;
						//if(status == CHV_VERIFIED) return APDU_SUCCESS;
						//return APDU_SECURITY_STATE_ERROR;
					} else {
						return APDU_WRONG_PARAMETER;
					}
				} else {
					return APDU_WRONG_LENGTH | 0x10;
				}
				return status;				

			case INS_CHANGE_CHV:		//admin or system change key	
				ygg_ins_change_chv:
				Get_Data();
				response_length = 0;
				//if(_os_config.os_state & YGG_ST_NO_INIT) return APDU_MEMORY_PROBLEM;
				if(command->P3 == 0x10) {
					if(command->P1 ==0x00) {
						switch(command->P2) {
							case ACC_ADM : 
								status = getCHVstatus(ACC_ADM);	  		//check admin login
								if(status & CHV_VERIFIED) return APDU_SECURITY_STATE_ERROR;
								status = changeCHV(ACC_ADM, command->bytes, (command->bytes+8));
								break;
							case ACC_SYS :
								status = getCHVstatus(ACC_SYS);	  		//check system login
								if(status & CHV_VERIFIED) return APDU_SECURITY_STATE_ERROR;
								status = changeCHV(ACC_SYS, command->bytes, (command->bytes+8));
								break;
							default :
								return APDU_WRONG_PARAMETER;
								break;
						}
						//return status;
						//if(status == CHV_BLOCK) return APDU_AUTH_BLOCKED;
						//if(status == CHV_VERIFIED) return APDU_SUCCESS;
						//return APDU_SECURITY_STATE_ERROR;
					} else {
						return APDU_WRONG_PARAMETER;
					}
				} else {
					return APDU_WRONG_LENGTH | 0x10;
				}
				return status;

			case INS_FILE_DELETE:				//0xC0	//00	00	02	[FID2]				Remove with the specified FID
				Get_Data();
				response_length = 0;
				//if(_os_config.os_state & YGG_ST_NO_INIT) return APDU_MEMORY_PROBLEM;
				status = getCHVstatus(ACC_ADM);	  		//check system login
				if((status & CHV_VERIFIED) == 0) return APDU_SECURITY_STATE_ERROR;
				len = command->P3;	//simpan dulu P3 ke len sehingga jika ada perubahan pd buffer nilainya masih terselamatkan
				if((command->P1|command->P2) == 0) {
					if(len != 0x02) {
						response_length = 0;
						return (APDU_WRONG_LENGTH | 0x02); //wrong length .OR. requested length
					}
					status = _remove(&_ygg_fs, *((uint16 *)command->bytes));
					response_length = 0;
				} else {
					return APDU_WRONG_PARAMETER;	//wrong p1,p2
				}
				return status;

			case INS_FILE_CREATE:
				Get_Data();
				response_length = 0;
				//if(_os_config.os_state & YGG_ST_NO_INIT) return APDU_MEMORY_PROBLEM;
				status = getCHVstatus(ACC_ADM);	  		//check system login
				if((status & CHV_VERIFIED) == 0) return APDU_SECURITY_STATE_ERROR;
				//if(command->P1 != 0) return APDU_WRONG_PARAMETER;	//wrong p1,p2
				switch(command->P2) {
					case 0x0D:
						if(command->P3 != 0x02) {
							response_length = 0;
							return (APDU_WRONG_LENGTH | 0x02); //wrong length .OR. requested length
						}
						status = _createdirectory(&_ygg_fs, *((uint16 *)command->bytes), command->P1);
						//status = _select(fromhex(command->bytes, 2));
						response_length = 0;
						break;
					case 0x01: 
						if(command->P3 != 0x07) {
							response_length = 0;
							return (APDU_WRONG_LENGTH | 0x07); //wrong length .OR. requested length
						}
						status = _createfilebin(&_ygg_fs, *((uint16 *)command->bytes), command->bytes[2], command->bytes[3], command->bytes[4], *((uint16 *)(command->bytes + 5)));
						//status = _select(fromhex(command->bytes, 2));
						response_length = 0;
						break;
					case 0x02:
						if(command->P3 != 0x08) {
							response_length = 0;
							return (APDU_WRONG_LENGTH | 0x08); //wrong length .OR. requested length
						}
						status = _createfilerec(&_ygg_fs, *((uint16 *)command->bytes), command->bytes[2], command->bytes[3], command->bytes[4], *((uint16 *)(command->bytes + 5)), command->bytes[7]);
						//status = _select(fromhex(command->bytes, 2));
						response_length = 0;
						break;
					case 0x03:
						if(command->P3 != 0x08) {
							response_length = 0;
							return (APDU_WRONG_LENGTH | 0x08); //wrong length .OR. requested length
						}
						status = _createfilecyclic(&_ygg_fs, *((uint16 *)command->bytes), command->bytes[2], command->bytes[3], command->bytes[4], *((uint16 *)(command->bytes + 5)), command->bytes[7]);
						//status = _select(fromhex(command->bytes, 2));
						response_length = 0;
						break;
					default:
						return APDU_WRONG_PARAMETER;	//wrong p1,p2
						break;	
				}
				return status;

			//////////////////////////SYSTEM COMMAND///////////////////////////
			case INS_FS_FORMAT:				//0xF0
				response_length = 0;
				if((_os_config.os_state & YGG_ST_NO_INIT) == 0) {
					status = getCHVstatus(ACC_SYS);	  		//check system login
					if((status & CHV_VERIFIED) == 0) return APDU_SECURITY_STATE_ERROR;
				}
				switch(command->P1) {
					case 1:
						fs_format(0x2000);		//8K
						break;
					case 2:
						fs_format(0x4000);		//16K
						break;
					case 3:
						fs_format(0x8000);		//32K
						break;
					case 4:
						fs_format(0x10000);		//64K
						break; 
					default:
						return APDU_WRONG_PARAMETER;
				}
				if(fs_init()==FS_UNFORMATTED) return APDU_MEMORY_PROBLEM;				//reinitialize file system
				//_os_config.os_state |= YGG_ST_NO_INIT; 
				Save_State();
				return APDU_SUCCESS;


			/*case INS_FS_DEFRAG:				//0xFD	//00	00	00					Defrag file system (reserved command)
				response_length = 0;
				//if(_os_config.os_state & YGG_ST_NO_INIT) return APDU_MEMORY_PROBLEM;
				status = getCHVstatus(ACC_SYS);	  		//check system login
				if(status != CHV_VERIFIED) return APDU_SECURITY_STATE_ERROR;
				return APDU_INSTRUCTION_INVALID;*/
			case INS_FS_LOCK:				//0xF7	//00	00	00					Lock file system (reserved command)
				response_length = 0;
				//if(_os_config.os_state & YGG_ST_NO_INIT) return APDU_MEMORY_PROBLEM;
				status = getCHVstatus(ACC_SYS);	  		//check system login
				if((status & CHV_VERIFIED) == 0) return APDU_SECURITY_STATE_ERROR;
				_os_config.os_state &= ~YGG_ST_NO_INIT;
				Save_State();
				return APDU_SUCCESS;

			case INS_FS_UNLOCK:				//0xF8	//00	00	00					Unlock file system (reserved command)
				response_length = 0;
				//if(_os_config.os_state & YGG_ST_NO_INIT) return APDU_MEMORY_PROBLEM;
				status = getCHVstatus(ACC_SYS);	  		//check system login
				if((status & CHV_VERIFIED) == 0) return APDU_SECURITY_STATE_ERROR;
				_os_config.os_state |= YGG_ST_NO_INIT;
				Save_State();
				return APDU_SUCCESS;

			case INS_FS_STATUS:				//0xF2	//00	00	10					File system status				
				response_length = 0;
				//if(_os_config.os_state & YGG_ST_NO_INIT) return APDU_MEMORY_PROBLEM;
				status = getCHVstatus(ACC_SYS);	  		//check system login
				if((status & CHV_VERIFIED) == 0) return APDU_SECURITY_STATE_ERROR;
				return APDU_INSTRUCTION_INVALID;

			case INS_OS_STATUS:
				response_length = 0;
				//if(_os_config.os_state & YGG_ST_NO_INIT) return APDU_MEMORY_PROBLEM;
				status = getCHVstatus(ACC_SYS);	  		//check system login
				if((status & CHV_VERIFIED) == 0) return APDU_SECURITY_STATE_ERROR;
				if(command->P3 != 0x10) return APDU_WRONG_LENGTH;
				((os_status *)(command))->os_tag = 'Y';
				((os_status *)(command))->os_ver = YGGDRASIL_VERSION;			//kernel version 
				((os_status *)(command))->os_state = _os_config.os_state;			//os status	(locked/unlocked, stk enable/disable, stk load, sleep)
				((os_status *)(command))->fs_ver = fs_info.fs_ver;			//file system version
				((os_status *)(command))->fs_mod = fs_info.fs_mod;			//16 bit/32 bit
				((os_status *)(command))->fs_ssize = fs_info.sector_size;			//sector size
				((os_status *)(command))->heap_size = m_get_allocated_space();  		//allocated heap					(memory)
				((os_status *)(command))->total_heap = MEM_HEAP_SIZE;		//total heap size, free+allocated	(memory)
				((os_status *)(command))->used_space = fs_get_allocated_space();		//allocated space					(flash)
				((os_status *)(command))->total_space = fs_info.fs_size;		//total partition					(flash)
				//memcopy(iobuf, &_os_config, 0, sizeof(os_status));
				response_length = command->P3;
				return APDU_SUCCESS;

			case INS_ENABLE_APP:	 					//enable user app
				response_length = 0;
				//status = getCHVstatus(ACC_SYS);	  		//check system login
				//if(status != CHV_VERIFIED) return APDU_INCONTRADICTION_W_CHV;
				if((_os_config.os_state & YGG_ST_NO_INIT) == 0) {
					status = getCHVstatus(ACC_SYS);	  		//check system login
					if((status & CHV_VERIFIED) == 0) return APDU_SECURITY_STATE_ERROR;
				}
				_os_config.os_state |= YGG_ST_ENABLE_APP;
				Save_State();
				return APDU_SUCCESS;

			case INS_DISABLE_APP:						//disable user app
				response_length = 0;
				//status = getCHVstatus(ACC_SYS);	  		//check system login
				//if(status != CHV_VERIFIED) return APDU_INCONTRADICTION_W_CHV;
				if((_os_config.os_state & YGG_ST_NO_INIT) == 0) {
					status = getCHVstatus(ACC_SYS);	  		//check system login
					if((status & CHV_VERIFIED) == 0) return APDU_SECURITY_STATE_ERROR;
				}
				_os_config.os_state &= ~(YGG_ST_ENABLE_APP);
				Save_State();
				return APDU_SUCCESS;

			case INS_LOAD_APP:						  	//load downloaded app (temporary space) to user program space
				response_length = 0;	  
				if((_os_config.os_state & YGG_ST_NO_INIT) == 0) {
					status = getCHVstatus(ACC_SYS);	  		//check system login
					if((status & CHV_VERIFIED) == 0) return APDU_SECURITY_STATE_ERROR;
				}
				//status = getCHVstatus(ACC_SYS);	  		//check system login
				//if(status != CHV_VERIFIED) return APDU_INCONTRADICTION_W_CHV;
				//_os_config.os_state &= ~(YGG_ST_LOAD_APP);
				//Save_State();
				//status = Load_User_App();
				status = ioman_program_copy(0x8000, 0xC000, 0x4000);
				return status;

			case INS_WRITE_APP:	 						//write an app to temporary space
				Get_Data();
				response_length = 0;  
				if((_os_config.os_state & YGG_ST_NO_INIT) == 0) {
					status = getCHVstatus(ACC_SYS);	  		//check system login
					if((status & CHV_VERIFIED) == 0) return APDU_SECURITY_STATE_ERROR;
				}
				//status = getCHVstatus(ACC_SYS);	  		//check system login
				//if(status != CHV_VERIFIED) return APDU_INCONTRADICTION_W_CHV;
				offset = ((uint16)((command->P1<<8) | command->P2));  
				for(len=0; len<command->P3 && len != command->P3; len+=8) {
					des_decode(appkey, command->bytes + len, command->bytes + len);
					//DES_Operation(0, 1, bytes + len, appkey, appkey, bytes + len);
				}	 
				status = Write_Temp_Space(offset, command->bytes, command->P3);
				return status;

			case INS_LOAD_BOOTLOADER:
				response_length = 0;
				if((_os_config.os_state & YGG_ST_NO_INIT) == 0) {
					status = getCHVstatus(ACC_SYS);	  		//check system login
					if((status & CHV_VERIFIED) == 0) return APDU_SECURITY_STATE_ERROR;
				}
				//status = getCHVstatus(ACC_SYS);	  		//check system login
				//if(status != CHV_VERIFIED) return APDU_INCONTRADICTION_W_CHV;
				ioman_set_to_bootloader();
				return APDU_SUCCESS;

			case INS_GET_TOKEN:
				if(command->P3 != 8) return APDU_WRONG_LENGTH;
				if(auth_token != NULL) m_free(auth_token);		//check if auth token exist
				auth_token = (uchar *)m_alloc(8);
				Random_Number_Generator(auth_token, 8);
				for(len=0;len<8;len++) {
					command->bytes[len] = auth_token[len] ^ auth_mask[len];
				}
				DES_Encrypt(tokenkey, command->bytes, command);
				response_length = 8;
				status = APDU_SUCCESS;
				return status;
			/*case 0xE1:
				memcopy(command, auth_token, 0, 8);
				response_length = 8;
				status = APDU_SUCCESS;
				return status; */

			case INS_SOFT_AUTH:
				if(command->P3 != 8) return APDU_WRONG_LENGTH;
				Get_Data();
				if(auth_token == NULL) return APDU_FATAL_ERROR;
				status = APDU_AUTH_BLOCKED;
				if(memcmp(auth_token, command->bytes, 8) == 0) {
					chv_set_status(ACC_ADM, CHV_VERIFIED); 	  //logged on
					chv_set_status(ACC_SYS, CHV_VERIFIED);
					m_free(auth_token);
					auth_token = NULL;
					status = APDU_SUCCESS;
				}
				return status;

			case INS_WRITE_FLASH:
				Get_Data();
				if((_os_config.os_state & YGG_ST_NO_INIT) == 0) {
					status = getCHVstatus(ACC_SYS);	  		//check system login
					if((status & CHV_VERIFIED) == 0) return APDU_SECURITY_STATE_ERROR;
				}
				offset = (uint16)(command->P1) << 8 | (command->P2);
				ioman_write_buffer(offset, command->bytes, command->P3);
				response_length = 0;
				return APDU_SUCCESS;

			case INS_READ_FLASH:  	
				if((_os_config.os_state & YGG_ST_NO_INIT) == 0) {
					status = getCHVstatus(ACC_SYS);	  		//check system login
					if((status & CHV_VERIFIED) == 0) return APDU_SECURITY_STATE_ERROR;
				}
				len = command->P3;
				offset = (uint16)(command->P1) << 8 | (command->P2);
				ioman_read_buffer(offset, command, len);
				response_length = len;
				return APDU_SUCCESS;

			case INS_ERASE_FLASH:	
				if((_os_config.os_state & YGG_ST_NO_INIT) == 0) {
					status = getCHVstatus(ACC_SYS);	  		//check system login
					if((status & CHV_VERIFIED) == 0) return APDU_SECURITY_STATE_ERROR;
				}
				ioman_erase_all();
				response_length = 0;
				return APDU_SUCCESS;

			case INS_BIFROST:	
				Get_Data();					 		//bifrost production mechanism
				status = getCHVstatus(ACC_SYS);	  		//check system login
				if((status & CHV_VERIFIED) == 0) return APDU_SECURITY_STATE_ERROR;
				//if(command->P3 != 0x10) return APDU_WRONG_LENGTH;
				return Bifrost_Decode(command->bytes, command->P3);

			case INS_AUTH_FSH:
				Get_Data();						 		
				status = getCHVstatus(ACC_SYS);	  		//check system login
				if((status & CHV_VERIFIED) == 0) return APDU_SECURITY_STATE_ERROR;
				if(command->P3 != 0x40) return APDU_WRONG_LENGTH; 
				//if(command->P3 != 0x10) return APDU_WRONG_LENGTH;
				len = fs_generate_FSH(command->bytes + 0x40);
				if(memcmp(command->bytes, command->bytes + 0x40, 0x40) == 0) {
					_os_config.os_state &= ~YGG_ST_ACTIVATED;
					Save_State();
					return APDU_SUCCESS;
				}
				return APDU_MEMORY_PROBLEM;

			case INS_GEN_FSH:						 		
				status = getCHVstatus(ACC_SYS);	  		//check system login
				if((status & CHV_VERIFIED) == 0) return APDU_INSTRUCTION_INVALID;
				if(command->P1 != 0x02) return APDU_INSTRUCTION_INVALID;
				if(command->P2 != 0x70) return APDU_INSTRUCTION_INVALID; 
				len = fs_generate_FSH(command->bytes);
				if(command->P3 != len) return APDU_WRONG_LENGTH;
				memcpy(command, command->bytes, len);
				response_length = len;
				return APDU_SUCCESS;

			case INS_WRITE_SERNUM:	
				Get_Data();	
				len = command->P3;
				MMU_SEL=1; 
				rP3 = 7;									//update bank 1	
				//memcpy(, command->bytes, 0x10);
				//if(Erase_Page((BYTEX *)0x8020) != SUCCESS) return APDU_FLASH_WERROR;
				ReadFlash(IOBuf, 0x8020, 0x10);
				for(offset = 0; offset < 0x10; offset++) {
					if(IOBuf[offset] != 0xFF) return APDU_FLASH_WERROR;
				}
				Write_Bytes((BYTEX *)0x8020, (BYTEX *)command->bytes, 0x10);
				//memcpy(IOBuf, command->bytes, len);
				//if(UpdateFlash(0x8020, IOBuf, 0x10) != SUCCESS) return APDU_FLASH_WERROR;
				//memset(command, 0, len);
				//memcpy(command, IOBuf, 0x10);
				//response_length = len;
				response_length = 0;
				return APDU_SUCCESS;

			case INS_READ_SERNUM:	
				len = command->P3;
				MMU_SEL=1; 
				rP3 = 7;									//update bank 1	
				//ReadFlash(IOBuf, 0x8020, 0x10);
				memset(command, 0, len);
				ReadFlash(command, 0x8020, 0x10);
				response_length = len;
				return APDU_SUCCESS;

			default:
				response_length = 0;
				return APDU_INSTRUCTION_INVALID;
		}
			//break;

	//GSM11 STANDARD CLASS APDU	
	} else if(command->CLA == CLA_GSM11) {
		switch(command->INS)
		{
			case INS_GET_RESPONSE :
				if(response_data != NULL) {
					if(command->P3 > ((response_buffer *)response_data)->length) return APDU_WRONG_LENGTH;
					Set_Response(((response_buffer *)response_data)->buffer, command->P3);
					m_free(response_data);
					response_data = NULL;
				}
				//status = (fetch_len)?(APDU_STK_RESPONSE | fetch_len):APDU_SUCCESS;
				/*if(lst == INS_RUN_GSM_ALGORITHM) {
					Set_Response(STK_buffer + 240, command->P3);
					status = APDU_SUCCESS;
				} else {
					Set_Response(command->bytes, command->P3);
					status = (fetch_len)?(APDU_STK_RESPONSE | fetch_len):APDU_SUCCESS;
				}*/
				status = SAT_status();
				return status;
			case INS_SELECT	:		//select file/direktori
				goto ygg_ins_select;
			case INS_STATUS	:		//get status file/direktori
				//lst = INS_STATUS;
				len =  command->P3;
				if((command->P1|command->P2) == 0) {
					memset(command->bytes, 0, len);
					response_length = Send_Status(STATUS_DIRECTORY, command->bytes);
					if(len > response_length) {
						response_length = len;
						status = (APDU_WRONG_LENGTH | response_length); //wrong length .OR. requested length
					} else {
						response_length = len;
						status = APDU_SUCCESS;
					} 
				} else {
					response_length = 0;
					status = APDU_WRONG_PARAMETER;	//wrong p1,p2
				}
				Set_Response(command->bytes, response_length);
				//return system_tick();	
				return SAT_status();	
			case INS_READ_BINARY :	//read binary
				goto ygg_ins_readbin;
			case INS_READ_RECORD :	//read record
				goto ygg_ins_readrec;				
			case INS_UPDATE_BINARY :	//update binary	 
				goto ygg_ins_writebin;			
			case INS_UPDATE_RECORD :	//update record
				goto ygg_ins_writerec;		
			case INS_VERIFY_CHV :   
				Get_Data();
				response_length = 0;
				Set_Response(command->bytes, 0);
				if(command->P3 == 0x08) {
					if(command->P1 ==0x00) {
						switch(command->P2) {
							case ACC_CHV1 :
							case ACC_CHV2 :
								status = chv_verify(command->P2, command->bytes);
								break;
							default :
								status = APDU_WRONG_PARAMETER;
								break;
						}
						//if(status & CHV_BLOCKED) return APDU_AUTH_BLOCKED;
						//if(status == CHV_VERIFIED) return APDU_SUCCESS;
						//return APDU_SECURITY_STATE_ERROR;
					} else {
						status = APDU_WRONG_PARAMETER;
					}
				} else {
					status = APDU_WRONG_LENGTH | 0x08;
				}
				return status;		
			case INS_REHABILITATE :		//rehabilitate
				goto ygg_ins_rehab;			
			case INS_INVALIDATE :		//invalidate
				goto ygg_ins_invalid;				
			case INS_CHANGE_CHV	:
				Get_Data();	
				//lst = INS_CHANGE_CHV;
				response_length = 0;
				//Set_Response(command->bytes, 0);
				if(command->P3 == 0x10) {
					if(command->P1 ==0x00) {
						switch(command->P2) {
							case ACC_CHV1 :
							case ACC_CHV2 :
								status = chv_change(command->P2, command->bytes, (command->bytes+8));
								break;
							default :
								status = APDU_WRONG_PARAMETER;
								break;
						}
						//if(status == CHV_BLOCK) return APDU_AUTH_BLOCKED;
						//if(status == CHV_VERIFIED) return APDU_SUCCESS;
						//return APDU_SECURITY_STATE_ERROR;
					} else {
						status = APDU_WRONG_PARAMETER;
					}
				} else {
					status = APDU_WRONG_LENGTH | 0x10;
				}
				return status;		
			case INS_DISABLE_CHV :		//operasi disable hanya mendukung CHV1 saja
				Get_Data();
				//lst = INS_DISABLE_CHV;
				response_length = 0;
				//Set_Response(command->bytes, 0);
				if(command->P3 == 0x08) {
					if(command->P1 ==0x00) {
						switch(command->P2) {
							case ACC_CHV1 :
								//status = chv_get_status(ACC_CHV1);
								status = chv_disable(ACC_CHV1, command->bytes);
								break;
							default :
								status = APDU_WRONG_PARAMETER;
								break;
						}
						//if(status == CHV_BLOCK) return APDU_AUTH_BLOCKED;
						//if(status == CHV_VERIFIED) return APDU_SUCCESS;
						//return APDU_SECURITY_STATE_ERROR; 
					} else {
						status = APDU_WRONG_PARAMETER;
					}
				} else {
					status = APDU_WRONG_LENGTH | 0x08;
				}
				return status;				
			case INS_ENABLE_CHV :		//operasi enable hanya mendukung CHV1 saja
				Get_Data();
				//lst = INS_ENABLE_CHV;
				response_length = 0;
				//Set_Response(command->bytes, 0);
				if(command->P3 == 0x08) {
					if(command->P1 ==0x00) {
						switch(command->P2) {
							case ACC_CHV1 :
								//status = chv_get_status(ACC_CHV1);
								status = chv_enable(ACC_CHV1, command->bytes);
								break;
							default :
								status = APDU_WRONG_PARAMETER;
								break;
						}
						//if(status == CHV_BLOCK) return APDU_AUTH_BLOCKED;
						//if(status == CHV_VERIFIED) return APDU_SUCCESS;
						//return APDU_SECURITY_STATE_ERROR;
					} else {
						status = APDU_WRONG_PARAMETER;
					}
				} else {
					status = APDU_WRONG_LENGTH | 0x08;
				}
				return status;				
			case INS_UNBLOCK_CHV :
				Get_Data();
				//lst = INS_UNBLOCK_CHV;
				response_length = 0; 
				//Set_Response(command->bytes, 0);
				if(command->P3 == 0x10) {
					if(command->P1 ==0x00) {
						switch(command->P2) {
							case 0x00 :		//khusus operasi unblock CHV1
								status = chv_unblock(ACC_CHV1, (command->bytes+8), command->bytes);
								break;
							case ACC_CHV2 :
								status = chv_unblock(ACC_CHV2, (command->bytes+8), command->bytes);
								break;
							default :
								status = APDU_WRONG_PARAMETER;
								break;
						}
						//if(status == CHV_BLOCK) return APDU_AUTH_BLOCKED;
						//if(status == CHV_VERIFIED) return APDU_SUCCESS;
						//return APDU_SECURITY_STATE_ERROR;
					} else {
						status = APDU_WRONG_PARAMETER;
					}
				} else {
					status = APDU_WRONG_LENGTH | 0x10;
				}
				return status;						
			case INS_SEEK :	
				Get_Data();
				//lst = INS_SEEK;
				response_length = 0;
				if((status = _check_access(&_ygg_fs, FILE_READ)) == APDU_SUCCESS) {
					status = _seek(&_ygg_fs, command->P2, command->bytes, command->bytes, command->P3);
					if((status & 0xFF00) == APDU_SUCCESS_RESPONSE) {
						switch(command->P2 >> 4) {
							case 0:
								response_length = 0;
								//Set_Response(command->bytes, 0);
								return APDU_SUCCESS;
							case 1:
								response_length = 0;
								command->bytes[0] = status & 0xff;
								//get_resp_length = 1;
								//Set_Response(command->bytes, 1);
								gsm_response(command->bytes, 1);
								return APDU_SUCCESS_RESPONSE | 1;
							default:
								response_length = 0;
								//Set_Response(command->bytes, 0);
								return APDU_WRONG_PARAMETER;
						}
					} 
				}
				//Set_Response(command->bytes, 0);
				return status;				
			case INS_INCREASE :	
				Get_Data();
				//lst = INS_INCREASE;	
				response_length = 0;
				len = command->P3;
				command->P3 = 0;
				//status = _increaserec(command->P3, command->bytes);
				response_length = 0;
				if((status = _check_access(&_ygg_fs, FILE_INCREASE)) == APDU_SUCCESS) {
					status = _increase(&_ygg_fs, *(uint32 *)&command->P3, command->bytes, len);
					//APDU_SUCCESS_RESPONSE | 1;
					response_length = 0;
				}
				if((status & 0xFF00) == APDU_SUCCESS_RESPONSE) {
					response_length = (uchar)status;		//simpan response length hasil increase untuk keperluan get response
				}
				gsm_response(command->bytes, response_length);
				//Set_Response(command->bytes, response_length);
				return status;		
			case INS_RUN_GSM_ALGORITHM:	//run GSM algorithm COMP128-1
				Get_Data();
				//lst = INS_RUN_GSM_ALGORITHM;
				len = command->P3;
				if((command->P1|command->P2) == 0) {
					if(len != A3A8_INPUT_SIZE) {
						response_length = 0;
						return APDU_OUT_OF_RANGE;
					}
					response_length = Authenticate_GSM(command->bytes, command->bytes);
					//response_length = 0;
					gsm_response(command->bytes, response_length);
					status = APDU_SUCCESS_RESPONSE | response_length;
					//memcopy(buf, iobuf->bytes, 0, A3A8_INPUT_SIZE);
					//nilai KI disimpan pada system file yang hanya bisa diakses kernel
				} else {
					response_length = 0;
					status = APDU_WRONG_PARAMETER;	//wrong p1,p2
				}
				//Set_Response(command->bytes, response_length);
				return status;
				break;	
			case INS_SLEEP :
				//lst = INS_SLEEP;
				//Set_Response(command->bytes, 0);
				return APDU_SUCCESS;

/////////////////////////////////////////////SIM APPLICATION TOOLKIT HANDLER/////////////////////////////////////////////////
			case INS_TERMINAL_PROFILE:
				Get_Data();
				status = APDU_SUCCESS;
				len = command->P3;
				SAT_profile_download(command->bytes, command->P3); 		//liquid profile download  
				//if(SAT_init(command->bytes, 0) == 0) return APDU_STK_OVERLOAD;		//clear SAT temporary file and filled it with terminal response
				if(VAS_activated()) {
#if (VAS_ALLOCATED)
					if(sat_init_state == SAT_MENU_INIT) {
						status = VAS_menu();
						sat_init_state = SAT_EVENT_INIT;
					}
#endif
				} else {
#if (LIQUID_ALLOCATED) 
					status = liquid_profile();
#endif
				}
				//_stk_state = ROOTMENU;
				//Set_Response(command->bytes, len);
				return status;
			case INS_TERMINAL_RESPONSE:			//result processing
				Get_Data();
				status = APDU_SUCCESS;
				//STK_pop(command->bytes, &tag, &size, STK_buffer);
				//memcpy(command->bytes, STK_buffer, size);
				if(VAS_activated()) {	
#if (VAS_ALLOCATED)
					//if(SAT_init(command->bytes, 0) == 0) return APDU_STK_OVERLOAD;		//clear SAT temporary file and filled it with terminal response
					//if(vas_state == VAS_STATE_WAIT_RESPONSE) return 0x9C00;		//ignore terminal response on wait response
					switch(sat_init_state) {
						case SAT_MENU_INIT:
							status = VAS_menu();		
							sat_init_state = SAT_EVENT_INIT;
							if(status != APDU_SUCCESS) break;
						case SAT_EVENT_INIT:
							//sat_init_state = SAT_DEFAULT;
							status = VAS_event();
							sat_init_state = SAT_STARTUP_INIT;
							if(status != APDU_SUCCESS) break;
						case SAT_STARTUP_INIT: 
							status = VAS_startup();
							sat_init_state = SAT_DEFAULT;
							if(status != APDU_SUCCESS) break;
						case SAT_DEFAULT:
							status = SAT_status(); 											//check if data already fetched
							if(status == APDU_SUCCESS) {
								//if((vas_state & 0x0F) != WAIT_FOR
								status = VAS_fetch(command->bytes, command->P3);
							}
							break;
					}
#endif
				} else {
#if (LIQUID_ALLOCATED)  
					res = 0;
					i = 0;
					len = command->P3;	  		//total size   
					if(SAT_init(command->bytes, len) == 0) return APDU_STK_OVERLOAD;		//clear SAT temporary file and filled it with terminal response
					if(_stk_menu_current == 0xFFFF) {
					 	_stk_menu_current = 0;
						_stk_menu_offset = 0;
						goto exit_response;
					}
					process_response:
					callback_result = CALLBACK_SUCCESS;
					terminal_result = STK_RES_SUCCESS;
					//status = SAT_response();
					//stknode = (stk_config *)m_alloc(sizeof(stk_config));		//current stk node
					//file_readbin(&stk_fs, _stk_menu_anchor, stknode, sizeof(stk_config)); 
					while(i != len) {
						i += SAT_file_pop(i, &tag, &size, STK_buffer);
						tag |= 0x80; 
						switch(tag) {			//TERMINAL HANDLER
							case STK_TAG_CMD_DETAIL:
							case STK_TAG_DEV_ID: break;			//don't process this tag
						 	case STK_TAG_ITEM_ID:  
								state = (STK_buffer[0] & 0x0F);
								//stknode = (stk_config *)m_alloc(sizeof(stk_config));	
								_readbin(&stk_fs, _stk_menu_anchor, &stknode, sizeof(stk_config));
								if(stknode.child == 0) break;
								_stk_menu_offset = stknode.child;
								_readbin(&stk_fs, _stk_menu_offset, &stknode, sizeof(stk_config));
								res = 0;
								while(++res != state) {
									_stk_menu_offset = stknode.sibling;
									if(_stk_menu_offset == 0) { return APDU_SUCCESS; }
									_readbin(&stk_fs, _stk_menu_offset, &stknode, sizeof(stk_config));
									//if(_stk_menu_offset == 0) break;
									//else 
									//res ++;
								} 
								_stk_menu_offset = stknode.child;
								break;
							case STK_TAG_RESULT:   //cek result
								terminal_result = STK_buffer[0];
								_readbin(&stk_fs, _stk_menu_current, &stknode, sizeof(stk_config));
								_stk_menu_offset = stknode.sibling;
								break;
							default: 
								break;
						} 	  
						/* callback mechanism */
						_readbin(&stk_fs, _stk_menu_current, &stknode, sizeof(stk_config));
						callback_result = user_callback(stknode.state, tag, size, STK_buffer);
						switch((callback_result >> 5) & 0x03) {
							case 1:	 					//message only (error/warning), return to tree (stay in current node)
								//callback_result |= 1;			 
							case 0:	 					//message only (goto next node)	 
								if(callback_result == 0) break;
								_readbin(&stk_fs, _stk_menu_current, &stknode, sizeof(stk_config));	  //redundant
								if(stknode.child == 0) break;
								_stk_menu_offset = stknode.child;
								_readbin(&stk_fs, _stk_menu_offset, &stknode, sizeof(stk_config));
								res = 0;
								while(++res != (callback_result & 0x0F)) {
									_stk_menu_offset = stknode.sibling;
									if(_stk_menu_offset == 0) { return APDU_SUCCESS; }
									_readbin(&stk_fs, _stk_menu_offset, &stknode, sizeof(stk_config));
									//if(_stk_menu_offset == 0) break;
								}
								i = len; 		//quit decode
								goto finish_decode;
								
	//#define CALLBACK_STAY_NODE		(1<<5)
	//#define CALLBACK_TAMPER_RESULT	(1<<6)
	//#define CALLBACK_FORCE_END		(1<<7)
							case 3:	  						//tamper STK tree operation (stay in current node)
								//callback_result |= 1;
							case 2:	  						//tamper STK tree operation (goto next node)
								status = SAT_status();
								//return status;
								//if(callback_result & 0x40) _stk_menu_offset = _stk_menu_current;		//stay node
								_stk_menu_offset = stknode.sibling;  		//next node
								if(_stk_menu_offset == 0) { 
									_stk_menu_offset = 0xFFFF; 	//disable liquid handler
								} 
								if(callback_result & CALLBACK_FORCE_END) {  		//cek force end
									_stk_menu_offset = 0xFFFF;
								} 
								goto finish_decode;
								break;
						}
						//}
						//m_free(stknode);	
					} 
					//default state
					switch(terminal_result) {
						case STK_RES_NETWORK_FAIL:	//result only warning, select current node (worth retrying)
						case STK_RES_ME_FAIL: 	   
						case STK_RES_USER_TIMEOUT:
						case STK_RES_USER_ABORT:   				
							//_readbin(_stk_menu_offset, STK_buffer, sizeof(stk_config));
							//_readbin(_stk_menu_offset + sizeof(stk_config), STK_buffer, ((stk_config *)STK_buffer)->length);
							if(++_tres_cntr < 4) {
								_stk_menu_offset = _stk_menu_current;
								break;
							}
						case STK_RES_ME_ERROR:		// error 
						case STK_RES_ME_TYPE_ERROR:
						case STK_RES_ME_DATA_ERROR:
						case STK_RES_ME_NUM_ERROR:
						case STK_RES_ERROR:
						case STK_RES_SMS_ERROR:	  		//result is error
						case STK_RES_SS_ERROR: //ME unable to process (not worth retrying)
							 _stk_menu_offset = 0xFFFF;
							 status = APDU_STK_OVERLOAD;
							 break;
						case STK_RES_TRANSACTION_ABORT:			//skip node on transaction abort
						case STK_RES_PARTIAL:
						case STK_RES_MISSING:
						case STK_RES_REFRESH:
						case STK_RES_NO_ICON:
						case STK_RES_MOD_CALL:
						case STK_RES_LIMIT_SERVICE:
						case STK_RES_SUCCESS: 	   //result success, select child node (direct child)
							_tres_cntr = 0;	 
							if(_stk_menu_offset == 0) { 
								_stk_menu_offset = 0xFFFF; 	//disable liquid handler
								//fetch_len = 0;
								status = APDU_SUCCESS;
							}
							break; 
						default:		   						//terminate session
						case STK_RES_TERMINATED:
						case STK_RES_NO_USER_RESPONSE:
							_tres_cntr = 0;
							_stk_menu_offset = 0xFFFF;		  
							//fetch_len = 0;	  
							status = APDU_SUCCESS;
							goto finish_decode;
							break;
						case STK_RES_HELP_REQUIRED:				 //stay on current node	  
							_tres_cntr = 0;
							_stk_menu_offset = _stk_menu_current;
							break;
						case STK_RES_BACKWARD:
							_tres_cntr = 0;
							_readbin(&stk_fs, _stk_menu_current, &stknode, sizeof(stk_config)); 	
							_stk_menu_offset = stknode.parent;
							if(_stk_menu_offset == 0) { 
								_stk_menu_offset = 0xFFFF; 	//disable liquid handler
								//fetch_len = 0;	
								status = APDU_SUCCESS;
							}
							goto finish_decode;	 		//don't execute callback
							break;
					}
					finish_decode:
					if(_stk_menu_offset != 0xFFFF) {	 //liquid handler
						_readbin(&stk_fs, _stk_menu_offset, &stknode, sizeof(stk_config));
						if(stknode.parent == 0xFFFF) goto exit_response;		//uninitialized tree structure (invalid parent)
						len = stknode.length;
						//check terminal profile first before sending any proactive SIM command here
						//and return the appropriate terminal response
						//.....
						if(liquid_proactive_check(stknode.tag) == 0) goto exit_response; 	//proactive not supported
				
						//..end of profile checking
						//set menu anchor for several operation
						//if((callback_result & CALLBACK_STAY_NODE) == 0) {
						if(stknode.tag == 0x25) { _stk_menu_anchor = _stk_menu_offset; }
						if(stknode.tag == 0x24) _stk_menu_anchor = _stk_menu_offset;
						if((callback_result & CALLBACK_STAY_NODE) == 0) {
							_stk_menu_current = _stk_menu_offset;
						}
						if((callback_result & CALLBACK_TAMPER_RESULT) == 0) { 			//use default action tree
							_readbin(&stk_fs, _stk_menu_offset + sizeof(stk_config), STK_buffer, len);
							status = SAT_response(STK_buffer, len);
						}	  														//return 91XX
					} else {	
						if((callback_result & CALLBACK_STAY_NODE) == 0) {
							_stk_menu_anchor = 0;
							_stk_menu_current = 0xFFFF;
						}
					}			
					if(callback_result & CALLBACK_STAY_NODE) { 
						_readbin(&stk_fs, _stk_menu_current, &stknode, sizeof(stk_config));
						if(stknode.parent != 0) {
							_stk_menu_current = stknode.parent;
							_stk_menu_offset = _stk_menu_current;
						}
					}	  
					if(callback_result & CALLBACK_FORCE_END) {  		//cek force end
						_stk_menu_anchor = 0;
						_stk_menu_current = 0xFFFF;
					}
					//m_free(stknode);
#endif
				}
				exit_response:
				return status;
			case INS_ENVELOPE:
				Get_Data();
				status = APDU_SUCCESS;
				if(VAS_activated()) {
#if (VAS_ALLOCATED)
					//if(vas_state == VAS_STATE_WAIT_RESPONSE) {
					status = VAS_execute(command->bytes);
					goto exit_response;	
#endif
				} else {
#if (LIQUID_ALLOCATED)
					//status = SAT_envelope(command->bytes, command->P3); 
					SAT_pop(command->bytes, &dtag, &size, STK_buffer);
					memcpy(command->bytes, STK_buffer, size); 
					res = 0;
					i = 0;
					len = size;	  		//total size
					if(SAT_init(command->bytes, len) == 0) return APDU_STK_OVERLOAD;
					switch(dtag) {
						case ENV_TAG_MENU:	   //MENU SELECTION	
							if(liquid_profile_check(TERMINAL_MENU_SELECT) == 0) break;
							goto process_response; 		//lempar ke TERMINAL RESPONSE
							//status = SAT_response();
							break;
						/*case ENV_TAG_SMS_PP:   //SMS POINT TO POINT
							if(liquid_profile_check(TERMINAL_SMSPP_DOWNLOAD) == 0) break;
							status = decode_SMSPP(command->bytes, len);
							break;
						case ENV_TAG_SMS_CB:   //SMS CELL BROADCAST	
							if(liquid_profile_check(TERMINAL_SMSCB_DOWNLOAD) == 0) break;
							status = decode_SMSCB(command->bytes, len);
							break;*/
						case ENV_TAG_CALL:	   //CALL CONTROL
				
							break;
						case ENV_TAG_SMS_PP:
						case ENV_TAG_SMS_CB:
						case ENV_TAG_EVENT:
							//goto process_response; 		//lempar ke TERMINAL RESPONSE
							//break;
						case ENV_TAG_TIMER:
							//len = command->P3;	  		//total size
							//goto process_response; 		//lempar ke TERMINAL RESPONSE
							while(i != len) {
								i += SAT_file_pop(i, &tag, &size, STK_buffer);
								tag |= 0x80; 
								switch(tag) {			//TERMINAL HANDLER
									case STK_TAG_CMD_DETAIL:
									case STK_TAG_DEV_ID: break;			//don't process this tag
								 	case STK_TAG_ITEM_ID:  
										//break;
									case STK_TAG_RESULT:   //cek result
										//terminal_result = STK_buffer[0];
										//file_readbin(&stk_fs, _stk_menu_current, stknode, sizeof(stk_config));
										//_stk_menu_offset = stknode->sibling;
									default: break;
									case STK_TAG_TIMER_IDENTIFIER:
										res = STK_buffer[0];
										break; 
									case STK_TAG_TIMER_VALUE:
										//file_readbin(&stk_fs, _stk_menu_current, stknode, sizeof(stk_config));
										if(dtag == ENV_TAG_TIMER) {
											//timer_callback(res, tag, size, STK_buffer);
										}
										break;
									case STK_TAG_SMS_TPDU:
										if(dtag == ENV_TAG_SMS_PP) {  
											status = decode_SMSTPDU(1, STK_buffer);
										}
										if(dtag == ENV_TAG_SMS_CB) {
											status = decode_SMSTPDU(0, STK_buffer);
										}
										break;
								}
							}
							//status = SAT_status();
							break;
						//default: return APDU_FATAL_ERROR;		//UNKNOWN TAG 
					}
#endif
				}
				return status;
			case INS_FETCH:
#if VAS_ALLOCATED
				if(vas_state == VAS_STATE_WAIT_RESPONSE) {
					vas_state = VAS_STATE_NORMAL;		//normal execution
				}
#endif
				SAT_read(command->bytes, command->P3);
				//fetch_len = 0;
				SAT_cleanup();
				status = APDU_SUCCESS;
				//status = liquid_fetch(command->bytes, command->P3);
				//SAT_cleanup();
				Set_Response(command->bytes, command->P3);
				return status;
/////////////////////////////////////////END OF SIM APPLICATION TOOLKIT HANDLER//////////////////////////////////////////////
			//case 0x55:
			//	while(1);
			default:
				//Set_Response(command->bytes, 0);
				return APDU_INSTRUCTION_INVALID;					
		}
	}
	return APDU_CLASS_INVALID;
	/*{
	//	default:
		if(_os_config.os_state & YGG_ST_ENABLE_APP) {
			return Execute_User_App(command);
		} else {
			response_length = 0;
			return APDU_CLASS_INVALID;
		}
		//break;
	}*/
}

#define PM_CMD_START	0
#define PM_CMD_ICCID	1
#define PM_CMD_DISPIN1	2
#define PM_CMD_ENPIN1	3
#define PM_CMD_PUK1		5
#define PM_CMD_PUK2		6
#define PM_CMD_PIN1		9
#define PM_CMD_PIN2		10
#define PM_CMD_PIN3		11
#define PM_CMD_ADM		12
#define PM_CMD_SYS		14
#define PM_CMD_IMSI		0x10
#define PM_CMD_ACC		0x11
#define PM_CMD_KI		0x12
#define PM_CMD_OTAKEY	0x13
#define PM_CMD_0348KEY00	0x20
#define PM_CMD_0348KEY01	0x21
#define PM_CMD_0348KEY02	0x22
#define PM_CMD_0348KEY03	0x23
#define PM_CMD_0348KEY04	0x24
#define PM_CMD_0348KEY05	0x25
#define PM_CMD_0348KEY06	0x26
#define PM_CMD_0348KEY07	0x27
#define PM_CMD_0348KEY08	0x28
#define PM_CMD_0348KEY09	0x29
#define PM_CMD_0348KEY10	0x2A
#define PM_CMD_0348KEY11	0x2B
#define PM_CMD_0348KEY12	0x2C
#define PM_CMD_0348KEY13	0x2D
#define PM_CMD_0348KEY14	0x2E
#define PM_CMD_0348KEY15	0x2F
#define PM_CMD_0348KEY16	0x30
#define PM_CMD_0348KEY17	0x31
#define PM_CMD_0348KEY18	0x32
#define PM_CMD_0348KEY19	0x33
#define PM_CMD_0348KEY20	0x34
#define PM_CMD_0348KEY21	0x35
#define PM_CMD_0348KEY22	0x36
#define PM_CMD_0348KEY23	0x37
#define PM_CMD_0348KEY24	0x38
#define PM_CMD_0348KEY25	0x39
#define PM_CMD_0348KEY26	0x3A
#define PM_CMD_0348KEY27	0x3B
#define PM_CMD_0348KEY28	0x3C
#define PM_CMD_0348KEY29	0x3D
#define PM_CMD_0348KEY30	0x3E
#define PM_CMD_0348KEY31	0x3F

uint16 Bifrost_Decode(uchar * buffer, uchar length) _REENTRANT_ {
	uint16 status = APDU_SUCCESS;
	uchar i = 0;
	uchar size;
	uchar tag;
	fs_handle temp_fs;
	chv_file chv;
	while(i < length) {
		tag = buffer[i++];
		size = buffer[i++];
		switch(tag) {
			case PM_CMD_START:
				break;
			case PM_CMD_ICCID:
				_select(&temp_fs, FID_MF);
				status = _select(&temp_fs, FID_ICCID);
				if(status < APDU_SUCCESS_RESPONSE) return status;
				status = _writebin(&temp_fs, 0, buffer + i, size); 
				if(status != APDU_SUCCESS) return status;
				break;
			case PM_CMD_DISPIN1:
				chv_get_config(1, &chv);
				chv_disable(1, chv.pin); 
				//m_free(chv);
				break;
			case PM_CMD_ENPIN1:
				chv_get_config(1, &chv);
				chv_enable(1, chv.pin);
				//m_free(chv);
				break; 
			case PM_CMD_PUK1:
				chv_get_config(1, &chv);
				memcpy(chv.puk, buffer + i, 8);
				ioman_write_buffer(CHV_DATA_OFFSET + (0 * sizeof(chv_file)), &chv, sizeof(chv_file));
				//m_free(chv);
				break;
			case PM_CMD_PUK2:
				chv_get_config(2, &chv);
				memcpy(chv.puk, buffer + i, 8);
				ioman_write_buffer(CHV_DATA_OFFSET + (1 * sizeof(chv_file)), &chv, sizeof(chv_file));
				//m_free(chv);
				break;
			case PM_CMD_PIN1: 
				chv_get_config(1, &chv);
				memcpy(chv.pin, buffer + i, 8);
				ioman_write_buffer(CHV_DATA_OFFSET + (0 * sizeof(chv_file)), &chv, sizeof(chv_file));
				//m_free(chv);
				break;
			case PM_CMD_PIN2:
				chv_get_config(2, &chv);
				memcpy(chv.pin, buffer + i, 8);
				ioman_write_buffer(CHV_DATA_OFFSET + (1 * sizeof(chv_file)), &chv, sizeof(chv_file));
				//m_free(chv);
				break;
			case PM_CMD_PIN3:
				break;
			case PM_CMD_ADM:
				chv_get_config(4, &chv);
				memcpy(chv.pin, buffer + i, 8);
				ioman_write_buffer(CHV_DATA_OFFSET + (3 * sizeof(chv_file)), &chv, sizeof(chv_file));
				//m_free(chv);
				break;
			case PM_CMD_SYS:
				chv_get_config(6, &chv);
				memcpy(chv.pin, buffer + i, 8);
				ioman_write_buffer(CHV_DATA_OFFSET + (5 * sizeof(chv_file)), &chv, sizeof(chv_file));
				//m_free(chv);
				break;
			case PM_CMD_IMSI:  
				_select(&temp_fs, FID_MF);
				_select(&temp_fs, FID_GSM);
				status = _select(&temp_fs, FID_IMSI);
				if(status < APDU_SUCCESS_RESPONSE) return status;
				status = _writebin(&temp_fs, 0, buffer + i, size); 
				if(status != APDU_SUCCESS) return status;
				break;
			case PM_CMD_ACC:
				_select(&temp_fs, FID_MF);
				_select(&temp_fs, FID_GSM);
				status = _select(&temp_fs, FID_ACC);
				if(status < APDU_SUCCESS_RESPONSE) return status;
				status = _writebin(&temp_fs, 0, buffer + i, size);
				if(status != APDU_SUCCESS) return status;
				break;
			case PM_CMD_KI:	  
				_select(&temp_fs, FID_MF);
				_select(&temp_fs, FID_LIQUID);
				status = _select(&temp_fs, FID_AUTHKEY);
				if(status < APDU_SUCCESS_RESPONSE) return status;
				status = _writebin(&temp_fs, 0, buffer + i, size);
				if(status != APDU_SUCCESS) return status;
				break;
			case PM_CMD_OTAKEY:
				_select(&temp_fs, FID_MF);
				_select(&temp_fs, FID_LIQUID);
				status = _select(&temp_fs, FID_RFMKEY);
				if(status < APDU_SUCCESS_RESPONSE) return status;
				status = _writebin(&temp_fs, 0, buffer + i, size);
				if(status != APDU_SUCCESS) return status;
				break;
			case PM_CMD_0348KEY00:
			case PM_CMD_0348KEY01:
			case PM_CMD_0348KEY02:
			case PM_CMD_0348KEY03:
			case PM_CMD_0348KEY04:
			case PM_CMD_0348KEY05:
			case PM_CMD_0348KEY06:
			case PM_CMD_0348KEY07:
			case PM_CMD_0348KEY08:
			case PM_CMD_0348KEY09:
			case PM_CMD_0348KEY10:
			case PM_CMD_0348KEY11:
			case PM_CMD_0348KEY12:
			case PM_CMD_0348KEY13:
			case PM_CMD_0348KEY14:
			case PM_CMD_0348KEY15:
			case PM_CMD_0348KEY16:
			case PM_CMD_0348KEY17:
			case PM_CMD_0348KEY18:
			case PM_CMD_0348KEY19:
			case PM_CMD_0348KEY20:
			case PM_CMD_0348KEY21:
			case PM_CMD_0348KEY22:
			case PM_CMD_0348KEY23:
			case PM_CMD_0348KEY24:
			case PM_CMD_0348KEY25:
			case PM_CMD_0348KEY26:
			case PM_CMD_0348KEY27:
			case PM_CMD_0348KEY28:
			case PM_CMD_0348KEY29:
			case PM_CMD_0348KEY30:
			case PM_CMD_0348KEY31: 
				_select(&temp_fs, FID_MF);
				_select(&temp_fs, FID_LIQUID);
				status = _select(&temp_fs, FID_0348_KEY);
				if(status < APDU_SUCCESS_RESPONSE) return status;
				status = _writerec(&temp_fs, (tag & 0x3F), buffer + i, size);
				if(status != APDU_SUCCESS) return status;
				break;
			default:
				if(tag > 0x3F) {	
					_select(&temp_fs, FID_MF);
					_select(&temp_fs, FID_LIQUID);
					status = _select(&temp_fs, FID_WIBKEY);
					if(status < APDU_SUCCESS_RESPONSE) return status;
					status = _writerec(&temp_fs, (tag & 0x3F), buffer + i, size);
					if(status != APDU_SUCCESS) return status;
				}
				break;	
		}
		i += size;
	}
	return status;
}

void Set_Response(uchar * buffer, uchar length) _REENTRANT_ {
	/*switch(ISO7816_case()) {
	 	case 0:				   	//no data, no response
		case 2:	   			   	//with data, no response
 	 		get_resp_length = length;
	 		memcopy(iobuf + 5, buffer, 0, length);
			break;
		case 1:				 	//no data, with response
		case 3:					//with data, with response
			response_length = length;
			break;
		default: */
	/*if(apdu_le_value != 0) {   		//iso7816-3 case 2 or 4
		length = apdu_le_value;
	} */
	/*_iso7816_case += 1;
	if(_iso7816_case == 4) {
		memcopy(iso7816_buffer + 5, buffer, 0, length);	   //copy to temp buffer
		get_resp_length = length;
		response_length = 0;
	} else {*/
		//get_resp_length = 0;
	memcpy(iso7816_buffer, buffer, length);	   //copy to temp buffer
	get_resp_length = 0;
	response_length = length;
	//}
			//break;
	//}
}

void Get_Data() _REENTRANT_ {
	//register uchar i;	 
	TX_NULL_BYTE_OFF(8000)
	send_byte(iso7816_buffer[1]);			//send acknowledgement in order to and wait for data
	_iso7816_state = YGG_RECEIVE_DATA;
	//_iso7816_case = 3;							 //command+data only (no response)
	while(_iso7816_state != ISO7816_RUNNING);
	//ISO_DelayETU();		 
	//_iso7816_state = ISO7816_RUNNING;
	TX_NULL_BYTE_ON(8000)
}

void Load_State() _REENTRANT_ {
	ioman_read_buffer(YGG_ALLOCATION_CONFIG_OFFSET, &_os_config, sizeof(os_config));
	if(_os_config.os_tag != 'Y') {
		_os_config.os_state = 0;		//default state
		Save_State();
	}
}

void Save_State() _REENTRANT_ {
	_os_config.os_tag = 'Y';
	ioman_write_buffer(YGG_ALLOCATION_CONFIG_OFFSET, &_os_config, sizeof(os_config));
}

uint16 Load_User_App(void) _REENTRANT_ { 	//copy program from temp space to program space, maximum program size 12KB
	register uint16 status;
	//register uchar state = _os_config.os_state;
	//_os_config.os_state &= ~YGG_ST_ENABLE_APP;		//disable user app
	//Save_State();									//save state
	status = ioman_program_copy(0x8000, 0xC000, 0x4000);		//program copy
	//_os_config.os_state = state;					//restore state
	//Save_State();									//save state
	return status;
}

uint16 Write_Temp_Space(uint16 pos, uchar * bytes, uint16 length) _REENTRANT_ {
	//write app is encrypted
	uint16 len;
	//uint16 size = length;
	//Write_Temp_Space(offset, command->bytes, command->P3);
	if(pos < 0x8000) return APDU_OUT_OF_RANGE;   
	if((pos + length) > 0x8000) {	 		//range 0x8000-0xffff
		//pos += 0xA000;
		return ioman_write_program(pos, bytes, length);
	} else {
		return APDU_OUT_OF_RANGE;
	}
}

uint16 Load_Temp_Space(void) _REENTRANT_ {	
	_os_config.os_state |= YGG_ST_LOAD_APP;
	return APDU_SUCCESS;
} 

uint16 Auth_COMP128(uchar * inbuf, uchar * outbuf) _REENTRANT_ {
	uint16 status = APDU_SUCCESS;
	/*uchar * temp = m_alloc(A3A8_INPUT_SIZE);
	memcopy(temp, inbuf, 0, A3A8_INPUT_SIZE);
	status = auth_A3A8(temp, _os_config.gsm_ki, outbuf);
	m_free(temp); */
	return status;
}

/*void Yggdrasil_Main() {
	unsigned short i=0;
	switch(yggdrasil_state) {
	 	case YGG_DORMANT:
			IoInit(0x11);						//	H/W initial
			Send_ATR();							//	Send ATR
			yggdrasil_state = YGG_READY;	
			break;
		case YGG_READY:
			iso7816_buffer[0] = receive_byte();		//CLA
			iso7816_buffer[1] = receive_byte();		//INS
			iso7816_buffer[2] = receive_byte();		//P1
			iso7816_buffer[3] = receive_byte();		//P2
			iso7816_buffer[4] = receive_byte();		//P3
			for(i=5;i<(5 + iso7816_buffer[4]);i++) {
				 iso7816_buffer[i] = receive_byte();
			}
			_iso7816_state = ISO7816_RUNNING;
			break;
		case YGG_RUNNING:
			switch(iso7816_buffer[0]) {
				case 0xff:	 						//default iso7816 system class
					PPS_Handler(iso7816_buffer);
					break;
				default:
					Tx_Status(Command_Interpreter(iso7816_buffer));
					break;
			}
			_iso7816_state = ISO7816_READY;
			break;
		case YGG_STOP:
			break;
	}
} */

