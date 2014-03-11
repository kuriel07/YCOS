#ifndef _DEFS_DEFINED
#include "..\defs.h"
#endif
#ifndef _CONFIG__H
#include "..\config.h"
#endif
#ifndef __SMS_H	   

								 
#define SMS_TYPE_PACKED7		0x10
#define SMS_TYPE_NOHEADER		8	
#define SMS_TYPE_NOREPORT		0x80
#define SMS_TYPE_DELIVER		0
#define SMS_TYPE_SUBMIT			1
#define SMS_TYPE_COMMAND		2

 
struct TARconfig {
	uchar type;
	uchar tar[3];
	uchar sc_index;
};

typedef struct TARconfig TARconfig;

/* SMS-PP APIs */
#if 1
uint16 decode_SMSCB(uchar * buffer, uchar length) _REENTRANT_ ;		/* SMS CB Handler */
uint16 decode_SMSPP(uchar * buffer, uchar length) _REENTRANT_ ;		/* SMS PP Handler */
#endif
uint16 decode_SMSTPDU(uchar p2c, uchar * buffer) _REENTRANT_ ;		//peer or cell
uint16 decode_SMSPPPacket(uchar * buffer) _REENTRANT_ ;
uint16 decode_SMSCBPacket(uchar * buffer) _REENTRANT_ ;
uchar encode_SMSTPDU(uchar type, uchar pid, uchar dcs, uchar length, uchar * address, response_packet * rspkt, uchar * buffer_out) _REENTRANT_ ;

uint16 p348_decode_command_packet() _REENTRANT_ ;
void p348_set_tar(uchar * tar) _REENTRANT_ ;
uchar p348_create_header(uchar * buffer) _REENTRANT_ ;
uchar p348_encode_response_packet(response_packet * rspkt) _REENTRANT_ ;
uchar p348_encode_command_packet(command_packet * cmpkt) _REENTRANT_ ;
//#if ((SAT_MENU_MODE == SAT_MENU_VAS) && VAS_ALLOCATED)
//uchar encode_SMSTPDU(uchar type, uchar length, response_packet * rspkt, uchar * buffer_out) _REENTRANT_ ;
//#elif ((SAT_MENU_MODE == SAT_MENU_LIQUID) && LIQUID_ALLOCATED)
//#endif
#define __SMS_H
#endif