#ifndef _HEXSTRING_H_DEFINED
#include "../defs.h"

char gethex();
char gethexvalue(char c);
uint16 fromhex(char * buf, uchar size);

#define swap(data) ((data << 8) | (data >> 8))



#define _HEXSTRING_H_DEFINED
#endif
