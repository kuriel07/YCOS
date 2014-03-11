#include "..\midgard\midgard.h"
#include "DCS.h"    
#include <string.h>

#if COMPACT_7BIT_SUPPORT 
uchar decode_728(uchar * buffer_in, uchar * buffer_out, uchar size) _REENTRANT_ {		//7bit to 8 bit charset
	//uchar buffer[7];
	uchar i, j, k, l, m;
	uchar * temp;
	//printf("allocate %i\n", ((uint16)size * 8) / 7);
	temp = (uchar *)m_alloc(((uint16)size * 8) / 7);	  //allocate new temporary for 8bit charset
	for(i=0, j=0;i<size;i+=7, j+=8) {
		for(k=0, l=0x7F, m=0;k<8;k++, l>>=1, m<<=1) {
			m|=1;
			temp[j+k] = (buffer_in[i+k] & l) << k | ((buffer_in[i+k-1] >> (8-k)) & m);
		}
		//temp[j+0] = (buffer_in[i+0] & 0x7F) << 0 | ((buffer_in[i+0-1] >> 8) & 0);
		//temp[j+1] = (buffer_in[i+1] & 0x3F) << 1 | ((buffer_in[i+1-1] >> 7) & 1);
		//temp[j+1] = (buffer_in[i+2] & 0x1F) << 2 | ((buffer_in[i+2-1] >> 6) & 3);
	}
	memcpy(buffer_out, temp, j);	  //copy to buffer_out
	m_free(temp);					//free temp
	return (((uint16)size * 8) / 7);
}

uchar encode_827(uchar * buffer_in, uchar * buffer_out, uchar size, uchar offset) _REENTRANT_ {		//8bit to 7 bit charset
	//uchar buffer[7];
	uchar i, j, k, l, m, o;
	uchar c;
	//offset value between 0..7
	//uchar roffset = 8 - offset;
	uchar * temp;
	//printf("allocate %i\n", ((uint16)size * 7) / 8);
	temp = (uchar *)m_alloc(7);	  //allocate new temporary for 8bit charset
	memset(temp, 0, 7);
	i=0;
	m=0;
	//initialize mask bit value	
	/*for(i=0;i<offset;i++,m<<=1) {
	 	m|=1;
	}*/
	i = 0;
	j = 0;
	k = offset;
	l = 0;	
	//if(i == 0 && offset == 0) i=1;
	//for(i=0, j=0;i<size;i+=8, j+=7) {
	while(i < size) {

		//block encoder (827)
		/*for(k=offset, l=0x7F; k<7, i<size; k++, l>>=1, m<<=1) {
			//temp[j+k] = (buffer_in[i+k] & l) << k | ((buffer_in[i] >> (8-k)) & m);
			m|=1;
			//temp[j+k] = (buffer_in[i+k] & l) << k | ((buffer_in[i] >> (8-k)) & m);
			o = (k - offset);
			buffer_out[j+o] = ((buffer_in[i] & 0x7F) >> k) | ((buffer_in[i+1] & m) << (7-k));
			i++;
			//temp[o] = c;
			//temp[k]=offset;
			//temp[k] = temp[k] & ~(0xff << offset);
			//temp[k] = temp[k] | (c << offset);
			//temp[k+1] = temp[k+1] & ~(0xff >> roffset);
			//temp[k+1] = temp[k+1] | (c >> roffset);
		}
		//memcpy(buffer_out + j, temp, k);
		j += (k - offset);	 
		//i += ((k - offset)+1);
		offset = 0;		//clear offset (start of new block)
		m = 0;			//clear mask bit */
		/*temp[j+0] = (buffer_in[i+0] & 0x7F) >> 0 | (buffer_in[i+1] & 0x1) << 7;
		temp[j+1] = (buffer_in[i+1] & 0x7F) >> 1 | (buffer_in[i+2] & 0x3) << 6;
		temp[j+2] = (buffer_in[i+2] & 0x7F) >> 2 | (buffer_in[i+3] & 0x7) << 5;
		temp[j+3] = (buffer_in[i+3] & 0x7F) >> 3 | (buffer_in[i+4] & 0x0f) << 4;
		temp[j+4] = (buffer_in[i+4] & 0x7F) >> 4 | (buffer_in[i+5] & 0x1f) << 3;
		temp[j+5] = (buffer_in[i+5] & 0x7F) >> 5 | (buffer_in[i+6] & 0x3f) << 2;
		temp[j+7] = (buffer_in[i+6] & 0x7F) >> 6 | (buffer_in[i+7] & 0x3f) << 1; */	 
		//temp = (buffer_out + j + k);
		switch(k) {
			case 0:
				//buffer_out[i] = ((buffer_in[i] & 0x7F) >> k) | ((buffer_in[i+1] & m) << (7-k));
				temp[k] = (buffer_in[i] & 0x7F);
				l++; 
				break;
			case 1:
				temp[k-1] &= ~((0x01) << 7);
				temp[k-1] |= ((buffer_in[i] & 0x01) << 7);
				temp[k] = ((buffer_in[i] & 0x7F) >> 1); 
				l++;	
				break; 
			case 2:		  
				temp[k-1] &= ~((0x03) << 6);
				temp[k-1] |= ((buffer_in[i] & 0x03) << 6);
				temp[k] = ((buffer_in[i] & 0x7F) >> 2); 
				l++; 
				break;
			case 3:	 
				temp[k-1] &= ~((0x07) << 5);
				temp[k-1] |= ((buffer_in[i] & 0x07) << 5);
				temp[k] = ((buffer_in[i] & 0x7F) >> 3);
				l++; 
				break;
			case 4:		
				temp[k-1] &= ~((0x0F) << 4);
				temp[k-1] |= ((buffer_in[i] & 0x0F) << 4);
				temp[k] = ((buffer_in[i] & 0x7F) >> 4);
				l++; 
				break;
			case 5:			
				temp[k-1] &= ~((0x1F) << 3);
				temp[k-1] |= ((buffer_in[i] & 0x1F) << 3);
				temp[k] = ((buffer_in[i] & 0x7F) >> 5); 
				l++; 
				break;	 
			case 6:				
				temp[k-1] &= ~((0x3F) << 2);
				temp[k-1] |= ((buffer_in[i] & 0x3F) << 2);
				temp[k] = ((buffer_in[i] & 0x7F) >> 6);  
				l++;			   
				break;	 
			case 7:	 
				//*(temp + 1) &= ~((0x7F) << 1);
				temp[k-1] |= ((buffer_in[i] & 0x7F) << 1); 
				break;	
			default: break; 
		}
		i++;	   
		k++;
		k = k % 8; 
		if(k == 0) {
			memcpy(buffer_out + j, temp + offset, l);	
			memset(temp, 0, 7);
			offset = 0;
			j += l; 
			l = 0;		//total octed wrote
		}
	}
	if(k != 0) {
		memcpy(buffer_out + j, temp + offset, l);
		j += l;
	}
	return j;
	//memcpy(buffer_out, temp, j);	  //copy to buffer_out
	/*m_free(temp);					//free temp
	if((size % 8) == 0) {
		return (((uint16)size * 7) / 8); 		//don't ceil this value
	}
	return (((uint16)size * 7) / 8) + 1;		//ceiling*/
}
#endif

/*uchar add_variable(uchar * operand1, uchar * operand2, uchar size) _REENTRANT_ {
	uchar i;
	uint16 temp;
	for(i = (size - 1); i != 0; i--) {
			
	}
} */

#if UNICODE_SUPPORT
uchar decode_ucs28(uint16 * buffer_in, uchar * buffer_out, uchar size) _REENTRANT_ {		//unicode to 8 bit charset
	uchar i;
	size = (size / 2);
	for(i=0; i < size; i++) {
		buffer_out[i] = buffer_in[i];		
	}
	return size;
}

uchar encode_82ucs(uchar * buffer_in, uint16 * buffer_out, uchar size) _REENTRANT_ {		//8bit to unicode charset
	uchar i;
	uchar * buffer = m_alloc(size);
	memcpy(buffer, buffer_in, size);
	size = (size * 2);
	for(i=0; i < size; i++) {
		buffer_out[i] = buffer[i];		
	}
	m_free(buffer);
	return size;
}
#endif