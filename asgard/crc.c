/**********************************************************************
 *
 * Filename:    crc.c
 * 
 * Description: Slow and fast implementations of the CRC standards.
 *
 * Notes:       The parameters for each supported CRC standard are
 *				defined in the header file crc.h.  The implementations
 *				here should stand up to further additions to that list.
 *
 * 
 * Copyright (c) 2000 by Michael Barr.  This software is placed into
 * the public domain and may be used for any purpose.  However, this
 * notice must not be changed or removed and no warranty is either
 * expressed or implied by its publication or distribution.
 **********************************************************************/
 
#include "crc.h"
#include "..\defs.h"	 
#include "..\ISO7816\ISO7816.h"

/*
 * Derive parameters from the standard-specific parameters in crc.h.
 */
#define WIDTH    (8 * sizeof(crc))
#define TOPbt   (1 << (WIDTH - 1))

#if (REFLECT_DATA == TRUE)
#undef  REFLECT_DATA
#define REFLECT_DATA(X)			((unsigned char) reflect((X), 8))
#else
#undef  REFLECT_DATA
#define REFLECT_DATA(X)			(X)
#endif

#if (REFLECT_REMAINDER == TRUE)
#undef  REFLECT_REMAINDER
#define REFLECT_REMAINDER(X)	((crc) reflect((X), WIDTH))
#else
#undef  REFLECT_REMAINDER
#define REFLECT_REMAINDER(X)	(X)
#endif


/*********************************************************************
 *
 * Function:    reflect()
 * 
 * Description: Reorder the bts of a binary sequence, by reflecting
 *				them about the middle position.
 *
 * Notes:		No checking is done that nbts <= 32.
 *
 * Returns:		The reflection of the original data.
 *
 *********************************************************************/
static unsigned long
reflect(unsigned long b, unsigned char nbts)
{
	unsigned long  reflection = 0x00000000;
	unsigned char  bt;

	/*
	 * Reflect the data about the center bt.
	 */
	for (bt = 0; bt < nbts; ++bt)
	{
		/*
		 * If the LSB bt is set, set the reflection of it.
		 */
		if (b & 0x01)
		{
			reflection |= (1 << ((nbts - 1) - bt));
		}

		b = (b >> 1);
	}

	return (reflection);

}	/* reflect() */


/*********************************************************************
 *
 * Function:    crcSlow()
 * 
 * Description: Compute the CRC of a given message.
 *
 * Notes:		
 *
 * Returns:		The CRC of the message.
 *
 *********************************************************************/
crc
crcSlow(unsigned char const message[], int nbyts)
{
    /*crc            remainder = INITIAL_REMAINDER;
	int            byt;
	unsigned char  bt;


    //Perform modulo-2 division, a byt at a time.
    for (byt = 0; byt < nbyts; ++byt)
    {
        //Bring the next byt into the remainder.
        remainder ^= (REFLECT_DATA(message[byt]) << (WIDTH - 8));

        //Perform modulo-2 division, a bt at a time.
        for (bt = 8; bt > 0; --bt)
        {
            //Try to divide the current data bt.
            if (remainder & TOPbt)
            {
                remainder = (remainder << 1) ^ POLYNOMIAL;
            }
            else
            {
                remainder = (remainder << 1);
            }
        }
    }

    //The final remainder is the CRC result.
    return (REFLECT_REMAINDER(remainder) ^ FINAL_XOR_VALUE);*/
	return 0;

}   /* crcSlow() */


crc  code crcTable[256];


/*********************************************************************
 *
 * Function:    crcInit()
 * 
 * Description: Populate the partial CRC lookup table.
 *
 * Notes:		This function must be rerun any time the CRC standard
 *				is changed.  If desired, it can be run "offline" and
 *				the table results stored in an embedded system's ROM.
 *
 * Returns:		None defined.
 *
 *********************************************************************/
void
crcInit(void)
{
    /*crc			   remainder;
	int			   dividend;
	unsigned char  bt;


    //Compute the remainder of each possible dividend.
    for (dividend = 0; dividend < 256; ++dividend)
    {
        //Start with the dividend followed by zeros.
        remainder = dividend << (WIDTH - 8);

        //Perform modulo-2 division, a bt at a time.
        for (bt = 8; bt > 0; --bt)
        {
            ///Try to divide the current data bt.		
            if (remainder & TOPbt)
            {
                remainder = (remainder << 1) ^ POLYNOMIAL;
            }
            else
            {
                remainder = (remainder << 1);
            }
        }

        //Store the result into the table.
        crcTable[dividend] = remainder;
    }*/

}   /* crcInit() */


/*********************************************************************
 *
 * Function:    crcFast()
 * 
 * Description: Compute the CRC of a given message.
 *
 * Notes:		crcInit() must be called first.
 *
 * Returns:		The CRC of the message.
 *
 *********************************************************************/
crc
crcFast(unsigned char const message[], int nbyts)
{
    /*crc	           remainder = INITIAL_REMAINDER;
    unsigned char  b;
	int            byt;


    //Divide the message by the polynomial, a byt at a time.
    for (byt = 0; byt < nbyts; ++byt)
    {
        b = REFLECT_DATA(message[byt]) ^ (remainder >> (WIDTH - 8));
  		remainder = crcTable[b] ^ (remainder << 8);
    }

    //The final remainder is the CRC.
    return (REFLECT_REMAINDER(remainder) ^ FINAL_XOR_VALUE); */
	//crc value;
	//CalCRC((BYTE *)&value, message, nbyts);
	//return value;
	return 0;
}   /* crcFast() */
