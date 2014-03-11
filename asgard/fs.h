#include "..\defs.h"
#include "..\config.h"
#include "..\drivers\ioman.h"
#include "..\drivers\flash.h"
#ifndef _FS_H
/* file system provide binary file abstraction and allocation for use with the upper layer */
/* the file system handle each file as plain binary format much like asgard 1.0 and fat based file system */
#define FS_TYPE_DIR		0x8
#define FS_TYPE_USER	0x4
#define FS_TYPE_SYS		0x2
#define FS_TYPE_EXIST	0x80

#define FS_EOS			0xFFFF
#define FS_VOID			0x0000	 

//#define ALLOCATION_TABLE_OFFSET 	4
//#define ALLOCATION_TABLE_SIZE 		508
//#define ALLOCATION_DATA_OFFSET 		(ALLOCATION_TABLE_OFFSET + ALLOCATION_TABLE_SIZE) 		///512

//data always on second page 
#define ALLOCATION_DATA_OFFSET 		(1 * FLASH_LOG_PAGE_SIZE)

#define actual_sector_size(x) (x << 5)
#define logical_sector_size(x) (x >> 5)
#define fs_fseek(handle, offset) { handle->data_ptr = offset; }

#if ASGARD_VERSION==3

#define ALLOCATION_DATA_OFFSET 		(ALLOCATION_TABLE_OFFSET + ALLOCATION_TABLE_SIZE)
struct ef_table {			//16 byte
	uchar fs_type[3];		//file system tag
	uchar fd_ver;			//file system version (default (3) for asgard 3.0)
	uchar fs_mod;			//16 bit/32 bit (unused, reserved for future use)
	uchar sector_size;		//sector_size = actual_sector_size / 32, for 32byte use 1, for 512 use 16 and for 2048 use 64
	uint32 fs_size;			//file system size
};
typedef struct ef_table ef_table;
//extern ef_table table;

struct fs_handle {
	hw_rca * rca;					//hardware rca (contain device id on machine scale)
	ef_table fs_table;				//file system information and configuration
	uint16 current_alloc_ptr;		//alloc ptr for faster file creation and allocation
};
typedef struct fs_handle fs_handle;

struct fs_entry {			//only 8 byte (entry header), much like DirEntry on fat based file system
	uchar type;				//determined whether this entry is a file or directory, system or user, deleted or active
	uchar rsv;				//reserved for future use
	uint16 fid;				//fs fid for select operation
	uint16 size;			//amount of allocated size on file system (maximum file size that can be allocated by file system is 64k)
	uint16 ptr;				//pointer to the entry data (and metadata) on file system
};
typedef struct fs_entry fs_entry;

struct fs_file {
	fs_handle * fs;			//pointer to the specified file system that contain this file
	fs_entry entry;			//current fs_entry for this file
	uint16 sector_index;	//address of the first sector contain this fs_entry, did not contain any offset so file deletion must calculate the offset by scanning all fs_entry
	uint16 data_ptr;		//this handle current offset for read/write data operation, fs automatically update this pointer for any read/write operation
};
typedef struct fs_file fs_file;

fs_handle * fs_init(hw_rca * rca);
/* internal file system operation */
uint16 fs_get_next_available_sector(fs_handle * fs);		//maximum allocation table size = 64K * sizeof(uint16)
void fs_set_next_sector(fs_handle * fs, uint16 sector_index, uint16 next_sector);
uint16 fs_get_next_sector(fs_handle * fs, uint16 sector_index);
uint16 fs_read_sector(fs_handle * fs, uint16 sector_index, uchar * buffer, uint16 sector_size);
uint16 fs_write_sector(fs_handle * fs, uint16 sector_index, uchar * buffer, uint16 sector_size);
fs_entry * fs_create_entry(fs_handle * fs, uchar type, uint16 fid, uint16 first_sector_index);
void fs_update_entry(fs_handle * fs, fs_entry * entry, uint16 first_sector_index);
void fs_delete_entry(fs_handle * fs, fs_entry * entry, uint16 first_sector_index);
void fs_unlink_sector_chain(fs_handle * fs, uint16 first_sector_index);
/* external file system operation */
fs_file * fs_fopen_q(fs_handle * fs, uint16 * fids, uchar lsize, uchar mode);		//fids = array of fid from root (queue type), lsize = size of array
fs_file * fs_fopen(fs_file * parent, uint16 fid, uchar mode);
fs_file * fs_root(fs_handle * fs);
uint16 fs_fwrite(fs_file * file, uchar * buffer, uint16 size);
uint16 fs_fread(fs_file * file, uchar * buffer, uint16 size);
fs_handle * fs_format(hw_rca * rca, uint32 partition_size, uchar requested_sector_size);
fs_file * fs_mkdir(fs_file * parent, uint16 fid, uchar mode);
fs_file * fs_fcreate(fs_file * parent, uint16 fid, uchar mode);
void fs_rmfile(fs_file * file, uchar mode);

uint16 fs_freespace();
void fs_defrag();
void fs_dismount(fs_handle * fs);
#endif

#if ASGARD_VERSION==4

//#define ALLOCATION_DATA_OFFSET		512
#define AW_32BIT		32			//32 bit system
#define AW_16BIT		16			//16 bit system
#define FS_ADDRESS_WIDTH	AW_16BIT

struct fs_table {
	uchar fs_type[3];		//file system tag
	uchar fs_ver;			//file system version (default (3) for asgard 3.0)
	uchar fs_mod;			//16 bit/32 bit (unused, reserved for future use)
	uchar sector_size;		//sector_size = actual_sector_size / 32, for 32byte use 1, for 512 use 16 and for 2048 use 64
	uint32 fs_size;			//file system size
};

struct fs_chain {
	#if FS_ADDRESS_WIDTH == AW_32BIT
    uint32 next;
	uint32 prev;
    uint32 size;
	#else
	uint16 next;
	uint16 prev;
	uint16 size;
	#endif
};

struct f_header {
	#if FS_ADDRESS_WIDTH == AW_32BIT
	uint32 parent;
	uint32 sibling;
	uint32 child;
	#else
	uint16 parent;
	uint16 sibling;
	uint16 child;
	#endif
	uint16 fid;				//directory id
	uchar type;				//type (mf,df,ef)
};

struct df_header {
	#if FS_ADDRESS_WIDTH == AW_32BIT
	uint32 parent;
	uint32 sibling;
	uint32 child;
	#else
	uint16 parent;
	uint16 sibling;
	uint16 child;
	#endif
	uint16 fid;				//directory id
	uchar type;				//type (mf,df,ef)
	uchar reserved1[5];		//reserved
	uchar length;			//always 9
	uchar file_char;		//file characteristic
	uchar num_of_df;		//number of df
	uchar num_of_ef;		//number of ef
	uchar num_of_chv;
};

struct ef_header {	
	#if FS_ADDRESS_WIDTH == AW_32BIT
	uint32 parent;
	uint32 sibling;
	uint32 child;
	#else
	uint16 parent;
	uint16 sibling;
	uint16 child;			//used only for cyclic ef
	#endif
	uint16 fid;				//file id
	uchar type;				//type (mf,df,ef)
	uint16 size;			//file size	
	uchar inc;				//increase allowed
	uchar acc_rw;			//access read/write
	uchar acc_inc;			//access increase
	uchar acc_ri;			//access inval/rehab
	uchar status;			//inval/rehab status 
	uchar length;			//always 2
	uchar structure;		//transparent, cyclic, linier
	uchar rec_size;			//record size
};

typedef struct f_header f_header;
typedef struct df_header df_header;
typedef struct ef_header ef_header;

typedef struct fs_table fs_table;
typedef struct fs_chain fs_chain;


uchar fs_init(void);	
uint32 fs_freespace(void) _REENTRANT_ ;	
uchar fs_generate_FSH(uchar * buffer) _REENTRANT_ ;	
#if FS_ADDRESS_WIDTH == AW_32BIT
uint16 fs_format(uint32 size) _REENTRANT_;
uint32 fs_alloc(uint32 size) _REENTRANT_ ;
uint16 fs_dealloc(uint32 address) _REENTRANT_ ;	 		//return status
uint32 fs_get_allocated_space(void) _REENTRANT_ ;
#else
uint16 fs_format(uint32 size) _REENTRANT_;
uint16 fs_alloc(uint16 size) _REENTRANT_ ;
uint16 fs_dealloc(uint16 address) _REENTRANT_ ;	  		//return status
uint16 fs_get_allocated_space(void) _REENTRANT_ ;
#endif


#endif

#define _FS_H
#endif
