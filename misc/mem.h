#include "../defs.h"
#ifndef _MEM__H
#if 0
uint16 memcopy(char *pdest, char *psrc, uint16 offset, uint16 size) _REENTRANT_;
uchar memcompare(char *pdest, char *psrc, uint16 offset, uint16 size) _REENTRANT_;
uchar mempadding(char *buffer, char pad_char, uchar size) _REENTRANT_;
uint16 memcat(char *pdest, char *psrc, uint16 offset, uint16 size) _REENTRANT_;
uint16 memadd(char *pdest, char *psrc, uint16 offset, uint16 size) _REENTRANT_;
//macro memory clear
//7 july 2010 : macro memclear diperbaiki, sebelumnya tidak mampu menghapus byte ke 0 dari semua sz-- menjadi --sz
/*#define memclear(buf, s) {  \
			uint16 sz = s; \
			char * bx = (char *)buf; \
			while(sz) { \
 				bx[--sz] = 0x00; \
			} \
		}*/
void memclear(char *buffer, uint16 size) _REENTRANT_ ;
#endif
void ls_printf(uchar *ptr, uchar *fmt, ...) _REENTRANT_ ;
#define _MEM__H
#endif

