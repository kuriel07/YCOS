
#include	"_17BD_UserCode.h"
#include	"..\ISO7816\ISO7816.h"
#include	"..\NORFlash\NORFlash.h"
#include 	"defs.h"

//====================================================
//	Erase the target page
BYTE	Erase_Page(BYTEX * pDest)
{
	BYTE	counter = 0;	

	FL_CON = 0x11; 							//	Erasing and verify enabled
	//FL_CON = 0x10; 							//	Erasing and verify enabled

retryErase:
	FL_SDP1 = 0x55;
	FL_SDP2 = 0xAA;

	*pDest = 0xFF;							//	Write a FFH to start address in the target page

	while(!FL_STS_F_OVER);					//	FL_CTL.FL_OVER	
	
	FL_STS_F_OVER = 0;						//	Clear FL_CTL.FL_OVER
	if(FL_STS_F_OP_ERR)						//	Mistaken operation
	{							   	
		FL_STS_F_OP_ERR = 0;
		FL_SDP1 = 0xFF;
		FL_SDP2 = 0xFF;
		return	WRERROR;
	}
	else if(FL_STS_F_CHKFF_ERR)
	{
		FL_STS_F_CHKFF_ERR = 0;

		if(counter == 2) {
			FL_SDP1 = 0xFF;
			FL_SDP2 = 0xFF;
			return	RDERROR;
		}

		counter ++;
		goto	retryErase;
	}
	//FL_STS = 0; 
	FL_SDP1 = 0xFF;
	FL_SDP2 = 0xFF;
	return	SUCCESS;

	/*HALFW	i;
	
 	for(i = 0;i < 0x200;i ++)
	{
		FL_CON = 0x01;
		//FL_CON = 0x01;
		FL_SDP1 = 0xAA;
		FL_SDP2 = 0x55;	

		*(pDest + i) = 0xFF;

		while(!FL_STS_F_OVER);					//	FL_CTL.FL_OVER	
	
		if(FL_STS_F_OP_ERR)						//	Mistaken operation
		{							   	
			FL_STS_F_OP_ERR = 0;
			return	WRERROR;
		}
		FL_STS_F_OVER = 0;						//	Clear FL_CTL.FL_OVER
	}
	return SUCCESS;*/
}

//====================================================
//	Write n bytes of flash memory.
BYTE	Write_Bytes(BYTEX * pDest, BYTEX * pSrc, HALFW len)
{
	HALFW	i;
	
 	for(i = 0;i < len;i ++)
	{
		FL_CON = 0x01;
		//FL_CON = 0x01;
		FL_SDP1 = 0xAA;
		FL_SDP2 = 0x55;	

		*(pDest + i) = pSrc[i];

		while(!FL_STS_F_OVER);					//	FL_CTL.FL_OVER	
	
		if(FL_STS_F_OP_ERR)						//	Mistaken operation
		{							   	
			FL_STS_F_OP_ERR = 0;
			FL_SDP1 = 0xFF;
			FL_SDP2 = 0xFF;
			return	WRERROR;
		}
		FL_STS_F_OVER = 0;						//	Clear FL_CTL.FL_OVER
	} 
	FL_SDP1 = 0xFF;
	FL_SDP2 = 0xFF;
	return SUCCESS;
}