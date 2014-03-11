/* Program showing sample usage of the dynamic memory analysis library */

#include <stdio.h>
#include <stdlib.h>
#include "../defs.h"
#include "barrenkalaheapcalc.h"

#ifndef SUCCESS
#define	SUCCESS	1
#endif
#ifndef FAIL
#define	FAIL	0 
#endif
#ifndef PLUS
#define	PLUS 	1
#endif
#ifndef MINUS
#define	MINUS	0
#endif


#if HEAP_CALC==1
#define HASHSIZE 200
#endif

#if HEAP_CALC==1
/*static void * ptr_address[HASHSIZE];
static int ptr_bytes[HASHSIZE]={0};
static int num_of_malloc=0;
static int num_of_free=0;*/
#endif


/****************************************************************************/
#if HEAP_CALC==1
int barren_insert(void *ptr,int value)
{    
	/*int i=0;
	num_of_malloc++;
	for( i =0; i < HASHSIZE; i++ ) {
		if(ptr_bytes[i] == 99 ) {
			ptr_address[i] = ptr;
			ptr_bytes[i] = value;
			barren_update(value,PLUS);
			return SUCCESS;
		}
	}
	printf("WARNING : insert to hashtable failed ptr = %u and size = %d\n", ptr,value);*/
	return FAIL; 
}

/****************************************************************************/
int barren_eject(void *ptr)
{
	/*int i=0;
	int tempvalue=0;
	if(ptr==NULL) {
		printf("WARNING : NULL pointer encountered by eject\n");
		return SUCCESS;
		}
	num_of_free++;
	for( i =0; i < HASHSIZE; i++ ) {
		if(ptr_address[i] == ptr)  {
			ptr_address[i] = (void *)99;
			tempvalue = ptr_bytes[i];
			ptr_bytes[i] = 99;
			barren_update(tempvalue,MINUS);
			return SUCCESS;
		}
	}
	printf("WARNING : eject to hashtable failed\n ptr = %u\n", ptr); */
	return FAIL;
}

/****************************************************************************/
void hash_init(void)
{
	/*int i=0;
	for(i=0;i<HASHSIZE;i++) {
		ptr_address[i]=(void *)99;
		ptr_bytes[i]=99;
	}  */
}

/****************************************************************************/
void barren_update(int val,int operation)
{
	/*static int max_malloc=0,peak_max_malloc=0;
	if(operation==PLUS) {
		max_malloc=max_malloc+val;
	} else if(operation==MINUS) {
		max_malloc=max_malloc-val;
	}
	if(peak_max_malloc<max_malloc) {
		peak_max_malloc=max_malloc;
	}
	printf("INFO : current heap size = % 4d byte, maximum allocated heap = % 4d byte\n",max_malloc,peak_max_malloc);*/
}   
  
/* This function is not necessary, its meant to dump the current snapshot of the hash table for debugging purposes */
void scan(void)
{
	/*int i=0;
	printf("INFO : scan hashtable contents \n");
	for( i =0; i < HASHSIZE; i++ ) {
		printf("\t ptr=%u, size=%d \n",ptr_address[i],ptr_bytes[i]);
	}*/
}
#endif

/****************************************************************************/
/*int main(void)
{
		int *ptr[100] = {0}, i=0;
  		hash_init();
        
        printf("start…\n");
                
        for (i  = 0; i < 50; i++) {
        	ptr[i]=malloc(sizeof(int));
            insert(ptr[i],sizeof(int));
        }
        
        for(i = 0;i < 49; i++) {
        	free(ptr[i]);
            eject(ptr[i]);
        }
}*/

/************************************************************ 
						OUTPUT
*************************************************************
$ ./tryal
start…
INFO:In function update() Max_malloc = 4 Peak max malloc = 4
INFO:In function update() Max_malloc = 8 Peak max malloc = 8
INFO:In function update() Max_malloc = 12 Peak max malloc = 12
INFO:In function update() Max_malloc = 16 Peak max malloc = 16
INFO:In function update() Max_malloc = 20 Peak max malloc = 20
INFO:In function update() Max_malloc = 24 Peak max malloc = 24
INFO:In function update() Max_malloc = 28 Peak max malloc = 28
INFO:In function update() Max_malloc = 32 Peak max malloc = 32
INFO:In function update() Max_malloc = 36 Peak max malloc = 36
INFO:In function update() Max_malloc = 40 Peak max malloc = 40
INFO:In function update() Max_malloc = 44 Peak max malloc = 44
INFO:In function update() Max_malloc = 48 Peak max malloc = 48
INFO:In function update() Max_malloc = 52 Peak max malloc = 52
INFO:In function update() Max_malloc = 56 Peak max malloc = 56
INFO:In function update() Max_malloc = 60 Peak max malloc = 60
INFO:In function update() Max_malloc = 64 Peak max malloc = 64
INFO:In function update() Max_malloc = 68 Peak max malloc = 68
INFO:In function update() Max_malloc = 72 Peak max malloc = 72
INFO:In function update() Max_malloc = 76 Peak max malloc = 76
INFO:In function update() Max_malloc = 80 Peak max malloc = 80
INFO:In function update() Max_malloc = 84 Peak max malloc = 84
INFO:In function update() Max_malloc = 88 Peak max malloc = 88
INFO:In function update() Max_malloc = 92 Peak max malloc = 92
INFO:In function update() Max_malloc = 96 Peak max malloc = 96
INFO:In function update() Max_malloc = 100 Peak max malloc = 100
INFO:In function update() Max_malloc = 104 Peak max malloc = 104
INFO:In function update() Max_malloc = 108 Peak max malloc = 108
INFO:In function update() Max_malloc = 112 Peak max malloc = 112
INFO:In function update() Max_malloc = 116 Peak max malloc = 116
INFO:In function update() Max_malloc = 120 Peak max malloc = 120
INFO:In function update() Max_malloc = 124 Peak max malloc = 124
INFO:In function update() Max_malloc = 128 Peak max malloc = 128
INFO:In function update() Max_malloc = 132 Peak max malloc = 132
INFO:In function update() Max_malloc = 136 Peak max malloc = 136
INFO:In function update() Max_malloc = 140 Peak max malloc = 140
INFO:In function update() Max_malloc = 144 Peak max malloc = 144
INFO:In function update() Max_malloc = 148 Peak max malloc = 148
INFO:In function update() Max_malloc = 152 Peak max malloc = 152
INFO:In function update() Max_malloc = 156 Peak max malloc = 156
INFO:In function update() Max_malloc = 160 Peak max malloc = 160
INFO:In function update() Max_malloc = 164 Peak max malloc = 164
INFO:In function update() Max_malloc = 168 Peak max malloc = 168
INFO:In function update() Max_malloc = 172 Peak max malloc = 172
INFO:In function update() Max_malloc = 176 Peak max malloc = 176
INFO:In function update() Max_malloc = 180 Peak max malloc = 180
INFO:In function update() Max_malloc = 184 Peak max malloc = 184
INFO:In function update() Max_malloc = 188 Peak max malloc = 188
INFO:In function update() Max_malloc = 192 Peak max malloc = 192
INFO:In function update() Max_malloc = 196 Peak max malloc = 196
INFO:In function update() Max_malloc = 200 Peak max malloc = 200
INFO:In function update() Max_malloc = 196 Peak max malloc = 200
INFO:In function update() Max_malloc = 192 Peak max malloc = 200
INFO:In function update() Max_malloc = 188 Peak max malloc = 200
INFO:In function update() Max_malloc = 184 Peak max malloc = 200
INFO:In function update() Max_malloc = 180 Peak max malloc = 200
INFO:In function update() Max_malloc = 176 Peak max malloc = 200
INFO:In function update() Max_malloc = 172 Peak max malloc = 200
INFO:In function update() Max_malloc = 168 Peak max malloc = 200
INFO:In function update() Max_malloc = 164 Peak max malloc = 200
INFO:In function update() Max_malloc = 160 Peak max malloc = 200
INFO:In function update() Max_malloc = 156 Peak max malloc = 200
INFO:In function update() Max_malloc = 152 Peak max malloc = 200
INFO:In function update() Max_malloc = 148 Peak max malloc = 200
INFO:In function update() Max_malloc = 144 Peak max malloc = 200
INFO:In function update() Max_malloc = 140 Peak max malloc = 200
INFO:In function update() Max_malloc = 136 Peak max malloc = 200
INFO:In function update() Max_malloc = 132 Peak max malloc = 200
INFO:In function update() Max_malloc = 128 Peak max malloc = 200
INFO:In function update() Max_malloc = 124 Peak max malloc = 200
INFO:In function update() Max_malloc = 120 Peak max malloc = 200
INFO:In function update() Max_malloc = 116 Peak max malloc = 200
INFO:In function update() Max_malloc = 112 Peak max malloc = 200
INFO:In function update() Max_malloc = 108 Peak max malloc = 200
INFO:In function update() Max_malloc = 104 Peak max malloc = 200
INFO:In function update() Max_malloc = 100 Peak max malloc = 200
INFO:In function update() Max_malloc = 96 Peak max malloc = 200
INFO:In function update() Max_malloc = 92 Peak max malloc = 200
INFO:In function update() Max_malloc = 88 Peak max malloc = 200
INFO:In function update() Max_malloc = 84 Peak max malloc = 200
INFO:In function update() Max_malloc = 80 Peak max malloc = 200
INFO:In function update() Max_malloc = 76 Peak max malloc = 200
INFO:In function update() Max_malloc = 72 Peak max malloc = 200
INFO:In function update() Max_malloc = 68 Peak max malloc = 200
INFO:In function update() Max_malloc = 64 Peak max malloc = 200
INFO:In function update() Max_malloc = 60 Peak max malloc = 200
INFO:In function update() Max_malloc = 56 Peak max malloc = 200
INFO:In function update() Max_malloc = 52 Peak max malloc = 200
INFO:In function update() Max_malloc = 48 Peak max malloc = 200
INFO:In function update() Max_malloc = 44 Peak max malloc = 200
INFO:In function update() Max_malloc = 40 Peak max malloc = 200
INFO:In function update() Max_malloc = 36 Peak max malloc = 200
INFO:In function update() Max_malloc = 32 Peak max malloc = 200
INFO:In function update() Max_malloc = 28 Peak max malloc = 200
INFO:In function update() Max_malloc = 24 Peak max malloc = 200
INFO:In function update() Max_malloc = 20 Peak max malloc = 200
INFO:In function update() Max_malloc = 16 Peak max malloc = 200
INFO:In function update() Max_malloc = 12 Peak max malloc = 200
INFO:In function update() Max_malloc = 8 Peak max malloc = 200
INFO:In function update() Max_malloc = 4 Peak max malloc = 200
*/

