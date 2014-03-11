#include <stdio.h>
#include "..\defs.h"
#include "midgard.h"

//area memory untuk heap
uchar alloc_buffer[MIDGARD_HEAP_SIZE];

//proses cast pointer dengan u_ptr sehingga bisa disesuaikan dengan target hardware
alloc_chain xdata * _chunkroot = (alloc_chain *)alloc_buffer;
u_ptr heap_end=0;
u_ptr heap_start;


void m_init_alloc()
{
	heap_start = (u_ptr)alloc_buffer;
	_chunkroot->size = 0;		//chunkroot tidak bisa di s_free
	_chunkroot->next = (alloc_chain *)((u_ptr)_chunkroot + sizeof(alloc_chain) );
	#ifdef MIDGARD_DEBUG_ACTIVATED
	//printf(" * midgard memory manager debug mode activated\n");
	//printf(" * root chunk at %i\n", (u_ptr)_chunkroot);
	#endif
}

alloc_chain xdata * alloc_ptr;
alloc_chain xdata * alloc_ptr_temp;
alloc_chain xdata * candidate; 
    //alloc_chain * alloc_ptr;
alloc_chain xdata * alloc_prev;

voidx * m_alloc(uint16 size) {
	//inisialisasi variabel
	alloc_ptr = _chunkroot;
	alloc_ptr_temp = (alloc_chain *)NULL;
	//alloc_chain *temp;
	get_next_chain:
	if(alloc_ptr->next==NULL) {		//allocate new chunk at the end of the heap
		#ifdef MIDGARD_DEBUG_ACTIVATED
		//printf(" * creating new chunk with size %i byte \n", size);
		#endif
		alloc_ptr->size = size;
		alloc_ptr->next = (alloc_chain *)((u_ptr)alloc_ptr + sizeof(alloc_chain) + size);
		//disini cek STACK POINTER, dikhawatirkan akan bertabrakan dengan stack, sebaiknya antara stack pointer dan heap dibuat
		//GAP yang cukup lebar karena dikhawatirkan ukuran stack akan bertambah setelah keluar dari fungsi s_alloc karena automatic allocation
		if((u_ptr)alloc_ptr->next >= (heap_start + MIDGARD_HEAP_SIZE)) { 
			return NULL;
		}
		alloc_ptr->prev = alloc_ptr_temp;
		alloc_ptr_temp = alloc_ptr->next;	//berubah fungsi untuk menjadi pointer chunk selanjutnya
		alloc_ptr_temp->prev = alloc_ptr;
		alloc_ptr_temp->next = NULL;
		#ifdef MIDGARD_DEBUG_ACTIVATED
		//printf(" * address of new chunk at %ld\n", (alloc_chain *)((u_ptr)alloc_ptr));
		#endif
		 
		heap_end = (u_ptr)alloc_ptr->next;
		return (alloc_chain *)((u_ptr)alloc_ptr + sizeof(alloc_chain));	//return pointer sekarang + ukuran header karena *[header]+[body]
	}
	//temp = alloc_ptr->next;		//isi dengan pointer chunk sebelumnya
	alloc_ptr_temp = alloc_ptr;
	alloc_ptr = alloc_ptr->next;
	if((u_ptr)alloc_ptr >= ((u_ptr)alloc_ptr_temp + alloc_ptr_temp->size + sizeof(alloc_chain) + sizeof(alloc_chain) + size)) {
		//allocate new heap using FFA (First Fit Algorithm)
		candidate = (alloc_chain *)((u_ptr)alloc_ptr_temp + alloc_ptr_temp->size + sizeof(alloc_chain));
		#ifdef MIDGARD_DEBUG_ACTIVATED
		//printf(" * creating new chunk with size %i\n", size);
		#endif
		candidate->size = size;
		alloc_ptr_temp->next = candidate;
		alloc_ptr_temp->next->prev = alloc_ptr_temp;
		candidate->next = alloc_ptr;
		candidate->next->prev = candidate;
		#ifdef MIDGARD_DEBUG_ACTIVATED
		//printf(" * address of new chunk at %ld\n", (alloc_chain *)((u_ptr)candidate));
		#endif
		return (alloc_chain *)((u_ptr)candidate + sizeof(alloc_chain));
	}
	goto get_next_chain;
}

void m_free(voidx *ptr) {
	alloc_ptr = (alloc_chain *)((u_ptr)ptr - sizeof(alloc_chain));
	alloc_prev = alloc_ptr->prev;
	#ifdef MIDGARD_DEBUG_ACTIVATED
	//printf(" * delete chunk at %ld\n", (u_ptr)alloc_ptr);
	//printf(" * chunk at %ld, next chunk at ", (u_ptr)alloc_prev);
	#endif
	//alloc_ptr = alloc_ptr->next;
	if(alloc_ptr==_chunkroot) {
		alloc_prev->next = alloc_ptr->next;
		alloc_prev->next->prev = alloc_prev;
		#ifdef MIDGARD_DEBUG_ACTIVATED
		//printf("%i\n", (u_ptr)alloc_prev->next);
		#endif
	} else { 
		alloc_prev->next = alloc_ptr->next;
		alloc_prev->next->prev = alloc_prev;
		#ifdef MIDGARD_DEBUG_ACTIVATED
		//printf("%i\n", (u_ptr)alloc_prev->next);
		#endif
		//s_gc();
		//getch();
	}
}

void * m_alloc_nd(uint16 size) {		//m_alloc(no debug)
	//untuk menghemat pemakaian memori static tidak diimplementasikan pada no debug
    alloc_chain *alloc_ptr;
	alloc_chain *alloc_ptr_temp;
	alloc_chain * candidate;
	//inisialisasi variabel
	alloc_ptr = _chunkroot;
	alloc_ptr_temp = (alloc_chain *)NULL;
	//alloc_chain *temp;
	get_next_chain:
	if(alloc_ptr->next==NULL) {		//allocate new chunk at the end of the heap
		alloc_ptr->size = size;
		alloc_ptr->next = (alloc_chain *)((u_ptr)alloc_ptr + sizeof(alloc_chain) + size);
		//disini cek STACK POINTER, dikhawatirkan akan bertabrakan dengan stack, sebaiknya antara stack pointer dan heap dibuat
		//GAP yang cukup lebar karena dikhawatirkan ukuran stack akan bertambah setelah keluar dari fungsi s_alloc karena automatic allocation
		alloc_ptr->prev = alloc_ptr_temp;
		alloc_ptr_temp = alloc_ptr->next;	//berubah fungsi untuk menjadi pointer chunk selanjutnya
		alloc_ptr_temp->prev = alloc_ptr;
		alloc_ptr_temp->next = NULL;
		return (alloc_chain *)((u_ptr)alloc_ptr + sizeof(alloc_chain));	//return pointer sekarang + ukuran header karena *[header]+[body]
	}
	//temp = alloc_ptr->next;		//isi dengan pointer chunk sebelumnya
	alloc_ptr_temp = alloc_ptr;
	alloc_ptr = alloc_ptr->next;
	if((uint32)alloc_ptr >= ((u_ptr)alloc_ptr_temp + alloc_ptr_temp->size + sizeof(alloc_chain) + sizeof(alloc_chain) + size)) {
		//allocate new heap using FFA (First Fit Algorithm)
		candidate = (alloc_chain *)((u_ptr)alloc_ptr_temp + alloc_ptr_temp->size + sizeof(alloc_chain));
		candidate->size = size;
		alloc_ptr_temp->next = candidate;
		alloc_ptr_temp->next->prev = alloc_ptr_temp;
		candidate->next = alloc_ptr;
		candidate->next->prev = candidate;
		return (alloc_chain *)((u_ptr)candidate + sizeof(alloc_chain));
	}
	goto get_next_chain;
}

void m_free_nd(void *ptr) {			//m_free(no debug)
	//untuk menghemat pemakaian memori static tidak diimplementasikan pada no debug
    alloc_chain *alloc_ptr;
	alloc_chain *alloc_prev;
	//inisialisasi
	alloc_ptr = (alloc_chain *)((u_ptr)ptr - sizeof(alloc_chain));
	alloc_prev = alloc_ptr->prev;
	//alloc_ptr = alloc_ptr->next;
	if(alloc_ptr==_chunkroot) {
		alloc_prev->next = alloc_ptr->next;
		alloc_prev->next->prev = alloc_prev;
	} else { 
		alloc_prev->next = alloc_ptr->next;
		alloc_prev->next->prev = alloc_prev;
		//s_gc();
		//getch();
	}
}

static uint32 m_shift(uchar *dst, uchar *src, uchar size)
{
	register uchar i = 0;
	#ifdef MIDGARD_DEBUG_ACTIVATED
	//printf(" * chunk at %i shifted to %i\n", (int)src, (int)dst);
	#endif
	while(i<size) {
		dst[i] = src[i++];
	}
	return (uint32)dst;
}

void m_gc() {			//proses defragmentasi heap, jangan dilakukan karena susunan pointer akan berubah
    alloc_chain *alloc_ptr;
	alloc_chain *alloc_ptr_temp;
	u_ptr offset;
	//inisialisasi
	alloc_ptr = _chunkroot;
	alloc_ptr_temp = (alloc_chain *)NULL;
	//alloc_ptr_temp = _chunkroot;
	//alloc_chain *temp, *temp2;
	//alloc_ptr = _chunkroot;
	#ifdef MIDGARD_DEBUG_ACTIVATED
	//printf(" * running garbage collector\n");
	#endif
	get_next_chain:
	if(alloc_ptr_temp != NULL) {
		offset = (u_ptr)alloc_ptr_temp + sizeof(alloc_chain) + alloc_ptr_temp->size;
		if(offset != (u_ptr)alloc_ptr) {	//chunk kosong
			//if(alloc_ptr->prev != _chunkroot) {
			alloc_ptr_temp->next = (alloc_chain *)m_shift((uchar *)offset, (uchar *)alloc_ptr, alloc_ptr->size + sizeof(alloc_chain));
			alloc_ptr = alloc_ptr_temp->next;
			//alloc_ptr_temp = (alloc_chain *)offset;
			//for(
		}
	}
	if(alloc_ptr->next==NULL) {
		return;
	}
	//isi dengan pointer chunk sebelumnya
	alloc_ptr_temp = alloc_ptr;
	alloc_ptr = alloc_ptr->next;
	goto get_next_chain;
}

uint32 m_heap_space() {			//hitung total byte chunk yang tidak terpakai
    alloc_chain *alloc_ptr;
	alloc_chain *alloc_ptr_temp;
	u_ptr offset;
	u_ptr heap_space = 0;
	alloc_ptr = _chunkroot;
	alloc_ptr_temp = (alloc_chain *)NULL;
	//alloc_ptr_temp = _chunkroot;
	//alloc_chain *temp, *temp2;
	//alloc_ptr = _chunkroot;
	#ifdef MIDGARD_DEBUG_ACTIVATED
	//printf(" * running garbage collector\n");
	#endif
	get_next_chain:
	if(alloc_ptr_temp != NULL) {
		offset = (u_ptr)alloc_ptr_temp + sizeof(alloc_chain) + alloc_ptr_temp->size;
		if(offset != (u_ptr)alloc_ptr) {	//chunk kosong
			//if(alloc_ptr->prev != _chunkroot) {
			//alloc_ptr_temp->next = m_shift((uchar *)offset, (uchar *)alloc_ptr, alloc_ptr->size + sizeof(alloc_chain));
			//alloc_ptr = alloc_ptr_temp->next;
			heap_space += (u_ptr)alloc_ptr - offset;
			//alloc_ptr_temp = (alloc_chain *)offset;
			//for(
		}
	}
	if(alloc_ptr->next==NULL) {
		return heap_space;
	}
	//isi dengan pointer chunk sebelumnya
	alloc_ptr_temp = alloc_ptr;
	alloc_ptr = alloc_ptr->next;
	goto get_next_chain;
}

uint32 m_used_heap() {
	return heap_end - heap_start;
}

void m_mem_dump()
{
	alloc_chain *alloc_ptr;
	register uchar i =0;
	register uchar j;
	alloc_ptr = _chunkroot;
	printf(" * midgard memory chunks dump : \n\n");
	putchar(0x20);
	
	get_next_chain:
	if(alloc_ptr->next==NULL) {
		printf("\n\n");
		return;
	}
	printf("\xb4% 8ld, % 3i\xc3\xc4", (uint32)alloc_ptr, alloc_ptr->size);
	i++;
	if(i%4==0) {
		printf("\xbf\n\xda");
		for(j=0;j<0x20;j++) {
			printf("\xc4\xc4");
		}
		printf("\xd9\n\xc0");
	} else {
		//putchar(0xc4);
	}
	alloc_ptr = alloc_ptr->next;
	goto get_next_chain;
}


#if _MIDGARD_MM_TEST
int main(void)
{
		int *ptr[100] = {0}, i=0;
  		//hash_init();
        m_init_alloc();
        printf("start…\n");
                
        for (i  = 0; i < 50; i++) {
        	ptr[i]=m_alloc(sizeof(int));
        }
		*ptr[24]=8;
        
        for(i = 10;i < 20; i++) {
        	m_free(ptr[i]);
			
        }

		for(i = 10;i < 20; i++) {
        	ptr[i]=m_alloc(sizeof(int));
        }

		for(i = 10;i < 20; i++) {
        	m_free(ptr[i]);
			
        }

				//m_mem_dump();
	
	//m_gc();
	m_mem_dump();
	printf("heap space : %i, used heap : %i\n", m_heap_space(), m_used_heap());

}
#endif
