#include "..\defs.h"
#include "..\asgard\file.h"
#include "DES.h"

extern void TDES_Decrypt(uchar * key1, uchar * key2, uchar * inbuf, uchar * outbuf) _REENTRANT_ ;
extern void TDES_Encrypt(uchar * key1, uchar * key2, uchar * inbuf, uchar * outbuf) _REENTRANT_ ;
void DES_Encrypt(uchar * key, uchar * inbuf, uchar * outbuf) _REENTRANT_;
void DES_Decrypt(uchar * key, uchar * inbuf, uchar * outbuf) _REENTRANT_;

#if DES_OPERATION_MEMORY
void DES_MemOperation(uint16 length, uchar mode, uchar * key, uchar * inbuf, uchar * outbuf) _REENTRANT_ {
	uint16 i, j;
	uchar cbc_buffer[8] = {0, 0, 0, 0, 0, 0, 0, 0};	
	uchar cbc_buffer2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	for(j=0;j<length;j+=8) {
		//DES/3DES encrypt/decrypt
		if(mode & DES_MODE_ENCRYPT) {
		//START ENCRYPTION
			if(mode & DES_MODE_CBC) {		//use CBC
				for(i=0;i<8;i++) {
					inbuf[j+i] ^= cbc_buffer[i];
					//cbc_buffer[i] = cbc_buffer2[i];
				}
			}
			if(mode & DES_MODE_TDES) {
				TDES_Encrypt(key, key + 8, inbuf + j, outbuf + j);
			} else {
				DES_Encrypt(key, inbuf + j, outbuf + j);
			}
			if(mode & DES_MODE_CBC) {		//use CBC	
				for(i=0;i<8;i++) {
					cbc_buffer[i] = outbuf[j+i];
				}
			}
		//END ENCRYPTION
		} else {
		//START DECRYPTION
			if(mode & DES_MODE_CBC) {		//use CBC	
				for(i=0;i<8;i++) {
					cbc_buffer2[i] = inbuf[j+i];
				}
			}
			if(mode & DES_MODE_TDES) {
				TDES_Decrypt(key, key + 8, inbuf + j, outbuf + j);
			} else {
				DES_Decrypt(key, inbuf + j, outbuf + j);
			}
			if(mode & DES_MODE_CBC) {		//use CBC
				for(i=0;i<8;i++) {
					outbuf[j+i] ^= cbc_buffer[i];
					cbc_buffer[i] = cbc_buffer2[i];
				}
			}
		//END DECRYPTION
		}
	}
}
#endif


#if DES_OPERATION_FILE
void DES_FileOperation(uint16 length, uint16 offset, uchar mode, uchar * key, fs_handle * handle) _REENTRANT_ {
	uint16 i, j;
	uchar k;
	uchar cbc_buffer[8] = {0, 0, 0, 0, 0, 0, 0, 0};	
	uchar cbc_buffer2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	uchar buffer[32];
	for(j=0;j<length;j+=32) {
		_readbin(handle, offset + j, buffer, 32);
		for(k=0;k<32&&(j+k)<length;k+=8) {
			//DES/3DES encrypt/decrypt
			if(mode & DES_MODE_ENCRYPT) {
			//START ENCRYPTION
				if(mode & DES_MODE_CBC) {		//use CBC
					for(i=0;i<8;i++) {
						buffer[k+i] ^= cbc_buffer[i];
					}
				}
				if(mode & DES_MODE_TDES) {
					TDES_Encrypt(key, key + 8, buffer+k, buffer+k);
				} else {
					DES_Encrypt(key, buffer+k, buffer+k);
				} 
				if(mode & DES_MODE_CBC) {		//use CBC	
					for(i=0;i<8;i++) {
						cbc_buffer[i] = buffer[k+i];
					}
				}
			//END ENCRYPTION
			} else {
			//START DECRYPTION
				if(mode & DES_MODE_CBC) {		//use CBC	
					for(i=0;i<8;i++) {
						cbc_buffer2[i] = buffer[k+i];
					}
				}
				if(mode & DES_MODE_TDES) {
					TDES_Decrypt(key, key + 8, buffer+k, buffer+k);
				} else {
					DES_Decrypt(key, buffer+k, buffer+k);
				} 
				if(mode & DES_MODE_CBC) {		//use CBC
					for(i=0;i<8;i++) {
						buffer[k+i] ^= cbc_buffer[i];
						cbc_buffer[i] = cbc_buffer2[i];
					}
				}
			//END DECRYPTION
			}
		}
		_writebin(handle, offset + j, buffer, 32);
	}
}
#endif

/*void DES_CBC_Encrypt(uchar * key, uchar * inbuf, uchar * outbuf) _REENTRANT_ {

}

void TDES_CBC_Decrypt(uchar * key1, uchar * key2, uchar * inbuf, uchar * outbuf) _REENTRANT_ {
	uchar i;
	uchar cbc_buffer[8] = {0, 0, 0, 0, 0, 0, 0, 0};	
	uchar cbc_buffer2[8] = {0, 0, 0, 0, 0, 0, 0, 0};	
	for(i=0;i<8;i++) {
		cbc_buffer2[i] = inbuf[i];
	} 
	TDES_Decrypt(key1, key2, inbuf, outbuf);
	for(i=0;i<8;i++) {
		outbuf[i] ^= cbc_buffer[i];
		cbc_buffer[i] = cbc_buffer2[i];
	}
}

void TDES_CBC_Encrypt(uchar * key1, uchar * key2, uchar * inbuf, uchar * outbuf) _REENTRANT_ {

}*/