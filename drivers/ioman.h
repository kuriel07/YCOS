#include "..\defs.h"
#include "..\asgard\crc.h"
#include "..\yggdrasil\yggdrasil.h"
#include <stdio.h>
#ifndef _IOMAN__H

//global access buffer
#ifdef _YGGDRASIL_MICRO_KERNEL
//extern apdu_command *iobuf;
#endif

void maze_init() _REENTRANT_ ;
uint16 maze_find_empty_sector() _REENTRANT_ ;

void ioman_init() _REENTRANT_ ;
char ioman_read_pos(uint16 pos);
char ioman_read();
void ioman_write_pos(uint16 pos, char byte);
void ioman_write(char byte);
void ioman_seek(uint16 pos);
void ioman_close();
void ioman_send_atr();
void ioman_erase_all(void);
void ioman_erase_page(uint16 pos);
void ioman_read_buffer(uint16 pos, uchar * bytes, uint16 length) _REENTRANT_ ;
uint16 ioman_write_buffer(uint16 pos, uchar * bytes, uint16 length) _REENTRANT_ ;
//void ioman_read_program(uint16 pos, ucharx * bytes, uint16 length);
uint16 ioman_write_program(uint16 pos, ucharx * bytes, uint16 length) _REENTRANT_ ;
uint16 ioman_program_copy(uint16 dest, uint16 src, uint16 length) _REENTRANT_ ;
void ioman_set_to_bootloader(void);

#ifdef _YGGDRASIL_MICRO_KERNEL		//prosedur/fungsi berikut didalam sistem operasi
void ioman_transmit(uchar size, uchar ins, char * buffer, uint16 status) _REENTRANT_ ;
uchar ioman_receive(uchar * buffer);
#endif

#define _IOMAN__H
#endif


