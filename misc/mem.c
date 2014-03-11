#include "mem.h"
#include "..\defs.h"
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#if 0
uint16 memcopy(char *pdest, char *psrc, uint16 offset, uint16 size) _REENTRANT_
{
	uint16 wrote = 0;
	while(size > offset){
		*((eint8*)pdest+wrote)=*((eint8*)psrc+wrote);
		size--;
		wrote++;
	}
	return wrote;
}

void memclear(char *buffer, uint16 size) _REENTRANT_ {
	uint16 sz = size; 
	char * bx = (char *)buffer; 
	while(sz) { 
 		bx[--sz] = 0x00; 
	}
}
/*
uint16 memadd(char *pdest, char *psrc, uint16 offset, uint16 size) _REENTRANT_
{
	uint16 wrote = 0;
	while(size>offset){
		*((eint8*)pdest+size-1) = *((eint8*)pdest+size-1) + *((eint8*)psrc+size-1);
		size--;
		wrote++;
	}
	return wrote;
}
*/
uchar memcompare(char *pdest, char *psrc, uint16 offset, uint16 size) _REENTRANT_
{
	while(size>offset){
		if(*((eint8*)pdest+size-1)!=*((eint8*)psrc+size-1)) return FALSE;
		size--;
	}
	return TRUE;
}
#endif

/*void memclear(char * buffer, char fill, uchar size) _REENTRANT_ {
  	while(size!=0) {
		buffer[size-1] = fill;
		size--;
	}
} */
/*
uchar mempadding(char *buffer, char pad_char, uchar size) _REENTRANT_
{
	uchar len_buf = strlen(buffer);
	while(size>len_buf) {
		buffer[--size] = pad_char;
	}
	return size;
} */
/*
uint16 memcat(char *pdest, char *psrc, uint16 offset, uint16 size) _REENTRANT_
{
	uint16 start_offset = offset;
	while(size!=0) {
		pdest[offset] = psrc[(offset++)-start_offset];
		size--;
	}
	return offset;	//total size
} */

/*void ls_printf(uchar *str, uchar *fmt, ...) _REENTRANT_ {
	va_list arg_ptr;

	va_start (arg_ptr, fmt);           // format string
	vsprintf (str, fmt, arg_ptr);
	va_end (arg_ptr);
}*/

void ls_printf(uchar * ptr, uchar * fmt, ...) _REENTRANT_ {
	va_list arg_ptr;
	uchar p;
	uchar * buf;
	uchar buffer[6];
	uint16 i = 0, j = 0, k;
	//ptr[0] = 0;		//create null pointer string
	va_start(arg_ptr, fmt);
	while((p = fmt[j++]) != 0) {
		switch(p) {
			case '%':
				p = fmt[j++];
				switch(p) {
					case 'S':
					case 's':
						buf = va_arg(arg_ptr, uchar *);
						strcpy(ptr + i, buf);
						i += strlen(buf);
						ptr[i] = 0;
						break;
					case 'C':
					case 'c':
						ptr[i++] = (uchar)va_arg(arg_ptr, uchar);
						break;
					case 'D':
					case 'd':
						//itoa(buffer, va_arg(arg_ptr, uint16)); 
						k = va_arg(arg_ptr, uint16);
						buffer[0] = 0;
						if((k / 10000) != 0) { ptr[i++] = (0x30 | ((uchar)(k / 10000) & 0x0F)); k = k % 10000; buffer[0] = 1; } 
						if((k / 1000) != 0) { ptr[i++] = (0x30 | ((uchar)(k / 1000) & 0x0F)); k = k % 1000; buffer[0] = 1;} else { if(buffer[0] == 1) ptr[i++] = 0x30; }
						if((k / 100) != 0) { ptr[i++] = (0x30 | ((uchar)(k / 100) & 0x0F)); k = k % 100; buffer[0] = 1;} else { if(buffer[0] == 1) ptr[i++] = 0x30; }
						if((k / 10) != 0) { ptr[i++] = (0x30 | ((uchar)(k / 10) & 0x0F)); k = k % 10; buffer[0] = 1;} else { if(buffer[0] == 1) ptr[i++] = 0x30; }
						ptr[i++] = (0x30 | (k & 0x0F));
						//strcpy(ptr, buffer);
						//ptr += strlen(buffer);
						break;	
					default:
						ptr[i++] = '%';
						ptr[i++] = p;
						break;

				}
				break;
			case '\\':
				p = fmt[j++];
				switch(p) {
					case 'n':
						ptr[i++] = 0x0d;
						break;
					case 'r':
						ptr[i++] = 0x0a;
						break;
					default:
						ptr[i++] = '\\';
						ptr[i++] = p;
						break;
				}
				break;
			default:
				ptr[i++] = p;
				break;
		}
	}
	ptr[i] = 0;		//null string
	va_end(arg_ptr);
}


