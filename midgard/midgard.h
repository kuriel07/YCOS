//#define MAX_MEMORY 1024 * 1024 * MEMORY_SIZE_IN_MEGABYTES
// using a fixed size array isn’t the best idea,
// the best implementation would be to manage the data with a chained list
//#define LIST_ITEMS 100
#ifndef _MIDGARD_H
#include "..\defs.h"
#include "..\config.h"
#include "stdlib.h"
#include "stdio.h"
//tested on x86
#define _MIDGARD_MM_TEST 0
#define _MIDGARD_DEBUG 0
#if _MIDGARD_DEBUG
#define MIDGARD_DEBUG_ACTIVATED
#endif
#define MW_32BIT		32			//32 bit system
#define MW_16BIT		16			//16 bit system
#define MEMORY_WIDTH	MW_16BIT
#define MEM_HEAP_SIZE 	MIDGARD_HEAP_SIZE

extern uint16 _total_heap;
extern uint16 _used_heap;
//extern char Image$$RW$$Limit[];
#define _USE_MIDGARD	 			1
#define ALLOC_CHAIN_USE_PREV		0
struct alloc_chain {
	#if MEMORY_WIDTH == MW_32BIT
    uint32 size;
	#else
	//uint16 size;
	uchar size;
	#endif
    struct alloc_chain * next;
    //prev never used actually, can be eliminated for better resources
	#if ALLOC_CHAIN_USE_PREV
	struct alloc_chain * prev;
	#endif
};

typedef struct alloc_chain alloc_chain;

struct Heap_Manager {
	alloc_chain * root;
	#if MEMORY_WIDTH == MW_32BIT
    uint32 size;
	#else
	uint16 size;
	#endif
};
typedef struct Heap_Manager Heap_Manager;

uint32 m_init_alloc(void) _REENTRANT_ ;
#ifdef _USE_MIDGARD
//standard ANSI C for heap management
#if MEMORY_WIDTH == MW_32BIT
void * m_alloc(uint32 size) _REENTRANT_;
uint32 m_get_allocated_space(void) _REENTRANT_;
#else
void * m_alloc(uint16 size) _REENTRANT_;
uint16 m_get_allocated_space(void) _REENTRANT_;
#endif
void m_free(void *ptr) _REENTRANT_;
#define malloc 	m_alloc
#define free	m_free
#else
#define m_alloc c_malloc
#define m_free c_free
#endif


#ifdef ENABLE_HEAP_ALLOC
//heap management for better performance on multitasking
//also to avoid memory sharing for task
#if MEMORY_WIDTH == MW_32BIT
Heap_Manager * m_create_heap(uint32 size);
void m_delete_heap(Heap_Manager * heap); 
void * m_heap_alloc(struct Heap_Manager * heap, uint32 size);
void * m_heap_alloc_b(struct Heap_Manager * heap, uint32 size, uint16 bound);		//with bound
void m_heap_free(struct Heap_Manager * heap, void * ptr);
uint32 m_heap_used(struct Heap_Manager * heap);
uint32 m_used_space(void);
void m_memcopy(void * dst, const void * src, size_t size);
#else
Heap_Manager * m_create_heap(uint16 size);
void m_delete_heap(Heap_Manager * heap); 
void * m_heap_alloc(struct Heap_Manager * heap, uint16 size);
void * m_heap_alloc_b(struct Heap_Manager * heap, uint16 size, uint16 bound);		//with bound
void m_heap_free(struct Heap_Manager * heap, void * ptr);
uint32 m_heap_used(struct Heap_Manager * heap);
uint32 m_used_space(void);
void m_memcopy(void * dst, const void * src, size_t size);
#endif

static uint32 m_shift(uchar *dst, uchar *src, uchar size);
void m_gc(void);
void m_mem_dump(void);
#endif
#define m_memcopy memcpy
#if MEMORY_WIDTH == MW_32BIT
typedef uint32 intptr;
#else 
typedef uint32 intptr;
#endif
//extern void m_memcopy(void * dst, const void * src, size_t size);
#define _MIDGARD_H
#endif
