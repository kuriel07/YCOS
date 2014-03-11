#ifndef _DEFS_DEFINED
#include "..\defs.h"
#endif
#ifndef _CONFIG__H
#include "..\config.h"
#endif
#ifndef __LIQUID_H
//#include "..\defs.h"
//#include "..\yggsys\yggapis.h" 
//#include "..\yggsys\gsm.h"
//#include "..\config.h"
//#include "..\yggsys\gsm.h" 
//#include "services\vas.h"
// SIM APPLICATION TOOLKIT DEFINITION
#define STK_ME_CAPABILITY_SIZE		6
#define APDU_STK_RESPONSE			0x9100
#define APDU_STK_ERROR				0x9E00
#define APDU_STK_OVERLOAD 			0x9300
	  
#define FID_LIQUID					0x7FA0	//DF_Liquid
#define FID_STKMENU					0x6F20	//EF_STKMenu
#define FID_0348_IN					0x6F71	//EF_0348In	
#define FID_0348_OUT				0x6F70	//EF_0348Out
#define FID_SMS_PACKET				0x6F7C
#define FID_0348_KEY				0x6FE1	//0348 keylist	 
#define FID_RFMKEY					0x6FE0	//EF_RFMKey
#define FID_WIBKEY					0x6FE8	//EF_WIBkey
#define FID_VAS						0x6FA4	//EF_VASTemp		//
#define FID_SAT						0x6FA7	//EF_SATTemp		//SAT command data
#define FID_RES						0x6FA2	//EF_RESTemp 
#define FID_AUTHKEY					0x6F38

/* STK TAG SET */
#define STK_TAG_CMD_DETAIL			0x81
#define STK_TAG_DEV_ID				0x82	
#define STK_TAG_RESULT				0x83
#define STK_TAG_DURATION			0x84
#define STK_TAG_ALPHA				0x85
#define STK_TAG_ADDRESS				0x86
#define STK_TAG_CAPABILITY			0x87
#define STK_TAG_SUBADDRESS			0x88
#define STK_TAG_SS_STRING			0x89
#define STK_TAG_USSD_STRING			0x8A
#define STK_TAG_SMS_TPDU			0x8B
#define STK_TAG_CB_PAGE				0x8C
#define STK_TAG_TEXT_STRING			0x8D
#define STK_TAG_TONE				0x8E
#define STK_TAG_ITEM				0x8F
#define STK_TAG_ITEM_ID				0x90
#define STK_TAG_RESPONSE_LENGTH		0x91
#define STK_TAG_FILE_LIST			0x92
#define STK_TAG_LOCATION_INFO		0x93
#define STK_TAG_IMEI				0x94
#define STK_TAG_HELP_REQUEST		0x95
#define STK_TAG_DEFAULT_TEXT		0x97
#define STK_TAG_EVENT_LIST			0x99
#define STK_TAG_TIMER_IDENTIFIER	0xA4
#define STK_TAG_TIMER_VALUE			0xA5
#define STK_TAG_DTMF_STRING			0xAC
//class B
#define STK_TAG_AT_COMMAND			0xA8
#define STK_TAG_AT_RESPONSE			0xA9
//class C
#define STK_TAG_BROWSER_ID			0xB0
#define STK_TAG_URL					0xB1
#define STK_TAG_BEARER				0xB2
//Called party subaddress tag 1 1 "08"
//SS string tag 1 1 "09"
//Reserved for USSD string tag 1 1 "0A"
//SMS TPDU tag 1 1 "0B"
//Cell Broadcast page tag 1 1 "0C"
//#define STK_TAG_TEXT				0x0D
//Tone tag 1 1 "0E"
//#define STK_TAG_ITEM				0x0F
//#define STK_TAG_ITEM_ID				0x10
//#define STK_TAG_RESPONSE_LGTH		0x11
//Address tag 1 1 "12"
//Menu selection tag 1 1 "13"
//File List tag 1 1 "14"
//Location Information tag 1 1 "15"
//IMEI tag 1 1 "16"
/* STK CMD DETAIL TYPE */
#define REFRESH 		1
#define MORE_TIME		2
#define POLL_INTERVAL	3
#define POLLING_OFF		4
#define SET_UP_EVENT_LIST	5
#define SET_UP_CALL		0x10
#define SEND_SS			0x11
#define SEND_USSD		0x12
//- "12" = Reserved for SEND USSD;
#define SEND_SHORT_MESSAGE 	0x13
#define SEND_DTMF			0x14

#define PLAY_TONE			0x20
#define DISPLAY_TEXT		0x21
#define GET_INKEY			0x22
#define GET_INPUT			0x23
#define SELECT_ITEM		0x24
#define SET_UP_MENU		0x25
#define PROVIDE_LOCAL_INFORMATION	0x26
#define TIMER_MANAGEMENT	0x27
#define SET_UP_IDLE_TEXT	0x28
//- "27" to "FF" are reserved values

#define ENV_TAG_SMS_PP		0xD1
#define ENV_TAG_SMS_CB		0xD2
#define ENV_TAG_MENU		0xD3
#define ENV_TAG_CALL		0xD4
#define ENV_TAG_MOSM		0xD5
#define ENV_TAG_EVENT		0xD6
#define ENV_TAG_TIMER		0xD7

#define FETCH_TAG_PROSIM 	0xD0

/* STK DEVICE SOURCE/DESTINATION SET */
#define STK_DEV_KEYPAD				0x01
#define STK_DEV_DISPLAY				0x02
#define STK_DEV_EARPIECE			0x03
#define STK_DEV_SIM					0x81
#define STK_DEV_ME					0x82
#define STK_DEV_NETWORK				0x83

#define STK_RES_SUCCESS				0x00	/* success */
#define STK_RES_PARTIAL				0x01
#define STK_RES_MISSING				0x02
#define STK_RES_REFRESH				0x03
#define STK_RES_NO_ICON				0x04
#define STK_RES_MOD_CALL			0x05
#define STK_RES_LIMIT_SERVICE		0x06
#define STK_RES_TERMINATED			0x10  
#define STK_RES_BACKWARD			0x11
#define STK_RES_NO_USER_RESPONSE	0x12
#define STK_RES_HELP_REQUIRED		0x13
#define STK_RES_TRANSACTION_ABORT	0x14

#define STK_RES_ME_FAIL				0x20	/* warning */
#define STK_RES_NETWORK_FAIL 		0x21
#define STK_RES_USER_TIMEOUT		0x22
#define STK_RES_USER_ABORT			0x23

#define STK_RES_ME_ERROR			0x30	/* error */
#define STK_RES_ME_TYPE_ERROR		0x31
#define STK_RES_ME_DATA_ERROR		0x32
#define STK_RES_ME_NUM_ERROR		0x33
#define STK_RES_SS_ERROR			0x34
#define STK_RES_SMS_ERROR			0x35
#define STK_RES_ERROR				0x36

#define RESULT_POR_OK				0
#define RESULT_RCCCDS_FAILED		1
#define RESULT_CNTR_LOW				2
#define RESULT_CNTR_HIGH			3
#define RESULT_CNTR_BLOCKED			4
#define RESULT_CNTR_CIPHER_ERROR	5
#define RESULT_MEMORY_INSUFFICIENT	7

#define EVENT_MT_CALL				0
#define EVENT_CALL_CONNECTED		1
#define EVENT_CALL_DISCONNECTED		2
#define EVENT_LOCATION_STATUS		3
#define EVENT_USER_ACTIVITY			4
#define EVENT_IDLE_SCREEN_AVAIL		5
#define EVENT_CARD_READER_STAT		6
#define EVENT_LANGUAGE_SELECT		7
#define EVENT_BROWSER_TERMINATION	8
#define EVENT_DATA_AVAILABLE		9
#define EVENT_CHANNEL_STATUS		0x0A

#define TERMINAL_PROFILE_DOWNLOAD 		0x00
#define TERMINAL_SMSPP_DOWNLOAD			0x01
#define TERMINAL_SMSCB_DOWNLOAD			0x02
#define TERMINAL_MENU_SELECT			0x03
#define TERMINAL_9EXX_RESPONSE			0x04
#define TERMINAL_TIMER_EXPIRE			0x05
#define TERMINAL_USSD_CALL_CONTROL		0x06
#define TERMINAL_AUTO_REDIAL			0x07

#define TERMINAL_COMMAND_RESULT			0x10
#define TERMINAL_CALL_CONTROL			0x11
#define TERMINAL_CELL_IDENTITY			0x12
#define TERMINAL_MO_SMC					0x13
#define TERMINAL_ALPHA_HANDLING			0x14
#define TERMINAL_UCS2_ENTRY				0x15
#define TERMINAL_UCS2_DISPLAY			0x16
#define TERMINAL_DISPLAY_EXT			0x17

#define TERMINAL_EVT_MT_CALL			0x41
#define TERMINAL_EVT_CALL_CONNECT		0x42
#define TERMINAL_EVT_CALL_DISCONNECT	0x43
#define TERMINAL_EVT_LOCATION_STAT		0x44
#define TERMINAL_EVT_USR_ACTIVITY		0x45
#define TERMINAL_EVT_IDLE_SCREEN		0x46
#define TERMINAL_EVT_CARD_READER		0x47

#define TERMINAL_EVT_LANG_SELECT		0x50
#define TERMINAL_EVT_BROWSER_EXIT		0x51
#define TERMINAL_EVT_DAT_AVAILABLE		0x52
#define TERMINAL_EVT_CHANNEL_STAT		0x53

//response packet status error
#define RESPONSE_PKT_POR_OK				0
#define RESPONSE_PKT_AUTH_FAIL			1
#define RESPONSE_PKT_CNTR_LOW			2
#define RESPONSE_PKT_CNTR_HIGH			3
#define RESPONSE_PKT_CNTR_BLOCKED		4
#define RESPONSE_PKT_CIPHER_ERROR		5
#define RESPONSE_PKT_UNKNOWN_ERROR		6
#define RESPONSE_PKT_MEMORY_ERROR		7
#define RESPONSE_PKT_MORE_TIME			8
#define RESPONSE_PKT_TAR_UNKNOWN		9


#define CALLBACK_SUCCESS		0 
#define CALLBACK_STAY_NODE		(1<<5)
#define CALLBACK_TAMPER_RESULT	(1<<6)
#define CALLBACK_FORCE_END		(1<<7)

/* Startup state machine */	
#define SAT_MENU_INIT			1
#define SAT_EVENT_INIT			2
#define SAT_STARTUP_INIT		3
#define SAT_DEFAULT				4

//parsing stk menu (stk config header)
struct stk_config {		
	uint16 parent;
	uint16 sibling;
	uint16 child; 
	uint16 length;
	uchar state;
	uchar tag;
};	

struct sms_tpdu {
	uchar type;
	uint16 destination;
	uchar pid;
	uchar dcs;
	uchar year;
	uchar month;
	uchar day;
	uchar hour;
	uchar minute;
	uchar second;
	uchar timezone;
	uchar bytes[256];
};

struct sms_tpdu_header {
	uchar type;
	uint16 destination;
	uchar pid;
	uchar dcs;
	uchar year;
	uchar month;
	uchar day;
	uchar hour;
	uchar minute;
	uchar second;
	uchar timezone;
};

struct sms_ud {
	uchar length;
  	uchar iei;
	uchar bytes[256];
};

struct sms_ud_header {
  	uchar length;
	uchar iei;
};

struct info_element_data {
 	uchar ref_num;
	uchar seq_num;
	uchar total_num;
};

//#if 0
struct concat_property {
	uchar counter;
	uchar refnum;
	//uchar rsv;
	uint16 offset;
	uchar rsv;
	uchar tag;
};
//#endif

struct command_packet_header {
	uint16 cpl;		//2 byte length		 		--> total packet length
	uchar chl;		//1 byte header length		--> CHL
	uchar spi[2];	//2 byte SPI
	uchar kic;		//1 byte KIc
	uchar kid;		//1 byte KID
	uchar tar[3];	//3 byte TAR 
	uchar cntr[5]; 	//5
	uchar pcntr;	//1
};

struct command_packet {
	uint16 cpl;		//2 byte length		 		--> total packet length
	uchar chl;		//1 byte header length		--> CHL
	uchar spi[2];	//2 byte SPI
	uchar kic;		//1 byte KIc
	uchar kid;		//1 byte KID
	uchar tar[3];	//3 byte TAR
	uchar cntr[5]; 	//5
	uchar pcntr;	//1
	uchar ud[256];		//byte including CNTR,PCNTR,RC/CC/DS,SECURED_DATA
};

struct response_packet {
	uint16 rpl;		//2 byte length		 		--> total packet length
	uchar rhl;		//1 byte header length		--> CHL
	uchar spi[2];	//2 byte SPI
	uchar kic;		//1 byte KIc
	uchar kid;		//1 byte KID
	uchar tar[3];	//3 byte TAR 
	uchar cntr[5]; 	//5
	uchar pcntr;	//1
	uchar status;	//1
	uchar ud[256];		//byte including CNTR,PCNTR,RC/CC/DS,SECURED_DATA
};

struct header_0348_packet {
  	uint16 ppl;
	uchar phl;		//1 byte header length
	uchar spi[2];	//2 byte SPI
	uchar kic;		//1 byte KIc
	uchar kid;		//1 byte KID
	uchar tar[3];	//3 byte TAR 
	uchar cntr[5]; 	//5
	uchar pcntr;	//1
};

struct response_tpdu {
	uchar udl;
	uchar udhl;
	uchar iei;
	uchar ieidl;
};

struct response_packet_header {
	uint16 rpl;		//2 byte length		 		--> total packet length
	uchar rhl;		//1 byte header length		--> CHL
	uchar spi[2];	//2 byte SPI
	uchar kic;		//1 byte KIc
	uchar kid;		//1 byte KID
	uchar tar[3];	//3 byte TAR 
	uchar cntr[5]; 	//5
	uchar pcntr;	//1
	uchar status;	//1
};

#define SMSUD_REF_NUM		0
#define SMSUD_TOTAL_NUM		1
#define SMSUD_SEQ_NUM		2

typedef struct stk_config stk_config;
typedef struct sms_tpdu sms_tpdu;
typedef struct sms_tpdu_header sms_tpdu_header;
typedef struct sms_ud sms_ud;
typedef struct sms_ud_header sms_ud_header;
typedef struct info_element_data info_element_data;
typedef struct concat_property concat_property;
typedef struct command_packet_header command_packet_header;
typedef struct header_0348_packet header_0348_packet;
typedef struct command_packet command_packet;
typedef struct response_packet response_packet;
typedef struct response_packet_header response_packet_header;
typedef struct response_tpdu response_tpdu;	 

void gsm_response(uchar * buffer, uchar length) _REENTRANT_ ;
  
extern uchar STK_buffer[];

//uint16 address_get_value(uchar * buffer) _REENTRANT_ ;

void SAT_profile_download(uchar * buffer, uchar len) _REENTRANT_ ; 
uchar liquid_profile_check(uchar cmd) _REENTRANT_ ;
uchar liquid_proactive_check(uchar cmd) _REENTRANT_ ;
uint16 liquid_error(uchar status) _REENTRANT_ ;
uint16 liquid_profile(void) _REENTRANT_ ;
uint16 liquid_status(void) _REENTRANT_ ;  
uint16 liquid_fetch(uchar * buffer, uint16 size) _REENTRANT_ ;
uchar allocate_liquid_space(void * fs, uint16 fid, uint16 size) _REENTRANT_ ;
void liquid_set_response_data(uchar status, uchar * buffer, uint16 length) _REENTRANT_ ; 

/* SAT APIs */
uint16 SAT_response(uchar * buffer, uchar len) _REENTRANT_ ;  
uint16 SAT_status(void) _REENTRANT_ ;
uint16 SAT_envelope(uchar * buffer, uchar length) _REENTRANT_ ;	  
uchar SAT_push(uchar * buffer, uchar tag, uchar length, uchar * value) _REENTRANT_ ;
uchar SAT_pop(uchar * buffer, uchar * tag, uchar * size, uchar * value)  _REENTRANT_ ;
uchar SAT_init(uchar * buffer, uint16 size) _REENTRANT_ ;
uchar SAT_read(uchar * buffer, uint16 size) _REENTRANT_ ;
uint16 SAT_file_flush(uchar length) _REENTRANT_ ;
uchar SAT_file_pop(uint16 i, uchar * tag, uchar * size, uchar * value) _REENTRANT_ ;
uchar SAT_file_push(uint16 i, uchar tag, uchar length, uchar * value) _REENTRANT_ ;
uchar SAT_command(uchar * buffer, uchar number, uchar type, uchar qualifier) _REENTRANT_ ;
uchar SAT_device(uchar * buffer, uchar src, uchar dst) _REENTRANT_ ;
uint16 SAT_file_write(uchar * fmt, ...) _REENTRANT_ ;
uint16 SAT_printf(uchar * fmt, ...) _REENTRANT_ ;
void SAT_cleanup(void) _REENTRANT_ ;

#define __LIQUID_H
#endif