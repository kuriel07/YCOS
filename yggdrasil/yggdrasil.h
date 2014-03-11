#ifndef _YGGDRASIL_H
#include "defs.h"
#include "config.h"

//struct untuk apdu, harus menggunakan pointer
/* jadi bingung harus diakses model struct atau buffer ????
 * kalau ditinjau dari masalah performance hampir sama, tetapi pemakaian struct tidak menghindari kemungkinan akses pointer */
struct apdu_command {
	uchar CLA;
	uchar INS;
	uchar P1;
	uchar P2;
	uchar P3;
	char bytes[MAX_DATA_LENGTH];
	uchar LE;
};

struct gsm_data {
	uchar file_char;
	uchar number_of_df;
	uchar number_of_ef;
	uchar number_of_chv;
	uchar rfu_1;
	uchar chv1;
	uchar unblock_chv1;
	uchar chv2;
	uchar unblock_chv2;
	uchar rfu_2;
	uchar padding[10];
};

typedef struct gsm_data gsm_data;

struct response_df {
	uint16 rfu_1;
	uint16 total_memory;
	uint16 fid;
	uchar type;
	uchar rfu_2;	//8
	uchar rfu_3;	//9
	uchar rfu_4;	//10
	uchar rfu_5;	//11
	uchar rfu_6;	//12
	uchar next_length;
	struct gsm_data gsm;
};

struct response_ef {
	uint16 rfu_1;
	uint16 file_size;
	uint16 fid;
	uchar type;
	uchar increase_allowed;
	uchar acc_rw;
	uchar acc_inc;
	uchar acc_ri;
	uchar status;
	uchar next_length;
	uchar structure;
	uchar rec_length;
};

struct os_status {
	uchar os_tag;			//must be Y (Yggdrasil)
	uchar os_ver;			//kernel version 
	uchar os_state;			//file system status	(locked/unlocked, stk enable/disable, stk load, sleep)
	uchar fs_ver;			//file system version
	uchar fs_mod;			//16 bit/32 bit
	uchar fs_ssize;			//sector size
	uint16 heap_size;  		//allocated heap					(memory)
	uint16 total_heap;		//total heap size, free+allocated	(memory)
	uint16 used_space;		//allocated space					(flash)
	uint32 total_space;		//total partition					(flash)
};

struct os_config {
	uchar os_tag;
	uchar os_state;			//saved on file system table
	uchar rsv[2];
	uchar gsm_ki[16];
};

typedef struct response_df response_df;
typedef struct response_ef response_ef;
typedef struct apdu_command apdu_command;
typedef struct os_status os_status;
typedef struct os_config os_config;

#define YGGDRASIL_VERSION			3
//GSM 11.11
#define INS_SELECT					0xA4
#define INS_STATUS					0xF2
#define INS_READ_BINARY				0xB0
#define INS_READ_RECORD				0xB2
#define INS_UPDATE_BINARY			0xD6
#define INS_UPDATE_RECORD			0xDc
#define INS_GET_RESPONSE			0xC0
#define INS_VERIFY_CHV				0x20
#define INS_CHANGE_CHV				0x24
#define INS_DISABLE_CHV				0x26
#define INS_ENABLE_CHV				0x28
#define INS_UNBLOCK_CHV				0x2c
#define INS_REHABILITATE			0x44
#define INS_INVALIDATE				0x04
#define INS_SEEK					0xA2
#define INS_INCREASE				0x32
#define INS_RUN_GSM_ALGORITHM		0x88
#define INS_SLEEP					0xFA 
//GSM 11.14
#define INS_TERMINAL_PROFILE		0x10
#define INS_FETCH					0x12
#define INS_ENVELOPE				0xC2
#define INS_TERMINAL_RESPONSE		0x14  

#define CLA_GSM11					0xA0
#define CLA_YGGDRASIL				0xA6 

#define INS_GET_TOKEN 				0xE2
#define INS_SOFT_AUTH				0xE0
#define INS_READ_SERNUM				0xEA
#define INS_WRITE_SERNUM			0xE9

//admin command
#define INS_FILE_DELETE				0xFD	//00	00	02	[FID2]				Remove with the specified FID
//#define INS_CREATE_BIN				0xCB	//00	00	07	[FID2][RW][INC][IR][SZ2]		Create file binary with size SZ
//#define INS_CREATE_REC				0xC2	//00	00	08	[FID2][RW][INC][IR][TRC2][RSZ]	Create file record with total record TRC and record size RSZ
//#define INS_CREATE_CYCLIC			0xCC	//00	00	08	[FID2][RW][INC][IR][TRC2][RSZ]	Create file cyclic with total record TRC and record size RSZ
#define INS_FILE_CREATE				0xFC	//00	00	02	[FID2]				Create directory with FID

//system (file system) command
#define INS_WRITE_FLASH				0xF6
#define INS_READ_FLASH				0xF4
#define	INS_ERASE_FLASH				0xF3
#define INS_FS_FORMAT				0xF0
//#define INS_FS_DEFRAG				0xFD	//00	00	00					Defrag file system (reserved command)
#define INS_FS_LOCK					0xF7	//00	00	00					Lock file system (reserved command)
#define INS_FS_UNLOCK				0xF8	//00	00	00					Unlock file system (reserved command)
#define INS_FS_STATUS				0xFE	//00	00	10					File system status
//system (OS) command
#define INS_DISABLE_APP				0x8D	//00	00	00					Disable stk
#define INS_ENABLE_APP				0x8E	//00	00	00					Enable stk
#define	INS_WRITE_APP				0x8A	//[OFF]	[OFF]	[SZ]	[BINARY]				Write binary file for STK at offset OFF with size SZ
#define	INS_LOAD_APP				0x8C	//00	00	00					Load app to user program space from temporary space
#define	INS_LOAD_BOOTLOADER			0x8F	//00	00	00					Back to bootloader
#define INS_OS_STATUS				0x80	//00	00	sizeof(os_config)
#define INS_SET_KI					0x81	//00 	00	10					//16 byte data
#define INS_BIFROST					0xBF
#define INS_AUTH_FSH				0xFA
#define INS_GEN_FSH					0xFF



#define P_NEXT_REC					0x02
#define P_PREV_REC					0x03
#define P_ABS_REC					0x04

//global access variabel
//extern uchar v_chv_status[0x10];
extern uchar response_length;
//extern uchar response[0x20];

//extern uchar code gsm_class_case[];
//extern uchar code yggdrasil_class_case[];
//#define iobuf iso7816_buffer
//extern apdu_command * iobuf;
extern os_config _os_config; 

struct response_buffer {
 	uchar length;
	uchar buffer[256];
};
typedef struct response_buffer response_buffer;

#define YGG_DORMANT  		0
#define YGG_READY			1
#define YGG_RECEIVE_CMD		3
#define YGG_SEND_ACK		2
#define YGG_RECEIVE_DATA	7
#define YGG_RUNNING			4
#define YGG_STOP			13

#define YGG_ALLOCATION_CONFIG_OFFSET 	32
#define YGG_ST_SLEEP			(1<<7)
#define YGG_ST_LOCK				(1<<6)
#define YGG_ST_ENABLE_APP		(1<<2)
#define YGG_ST_LOAD_APP			(1<<3)
#define YGG_ST_NO_INIT			(1<<4)
#define YGG_ST_ACTIVATED		(1<<1)

void Initialize_Hardware() _REENTRANT_ ;
void Initialize_Operating_System() _REENTRANT_ ;
uint16 Yggdrasil_Decode(apdu_command * command);
void Yggdrasil_Main();
uchar Command_Test_Case();
void Set_Response(uchar * bytes, uchar length) _REENTRANT_;
void Get_Data() _REENTRANT_;
void Load_State() _REENTRANT_ ;
void Save_State() _REENTRANT_ ;
uint16 Bifrost_Decode(uchar * buffer, uchar length) _REENTRANT_ ;

/* user app APIs */
uint16 Write_Temp_Space(uint16 pos, uchar * bytes, uint16 length) _REENTRANT_;		   
uint16 Load_Temp_Space(void) _REENTRANT_;
uint16 Auth_COMP128(uchar * inbuf, uchar * outbuf) _REENTRANT_ ;
uint16 Load_User_App(void) _REENTRANT_ ;			//copy from memory onto user app program space
uint16 Execute_User_App(apdu_command * command) _REENTRANT_ ;
void Initialize_User_App() _REENTRANT_ ;


#define _YGGDRASIL_H
#endif
