#ifndef _SECURITY_H
#include "..\defs.h"
#include "..\yggdrasil\yggdrasil.h"
//#include "file.h"
//#include "fs.h"

//#define ASGARD_VERSION	4

//#if ASGARD_VERSION == 4
#define MAX_CHV						6
#define CHV_TAG						0xA6

struct chv_file {
	uchar tag;					//1byte		chv header tag
	uchar status;				//1byte		b7 - secret code(init/uninit), b0-b3 chv status
	uchar pin[8];				//8byte
	uchar puk[8];				//8byte
	uchar pin_attempts;			//1byte
	uchar pin_max_attempts;		//1byte
	uchar puk_attempts;			//1byte
	uchar puk_max_attempts;		//1byte
};

typedef struct chv_file	chv_file;

//#define CHV_DATA_OFFSET			(ALLOCATION_DATA_OFFSET - (MAX_CHV * sizeof(chv_file)))		
#define CHV_DATA_OFFSET				320

extern uchar _chv_status[MAX_CHV];		//maximum 6 chv		1~6

void chv_init(void); 		//load all chv status onto memory		new function
void chv_create(uchar chv_no, uchar *pin, uchar *puk, uchar pin_max_retry, uchar puk_max_retry);
uchar chv_verify(uchar chv_no, uchar *pin);
uchar chv_enable(uchar chv_no, uchar *pin);
uchar chv_disable(uchar chv_no, uchar *pin);
//uchar getCHVstatus(uchar chv_no);
uchar chv_unblock(uchar chv_no, uchar *pin, uchar *puk);
uchar chv_change(uchar chv_no, uchar *old, uchar *new);
uchar chv_get_status(uchar chv_no);

//ported functions
#define initCHV			chv_init
#define createCHV		chv_create
#define verifyCHV		chv_verify
#define enableCHV		chv_enable
#define disableCHV		chv_disable
#define unblockCHV		chv_unblock
#define changeCHV		chv_change
#define getCHVstatus  	chv_get_status

//#endif


#define _SECURITY_H
#endif
