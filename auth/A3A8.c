#include "A3A8.h"
#include "..\defs.h"	 
#include "..\config.h"
#include "..\liquid.h" 
#include "..\midgard\midgard.h"
#include "..\asgard\file.h"	  
#include <string.h>
#include <stdlib.h>

extern uchar STK_buffer[];
#if AUTH_GSM_MODE==AUTH_USE_COMP128_1 
/* An implementation of the GSM A3A8 algorithm.  (Specifically, COMP128.) 
 *
 * Copyright 1998, Marc Briceno, Ian Goldberg, and David Wagner.
 * All rights reserved.
 *
 * For expository purposes only.  Coded in C merely because C is a much
 * more precise, concise form of expression for these purposes.  See Judge
 * Patel if you have any problems with this...
 * Of course, it's only authentication, so it should be exportable for the
 * usual boring reasons.
 *
 *
 * This software is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.
 * Copyright remains the authors' and as such any Copyright notices in
 * the code are not to be removed.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * The license and distribution terms for any publicly available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution license
 * [including the GNU Public License.]
 */

/* The compression tables. */
//kenapa ini harus static????
static uint8_t table_0[512] = {
        102,177,186,162,  2,156,112, 75, 55, 25,  8, 12,251,193,246,188,
        109,213,151, 53, 42, 79,191,115,233,242,164,223,209,148,108,161,
        252, 37,244, 47, 64,211,  6,237,185,160,139,113, 76,138, 59, 70,
         67, 26, 13,157, 63,179,221, 30,214, 36,166, 69,152,124,207,116,
        247,194, 41, 84, 71,  1, 49, 14, 95, 35,169, 21, 96, 78,215,225,
        182,243, 28, 92,201,118,  4, 74,248,128, 17, 11,146,132,245, 48,
        149, 90,120, 39, 87,230,106,232,175, 19,126,190,202,141,137,176,
        250, 27,101, 40,219,227, 58, 20, 51,178, 98,216,140, 22, 32,121,
         61,103,203, 72, 29,110, 85,212,180,204,150,183, 15, 66,172,196,
         56,197,158,  0,100, 45,153,  7,144,222,163,167, 60,135,210,231,
        174,165, 38,249,224, 34,220,229,217,208,241, 68,206,189,125,255,
        239, 54,168, 89,123,122, 73,145,117,234,143, 99,129,200,192, 82,
        104,170,136,235, 93, 81,205,173,236, 94,105, 52, 46,228,198,  5,
         57,254, 97,155,142,133,199,171,187, 50, 65,181,127,107,147,226,
        184,218,131, 33, 77, 86, 31, 44, 88, 62,238, 18, 24, 43,154, 23,
         80,159,134,111,  9,114,  3, 91, 16,130, 83, 10,195,240,253,119,
        177,102,162,186,156,  2, 75,112, 25, 55, 12,  8,193,251,188,246,
        213,109, 53,151, 79, 42,115,191,242,233,223,164,148,209,161,108,
         37,252, 47,244,211, 64,237,  6,160,185,113,139,138, 76, 70, 59,
         26, 67,157, 13,179, 63, 30,221, 36,214, 69,166,124,152,116,207,
        194,247, 84, 41,  1, 71, 14, 49, 35, 95, 21,169, 78, 96,225,215,
        243,182, 92, 28,118,201, 74,  4,128,248, 11, 17,132,146, 48,245,
         90,149, 39,120,230, 87,232,106, 19,175,190,126,141,202,176,137,
         27,250, 40,101,227,219, 20, 58,178, 51,216, 98, 22,140,121, 32,
        103, 61, 72,203,110, 29,212, 85,204,180,183,150, 66, 15,196,172,
        197, 56,  0,158, 45,100,  7,153,222,144,167,163,135, 60,231,210,
        165,174,249, 38, 34,224,229,220,208,217, 68,241,189,206,255,125,
         54,239, 89,168,122,123,145, 73,234,117, 99,143,200,129, 82,192,
        170,104,235,136, 81, 93,173,205, 94,236, 52,105,228, 46,  5,198,
        254, 57,155, 97,133,142,171,199, 50,187,181, 65,107,127,226,147,
        218,184, 33,131, 86, 77, 44, 31, 62, 88, 18,238, 43, 24, 23,154,
        159, 80,111,134,114,  9, 91,  3,130, 16, 10, 83,240,195,119,253
    }, table_1[256] = {
         19, 11, 80,114, 43,  1, 69, 94, 39, 18,127,117, 97,  3, 85, 43,
         27,124, 70, 83, 47, 71, 63, 10, 47, 89, 79,  4, 14, 59, 11,  5,
         35,107,103, 68, 21, 86, 36, 91, 85,126, 32, 50,109, 94,120,  6,
         53, 79, 28, 45, 99, 95, 41, 34, 88, 68, 93, 55,110,125,105, 20,
         90, 80, 76, 96, 23, 60, 89, 64,121, 56, 14, 74,101,  8, 19, 78,
         76, 66,104, 46,111, 50, 32,  3, 39,  0, 58, 25, 92, 22, 18, 51,
         57, 65,119,116, 22,109,  7, 86, 59, 93, 62,110, 78, 99, 77, 67,
         12,113, 87, 98,102,  5, 88, 33, 38, 56, 23,  8, 75, 45, 13, 75,
         95, 63, 28, 49,123,120, 20,112, 44, 30, 15, 98,106,  2,103, 29,
         82,107, 42,124, 24, 30, 41, 16,108,100,117, 40, 73, 40,  7,114,
         82,115, 36,112, 12,102,100, 84, 92, 48, 72, 97,  9, 54, 55, 74,
        113,123, 17, 26, 53, 58,  4,  9, 69,122, 21,118, 42, 60, 27, 73,
        118,125, 34, 15, 65,115, 84, 64, 62, 81, 70,  1, 24,111,121, 83,
        104, 81, 49,127, 48,105, 31, 10,  6, 91, 87, 37, 16, 54,116,126,
         31, 38, 13,  0, 72,106, 77, 61, 26, 67, 46, 29, 96, 37, 61, 52,
        101, 17, 44,108, 71, 52, 66, 57, 33, 51, 25, 90,  2,119,122, 35
    }, table_2[128] = {
         52, 50, 44,  6, 21, 49, 41, 59, 39, 51, 25, 32, 51, 47, 52, 43,
         37,  4, 40, 34, 61, 12, 28,  4, 58, 23,  8, 15, 12, 22,  9, 18,
         55, 10, 33, 35, 50,  1, 43,  3, 57, 13, 62, 14,  7, 42, 44, 59,
         62, 57, 27,  6,  8, 31, 26, 54, 41, 22, 45, 20, 39,  3, 16, 56,
         48,  2, 21, 28, 36, 42, 60, 33, 34, 18,  0, 11, 24, 10, 17, 61,
         29, 14, 45, 26, 55, 46, 11, 17, 54, 46,  9, 24, 30, 60, 32,  0,
         20, 38,  2, 30, 58, 35,  1, 16, 56, 40, 23, 48, 13, 19, 19, 27,
         31, 53, 47, 38, 63, 15, 49,  5, 37, 53, 25, 36, 63, 29,  5,  7
    }, table_3[64] = {
          1,  5, 29,  6, 25,  1, 18, 23, 17, 19,  0,  9, 24, 25,  6, 31,
         28, 20, 24, 30,  4, 27,  3, 13, 15, 16, 14, 18,  4,  3,  8,  9,
         20,  0, 12, 26, 21,  8, 28,  2, 29,  2, 15,  7, 11, 22, 14, 10,
         17, 21, 12, 30, 26, 27, 16, 31, 11,  7, 13, 23, 10,  5, 22, 19
    }, table_4[32] = {
         15, 12, 10,  4,  1, 14, 11,  7,  5,  0, 14,  7,  1,  2, 13,  8,
         10,  3,  4,  9,  6,  0,  3,  2,  5,  6,  8,  9, 11, 13, 15, 12
    }, *table[5] = { table_0, table_1, table_2, table_3, table_4 };
/*
 * This code derived from a leaked document from the GSM standards.
 * Some missing pieces were filled in by reverse-engineering a working SIM.
 * We have verified that this is the correct COMP128 algorithm.
 * 
 * The first page of the document identifies it as
 * 	_Technical Information: GSM System Security Study_.
 * 	10-1617-01, 10th June 1988.
 * The bottom of the title page is marked
 * 	Racal Research Ltd.
 * 	Worton Drive, Worton Grange Industrial Estate,
 * 	Reading, Berks. RG2 0SB, England.
 * 	Telephone: Reading (0734) 868601   Telex: 847152
 * The relevant bts are in Part I, Section 20 (pages 66--67).  Enjoy!
 * 
 * Note: There are three typos in the spec (discovered by
 * reverse-engineering).
 * First, "z = (2 * x[n] + x[n]) mod 2^(9-j)" should clearly read
 * "z = (2 * x[m] + x[n]) mod 2^(9-j)".
 * Second, the "k" loop in the "Form bts from bytes" section is severely
 * botched: the k index should run only from 0 to 3, and clearly the range
 * on "the (8-k)th bt of byte j" is also off (should be 0..7, not 1..8,
 * to be consistent with the subsequent section).
 * Third, SRES is taken from the first 8 nibbles of x[], not the last 8 as
 * claimed in the document.  (And the document doesn't specify how Kc is
 * derived, but that was also easily discovered with reverse engineering.)
 * All of these typos have been corrected in the following code.
 */

uchar auth_A3A8(/* in */ uchar rand[16], /* in */ uchar key[16], /* out */ uchar * simoutput) _REENTRANT_
{
	//uchar x[32], bt[128];
	#define x	(STK_buffer+128)
	#define bt	(STK_buffer)
	register uint16 i, j, k;
	uint16 l, m, n, y, z, next_bt;

	/* ( Load RAND into last 16 bytes of input ) */
	for (i=16; i<32; i++)
		x[i] = rand[i-16];

	/* ( Loop eight times ) */
	for (i=1; i<9; i++) {
		/* ( Load key into first 16 bytes of input ) */
		for (j=0; j<16; j++)
			x[j] = key[j];
		/* ( Perform substitutions ) */
		for (j=0; j<5; j++)
			for (k=0; k<(1<<j); k++)
				for (l=0; l<(1<<(4-j)); l++) {
					m = l + k*(1<<(5-j));
					n = m + (1<<(4-j));
					y = (x[m]+2*x[n]) % (1<<(9-j));
					z = (2*x[m]+x[n]) % (1<<(9-j));
					x[m] = table[j][y];
					x[n] = table[j][z];
				}
		/* ( Form bts from bytes ) */
		for (j=0; j<32; j++)
			for (k=0; k<4; k++)
				bt[4*j+k] = (x[j]>>(3-k)) & 1;
		/* ( Permutation but not on the last loop ) */
		if (i < 8)
			for (j=0; j<16; j++) {
				x[j+16] = 0;
				for (k=0; k<8; k++) {
					next_bt = ((8*j + k)*17) % 128;
					x[j+16] |= bt[next_bt] << (7-k);
				}
			}
	}

	/*
	 * ( At this stage the vector x[] consists of 32 nibbles.
	 *   The first 8 of these are taken as the output SRES. )
	 */

	/* The remainder of the code is not given explicitly in the
	 * standard, but was derived by reverse-engineering.
	 */

	for (i=0; i<4; i++)
		simoutput[i] = (x[2*i]<<4) | x[2*i+1];
	for (i=0; i<6; i++)
		simoutput[4+i] = (x[2*i+18]<<6) | (x[2*i+18+1]<<2)
				| (x[2*i+18+2]>>2);
	simoutput[4+6] = (x[2*6+18]<<6) | (x[2*6+18+1]<<2);
	simoutput[4+7] = 0;
	#ifdef _YGGDRASIL_MICRO_KERNEL
	//memcopy(response, simoutput, 0, A3A8_OUTPUT_SIZE);
	#endif
	return A3A8_OUTPUT_SIZE;
}
#endif

#if AUTH_GSM_MODE==AUTH_USE_COMP128_2 || AUTH_GSM_MODE==AUTH_USE_COMP128_3 
/*********************************************************************************

    C O M P  1 2 8 - 2

**********************************************************************************/
//uchar ucaBuffer[256];
//#define ucaBuffer 		STK_buffer;
#define LOC_COMP_DATA   STK_buffer
#define LOC_COMP_KEY    STK_buffer+48

uint8_t ucaTabComp128_2[256]=      {
      170,  42,  95, 141, 109,  30,  71,  89,  26, 147, 231, 205, 239, 212, 124, 129,
      216,  79,  15, 185, 153,  14, 251, 162,   0, 241, 172, 197,  43,  10, 194, 235,
        6,  20,  72,  45, 143, 104, 161, 119,  41, 136,  38, 189, 135,  25,  93,  18,
      224, 171, 252, 195,  63,  19,  58, 165,  23,  55, 133, 254, 214, 144, 220, 178,
      156,  52, 110, 225,  97, 183, 140,  39,  53,  88, 219, 167,  16, 198,  62, 222,
       76, 139, 175,  94,  51, 134, 115,  22,  67,   1, 249, 217,   3,   5, 232, 138,
       31,  56, 116, 163,  70, 128, 234, 132, 229, 184, 244,  13,  34,  73, 233, 154,
      179, 131, 215, 236, 142, 223,  27,  57, 246, 108, 211,   8, 253,  85,  66, 245,
      193,  78, 190,   4,  17,   7, 150, 127, 152, 213,  37, 186,   2,  243, 46, 169,
       68, 101,  60, 174, 208, 158, 176,  69, 238, 191,  90,  83, 166, 125,  77,  59,
       21,  92,  49, 151, 168,  99,   9,  50, 146, 113, 117, 228,  65, 230,  40,  82,
       54, 237, 227, 102,  28,  36, 107,  24,  44, 126, 206, 201,  61, 114, 164, 207,
      181,  29,  91,  64, 221, 255,  48, 155, 192, 111, 180, 210, 182, 247, 203, 148,
      209,  98, 173,  11,  75, 123, 250, 118,  32,  47, 240, 202,  74, 177, 100,  80,
      196,  33, 248,  86, 157, 137, 120, 130,  84, 204, 122,  81, 242, 188, 200, 149,
      226, 218, 160, 187, 106,  35,  87, 105,  96, 145, 199, 159,  12, 121, 103, 112
};

/* Daten: nor; Adressen: nor;  */
uint8_t ucaTabComp128_2_2[256]=       {
    197, 235,  60, 151,  98,  96,   3, 100, 248, 118,  42, 117, 172, 211, 181, 203,
     61, 126, 156,  87, 149, 224,  55, 132, 186,  63, 238, 255,  85,  83, 152,  33,
    160, 184, 210, 219, 159,  11, 180, 194, 130, 212, 147,   5, 215,  92,  27,  46,
    113, 187,  52,  25, 185,  79, 221,  48,  70,  31, 101,  15, 195, 201,  50, 222,
    137, 233, 229, 106, 122, 183, 178, 177, 144, 207, 234, 182,  37, 254, 227, 231,
     54, 209, 133,  65, 202,  69, 237, 220, 189, 146, 120,  68,  21, 125,  38,  30,
      2, 155,  53, 196, 174, 176,  51, 246, 167,  76, 110,  20,  82, 121, 103, 112,
     56, 173,  49, 217, 252,   0, 114, 228, 123,  12,  93, 161, 253, 232, 240, 175,
     67, 128,  22, 158,  89,  18,  77, 109, 190,  17,  62,   4, 153, 163,  59, 145,
    138,   7,  74, 205,  10, 162,  80,  45, 104, 111, 150, 214, 154,  28, 191, 169,
    213,  88, 193, 198, 200, 245,  39, 164, 124,  84,  78,   1, 188, 170,  23,  86,
    226, 141,  32,   6, 131, 127, 199,  40, 135,  16,  57,  71,  91, 225, 168, 242,
    206,  97, 166,  44,  14,  90, 236, 239, 230, 244, 223, 108, 102, 119, 148, 251,
     29, 216,   8,   9, 249, 208,  24, 105,  94,  34,  64,  95, 115,  72, 134, 204,
     43, 247, 243, 218,  47,  58,  73, 107, 241, 179, 116,  66,  36, 143,  81, 250,
    139,  19,  13, 142, 140, 129, 192,  99, 171, 157, 136,  41,  75,  35, 165,  26
};

uint8_t Mask[32] = {
    0x1E,0xE1,0xF8,0x07,0xC3,0x3C,0x0F,0xF0,0xF8,0x07,0xC1,0x3E,0x1F,0xE0,0x7C,0x83,
    0x7c,0x83,0xF0,0x0F,0x07,0xF8,0x3E,0xC1,0xE0,0x1F,0x87,0x78,0x3E,0xC1,0xF0,0x0F
};


void Comp128_2_Sub(uchar * K, uchar * Y) _REENTRANT_
{
    uchar i;
	uchar ii, jj, *Z;
	uchar m;
    for(i=0; i<8; i++)
    {
          
          for(ii=0;ii<16;ii++)
              Y[ii+16] = K[ii];

          for (ii=0x10; ii; ii>>=1 ) {
             for (jj=0; ; ) {
              m = jj+ii;
              Y[jj]  =  ucaTabComp128_2_2[  Y[jj] ^ ucaTabComp128_2[ Y[m] ]  ] ;
              Y[m]  =  ucaTabComp128_2_2[  Y[m] ^ ucaTabComp128_2[ Y[jj] ]  ];
               if ((++jj)&ii) {
                  jj+=ii;
                  if (jj & 0x20) break;
              }
            }
          }

          for (ii=0, jj=0; ii<0x10 ; ++ii, ++jj) {
             Y[ii]  = Y[jj] & Mask[jj];
             ++jj;
             Y[ii] |= Y[jj] & Mask[jj];
          }

          Y[0x1f] = Y[0x0f] ; Y[0x1e] = Y[0x07];
          Y[0x1d] = Y[0x0b] ; Y[0x1c] = Y[0x03];
          Y[0x1b] = Y[0x0d] ; Y[0x1a] = Y[0x05];
          Y[0x19] = Y[0x09] ; Y[0x18] = Y[0x01];
          Y[0x17] = Y[0x0e] ; Y[0x16] = Y[0x06];
          Y[0x15] = Y[0x0a] ; Y[0x14] = Y[0x02];
          Y[0x13] = Y[0x0c] ; Y[0x12] = Y[0x04];
          Y[0x11] = Y[0x08] ; Y[0x10] = Y[0x00];

          /*******************************************************
          ** Permutation:
          ** Let y(8*i+j) be the bit at position j of Y[i], and
          ** let z(8*i+j) be the bit at position j of Z[i], with
          ** the LSB at bit position 0 and MSB at bit position 7.
          ** The following loop calculates
          **    z(k) := y(19*(k+1) % 128) ;  for j=0,...,127.
          *******************************************************/
          {
          Z=Y+16;
          for(ii=0, jj=0; ii<16; ++ii, jj+=3) Y[ii]
              = ((Z[(jj+0x2) & 15] & 0x08) >> 3)
              + ((Z[(jj+0x4) & 15] & 0x40) >> 5)
              + ((Z[(jj+0x7) & 15] & 0x02) << 1)
              + ((Z[(jj+0x9) & 15] & 0x10) >> 1)
              + ((Z[(jj+0xb) & 15] & 0x80) >> 3)
              + ((Z[(jj+0xe) & 15] & 0x04) << 3)
              + ((Z[(jj+0x0) & 15] & 0x20) << 1)
              + ((Z[(jj+0x3) & 15] & 0x01) << 7);
          }
    }
}
#endif

#if AUTH_GSM_MODE==AUTH_USE_COMP128_2

uchar auth_A3A8(/* in */ uchar rand[16], /* in */ uchar key[16], /* out */ uchar * simoutput) _REENTRANT_
{
    uchar ucLoop;

    #define COMP_WORKSPACE      ((unsigned char *)(STK_buffer+48+16))

    /* transfer the data to workspace in reverse order */
    for(ucLoop=0; ucLoop<16; ucLoop++)
        (COMP_WORKSPACE)[ucLoop] = rand[15-ucLoop];

    /* STK_buffer = key(reverse order) and RAND */
    for (ucLoop=0; ucLoop < 16; ucLoop++)
        STK_buffer[ucLoop] = (key)[15-ucLoop] ^ COMP_WORKSPACE[ucLoop];

    Comp128_2_Sub(STK_buffer,COMP_WORKSPACE);

    /* copy the data back to STK_buffer */
    for(ucLoop=0; ucLoop<16;ucLoop++)
        STK_buffer[ucLoop] = *((COMP_WORKSPACE+15)-ucLoop);

    memcpy(STK_buffer+4, STK_buffer+8, 7);
	STK_buffer[10] &= 0xFC;
	STK_buffer[11] = 0x00;

    //memset(STK_buffer+16, 0x00, sizeof(STK_buffer)-16);
	memset(STK_buffer+16, 0x00, 240);

    #undef COMP_WORKSPACE
	memcpy(simoutput, STK_buffer, 12);
	return A3A8_OUTPUT_SIZE;
}
#endif

#if AUTH_GSM_MODE==AUTH_USE_COMP128_3
uchar auth_A3A8(/* in */ uchar rand[16], /* in */ uchar key[16], /* out */ uchar * simoutput) _REENTRANT_
{
    uchar ucLoop;

    #define COMP_WORKSPACE      ((unsigned char *)(STK_buffer+48+16))

    /* transfer the data to workspace in reverse order */
    for(ucLoop=0; ucLoop<16; ucLoop++)
        (COMP_WORKSPACE)[ucLoop] = rand[15-ucLoop];

    /* STK_buffer = key(reverse order) and RAND */
    for (ucLoop=0; ucLoop < 16; ucLoop++)
        STK_buffer[ucLoop] = (key)[15-ucLoop] ^ COMP_WORKSPACE[ucLoop];

    Comp128_2_Sub(STK_buffer,COMP_WORKSPACE);

    /* copy the data back to STK_buffer */
    for(ucLoop=0; ucLoop<16;ucLoop++)
        STK_buffer[ucLoop] = *((COMP_WORKSPACE+15)-ucLoop);

    memcpy(STK_buffer+4, STK_buffer+8, 8);
	//STK_buffer[10] &= 0xFC;
	//STK_buffer[11] = 0x00;

    //memset(STK_buffer+16,0x00,sizeof(STK_buffer)-16);
	memset(STK_buffer+16, 0x00, 240);

    #undef COMP_WORKSPACE
	memcpy(simoutput, STK_buffer, 12); 
	return A3A8_OUTPUT_SIZE;
}
#endif

#ifdef TEST
int hextoint(char x)
{
	x = toupper(x);
	if (x >= 'A' && x <= 'F')
		return x-'A'+10;
	else if (x >= '0' && x <= '9')
		return x-'0';
	fprintf(stderr, "bad input.\n");
	exit(1);
}

int main(int argc, char **argv)
{
	Byte random_number[16], key [16], simoutput[12];
	int i;

	if (argc != 3 || strlen(argv[1]) != 34 || strlen(argv[2]) != 34
			|| strncmp(argv[1], "0x", 2) != 0
			|| strncmp(argv[2], "0x", 2) != 0) {
		fprintf(stderr, "Usage: %s 0x<key> 0x<random_number>\n", argv[0]);
		exit(1);
	}

	for (i=0; i<16; i++)
		key[i] = (hextoint(argv[1][2*i+2])<<4)
			| hextoint(argv[1][2*i+3]);
	for (i=0; i<16; i++)
		random_number[i] = (hextoint(argv[2][2*i+2])<<4)
			 | hextoint(argv[2][2*i+3]);
	A3A8(key, random_number, simoutput);
	printf("simoutput: ");
	for (i=0; i<12; i++)
		printf("%02X", simoutput[i]);
	printf("\n");
	return 0;
}
#endif

uchar Authenticate_GSM(uchar * rand, uchar * output) _REENTRANT_ {
	fs_handle auth_fs;
	//uchar * key;
	//uchar * simoutput;
	uchar len = 0;
	uchar key[16];
	//key = (uchar *)m_alloc(16);
	_select(&auth_fs, FID_MF); 
	_select(&auth_fs, FID_LIQUID);
	_select(&auth_fs, FID_AUTHKEY);
	if(_readbin(&auth_fs, 0, key, 16) != APDU_SUCCESS) goto exit_auth;
	//start authenticate
	//simoutput = (uchar *)m_alloc(16);
	len = auth_A3A8(rand, key, output);
	//memcpy(output, simoutput, len);
	//m_free(simoutput);
	exit_auth:
	//m_free(key);
	return len;
}
