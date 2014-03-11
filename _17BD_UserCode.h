#ifndef __usercode_H__
#define __usercode_H__
typedef	void xdata	VOIDX;
typedef	unsigned char BYTE;
typedef unsigned int  HALFW;
typedef unsigned long WORD;
typedef	unsigned short	USHORT;
typedef	BYTE xdata	BYTEX;
typedef	HALFW xdata	HALFWX;
typedef	WORD xdata	WORDX;
typedef	USHORT xdata	USHORTX;
typedef BYTE code	BYTEC;

#define	OPEN		1
#define	CLOSE		0
#define	TRUE		1
#define	FALSE		0

extern BYTE		ISO7816_Time;

//	The macros here could be used for normal bit operation
#define	Bit0_En		0x00000001//0x1
#define	Bit1_En		0x00000002//0x1<<1
#define	Bit2_En		0x00000004//0x1<<2
#define	Bit3_En		0x00000008//0x1<<3
#define	Bit4_En		0x00000010//0x1<<4
#define	Bit5_En		0x00000020//0x1<<5
#define	Bit6_En		0x00000040//0x1<<6
#define	Bit7_En		0x00000080//0x1<<7
#define	Bit8_En		0x00000100//0x1<<8
#define	Bit9_En		0x00000200//0x1<<9
#define	Bit10_En	0x00000400//0x1<<10
#define	Bit11_En	0x00000800//0x1<<11
#define	Bit12_En	0x00001000//0x1<<12
#define	Bit13_En	0x00002000//0x1<<13
#define	Bit14_En	0x00004000//0x1<<14
#define	Bit15_En	0x00008000//0x1<<15
#define	Bit16_En	0x00010000//0x1<<16
#define	Bit17_En	0x00020000//0x1<<17
#define	Bit18_En	0x00040000//0x1<<18
#define	Bit19_En	0x00080000//0x1<<19
#define	Bit20_En	0x00100000//0x1<<20
#define	Bit21_En	0x00200000//0x1<<21
#define	Bit22_En	0x00400000//0x1<<22
#define	Bit23_En	0x00800000//0x1<<23
#define	Bit24_En	0x01000000//0x1<<24
#define	Bit25_En	0x02000000//0x1<<25
#define	Bit26_En	0x04000000//0x1<<26
#define	Bit27_En	0x08000000//0x1<<27
#define	Bit28_En	0x10000000//0x1<<28
#define	Bit29_En	0x20000000//0x1<<29
#define	Bit30_En	0x40000000//0x1<<30
#define	Bit31_En	0x80000000//0x1<<31

#define	Bit0_Dis	0xFFFFFFFE//~(0x1)
#define	Bit1_Dis	0xFFFFFFFD//~(0x1<<1)
#define	Bit2_Dis	0xFFFFFFFB//~(0x1<<2)
#define	Bit3_Dis	0xFFFFFFF7//~(0x1<<3)
#define	Bit4_Dis	0xFFFFFFEF//~(0x1<<4)
#define	Bit5_Dis	0xFFFFFFDF//~(0x1<<5)
#define	Bit6_Dis	0xFFFFFFBF//~(0x1<<6)
#define	Bit7_Dis	0xFFFFFF7F//~(0x1<<7)
#define	Bit8_Dis	0xFFFFFEFF//~(0x1<<8)
#define	Bit9_Dis	0xFFFFFDFF//~(0x1<<9)
#define	Bit10_Dis	0xFFFFFBFF//~(0x1<<10)
#define	Bit11_Dis	0xFFFFF7FF//~(0x1<<11)
#define	Bit12_Dis	0xFFFFEFFF//~(0x1<<12)
#define	Bit13_Dis	0xFFFFDFFF//~(0x1<<13)
#define	Bit14_Dis	0xFFFFBFFF//~(0x1<<14)
#define	Bit15_Dis	0xFFFF7FFF//~(0x1<<15)
#define	Bit16_Dis	0xFFFEFFFF//~(0x1<<16)
#define	Bit17_Dis	0xFFFDFFFF//~(0x1<<17)
#define	Bit18_Dis	0xFFFBFFFF//~(0x1<<18)
#define	Bit19_Dis	0xFFF7FFFF//~(0x1<<19)
#define	Bit20_Dis	0xFFEFFFFF//~(0x1<<20)
#define	Bit21_Dis	0xFFDFFFFF//~(0x1<<21)
#define	Bit22_Dis	0xFFBFFFFF//~(0x1<<22)
#define	Bit23_Dis	0xFF7FFFFF//~(0x1<<23)
#define	Bit24_Dis	0xFEFFFFFF//~(0x1<<24)
#define	Bit25_Dis	0xFDFFFFFF//~(0x1<<25)
#define	Bit26_Dis	0xFBFFFFFF//~(0x1<<26)
#define	Bit27_Dis	0xF7FFFFFF//~(0x1<<27)
#define	Bit28_Dis	0xEFFFFFFF//~(0x1<<28)
#define	Bit29_Dis	0xDFFFFFFF//~(0x1<<29)
#define	Bit30_Dis	0xBFFFFFFF//~(0x1<<30)
#define	Bit31_Dis	0x7FFFFFFF//~(0x1<<31)

#include	"..\8051SYS\THC20F17BDV10SFR.h"

void	main(void);
void	HW_Init(void);
void	mem_cpy(BYTE * dst,BYTE * src,USHORT len);
BYTE	mem_cmp(BYTE * dst,BYTE * src,USHORT len);

#endif
