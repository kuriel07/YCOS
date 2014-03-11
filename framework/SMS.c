#include "..\liquid.h"
#include "..\yggdrasil\application.h"			//user application callback	 
#include "..\midgard\midgard.h"
#include "..\framework\rfm.h"
#include "..\framework\vas.h"
#include "..\auth\crc.h"
#include "..\framework\dcs.h"
#include "..\framework\des.h"
#include "SMS.h"
#include <string.h>

static fs_handle _liquid_fs;
uint16 decode_SMSTPDU(uchar p2c, uchar * buffer) _REENTRANT_ { 		//+16~21 bytes
	uchar i = 0;//, j;
	uint16 status = APDU_SUCCESS;
	uchar type;
	//uchar pid;
	uchar dcs;
	uchar udl; 
	uchar addr_len;
	uchar * originating_address = NULL;			
	//TP-MTI
	type = buffer[i++];
	switch(type & 0x03) {
	 	case 0:		//sms deliver		  		//SMS-DELIVER - SMS-DELIVER_REPORT	 
			//TP-MMS
			if(type & (1<<2)) { //no more messages
		
			}
			//TP-RP
			if(type & (1<<7)) {	//reply path is set
		
			}
			//TP-OA (Originating Address)  		//4 bytes
			addr_len = (buffer[i] + 1) >> 1;						 //address length
			addr_len += 2;	
			//get TP-OA
			originating_address = (uchar *)m_alloc(addr_len);
			memcpy(originating_address, buffer + i, addr_len);
			i += addr_len;
			//TP-PID
			buffer[i++];
			//TP-DCS
			dcs = buffer[i++];
			//TP-SCTS
			i += 7;			//skip timestamp
			//TP-UDL
			udl = buffer[i++];
			break;
		case 1:		//sms submit
		case 2:		//sms command
		case 3:		//reserved
			return status;
	}
	//process DCS first before decode any packet, UCS2 to 8, 7 to 8
	switch(dcs >> 4) {
		case 0x08:	//reserved coding group
		case 0x09:
		case 0x0A:
		case 0x0B:
			break;
		case 0x0C:	//message waiting (discard message)
			break;
		case 0x0D:	//message waiting (store message)
			break;
		case 0x0E:	//message waiting (store message), UCS2 uncompressed
			break;
		case 0x00:	//general data coding information
		case 0x01:
		case 0x02:
		case 0x03:
		case 0x04:	//message marked for automatic deletion
		case 0x05:
		case 0x06:
		case 0x07:
		case 0x0F:	//data coding message class
			//buffer + i = pointer to user data length(2 bytes) + user data(variable)
			switch(dcs & 0x03) {
			 	case 0x00:	break;	//class 0
				case 0x01:	break;	//ME specific
				case 0x02:			//SIM specific
					if((dcs & 0x04) == 0) {
						//decompacting sms user data (including headers) and update user data length
						udl = (uint16)decode_728(buffer + i, buffer + i, udl); 	
					} 
					//decode user data	 (SIM specific message) in 8 bit mode
					if(p2c) {
						if((type & (1<<6)) == 0) { //contains header AND (U)SIM Data Download 
							//check sri and send feedback
							#if 1	  
							status = user_packet_decode(buffer + i, originating_address, udl);
							#endif
							m_free(originating_address);
						} else {
							m_free(originating_address);
							status = decode_SMSPPPacket(buffer + i);
						}
					} else {
						m_free(originating_address);
						status = decode_SMSCBPacket(buffer + i);
						//don't send any feedback to network
					}
					break;
				default: break;	
			}
			break;
	}

	//return tpdu; 
	//TP-SRI	(Status Report Indicator)
	if(type & (1<<5)) {	//a status report shall be returned to SME	return 9FXX on success and 9EXX on error   (target network)

	}
	//if(originating_address != NULL) {
	// 	m_free(originating_address);
	//}
	return status;
}

uint16 decode_SMSCBPacket(uchar * buffer) _REENTRANT_ {
	uint16 status = APDU_SUCCESS;
	uint16 sn;
	uint16 mid;
	//uchar dcs;
	//uchar pp;
	uchar i = 4;
	concat_property cp = {0, 0, 0, 0, 0xFF};		//TAG = 0xFF, invalid		 
	//concat_property * cp = m_alloc(sizeof(concat_property));
	//memset(cp, 0, sizeof(concat_property));
	sn = *(uint16 *)buffer;
	mid = *(uint16 *)(buffer+2);
	#define OFFDCS		0
	#define OFFPP		1
	//dcs = buffer[OFFDCS];
	//pp = buffer[OFFPP];
	i += 2;
	//decode command packet (different temporary file)
	//status =
	_select(&_liquid_fs, FID_MF);
	_select(&_liquid_fs, FID_LIQUID);  
	_select(&_liquid_fs, FID_0348_IN);
	//if(allocate_liquid_space(&_liquid_fs, FID_LIQTEMP_IN, 153)) {		   
		//write to temporary file													
	_writebin(&_liquid_fs, sizeof(concat_property) + cp.offset, buffer + i, 82);	//fixed block 82 octet
		//update concat property
	_writebin(&_liquid_fs, 0, (uchar *)&cp, sizeof(concat_property));		//update concat_property
	//}
	return p348_decode_command_packet();
}

uint16 decode_SMSPPPacket(uchar * buffer) _REENTRANT_ {	   //+11-16 bytes
	uint16 status = APDU_SUCCESS;
	uchar i = 0, j = 0;
	uchar udhl = 0, iei = 0xFF, ieidl = 0, udl = 0;

	#if 0
	concat_property cp = {0, 0, 0, 0, 0xFF};		//TAG = 0xFF, invalid
	#endif

	uchar rec_size = 0;
	#define OFFUDL		0
	#define OFFUDHL		1
	//#define OFFIEI		(i)
	//#define OFFIEIDL		(i+1)  
	#define OFFREFNUM	(0)
	#define OFFTOTALNUM	(1)
	#define OFFSEQNUM	(2)

	uchar ref_num = 0, seq_num = 1, total_num = 1, tag = 0xFF;
	uchar * ied;	

	_select(&_liquid_fs, FID_MF);
	_select(&_liquid_fs, FID_LIQUID);  
	_select(&_liquid_fs, FID_SMS_PACKET);
	//get current file record size
	ied = file_get_current_header(&_liquid_fs);
	rec_size = ((ef_header *)ied)->rec_size;
	m_free(ied);

	udl = buffer[i++];
	udhl = buffer[i++];		//udhl = header length + offset, end of header offset
	next_header:
	while(i != (udhl + 2)) {		  //decode each element data   
		iei = buffer[i++];	 	//IEI		Information Element Identifier
		ieidl = buffer[i++];	//information element identifier data length
		
		ied = (uchar *)m_alloc(ieidl);
		if(ied != NULL) {
			for(j=0;j<ieidl;j++) {
				ied[j] = buffer[i++];
			}
		}
		//callback mechanism here
		switch(iei) {
			case 0x00:		//concatenated short message (8-bit)
				//#define SMSUD_REF_NUM		0
				ref_num = ied[OFFREFNUM];
				total_num = ied[OFFTOTALNUM];//buffer[OFFTOTALNUM]; 
				seq_num = ied[OFFSEQNUM];

				//if(allocate_liquid_space(&_liquid_fs, FID_LIQTEMP_IN, (uint16)(153 * buffer[OFFTOTALNUM]))) {
				#if 0
				if(ied[OFFSEQNUM] == 1) {					//1st message, zeroing concat property
					cp.offset = 0;											  //reset concat property
					cp.counter = 0;
					cp.refnum = ied[OFFREFNUM];//buffer[OFFREFNUM];
				} else {							//1+n message, read concat property from file 
					file_readbin(&_liquid_fs, 0, (uchar *)&cp, sizeof(concat_property));
				}
				
				//check for valid sequence number
				if((cp.counter + 1) != ied[OFFSEQNUM]) { m_free(ied); status = APDU_STK_OVERLOAD; goto exit_decode; }		//not a valid sequence				  
				if(cp.refnum != ied[OFFREFNUM]) { m_free(ied); status = APDU_STK_OVERLOAD; goto exit_decode; }				//invalid refnum
				#endif
				//}
				break;
		 	case 0x70:		//SIM Toolkit security header (CPI)
				#if 0
				cp.tag = 0x70;
				#else
				tag = 0x70;
				#endif
				break;
			//default:
				//buffer[OFFREFNUM] = 0;
				//buffer[OFFTOTALNUM] = 0; 
				//buffer[OFFSEQNUM] = 1;
				//goto exit_decode;
		}
		//i += buffer[OFFIEIDL];
		//i += 2;		//sizeof(IEI) + sizeof(IEIDL)
		//i += ieidl;
		m_free(ied);
	}		   
	//write to temporary file
	buffer[udhl + 1] = (udl - udhl);		//size
	_writerec(&_liquid_fs, (uint16)seq_num, buffer + 1 + udhl, rec_size); //value
	_readrec(&_liquid_fs, (uint16)0, buffer, rec_size);					//concat property
	buffer[seq_num] = ref_num;
	if(tag == 0x70) { 														//command packet detected
		buffer[total_num + 1] = ref_num;
		buffer[0] = tag;
		if(total_num == 1) goto start_copy_packet; 						//single packet detected
	}
	_writerec(&_liquid_fs, (uint16)0, buffer, rec_size);					//concat property
	if(buffer[0] != 0x70) goto exit_decode;	 		//not command packet

	#if 0
	_writebin(&_liquid_fs, sizeof(concat_property) + cp.offset, buffer + 2 + udhl, (uint16)(udl - udhl));
	//update concat property
	cp.offset += (uint16)(udl - udhl) - 1;
	cp.counter++;
	_writebin(&_liquid_fs, 0, (uchar *)&cp, sizeof(concat_property));		//update concat_property   */
	#endif
	//check if all reference existed in the buffer (including command packet reference)
	for(iei = 1; iei <= (total_num + 1); iei++) {
		if(buffer[iei] != ref_num) goto exit_decode;	
	}
	//clear concat property
	buffer[0] = 0xFF;
	for(iei = 1; iei <= total_num; iei ++) {
		buffer[iei] = iei;
	}
	buffer[total_num + 1] = 0;
	_writerec(&_liquid_fs, (uint16)0, buffer, rec_size);					//concat proper
	//concat all message to temporary in for 0348 processing
	start_copy_packet:
	status = sizeof(concat_property);			//use status as offset
	for(iei = 1; iei <= total_num; iei++) {
		_select(&_liquid_fs, FID_LIQUID);
		_select(&_liquid_fs, FID_SMS_PACKET);		
		_readrec(&_liquid_fs, iei, buffer, rec_size);  
		_select(&_liquid_fs, FID_LIQUID);
		_select(&_liquid_fs, FID_0348_IN);
		_writebin(&_liquid_fs, status, buffer + 1, buffer[0]);
		status += (buffer[0] - 1);		
	}

	#if 0
	if(cp.tag == 0x70 && cp.counter == total_num) { 				//last message concatenated	
	#endif
		status = p348_decode_command_packet();  		//03.48 command packet decoder
	#if 0
	} else {
			
	}   
	#endif
	exit_decode:
	return status;
}

//#if ((SAT_MENU_MODE == SAT_MENU_VAS) && VAS_ALLOCATED)
//uchar encode_SMSTPDU(uchar type, uchar length, response_packet * rspkt, uchar * buffer_out) _REENTRANT_ {
//#elif ((SAT_MENU_MODE == SAT_MENU_LIQUID) && LIQUID_ALLOCATED)
uchar encode_SMSTPDU(uchar type, uchar pid, uchar dcs, uchar length, uchar * address, response_packet * rspkt, uchar * buffer_out) _REENTRANT_ {
//#endif
	//read from rspkt pointer, encode to 7 bit, calculate the response size, add smstpdu header store result to buffer_out, return SW 
	//message cannot be concatenated message!!
	uchar i = 0, j;
	uchar len = 0;
	//uchar data_offset = 0;
//#if ((SAT_MENU_MODE == SAT_MENU_VAS) && VAS_ALLOCATED)
   	
	
//#elif ((SAT_MENU_MODE == SAT_MENU_LIQUID) && LIQUID_ALLOCATED)
	//response_tpdu * rstpdu;
	switch(type & 0x03) {
	 	case 0:		//sms deliver		  		//SMS-DELIVER
			break;
		case 1:		//sms submit
			//TP-MTI
			buffer_out[i] = (type & 0x03) | 0x20;	//bit 7 = TP-RP (reply path), bit 5 = TP-SRR (request status), bit 4,3 = TP-VPF
			if(type & SMS_TYPE_NOREPORT) {
				buffer_out[i] &= ~(SMS_TYPE_NOREPORT);
			}
			if((type & SMS_TYPE_NOHEADER) == 0) {
				 buffer_out[i] |= 0x40;				//bit 6 = TP-UDHI
			}
			i++;
			//TP-MR
			buffer_out[i++] = 0xFF;		//auto corrected by ME
			//TP-DA
			buffer_out[i++] = address[0];		//length
			buffer_out[i++] = address[1];		//TON+NPI
			//memcpy(buffer_out + i, address + 1, address[0]);
			address[0] = (address[0]+1) >> 1;
			for(j=0;j<address[0];j++) {
				buffer_out[i++] = address[j+2];
			}
			//i += address[0];				   
			//TP-PID
			//buffer_out[i++] = 0x7C;		//ANSI136 R-DATA
			buffer_out[i++] = pid;		//short message type 0
			buffer_out[i++] = dcs;		//8bit, class 2	

			//TP-UDL
			//i += 2; 
			buffer_out[i++] = length;
			//buffer_out[i++] = length;	//user data length
			//TP-UD
			memcpy(buffer_out + i, rspkt, length);
			i += length;
			len = i;
			break;
		case 2:		//sms command
		case 3:		//reserved
			break;
	}
//#endif
	return len;			
}


//extern uint16 send_registration(uchar mode) _REENTRANT_;
///////////////////////////////////////////////////////////////////////////////////////////////////
///									03.48 Implementation										///
//decode command packet here (SPI2,KIC,KID,TAR,CNTR,PCNTR)										///
///////////////////////////////////////////////////////////////////////////////////////////////////
uint16 p348_decode_command_packet(void) _REENTRANT_ {		 //+36~41 bytes
	uint16 status = APDU_STK_RESPONSE;	  //2byte
	uint16 i = 0, j = 0;			//4byte
	uint16 length;					//2byte
	vas_config * vc = NULL;			//3byte
	command_packet * cmpkt;			//3byte
	//uchar rc_length = 0; 		//no ciphering
	uint32 crc32;					//4byte
	uint32 crc32_2;					//4byte
	uchar auth_length = 0;			//1byte
	//concat_property * cp = m_alloc(sizeof(concat_property));
	fs_handle temp_fs;				//8byte
	uchar * key = NULL;				//16byte 			-->47 byte	- 13 byte
	//static fs_handle dec_fs;
	//2 byte length		 		--> total packet length
	//1 byte header length		--> CHL
	//2 byte SPI				--> security parameter indication
	//1 byte KIc				--> ciphering key
	//1 byte KID				--> checksum
	//3 byte TAR				--> toolkit application reference
	//5 byte CNTR
	//1 byte PCNTR
	//n byte RC/CC/DS
	//select KEY  
	//memset(cp, 0, sizeof(concat_property));
	start_decode:
	//select temporary file

	cmpkt = (command_packet *)STK_buffer;
	//03.48 HEADER PROCESSING
	_readbin(&_liquid_fs, sizeof(concat_property), STK_buffer, sizeof(command_packet_header));
	#if 0
	if(memcmp(STK_buffer, "$REG$", 5) == 0) {
		return send_registration(1);
	}
	#endif

	///////////////////////////////////--------03.48 PACKET PREPROCESSSING--------///////////////////////////////////
	//TAR checking VAS or RFM	 
	if(memcmp(cmpkt->tar, "\x0\x0\x0", 3) == 0) {  		//use remote file management (OTA)
		//RFM SETUP
		//vc = m_alloc(sizeof(vas_config));
		//END OF RFM SETUP
	} else {	
#if VAS_ALLOCATED 
		//VAS SETUP
		//TAR/SC checking mechanism, verifying security configuration 
		vc = VAS_preprocess(cmpkt);			   //VAS checking by TAR
		if(vc == NULL) {					 
			//invalid TAR/SC, check if VAS config is valid
			liquid_set_response_data(RESPONSE_PKT_TAR_UNKNOWN, STK_buffer, 0);			//TAR unknown (key not found)
			goto exit_decode;
		}
		//counter checking mechanism

		 	 
		//END OF VAS SETUP
#else
		//VAS unallocated, immedietly quit
		liquid_set_response_data(RESPONSE_PKT_TAR_UNKNOWN, STK_buffer, 0);			//TAR unknown (key not found)
		goto exit_decode;
#endif
	}
	///////////////////////////////--------END OF 03.48 PACKET PREPROCESSSING--------///////////////////////////////


	/////////////////////////////////////--------03.48 HEADER PROCESSSING--------/////////////////////////////////////
	//calculate user data size from command_packet_header (0348_header)
	switch(cmpkt->spi[0] & 0x03) {
		case 0x00:	
			length = (cmpkt->cpl - cmpkt->chl) + 6; 		//8 = sizeof(cpl)+ sizeof(cntr)+ sizeof(pcntr) = 2+5+1
			break;		//no RC,CC,DS, no ciphering
		case 0x01:	//redundancy check 		(use crc32, no need to check KID)
			length = (cmpkt->cpl - cmpkt->chl) + 6;		//total packet length + CNTR + PCNTR + sizeof(CRC32)
			break;
		case 0x02:	//cryptography checksum	  (check KID), use MD5 (128bit)
			//rc_length = 16;
			break;
		case 0x03:	//digital signature	   (check KID) , use RSA
			//rc_length = 16;
			break;
	}
	//length = length of additional user data + checksum
	//Set_Response(STK_buffer, length + sizeof(command_packet_header) + 10);
	//status = (0x9100 | (length + sizeof(command_packet_header))+10);
	//return status; 
	_select(&temp_fs, FID_MF);	
	_select(&temp_fs, FID_LIQUID);
	//03.48 DECODE CIPHER TEXT
	if(cmpkt->spi[0] | 0x04) {		//cipher text (user data is encrypted)
		//check if remote file management (OTA)
		if(memcmp(cmpkt->tar, "\x0\x0\x0", 3) == 0) {  		//use remote file management (OTA)
			//LOAD RFM key 
			_select(&temp_fs, FID_RFMKEY);
			key = m_alloc(0x10);
			if(key == NULL) { status = APDU_STK_OVERLOAD; goto exit_decode; } 
			_readbin(&temp_fs, 0, key, 0x10);	
#if VAS_ALLOCATED
		} else if(vc != NULL) {
			//key load mechanism
			key = m_alloc(0x10);
			if(key == NULL) { status = APDU_STK_OVERLOAD; goto exit_decode; }
			//if(VAS_loadkey(vc, VAS_LOAD_KIC, (cmpkt->kic >> 4), key) == FALSE) {		//load key failed (invalid key
			//	m_free(key); goto exit_decode;			
			//}
			VAS_get_security_config(cmpkt->tar, VAS_SC_KIC, (cmpkt->kic >> 4), key);
#endif
		} else {
			//default action
		 	goto exit_decode;
		}

		switch(cmpkt->kic & 0x0F) {
			case 0x0C:
		 	case 0x08:
		 	case 0x04:
		 	case 0x00:	break;	//known algorithm
			case 0x0E:
			case 0x0A:
			case 0x06:
			case 0x02:	break;		//reserved
			case 0x0F:
			case 0x0B:
			case 0x07:
			case 0x03:	break;		//propietary implementation
			case 0x01:	//DES in CBC mode
				#if 0
				DES_Operation(length, DES_MODE_CBC, key, cmpkt->cntr, cmpkt->cntr);
				#else
				DES_FileOperation(length, sizeof(concat_property) + 10, DES_MODE_CBC, key, &_liquid_fs);
				#endif
				break;
			case 0x0D:	//DES in ECB mode
				#if 0
				DES_Operation(length, 0, vc->key, cmpkt->cntr, cmpkt->cntr); 
				#else
				DES_FileOperation(length, sizeof(concat_property) + 10, 0, key, &_liquid_fs);
				#endif
				break;
			case 0x05:	//3DES 2 key
				#if 0
				DES_Operation(length, DES_MODE_TDES | DES_MODE_CBC, key, cmpkt->cntr, cmpkt->cntr);
				#else
				DES_FileOperation(length, sizeof(concat_property) + 10, DES_MODE_TDES | DES_MODE_CBC, key, &_liquid_fs);
				#endif
				break;
			case 0x09:	//3DES 3 key
				//unsupported algorithm
				break;
		}  
		m_free(key);
	}

	//03.48 AUTHENTICATE COMMAND PACKET
	_readbin(&_liquid_fs, sizeof(concat_property),
		STK_buffer, sizeof(command_packet_header) + 6);			//6 = sizeof(cntr)+sizeof(pcntr)  
	length += cmpkt->pcntr;	
	//load WIB key for KID (only for cryptographic checksum)  , not implemented, only support crc32
	//VAS_loadkey(vc->kid_index, key);
	switch(cmpkt->spi[0] & 0x03) {
		case 0x00:	break;		//no RC,CC,DS, no ciphering
		case 0x01:	//redundancy check 		(use crc32, no need to check KID)
			#if 0
			crc32 = *(uint32 *)(cmpkt->ud);
			memcpy(cmpkt->ud, cmpkt->ud + 4, length - 4);		//compacting user data, removing checksum
			//calculate crc32 from user data
			auth_length = 4;
			//calculated_size = length + 10 - auth_length;
			//!!!!!!!!!!!!!!!MASALAH DISINI!!!!!!!!!!!	 (CRC32 ga jalan)
			if(CalCRC32(cmpkt, cmpkt->cpl - 2) != crc32) {		//skip cpl+chi from calculation
				//CRC32 failed
				status = APDU_SUCCESS;
				liquid_set_response_data(RESPONSE_PKT_AUTH_FAIL, STK_buffer, 0);			//RC/CC/DS failed
				goto exit_decode;
			} 
			#else
			_readbin(&_liquid_fs, sizeof(concat_property) + sizeof(command_packet_header), (uchar *)&crc32, 4);		//read crc32 value from temporary file
			auth_length = 4;
			//shift packet to right by 4 bytes, and replace crc32 value, start calculating crc from there
			_writebin(&_liquid_fs, sizeof(concat_property) + 4, (uchar *)cmpkt, sizeof(command_packet_header));
			
			//CPL+CHL included on CRC32 calculation (calculation length = CPL - 2)
			crc32_2 = FileCRC32(&_liquid_fs, cmpkt->cpl - 2, sizeof(concat_property) + 4);
			if(crc32_2 != crc32) {		//skip cpl+chi from calculation		
				_writebin(&_liquid_fs, sizeof(concat_property), (uchar *)&crc32_2, 4);
				_writebin(&_liquid_fs, sizeof(concat_property) + 4, (uchar *)&crc32, 4);
				//CRC32 failed
				//file_writebin(_liquid_fs, sizeof(concat_property) + 4, &crc32, 4);
				status = APDU_SUCCESS | (uchar)(length + 6);
				status = 0x9C00 | (uchar)(length + 6);
				//status = APDU_SUCCESS | cmpkt->pcntr;
				liquid_set_response_data(RESPONSE_PKT_AUTH_FAIL, STK_buffer, 0);			//RC/CC/DS failed
				goto exit_decode;
			}
			#endif
			break;
		case 0x02:	//cryptography checksum	  (check KID)
			break;
		case 0x03:	//digital signature	   (check KID)
			break;
	} 
	
	//prepare temporary out file before decoding running any service  
	//status = APDU_STK_RESPONSE | (uchar)length; 
	//m_free(cph);				//free command packet header (packet configuration)
	length = length - auth_length;		//actual command without padding
	length = length - cmpkt->pcntr;		//actual command packet without padding
	//Set_Response(cmpkt->cntr, length);
	//status = (0x9100 | length);
	//return status;
	//save command packet, STK buffer will be used as copy temporary
	//allocate command packet, copy from STK_buffer to command_packet
	cmpkt = (command_packet *)m_alloc(sizeof(command_packet_header));
	memcpy(cmpkt, STK_buffer, sizeof(command_packet_header));
	//copy from liquidtempin to liquidtempout (STK_buffer freed here)
#if 0

#else 
	//copy concatenated message to temporary out
	//if(allocate_liquid_space(&temp_fs, FID_LIQTEMP_OUT, length + 128)) {
	//status = 0x9A00;
	//goto exit_decode;	 
	_select(&temp_fs, FID_0348_OUT);		   
	for(i=0;i<length;i+=128) {
		_readbin(&_liquid_fs, sizeof(concat_property) + auth_length + sizeof(command_packet_header) + i, STK_buffer, 128);
		_writebin(&temp_fs, i, STK_buffer, 128);	
	}
	//status = 0x9A00;
	//goto exit_process_packet;
	//} else {
	 	//error allocating temporary out, not enough space
		
	//}
#endif
	liquid_set_response_data(RESPONSE_PKT_POR_OK, STK_buffer, 0);			//clear response packet
	/////////////////////////////////--------END OF 03.48 HEADER PROCESSSING--------/////////////////////////////////


	//------------------------------ START OF SELECT SERVICE ------------------------------
	//check TAR here, decide if it belong to RFM(OTA), VAS or other
	if(memcmp(cmpkt->tar, "\x0\x0\x0", 3) == 0) {		//check if RFM (OTA)
		p348_set_tar(cmpkt->tar);	  					//set current 0348TAR for packet encoding
		status = RFM_decode(&temp_fs, 0, length);
		//restart SIM initialization sequence 
		SAT_printf("cd", (REFRESH & 0x7F), STK_DEV_ME);
		status = SAT_status();
#if VAS_ALLOCATED 
	} else if(vc != NULL) {											//use VAS(WIB)
		switch(cmpkt->spi[0] >> 3) {
			case 0x00: break;	//no counter, abort process mechanism
			case 0x01: break;						//counter available, no checking
			case 0x02:								//counter available, checking mechanism
			case 0x03: 
				if(VAS_get_security_config(cmpkt->tar, VAS_SC_ISPI, 0, STK_buffer) == FALSE) goto exit_process_packet;
				if(VAS_replay_check((cmpkt->spi[0] >> 3), cmpkt->cntr, STK_buffer) == FALSE) goto exit_process_packet;
				_select(&_liquid_fs, FID_MF);	
				_select(&_liquid_fs, FID_WIB); 
				_select(&_liquid_fs, FID_W0348CNTR);
				_writerec(&_liquid_fs, vc->icntr_index, cmpkt->cntr, 5);		//update counter
				break; 
		}   
		//select EF_tar
		//VAS_init(cmpkt->ud + 2, *(uint16 *)cmpkt->ud);  
		p348_set_tar(cmpkt->tar);			   			//set current 0348TAR for packet encoding
		_readbin(&temp_fs, 0, &length, 2);
		if(VAS_init(&temp_fs, 2, length) == TRUE)			//initialize VAS
			status = VAS_decode();		//ignore CNTR and PCNTR, (also might checksum, implementation dependent) 
	}
#else 
	}
#endif
	exit_process_packet:
	//------------------------------  END OF SELECT SERVICE  ------------------------------


	m_free(cmpkt);				//free command packet, obsolote variable
	//memcpy(STK_buffer, "ABC", 3);
	//liquid_set_response_data(0, STK_buffer, 3);
	exit_decode:
	if(vc != NULL) {
		m_free(vc);					//free key config (contain address + ota/wib key)
	}
	//push response TPDU as response so it can be fetched at the next instruction
	return status;
	//return cmpkt;
}

uchar _tar_0348[3] = { 0, 0, 0} ;
void p348_set_tar(uchar * tar) _REENTRANT_ {
	memcpy(_tar_0348, tar, 3);
	#if VAS_ALLOCATED
	VAS_set_tar(_tar_0348);
	#endif
}

uchar p348_create_header(uchar * buffer) _REENTRANT_ {
	//fs_handle temp_fs;
	TARconfig tar_config;
	uchar length = 0;
	uint16 i = 0, j;
	//header->ppl = 0;
	//header->phl = sizeof(header_0348_packet) - 2;
	if(memcmp(_tar_0348, "\x0\x0\x0", 3) == 0) {
		((header_0348_packet *)buffer)->ppl = 0;
		((header_0348_packet *)buffer)->phl = sizeof(header_0348_packet) - 2 + 6;
		memset(buffer + 3, 0, 12);
		((header_0348_packet *)buffer)->pcntr = 0;
		length = sizeof(header_0348_packet);
	} else {
#if VAS_ALLOCATED 
		VAS_get_security_config(_tar_0348, VAS_SC_OSPI, 0, buffer + 12); 
		memcpy(buffer + 16, _tar_0348, 3);						//copy application TAR
		VAS_get_security_config(_tar_0348, VAS_SC_OCNTR, 0, buffer + 19);
		//set command packet header
		((header_0348_packet *)buffer)->ppl = 0;					//packet length					(2byte)
		((header_0348_packet *)buffer)->phl = sizeof(header_0348_packet) - 2 + 6;  	//header length (1byte)
		memcpy(buffer + 3, buffer + 12, 12);						//spi+kic+kid+tar+cntr 			(4+3+5byte)
		((header_0348_packet *)buffer)->pcntr = 0;					//padding counter 				(1byte)
		length = sizeof(header_0348_packet);
	}
#else
		return 0;
 	}
#endif
	exit_create_header:
	return length;
}

#if 0
//encode response packet using on data on EFres
uchar p348_encode_response_packet(response_packet * rspkt) _REENTRANT_ {
	//read response from EF_SATTempout, add response packet header based on cmpkt, use cipher, cc depend on spi,kic,kid
	//return null on no response 
	//uint16 status = APDU_STK_RESPONSE;
	uint16 i = 0; 					//2byte
	uchar j = 0;					//1byte
	uint16 length;					//1byte
	uchar pcntr = 0;					//1byte
	//vas_config * vc = NULL;			//3byte
	//response_packet * rspkt = NULL;	  //3byte
	//uchar rc_length = 0; 			//no ciphering
	uint32 crc32;					//4byte
	uchar auth_length = 0;			//1byte
	uchar key[0x10];				//16byte
	//concat_property cp = {0, 0, 0};
	//2 byte length		 		--> total packet length
	//1 byte header length		--> CHL
	//2 byte SPI				--> security parameter indication
	//1 byte KIc				--> ciphering key
	//1 byte KID				--> checksum
	//3 byte TAR				--> toolkit application reference
	//5 byte CNTR
	//1 byte PCNTR
	//1 byte STATUS
	//n byte RC/CC/DS
	//select KEY
	//vc = (vas_config *)m_alloc(sizeof(vas_config));
	//status = APDU_SUCCESS;		//no key available
	//goto exit_encode;
	start_encode: 
	switch(rspkt->spi[0] & 0x03) {	 		//calculate auth_length first before allocating any response_packet
		case 0x00: break;						//no RC,CC,DS, no ciphering
		case 0x01: auth_length = 4; break; 		//redundancy check (CRC32)
		case 0x02: break; 						//cryptography checksum
		case 0x03: break;						//digital signature
		default: break;
	}
	
	//select temporary file
	_select(&_liquid_fs, FID_MF); 
	_select(&_liquid_fs, FID_LIQUID); 
	if(_select(&_liquid_fs, FID_RES) < 0x9F00) { goto exit_encode; }		//read temporary out, it should have been filled with data
	//read temporary file, calculate it length, allocate memory based on response length for rspkt
	//...
	_readbin(&_liquid_fs, 0, (uchar *)&length, sizeof(uint16));  //get response size
	//if(length == 0) goto exit_encode;						//no response available 	
	length += auth_length;		//add length with auth_length, userdata = auth code + additional code 
	if(rspkt->spi[0] & 0x04) { 
		pcntr = ((uint16)8 - ((length + 7) % 8)) % 8;
		//clear user data, also create padding
		memset(rspkt->ud, 0, length + pcntr);
		rspkt->pcntr = pcntr;												//set padding counter
	} else { 
		memset(rspkt->ud, 0, length);
	}
	_readbin(&_liquid_fs, 2, &rspkt->status, 1);  		//fill header->status 
	//fill user data with response data on temporary file
	_readbin(&_liquid_fs, 3, rspkt->ud, length - auth_length);	
	//calculate new RPL and RHL
	rspkt->rpl = length + (sizeof(response_packet_header) - 2) + pcntr;	//header+userdata+auth (RPL not included)
	rspkt->rhl = sizeof(response_packet_header) - 3;			//RPL+RHL not included
	
	
	switch(rspkt->spi[0] & 0x03) {
		case 0x00:	
			//length = cmpkt->cpl - (sizeof(command_packet_header) - 2);
			break;		//no RC,CC,DS, no ciphering
		case 0x01:	//redundancy check 		(use crc32, no need to check KID)
			//calculate crc32 from user data
			//auth_length = 4;
			crc32 = CalCRC32((uchar *)rspkt, rspkt->rpl - (auth_length));
			for(j=(length - auth_length);j!=0;j--) {
				//memcpy(cmpkt->ud + 4, cmpkt->ud, (length - auth_length));
				//cmpkt->ud[j + 4] = cmpkt->ud[j];
				rspkt->ud[j + 3] = rspkt->ud[j - 1];
			}
			//memcpy(rspkt->ud + 4, rspkt->ud, (length - auth_length));
			memcpy(rspkt->ud, &crc32, 4);			//copy auth code to response packet
			break;
		case 0x02:	//cryptography checksum	  (check KID), use MD5 (128bit)
			//rc_length = 16;
			break;
		case 0x03:	//digital signature	   (check KID) , use RSA
			//rc_length = 16;
			break;
	}
	//return length + sizeof(response_packet_header);
	if(rspkt->spi[0] & 0x04) {		//cipher text
		//length = rspkt->rpl - 7; 
		//load configuration key
		if(VAS_get_security_config(_tar_0348, VAS_SC_KIC, (rspkt->kic >> 4), key) != TRUE) goto exit_encode;		//failed to load key
		switch(rspkt->kic & 0x0F) {
		 	case 0x0C:
		 	case 0x08:
		 	case 0x04:
		 	case 0x00:	break;	//known algorithm
			case 0x0E:
			case 0x0A:
			case 0x06:
			case 0x02:	break;		//reserved
			case 0x0F:
			case 0x0B:
			case 0x07:
			case 0x03:	break;		//propietary implementation
			
			case 0x01:	//DES in CBC mode
				#if 1
				DES_Operation(length, DES_MODE_CBC | DES_MODE_ENCRYPT, key, rspkt->cntr, rspkt->cntr);
				#else
				DES_FileOperation(length, sizeof(concat_property) + 10, DES_MODE_CBC, key, _liquid_fs);
				#endif
				break;
			case 0x0D:	//DES in ECB mode
				#if 1
				DES_Operation(length, DES_MODE_ENCRYPT, key, rspkt->cntr, rspkt->cntr); 
				#else
				DES_FileOperation(length, sizeof(concat_property) + 10, 0, key, _liquid_fs);
				#endif
				break;
			case 0x05:	//3DES 2 key
				#if 1
				DES_Operation(length, DES_MODE_TDES | DES_MODE_CBC | DES_MODE_ENCRYPT, key, rspkt->cntr, rspkt->cntr);
				#else
				DES_FileOperation(length, sizeof(concat_property) + 10, DES_MODE_TDES | DES_MODE_CBC, key, _liquid_fs);
				#endif
				break;
			case 0x09:	//3DES 3 key
				//unsupported algorithm
				break;
		}
		//length = j;		//length automatically padded to 8 byte, might contain alloc_chain (memory allocator)
	}
	//status = APDU_STK_RESPONSE | (uchar)length; 
	//m_free(cph);				//free command packet header (packet configuration)
	//check TAR here, decide if it belong to RFM(OTA), VAS or other
	exit_encode:
	//m_free(vc);					//free key config (contain address + ota/wib key)
	//STK buffer now contain CNTR, PCNTR, (might also checksum), Data
	//Set_Response(STK_buffer, length);
	//return rspkt;
	return length + sizeof(response_packet_header) + pcntr;
}
#endif

#if 1
uchar p348_encode_command_packet(command_packet * cmpkt) _REENTRANT_ {
	//read response from EF_SATTempout, add response packet header based on cmpkt, use cipher, cc depend on spi,kic,kid
	//return null on no response 
	//uint16 status = APDU_STK_RESPONSE;
	uint16 i = 0; 					//2byte
	uchar j = 0;					//1byte
	uint16 length;					//1byte
	uchar pcntr = 0;					//1byte
	//vas_config * vc = NULL;			//3byte
	//response_packet * cmpkt = NULL;	  //3byte
	//uchar rc_length = 0; 			//no ciphering
	uint32 crc32;					//4byte
	uchar auth_length = 0;			//1byte
	uchar key[0x10];				//16byte
	//concat_property cp = {0, 0, 0};
	//2 byte length		 		--> total packet length
	//1 byte header length		--> CHL
	//2 byte SPI				--> security parameter indication
	//1 byte KIc				--> ciphering key
	//1 byte KID				--> checksum
	//3 byte TAR				--> toolkit application reference
	//5 byte CNTR
	//1 byte PCNTR
	//1 byte STATUS
	//n byte RC/CC/DS
	//select KEY
	//vc = (vas_config *)m_alloc(sizeof(vas_config));
	//status = APDU_SUCCESS;		//no key available
	//goto exit_encode;
	start_encode: 
	switch(cmpkt->spi[0] & 0x03) {	 		//calculate auth_length first before allocating any response_packet
		case 0x00: break;						//no RC,CC,DS, no ciphering
		case 0x01: auth_length = 4; break; 		//redundancy check (CRC32)
		case 0x02: break; 						//cryptography checksum
		case 0x03: break;						//digital signature
		default: break;
	}
	
	//select temporary file
	_select(&_liquid_fs, FID_MF); 
	_select(&_liquid_fs, FID_LIQUID); 
	if(_select(&_liquid_fs, FID_RES) < 0x9F00) { goto exit_encode; }		//read temporary out, it should have been filled with data
	//read temporary file, calculate it length, allocate memory based on response length for cmpkt
	//...
	_readbin(&_liquid_fs, 0, (uchar *)&length, sizeof(uint16));  //get response size
	//if(length == 0) goto exit_encode;						//no response available 	
	length += auth_length;		//add length with auth_length, userdata = auth code + additional code 
	if(cmpkt->spi[0] & 0x04) {
		pcntr = ((uint16)8 - ((length + 6) % 8)) % 8;
		//clear user data, also create padding
		memset(cmpkt->ud, 0, length + pcntr);
		cmpkt->pcntr = pcntr;												//set padding counter
	} else { 
		memset(cmpkt->ud, 0, length);
	}
	//file_readbin(&_liquid_fs, 2, &cmpkt->status, 1);  		//fill header->status 
	//fill user data with response data on temporary file
	_readbin(&_liquid_fs, 3, cmpkt->ud, length - auth_length);	
	//calculate new RPL and RHL
	cmpkt->cpl = length + (sizeof(command_packet_header) - 2) + pcntr;	//header+userdata+auth (RPL not included)
	cmpkt->chl = sizeof(command_packet_header) - 3;			//RPL+RHL not included
	
	
	switch(cmpkt->spi[0] & 0x03) {
		case 0x00:	
			//length = cmpkt->cpl - (sizeof(command_packet_header) - 2);
			break;		//no RC,CC,DS, no ciphering
		case 0x01:	//redundancy check 		(use crc32, no need to check KID)
			//calculate crc32 from user data
			//auth_length = 4;
			crc32 = CalCRC32((uchar *)cmpkt, cmpkt->cpl - (auth_length));
			for(j=(length - auth_length);j!=0;j--) {
				//memcpy(cmpkt->ud + 4, cmpkt->ud, (length - auth_length));
				//cmpkt->ud[j + 4] = cmpkt->ud[j];
				cmpkt->ud[j + 3] = cmpkt->ud[j - 1];
			}
			memcpy(cmpkt->ud, &crc32, 4);			//copy auth code to response packet
			break;
		case 0x02:	//cryptography checksum	  (check KID), use MD5 (128bit)
			//rc_length = 16;
			break;
		case 0x03:	//digital signature	   (check KID) , use RSA
			//rc_length = 16;
			break;
	}
	//return length + sizeof(response_packet_header);
	if(cmpkt->spi[0] & 0x04) {		//cipher text
		//length = cmpkt->rpl - 7;
		#if VAS_ALLOCATED
		if(VAS_get_security_config(_tar_0348, VAS_SC_KIC, (cmpkt->kic >> 4), key) != TRUE) goto exit_encode;		//failed to load key
		#endif
		switch(cmpkt->kic & 0x0F) {
		 	case 0x0C:
		 	case 0x08:
		 	case 0x04:
		 	case 0x00:	break;	//known algorithm
			case 0x0E:
			case 0x0A:
			case 0x06:
			case 0x02:	break;		//reserved
			case 0x0F:
			case 0x0B:
			case 0x07:
			case 0x03:	break;		//propietary implementation
			
			case 0x01:	//DES in CBC mode
				#if 1
				DES_MemOperation(length, DES_MODE_CBC | DES_MODE_ENCRYPT, key, cmpkt->cntr, cmpkt->cntr);
				#else
				DES_FileOperation(length, sizeof(concat_property) + 10, DES_MODE_CBC, key, _liquid_fs);
				#endif
				break;
			case 0x0D:	//DES in ECB mode
				#if 1
				DES_MemOperation(length, DES_MODE_ENCRYPT, key, cmpkt->cntr, cmpkt->cntr); 
				#else
				DES_FileOperation(length, sizeof(concat_property) + 10, 0, key, _liquid_fs);
				#endif
				break;
			case 0x05:	//3DES 2 key
				#if 1
				DES_MemOperation(length, DES_MODE_TDES | DES_MODE_CBC | DES_MODE_ENCRYPT, key, cmpkt->cntr, cmpkt->cntr);
				#else
				DES_FileOperation(length, sizeof(concat_property) + 10, DES_MODE_TDES | DES_MODE_CBC, key, _liquid_fs);
				#endif
				break;
			case 0x09:	//3DES 3 key
				//unsupported algorithm
				break;
		}
		//length = j;		//length automatically padded to 8 byte, might contain alloc_chain (memory allocator)
	}
	//status = APDU_STK_RESPONSE | (uchar)length; 
	//m_free(cph);				//free command packet header (packet configuration)
	//check TAR here, decide if it belong to RFM(OTA), VAS or other
	exit_encode:
	//m_free(vc);					//free key config (contain address + ota/wib key)
	//STK buffer now contain CNTR, PCNTR, (might also checksum), Data
	//Set_Response(STK_buffer, length);
	//return cmpkt;
	return length + sizeof(command_packet_header) + pcntr;
}
#endif