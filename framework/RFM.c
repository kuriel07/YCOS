#include "RFM.h" 
#include "..\asgard\file.h"
#include "..\midgard\midgard.h"
#include "..\liquid.h"
#include <string.h>

extern uchar STK_buffer[];
uint16 RFM_decode(fs_handle * handle, uint16 address, uint16 length) _REENTRANT_  {
	uint16 status = APDU_SUCCESS;
	//uint16 i = address;
	uchar j;
	ef_header * curfile;
	fs_handle rfm_fs;
	_select(&rfm_fs, FID_MF);
	while(address < length) {
		_readbin(handle, address, STK_buffer, 5);	 //read command
		address += 5;
		if(STK_buffer[0] == CLA_GSM11) {		//GSM operation
			switch(STK_buffer[1]) {
				case INS_SELECT	:
					_readbin(handle, address, STK_buffer + 5, STK_buffer[4]);	 	//Get_Data()
					address += STK_buffer[4];
					for(j=0;j<STK_buffer[4];j+=2) {
						status = _select(&rfm_fs, *((uint16 *)(STK_buffer + 5 + j)));
						if(status < 0x9F00) { //file not found
							address=length;		//cancel all commands to prevent corrupting operation
							break;
						}
					}
					break;			
				case INS_READ_BINARY :
					break;
				case INS_READ_RECORD :
					break;				
				case INS_UPDATE_BINARY :
					curfile = NULL;
					curfile = file_get_current_header(&rfm_fs);
					if(curfile != NULL) {
						if(curfile->type == T_EF) {
							if((curfile->acc_inc & 0xF0) == 0) { 		//check if OTA updateable
								_readbin(handle, address, STK_buffer + 5, STK_buffer[4]);		//Get_Data()
								address += STK_buffer[4];
								status = _writebin(&rfm_fs, *((uint16 *)(STK_buffer + 2)), STK_buffer + 5, STK_buffer[4]);
							} 
						}
						m_free(curfile);
					}
					break;			
				case INS_UPDATE_RECORD : 
					curfile = NULL;
					curfile = file_get_current_header(&rfm_fs);
					if(curfile != NULL) {
						if(curfile->type == T_EF) {
							if((curfile->acc_inc & 0xF0) == 0) { 		//check if OTA updateable
								_readbin(handle, address, STK_buffer + 5, STK_buffer[4]);		//Get_Data()
								address += STK_buffer[4];
								switch(STK_buffer[3]) {
									case P_NEXT_REC :
										status = _writerec_next(&rfm_fs, STK_buffer + 5, STK_buffer[4]);
										break;
									case P_PREV_REC :
										status = _writerec_prev(&rfm_fs, STK_buffer + 5, STK_buffer[4]);
										break;
									case P_ABS_REC :
										status = _writerec(&rfm_fs, STK_buffer[2] - 1, STK_buffer + 5, STK_buffer[4]);
										break;
									default :
										break;
								}
							} 
						}
						m_free(curfile);
					}
					break;				
				case INS_REHABILITATE :	
					status = _rehabilitate(&rfm_fs);
					break;			
				case INS_INVALIDATE : 
					status = _invalidate(&rfm_fs);
					break;				
				case INS_SEEK :	
					break;				
				case INS_INCREASE :
					break;
			}	
		}
	}
	//locate response+status(SW) and put in response buffer
	status = APDU_SUCCESS;			//finished RFM
	//status = RFM_refresh()
	return status;
}