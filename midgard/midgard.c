#include "..\defs.h"
#include "midgard.h"
//#include "ucos_ii.h"
//#include "2440lib.h"

//#else
static volatile uchar alloc_buffer[MEM_HEAP_SIZE];
//#endif

//fixed heap allocation bug, tested on x86 and ARM (April 12 2011)
//fixed heap information bug, tested on x86 and ARM (April 13 2011)

//heap information
//uint16 _total_heap;
//uint16 _used_heap;
//uint16 _maximum_total_heap = 0;

//extern char Image$$RW$$Limit[];
//alloc_chain * _chunkroot = (alloc_chain *)Image$$RW$$Limit;
//alloc_chain * _chunkroot = (alloc_chain *)alloc_buffer;

uint32 m_init_alloc(void) _REENTRANT_
{
	//global memory management allocation
	//chunkroot needed in order to initialize memory manager
	((alloc_chain *)alloc_buffer)->size = 0;		//chunkroot tidak bisa di s_free
	((alloc_chain *)alloc_buffer)->next = (alloc_chain *)((intptr)((alloc_chain *)alloc_buffer) + (sizeof(alloc_chain) + (sizeof(alloc_chain) % 4) ));
	#if ALLOC_CHAIN_USE_PREV
	((alloc_chain *)alloc_buffer)->prev = NULL;
	#endif
	((alloc_chain *)alloc_buffer)->next->next = NULL;		//if not null might caused bug on allocation
	//_chunkroot->next = 0;
	//_total_heap = _chunkroot->size;
	return 0;
}

#ifdef _USE_MIDGARD
#if MEMORY_WIDTH == MW_32BIT
void * m_alloc(uint32 size) _REENTRANT_ {
#else
void * m_alloc(uint16 size) _REENTRANT_ {
#endif
	//#ifdef OS_uCOS_II_H
	//heap used by several task simultaneously like sharing memory
	//#if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    //OS_CPU_SR  cpu_sr;
    //#endif
    //#endif
    alloc_chain *alloc_ptr = ((alloc_chain *)alloc_buffer);
	alloc_chain *alloc_ptr_temp = (alloc_chain *)NULL;
	alloc_chain * candidate;
	//alloc_chain *temp;
	//#ifdef OS_uCOS_II_H
	//OS_ENTER_CRITICAL();
	//#endif
	//size = size + (4 - (size % 4));
	get_next_chain:
	if(alloc_ptr->next == NULL) {		//allocate new chunk at the end of the heap
		#ifdef MIDGARD_DEBUG_ACTIVATED
		//printf(" * creating new chunk with size %i\n", size);
		#endif
		alloc_ptr->size = size;
		alloc_ptr->next = (alloc_chain *)((intptr)alloc_ptr + sizeof(alloc_chain) + size);
		//disini cek STACK POINTER, dikhawatirkan akan bertabrakan dengan stack, sebaiknya antara stack pointer dan heap dibuat
		//GAP yang cukup lebar karena dikhawatirkan ukuran stack akan bertambah setelah keluar dari fungsi s_alloc karena automatic allocation
		#if ALLOC_CHAIN_USE_PREV
		alloc_ptr->prev = alloc_ptr_temp;
		#endif
		alloc_ptr_temp = alloc_ptr->next;	//berubah fungsi untuk menjadi pointer chunk selanjutnya
		#if ALLOC_CHAIN_USE_PREV
		alloc_ptr_temp->prev = alloc_ptr;
		#endif
		alloc_ptr_temp->next = NULL;
		alloc_ptr_temp->size = 0;
		//alloc_ptr = alloc_ptr_temp;
		#ifdef MIDGARD_DEBUG_ACTIVATED
		//printf(" * address of new chunk at %ld\n", (alloc_chain *)((intptr)alloc_ptr));
		#endif
		//Uart_Printf("%d\n", alloc_ptr);
		//#ifdef OS_uCOS_II_H
		//OS_EXIT_CRITICAL();
		//#endif
		//_total_heap += alloc_ptr->size;
		/*if(_total_heap > _maximum_total_heap) {
			printf("Maximum total heap : %d\n", _total_heap);
			_maximum_total_heap = _total_heap;
		}  */
		return (void *)((intptr)alloc_ptr + sizeof(alloc_chain));	//return pointer sekarang + ukuran header karena *[header]+[body]
	}
	//temp = alloc_ptr->next;		//isi dengan pointer chunk sebelumnya
	alloc_ptr_temp = alloc_ptr;
	alloc_ptr = alloc_ptr->next;
	if((intptr)alloc_ptr >= (intptr)((intptr)alloc_ptr_temp + (alloc_ptr_temp->size + sizeof(alloc_chain)) + (sizeof(alloc_chain) + size))) {
		//printf("allocate in previous memory : %x, %x\n", (uint32)alloc_ptr, ((uint32)alloc_ptr_temp + alloc_ptr_temp->size + sizeof(alloc_chain) + sizeof(alloc_chain) + size + (size % 4)));
		//printf("allocate in previous memory : %x, %x\n", (intptr)alloc_ptr, ((intptr)alloc_ptr_temp + alloc_ptr_temp->size + sizeof(alloc_chain)));
		//printf("allocate in previous memory : %x, %x\n", (uint32)alloc_ptr, sizeof(alloc_chain));
		//allocate new heap using FFA (First Fit Algorithm)
		//Uart_Printf("allocate previous heap\n");
		candidate = (alloc_chain *)((intptr)alloc_ptr_temp + (alloc_ptr_temp->size + sizeof(alloc_chain)));
		#ifdef MIDGARD_DEBUG_ACTIVATED
		//printf(" * creating new chunk with size %i\n", size);
		#endif
		//candidate->size = size + (size % 4);
		candidate->size = size;
		alloc_ptr_temp->next = candidate;
		#if ALLOC_CHAIN_USE_PREV
		candidate->prev = alloc_ptr_temp;
		#endif
		candidate->next = alloc_ptr;
		#if ALLOC_CHAIN_USE_PREV
		alloc_ptr->prev = candidate;
		#endif
		//Uart_Printf("%d %d %d %d\n", candidate->prev, candidate, candidate->size, candidate->next);
		#ifdef MIDGARD_DEBUG_ACTIVATED
		//printf(" * address of new chunk at %x\n", (uint32)candidate);
		#endif
		//#ifdef OS_uCOS_II_H
		//Uart_Printf("exit\n");
		//OS_EXIT_CRITICAL();
		//#endif
		//_total_heap += candidate->size;
		/*if(_total_heap > _maximum_total_heap) {
			printf("Maximum total heap : %d\n", _total_heap);
			_maximum_total_heap = _total_heap;
		}*/
		return (void *)((intptr)candidate + (intptr)sizeof(alloc_chain));
	}
	goto get_next_chain;
}

#if MEMORY_WIDTH == MW_32BIT
uint32 m_get_allocated_space(void) _REENTRANT_ {
	uint32 alloc_space = 0;
#else
uint16 m_get_allocated_space(void) _REENTRANT_ {
	uint16 alloc_space = 0;
#endif
    alloc_chain *alloc_ptr = ((alloc_chain *)alloc_buffer);
	//size = size + (4 - (size % 4));
	get_next_chain:
	if(alloc_ptr->next==NULL) {		//allocate new chunk at the end of the heap
		alloc_space += alloc_ptr->size;
		return alloc_space;	//return pointer sekarang + ukuran header karena *[header]+[body]
	}
	//temp = alloc_ptr->next;		//isi dengan pointer chunk sebelumnya
	//alloc_ptr_temp = alloc_ptr;
	alloc_space += alloc_ptr->size;
	alloc_ptr = alloc_ptr->next;
	goto get_next_chain;
}

void m_free(void *ptr) _REENTRANT_ {
	//#ifdef OS_uCOS_II_H
	//#if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    //OS_CPU_SR  cpu_sr;
    //#endif
    //#endif
	#if ALLOC_CHAIN_USE_PREV
    alloc_chain *alloc_ptr = (alloc_chain *)((intptr)ptr - (intptr)sizeof(alloc_chain));
	alloc_chain *alloc_prev = alloc_ptr->prev;
	alloc_chain *alloc_next = alloc_ptr->next;
	if(ptr == NULL) return;
	if(alloc_prev == NULL) return;		//this memory is already freed
	if(alloc_ptr->size == 0) return;		//this memory is already freed
	/*if(alloc_next == NULL) {		//this is the last chunk
		printf("this is the last chunk\n");	
	}*/
	//#ifdef OS_uCOS_II_H
	//OS_ENTER_CRITICAL();
	//#endif
	//alloc_prev = alloc_ptr->prev;
	#ifdef MIDGARD_DEBUG_ACTIVATED
	//printf(" * delete chunk at %x\n", (intptr)alloc_ptr);
	//printf(" * chunk at %x, next chunk at ", (intptr)alloc_prev);
	#endif
	//_total_heap -= alloc_ptr->size;
	//alloc_ptr = alloc_ptr->next;
	if(alloc_ptr == ((alloc_chain *)alloc_buffer)) {
		//alloc_prev->next = alloc_ptr->next;
		//alloc_prev->next->prev = alloc_prev;
		#ifdef MIDGARD_DEBUG_ACTIVATED
		//printf("%x\n", (intptr)alloc_prev->next);
		#endif
	} else { 
		alloc_next->prev = alloc_prev;
		alloc_prev->next = alloc_next;
		//alloc_prev->next->prev = alloc_prev;
		#ifdef MIDGARD_DEBUG_ACTIVATED
		//printf("%x\n", (intptr)alloc_prev->next);
		#endif
		//s_gc();
		//getch();
	}
	alloc_ptr->next = NULL;
	alloc_ptr->prev = NULL;
	alloc_ptr->size = 0;
	#else
	volatile alloc_chain *alloc_ptr = ((alloc_chain *)alloc_buffer);
	alloc_chain * temp;
	//alloc_chain *alloc_prev = NULL;
	while(alloc_ptr != NULL) {
		//if(alloc_ptr->next == NULL) return;		//not found
		if((intptr)alloc_ptr->next == (intptr)(ptr - sizeof(alloc_chain))) {		//found
			temp = alloc_ptr->next;
			alloc_ptr->next = temp->next;
			temp->size = 0;
			temp->next = NULL;
			return;
		}
		alloc_ptr = alloc_ptr->next;		//next iterator
	}

	#endif
	//#ifdef OS_uCOS_II_H
	//OS_EXIT_CRITICAL();
	//#endif
}
#endif

#ifdef ENABLE_HEAP_ALLOC
#if MEMORY_WIDTH == MW_32BIT
void * m_heap_alloc(struct Heap_Manager * heap, uint32 size) {
#else
void * m_heap_alloc(struct Heap_Manager * heap, uint16 size) {
#endif
	//#ifdef OS_uCOS_II_H
	//heap used by several task simultaneously like sharing memory
	//#if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    //OS_CPU_SR  cpu_sr;
    //#endif
    //#endif
    alloc_chain *alloc_ptr = heap->root;
	static alloc_chain *alloc_ptr_temp = (alloc_chain *)NULL;
	alloc_chain * candidate;
	//#ifdef OS_uCOS_II_H
	//OS_ENTER_CRITICAL();
	//#endif
	size = size + (4 - (size % 4));		//2 byte boundary
	get_next_chain:
	//Uart_Printf("%d\n", alloc_ptr);
	//printf("%d %d\n", ((intptr)alloc_ptr + (intptr)size + ((size%4)>0)), ((intptr)heap + heap->size + sizeof(alloc_chain)));
	if(((intptr)alloc_ptr + (intptr)size + (size % 4)) > ((intptr)heap + heap->size + sizeof(alloc_chain)))
	{
		//heap is already full, please allocate smaller size
		//printf("heap full\n");
		return NULL;
	}
	if(alloc_ptr->next==NULL) {		//allocate new chunk at the end of the heap
		#ifdef MIDGARD_DEBUG_ACTIVATED
		//printf(" * creating new chunk with size %i\n", size);
		#endif
		
		alloc_ptr->size = size + (size % 4);
		alloc_ptr->next = (alloc_chain *)((intptr)alloc_ptr + (intptr)sizeof(alloc_chain) + alloc_ptr->size);
		//disini cek STACK POINTER, dikhawatirkan akan bertabrakan dengan stack, sebaiknya antara stack pointer dan heap dibuat
		//GAP yang cukup lebar karena dikhawatirkan ukuran stack akan bertambah setelah keluar dari fungsi s_alloc karena automatic allocation
		alloc_ptr->prev = alloc_ptr_temp;
		alloc_ptr_temp = alloc_ptr->next;	//berubah fungsi untuk menjadi pointer chunk selanjutnya
		alloc_ptr_temp->prev = alloc_ptr;
		alloc_ptr_temp->size = 0;
		alloc_ptr_temp->next = NULL;
		//Uart_Printf("%d\n", alloc_ptr);
		//printf("%d\n", ((intptr)alloc_ptr + (intptr)sizeof(alloc_chain)));
		//#ifdef OS_uCOS_II_H
		//Uart_Printf("exit\n");
		//OS_EXIT_CRITICAL();
		//#endif
		_used_heap += alloc_ptr->size;
		return (void *)((intptr)alloc_ptr + (intptr)sizeof(alloc_chain));	//return pointer sekarang + ukuran header karena *[header]+[body]
	}
	//temp = alloc_ptr->next;		//isi dengan pointer chunk sebelumnya
	alloc_ptr_temp = alloc_ptr;
	alloc_ptr = alloc_ptr->next;
	if((intptr)alloc_ptr >= ((intptr)alloc_ptr_temp + alloc_ptr_temp->size + sizeof(alloc_chain) + sizeof(alloc_chain) + size)) {
		//allocate new heap using FFA (First Fit Algorithm)
		candidate = (alloc_chain *)((intptr)alloc_ptr_temp + alloc_ptr_temp->size + sizeof(alloc_chain));
		candidate->size = size + (size % 4);
		alloc_ptr_temp->next = candidate;
		alloc_ptr_temp->next->prev = alloc_ptr_temp;
		candidate->next = alloc_ptr;
		candidate->next->prev = candidate;
		//Uart_Printf("%d\n", candidate);
		//printf("%d\n", ((intptr)candidate + (intptr)sizeof(alloc_chain)));
		//#ifdef OS_uCOS_II_H
		//Uart_Printf("exit\n");
		//OS_EXIT_CRITICAL();
		//#endif
		_used_heap += candidate->size;
		return (void *)((intptr)candidate + (intptr)sizeof(alloc_chain));
	}
	goto get_next_chain;
}

void m_heap_free(struct Heap_Manager * heap, void * ptr)
{
    alloc_chain *alloc_ptr = (alloc_chain *)((intptr)ptr - (intptr)sizeof(alloc_chain));
	static alloc_chain *alloc_prev;
	if(ptr == NULL) return;
	alloc_prev = alloc_ptr->prev;
	//alloc_ptr = alloc_ptr->next;
	if(alloc_ptr == heap->root) {
		alloc_prev->next = alloc_ptr->next;
		alloc_prev->next->prev = alloc_prev;
	} else { 
		alloc_prev->next = alloc_ptr->next;
		alloc_prev->next->prev = alloc_prev;
	}
	_used_heap -= alloc_ptr->size;
}	

#if MEMORY_WIDTH == MW_32BIT
Heap_Manager * m_create_heap(uint32 size) {
#else
Heap_Manager * m_create_heap(uint16 size) {
#endif
	Heap_Manager * heap = m_alloc(sizeof(Heap_Manager) + size + sizeof(alloc_chain) + sizeof(alloc_chain) + sizeof(alloc_chain));
	heap->root = (alloc_chain *)((intptr)heap + sizeof(Heap_Manager));
	heap->size = (size - (size % 4)) + sizeof(Heap_Manager);
	
	heap->root->size = 0;		//chunkroot tidak bisa di s_free
	heap->root->prev = NULL;
	//tidak dialign 4 byte
	heap->root->next = (alloc_chain *)((intptr)heap->root + sizeof(alloc_chain) + (sizeof(alloc_chain) % 4));
	heap->root->next->next = NULL;
	//printf("%d\n", (intptr)heap->root->next);
	//align 4 byte (bisa jadi m_heap_alloc hang disini?? 
	//jika ukuran heap sama dengan jumlah yang dialokasikan dan memori direallocate)
	//heap->root->next = (alloc_chain *)((alloc_chain *)heap->root + (sizeof(alloc_chain) + (sizeof(alloc_chain)%4)));
	//Uart_Printf("heap : %d\n", heap->size);
	return heap;
}

void m_delete_heap(Heap_Manager * heap) {
	//because heap_manager is allocated using m_alloc then deallocating it with m_free
	alloc_chain * target;
	Heap_Manager * c_heap = heap;
	alloc_chain * alloc_ptr = heap->root;
	while(alloc_ptr != NULL)
	{
		_used_heap -= alloc_ptr->size;
		target = alloc_ptr;
		alloc_ptr = alloc_ptr->next;
		//nullifies all heap allocate variables (PATCH untuk m_heap_alloc??)
		//mirip garbage collector hanya untuk heap manager
		target->prev = NULL;
		target->size = 0;
		target->next = NULL;
	}
	//printf("delete heap\n");
	m_free(heap);
	//chunkroot->next diset NULL 
	c_heap->root->next = NULL;
	//chunkroot baru diset NULL
	c_heap->root = NULL;
}

uint32 m_heap_used(struct Heap_Manager * heap) {
	uint32 space = 0;
	uint32 size = 4;
	alloc_chain *alloc_ptr = heap->root;
	get_next_chain:
	//Uart_Printf("%d\n", alloc_ptr->next);
	if(alloc_ptr->next == (alloc_chain *)NULL) {		//allocate new chunk at the end of the heap
		//alloc_ptr->size = size + (size%4);
		//alloc_ptr->next = (alloc_chain *)((intptr)alloc_ptr + (intptr)sizeof(alloc_chain) + (intptr)size + ((size%4)>0));
		//disini cek STACK POINTER, dikhawatirkan akan bertabrakan dengan stack, sebaiknya antara stack pointer dan heap dibuat
		//GAP yang cukup lebar karena dikhawatirkan ukuran stack akan bertambah setelah keluar dari fungsi s_alloc karena automatic allocation
		//space += alloc_ptr->size;
		return space;	//return pointer sekarang + ukuran header karena *[header]+[body]
	}
	//temp = alloc_ptr->next;		//isi dengan pointer chunk sebelumnya
	space += alloc_ptr->size;
	alloc_ptr = alloc_ptr->next;
	goto get_next_chain;
}

uint32 m_used_space(void)
{
	uint32 space = 0;
	uint32 size = 4;
	alloc_chain *alloc_ptr = ((alloc_chain *)alloc_buffer);
	get_next_chain:
	//Uart_Printf("%d\n", alloc_ptr->next);
	if(alloc_ptr->next == (alloc_chain *)NULL) {		//allocate new chunk at the end of the heap
		//alloc_ptr->size = size + (size%4);
		//alloc_ptr->next = (alloc_chain *)((intptr)alloc_ptr + (intptr)sizeof(alloc_chain) + (intptr)size + ((size%4)>0));
		//disini cek STACK POINTER, dikhawatirkan akan bertabrakan dengan stack, sebaiknya antara stack pointer dan heap dibuat
		//GAP yang cukup lebar karena dikhawatirkan ukuran stack akan bertambah setelah keluar dari fungsi s_alloc karena automatic allocation
		//space += alloc_ptr->size;
		return space;	//return pointer sekarang + ukuran header karena *[header]+[body]
	}
	//temp = alloc_ptr->next;		//isi dengan pointer chunk sebelumnya
	space += alloc_ptr->size;
	alloc_ptr = alloc_ptr->next;
	goto get_next_chain;
}

static uint32 m_shift(uchar *dst, uchar *src, uchar size)
{
	uchar i;
	#ifdef MIDGARD_DEBUG_ACTIVATED
	//printf(" * chunk at %i shifted to %i\n", (int)src, (int)dst);
	#endif
	while(i<size) {
		dst[i] = src[i];
		i++;
	}
	return (intptr)dst;
}

void m_gc(void) {	
    alloc_chain *alloc_ptr = ((alloc_chain *)alloc_buffer)->next;
	static alloc_chain *alloc_ptr_temp = NULL;
	alloc_ptr_temp = ((alloc_chain *)alloc_buffer);
	//alloc_chain *temp, *temp2;
	//alloc_ptr = _chunkroot;
	#ifdef MIDGARD_DEBUG_ACTIVATED
	//printf(" * running garbage collector\n");
	#endif
	get_next_chain:
	if(alloc_ptr->prev != alloc_ptr_temp) {	//chunk kosong
		//if(alloc_ptr->prev != _chunkroot) {
		alloc_ptr_temp->next = (alloc_chain *)m_shift((uchar *)alloc_ptr->prev, (uchar *)alloc_ptr, alloc_ptr->size + sizeof(alloc_chain));
		alloc_ptr = alloc_ptr_temp->next;
		//printf("alloc_temp : %i, alloc_ptr : %i\n", (intptr)alloc_ptr_temp, (intptr)alloc_ptr);
		//printf("alloc_temp->next : %i, alloc_ptr->next : %i\n", (intptr)alloc_ptr_temp->next, (intptr)alloc_ptr->next);
		//printf("alloc_temp->prev : %i, alloc_ptr->prev : %i\n", (intptr)alloc_ptr_temp->prev, (intptr)alloc_ptr->prev);
			//getch();
		//}
	}
	if(alloc_ptr->next==NULL) {
		alloc_ptr->prev->next=NULL;
		return;
	}
	//isi dengan pointer chunk sebelumnya
	alloc_ptr_temp = alloc_ptr;
	alloc_ptr = alloc_ptr->next;
	goto get_next_chain;
}

void m_mem_dump(void)
{
	alloc_chain *alloc_ptr = ((alloc_chain *)alloc_buffer);
	uchar i =0;
	uchar j;
	putchar(0x20);
	get_next_chain:
	if(alloc_ptr->next==NULL) {
		//printf("\n");
		return;
	}
	printf("[% 8x, % 3i]\xc4", (intptr)alloc_ptr, alloc_ptr->size);
	i++;
	if(i%4==0) {
		printf("\xbf\n\xda");
		for(j=0;j<0x20;j++) {
			printf("\xc4\xc4");
		}
		printf("\xd9\n\xc0");
	}
	alloc_ptr = alloc_ptr->next;
	goto get_next_chain;
}

//allocate heap with address boundary (for structure like hcca on usb ohci)
#if MEMORY_WIDTH == MW_32BIT
void * m_heap_alloc_b(struct Heap_Manager * heap, uint32 size, uint16 bound) {
#else
void * m_heap_alloc_b(struct Heap_Manager * heap, uint16 size, uint16 bound) {
#endif
	//#ifdef OS_uCOS_II_H
	//heap used by several task simultaneously like sharing memory
	//#if OS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    //OS_CPU_SR  cpu_sr;
    //#endif
    //#endif
    uint32 addr;
    alloc_chain *alloc_ptr = heap->root;
	static alloc_chain *alloc_ptr_temp = (alloc_chain *)NULL;
	alloc_chain * candidate;
	//#ifdef OS_uCOS_II_H
	//OS_ENTER_CRITICAL();
	//#endif
	get_next_chain:
	//Uart_Printf("%d\n", alloc_ptr);
	//printf("%d %d\n", ((intptr)alloc_ptr + (intptr)size + ((size%4)>0)), ((intptr)heap + heap->size + sizeof(alloc_chain)));
	addr = ((intptr)alloc_ptr + (intptr)size + (size % 4));
	addr = addr + (bound - (addr % bound));
	if(addr > ((intptr)heap + heap->size + sizeof(alloc_chain)))
	{
		//heap is already full, please allocate smaller size
		//printf("heap full\n");
		return NULL;
	}
	if(alloc_ptr->next==NULL) {		//allocate new chunk at the end of the heap
		#ifdef MIDGARD_DEBUG_ACTIVATED
		//printf(" * creating new chunk with size %i\n", size);
		#endif
		addr = (intptr)alloc_ptr + sizeof(alloc_chain);
		addr = addr + (bound - (addr % bound));
		alloc_ptr_temp->next = (alloc_chain *)(addr - sizeof(alloc_chain));
		alloc_ptr = alloc_ptr_temp->next;
		alloc_ptr->size = size + (size % 4);
		addr = ((intptr)alloc_ptr + (intptr)sizeof(alloc_chain) + alloc_ptr->size);
		addr = addr + (bound - (addr % bound));
		alloc_ptr->next = (alloc_chain *)addr;
		//disini cek STACK POINTER, dikhawatirkan akan bertabrakan dengan stack, sebaiknya antara stack pointer dan heap dibuat
		//GAP yang cukup lebar karena dikhawatirkan ukuran stack akan bertambah setelah keluar dari fungsi s_alloc karena automatic allocation
		alloc_ptr->prev = alloc_ptr_temp;
		alloc_ptr_temp = alloc_ptr->next;	//berubah fungsi untuk menjadi pointer chunk selanjutnya
		alloc_ptr_temp->prev = alloc_ptr;
		alloc_ptr_temp->size = 0;
		alloc_ptr_temp->next = NULL;
		//Uart_Printf("%d\n", alloc_ptr);
		//Uart_Printf("allocate at %x\n", ((intptr)alloc_ptr + (intptr)sizeof(alloc_chain)));
		//#ifdef OS_uCOS_II_H
		//Uart_Printf("exit\n");
		//OS_EXIT_CRITICAL();
		//#endif
		_used_heap += alloc_ptr->size;
		return (void *)((intptr)alloc_ptr + sizeof(alloc_chain));	//return pointer sekarang + ukuran header karena *[header]+[body]
	}
	//temp = alloc_ptr->next;		//isi dengan pointer chunk sebelumnya
	alloc_ptr_temp = alloc_ptr;
	alloc_ptr = alloc_ptr->next;
	addr = ((intptr)alloc_ptr_temp + alloc_ptr_temp->size + sizeof(alloc_chain) + sizeof(alloc_chain) + size);
	addr = addr + (bound - (addr % bound));
	if((intptr)alloc_ptr >= addr) {
		//allocate new heap using FFA (First Fit Algorithm)
		addr = ((intptr)alloc_ptr_temp + alloc_ptr_temp->size + sizeof(alloc_chain)) + sizeof(alloc_chain);
		addr = addr + (bound - (addr % bound));
		candidate = (alloc_chain *)(addr - sizeof(alloc_chain));
		candidate->size = size + (size % 4);
		alloc_ptr_temp->next = candidate;
		alloc_ptr_temp->next->prev = alloc_ptr_temp;
		candidate->next = alloc_ptr;
		candidate->next->prev = candidate;
		//Uart_Printf("%d\n", candidate);
		//printf("%d\n", ((intptr)candidate + (intptr)sizeof(alloc_chain)));
		//#ifdef OS_uCOS_II_H
		//Uart_Printf("exit\n");
		//OS_EXIT_CRITICAL();
		//#endif
		_used_heap += candidate->size;
		return (void *)((intptr)candidate + sizeof(alloc_chain));
	}
	goto get_next_chain;
}

#endif

/*void main()
{
	Heap_Manager * h1;
	Heap_Manager * h2;
	m_init_alloc();
	h1 = m_create_heap(20000);
	h2 = m_create_heap(10000);
	m_mem_dump();
	m_heap_alloc(h1, 20000);
	m_heap_alloc(h2, 10000);
	m_mem_dump();
	m_delete_heap(h1);
	m_delete_heap(h2);
	m_mem_dump();
	
	h1 = m_create_heap(20000);
	h2 = m_create_heap(10000);
	m_mem_dump();
	if(m_heap_alloc(h1, 20000) == NULL) {
		printf("success\n");
	}
	m_heap_alloc(h2, 10000);
}*/


