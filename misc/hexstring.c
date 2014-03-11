#include "hexstring.h"
#include "..\defs.h" 
#include <stdio.h>

char gethex()
{
	/*uchar c;
	ulang_get_key:
	c = getch();
	//printf("%c ditekan \n", c);
	switch(c)
	{
		case 0x0d:
			return 0x10;
			break;
		case '0':
			putchar('0');
			return 0;
			break;
		case '1':
			putchar('1');
			return 1;
			break;
		case '2':
			putchar('2');
			return 2;
			break;
		case '3':
			putchar('3');
			return 3;
			break;
		case '4':
			putchar('4');
			return 4;
			break;
		case '5':
			putchar('5');
			return 5;
			break;
		case '6':
			putchar('6');
			return 6;
			break;
		case '7':
			putchar('7');
			return 7;
			break;
		case '8':
			putchar('8');
			return 8;
			break;
		case '9':
			putchar('9');
			return 9;
			break;
		case 'A':
			putchar('A');
			return 0x0a;
			break;
		case 'B':
			putchar('B');
			return 0x0b;
			break;
		case 'C':
			putchar('C');
			return 0x0c;
			break;
		case 'D':
			putchar('D');
			return 0x0d;
			break;
		case 'E':
			putchar('E');
			return 0x0e;
			break;
		case 'F':
			putchar('F');
			return 0x0f;
			break;
		case 'a':
			putchar('A');
			return 0x0a;
			break;
		case 'b':
			putchar('B');
			return 0x0b;
			break;
		case 'c':
			putchar('C');
			return 0x0c;
			break;
		case 'd':
			putchar('D');
			return 0x0d;
			break;
		case 'e':
			putchar('E');
			return 0x0e;
			break;
		case 'f':
			putchar('F');
			return 0x0f;
			break;
		default:
			goto ulang_get_key;
			break;
	} */
	return 0;
}

char gethexvalue(char c)
{
	/*switch(c)
	{
		case 0x0d:
			return 0x10;
			break;
		case '0':
			putchar('0');
			return 0;
			break;
		case '1':
			putchar('1');
			return 1;
			break;
		case '2':
			putchar('2');
			return 2;
			break;
		case '3':
			putchar('3');
			return 3;
			break;
		case '4':
			putchar('4');
			return 4;
			break;
		case '5':
			putchar('5');
			return 5;
			break;
		case '6':
			putchar('6');
			return 6;
			break;
		case '7':
			putchar('7');
			return 7;
			break;
		case '8':
			putchar('8');
			return 8;
			break;
		case '9':
			putchar('9');
			return 9;
			break;
		case 'A':
			putchar('A');
			return 0x0a;
			break;
		case 'B':
			putchar('B');
			return 0x0b;
			break;
		case 'C':
			putchar('C');
			return 0x0c;
			break;
		case 'D':
			putchar('D');
			return 0x0d;
			break;
		case 'E':
			putchar('E');
			return 0x0e;
			break;
		case 'F':
			putchar('F');
			return 0x0f;
			break;
		case 'a':
			putchar('A');
			return 0x0a;
			break;
		case 'b':
			putchar('B');
			return 0x0b;
			break;
		case 'c':
			putchar('C');
			return 0x0c;
			break;
		case 'd':
			putchar('D');
			return 0x0d;
			break;
		case 'e':
			putchar('E');
			return 0x0e;
			break;
		case 'f':
			putchar('F');
			return 0x0f;
			break;
		default:
			return 0x00;
			break;
	}*/
	return c;
}

uint16 fromhex(char * buf, uchar size)
{
	uint16 temp = 0;
	uchar i;
	for(i=0;i<size;i++)
	{
		temp <<= 8;
		temp |= (uchar)buf[i];
	}
	return temp;
}
