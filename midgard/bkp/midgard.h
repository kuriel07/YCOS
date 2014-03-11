//#define MAX_MEMORY 1024 * 1024 * MEMORY_SIZE_IN_MEGABYTES
// using a fixed size array isn’t the best idea,
// the best implementation would be to manage the data with a chained list
//#define LIST_ITEMS 100
#include "..\defs.h"
#ifndef _MIDGARD__H
#define MIDGARD_HEAP_SIZE	0x120UL		//512 bytes

#if _MIDGARD_DEBUG
#define MIDGARD_DEBUG_ACTIVATED
#endif


struct alloc_chain {
    struct alloc_chain * next;
	struct alloc_chain * prev;
    uint16 size;
};

typedef struct alloc_chain alloc_chain;

void m_init_alloc();
voidx * m_alloc(uint16 size);
void m_free(voidx *ptr);
void * m_alloc_nd(uint16 size);		//non debug version
void m_free_nd(void *ptr);			//non debug version
static uint32 m_shift(uchar *dst, uchar *src, uchar size);
void m_gc();
uint32 m_heap_space();
uint32 m_used_heap();
void m_mem_dump();


#define _MIDGARD__H
#endif
