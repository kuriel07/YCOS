
#include	"_17BD_UserCode.h"
#include 	"defs.h"

#define CHGBASE		0x50	//	Change bank
#define	WRFLASH		0x58	//	Write flash
#define	ERFLASH		0x2E	//	Erase flash
#define	CRCCALL		0x30	//	Read CRC data
#define	RDFLASH		0x5A	//	Read flash
#define	UDFLASH		0x5C	//	Updata flash
#define	DESOPER		0x56	//	DES operation: Encryption/Decrytion/Read result
#define	GETRDMN		0x84	//	Get random number 

//	Define Status Word
//#define	SUCCESS	0		//	Success,9000
#define	IVDINS	2		//	Invalid INS,6D00
#define	P3ERROR	4		//	P3 error,6C00
#define	WRERROR	6		//	Flash write error,6501
#define	P1P2ERR	8		//	P1 and/or P2 error,6A00
#define	RDERROR	10		//	Verify error,6504

#define	FlashStart	0x00000
#define	FlashLimit	0x10000
#define	RAMBase		0x0000
#define	RAMLimit	0x0800
#define	PageSize	0x200
#define	BlockSize	0x800

//extern BYTEX  	gCommand[];

//#define	CLA gCommand[0]
//#define	INS gCommand[1]
//#define	P1  gCommand[2]
//#define	P2  gCommand[3]
//#define	P3  gCommand[4]

#define ISO7816_DORMANT  		0
#define ISO7816_READY			1
#define ISO7816_WAIT_PPS		15
#define ISO7816_RECEIVE_CMD		3
#define ISO7816_SEND_ACK		2
#define ISO7816_RECEIVE_DATA	7
#define ISO7816_WAIT_LE			9
#define ISO7816_RUNNING			4
#define ISO7816_STOP			13 

extern uchar _iso7816_ins;
extern uchar _iso7816_cla;
extern uchar _iso7816_case;
extern BYTEX	_iso7816_state;
extern uint16 _tick_counter;
//extern BYTEX	IOBuf[512];
extern BYTE		SWptr;
extern HALFWX	Foffset;
extern BYTEX	PPSFlag;
extern BYTEC	ATR[];
extern BYTEX 	iso7816_buffer[263];
//extern BYTEX	IOBuf[];
//extern BYTEX	IOBuf[512];
extern BYTEX	FlashBuffer[512];
//extern BYTEX	IOBuf[512];	
#define IOBuf 	(FlashBuffer)
#define gCommand iso7816_buffer 
//#define	CLA 	iso7816_buffer[0]
//#define	INS 	iso7816_buffer[1]
//#define	P1  	iso7816_buffer[2]
//#define	P2  	iso7816_buffer[3]
//#define	P3  	iso7816_buffer[4]

void 	ISO7816_init(void);
//uchar 	ISO7816_case(void);
void 	ISO7816_main(void);
void	IoInit(BYTE FIDI) _REENTRANT_;
void 	PPS_ACK(void);
void	PPS(void);
void	Set_PPS(BYTEX * buffer)	_REENTRANT_ ;
void	Send_ATR(void);
BYTE	RcvAPDU(void);
void	CMMD_Handle(void);
BYTE	receive_byte(void);
void 	Rx_n_Bytes(short n) _REENTRANT_ ;
void	send_byte(char c) _REENTRANT_ ;
void 	Tx_n_Bytes(USHORT n,BYTE * databuf) _REENTRANT_ ;
//void	Response(void);
void	TxStatus(unsigned char  ptr) _REENTRANT_ ;
void	Sleep_Mode(void);
void	SetBase(BYTE BankNum) _REENTRANT_ ;
uint16	CalCRC(BYTE * SrcAddr, HALFW length) _REENTRANT_;
BYTE	Erase_Pages(HALFW FlashAddr,HALFW PageNum) _REENTRANT_;
void	ReadFlash(BYTEX *ramAddr, HALFW FlashAddr, HALFW  length) ;
BYTE	UpdateFlash(HALFW foffset,BYTEX * RAMbuf,BYTE length) ;
void 	DES_Decrypt(BYTE * key, BYTE * inbuf, BYTE * outbuf) _REENTRANT_;
void 	DES_Encrypt(BYTE * key, BYTE * inbuf, BYTE * outbuf) _REENTRANT_;
void 	TDES_Decrypt(BYTE * key1, BYTE * key2, BYTE * inbuf, BYTE * outbuf) _REENTRANT_;
void 	TDES_Encrypt(BYTE * key1, BYTE * key2, BYTE * inbuf, BYTE * outbuf) _REENTRANT_;
BYTE 	DES_Operation(BYTE mode, BYTE oper, BYTE * inbuf, BYTE * key1, BYTE * key2, BYTE * outbuf);
//BYTE	DES_Operation(BYTE mode,BYTE oper,BYTE * desdata,USHORT len);
void	Random_Number_Generator(BYTE *RNGBuf, BYTE length) _REENTRANT_;
void	ReturnToBL(void);
void 	Tx_Status(USHORT sw) _REENTRANT_ ;
USHORT	Command_Interpreter(BYTEX * buffer) _REENTRANT_ ;
USHORT 	GSM_Handler(BYTEX * buffer) ;
USHORT 	PPS_Handler(BYTEX * buffer) ;
void StartTimeoutSequence(void);
void EndTimeoutSequence(void);	

#define	TX_NULL_BYTE_ON(etu)	ISO_AutoTxNULL(1,etu-1);
#define	TX_NULL_BYTE_OFF(etu)	ISO_AutoTxNULL(0,etu-1);
void	ISO_AutoTxNULL(BYTE mode,HALFW ETUcount) _REENTRANT_ ;
void 	ISO_DelayETU(void);
