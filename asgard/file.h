#include "..\defs.h"
#ifndef _FILE_H
#include "fs.h"

//CHANGELOG : 
/*
(before) 4 july 2010:
 * dilengkapi dengan keamanan akses chv1, chv2...adm(n)
 * linked list type file system, space eeprom lebih dihemat dengan pemakaian cluster, mampu meminimalisir space yang kosong
 * ukuran per-cluster adalah 32 byte
 * mendukung ef_key (belum diimplementasikan)
 * mendukung invalidate, rehabilitate file
 * format standar header dengan TLV(type-length/size-value) encoding, untuk direktori size/length = 0
 * fs.c dilengkapi kemampuan untuk menghitung sisa cluster yang kosong
 * ukuran maksimum eeprom yang mampu diakses oleh file system(32 byte cluster) ±128KB(sudah termasuk allocation table, chv file, dst)
   tapi ukuran yang teralokasi maksimum hanya ((508 - 40) x 8) x 32 byte 
 * kemampuan pengecekan akses untuk operasi pembacaan dan penulisan file baik record maupun transparent

5 July 2010 : 
 * Implement CRC for record and transparent data, data_size reduced from 30 to 28, cluster_size still 32 byte ^^V
 * _select modified	(improved)
 * _remove mengubah nilai number_of_ef dan number_of_df dari parent
 * pin dan puk dienkripsi dengan des (password defineds), library des hanya mendukung encrypt saja	
 * header belum dilengkapi deteksi CRC <-- IMPORTANT -->
 * header dilengkapi kemampuan crc, tinggal di file_get_header dilengkapi kemampuan deteksi crc	

6 July 2010 :
 * dicoba untuk menggunakan macro file_write_cluster dan file_read_cluster, tapi file_get_header malah error
 * macro check_crc digunakan setelah file_get_header dipanggil pertama kali untuk memastikan data tersebut valid
 * untuk sementara pemakaian memori masih boros karena ada pointer yang diinisialisasi tapi tdk digunakan 

13 June 2012
 * Asgard 3.0 Commencing
26 June 2012
 * use as_get_current_directory, as_set_current_file, as_get_current_file for getting current directory/file and set current file instead of as_push, as_peek
   for different asgard_handle implementation (either with stack or not)
*/


#define ASGARD30PORT20			1
#if ASGARD_VERSION == 3
//Asgard 3.0
//since version 3, Asgard use binary file abstraction provided by file system in order to access low level specific
//datatype in plain binary format, EF or DF information provided by file.h using the specified metadata on file system level
//access directly to low level is restricted without the use of binary file abstraction, this way each file configuration
//can be managed in much more elegant manner without changing internal file system format/allocation.
#define EF_TRANSPARENT			0
#define EF_LINFIX				1			//(bit 0, 0 = fixed, 1=variable)
#define EF_CYCLIC				3

#define EF_VALID				1

#define FILE_EOR				0xffff		//end of record
#define FILE_EOC				0xff		//end of alloc table

struct ef_bin_header {
	uchar structure;			//1byte		(transparent)
	uchar access_rw;			//1byte
	uchar access_inc;			//1byte
	uchar access_ri;			//1byte
	uchar status;				//1byte 	invalidate	/	validate
	uint16 size;				//2byte		virtual file size
};

struct ef_rec_header {
	uchar structure;			//1byte		(cyclic, linier fixed, linier variable)
	uchar access_rw;			//1byte
	uchar access_inc;			//1byte
	uchar access_ri;			//1byte
	uchar status;				//1byte 	invalidate	/	validate
	uchar num_of_records;		//1byte		total record
	uchar rec_size;				//1byte		record size for linfix
	uchar first;				//1byte 	index of the first record, use index for faster file access
};

typedef struct ef_bin_header ef_bin_header;
typedef struct ef_rec_header ef_rec_header;

#define MAX_STACK_HANDLE		8
struct asgard_handle {					//provide handle to upper layer application, allow each upper application to access file system simultaneously
	fs_handle * fs;						//current file system of asgard
	#if USE_ENTRY_STACK
	fs_file * stack[MAX_STACK_HANDLE];	//fs_file stack, contain any directory/file information used by file system
	uchar stack_index;
	#else
	fs_file * cur_dir;
	fs_file * cur_file;
	#endif
	uchar mode;							//current file system mode (USER|SYS)
	uchar cur_rec_index;				//current record index, for use with read_rec_next and read_rec_prev
	uchar num_of_df;					//number of df on parent directory
	uchar num_of_ef;					//number of ef on parent directory
};

typedef struct asgard_handle asgard_handle;

struct ef_record {
	uchar index;						//current record index
	uchar next;							//next record index
};

typedef struct ef_record ef_record;

#elsif ASGARD_VERSION == 2
//Asgard 2.0 
struct ef_body{
	uchar data[DATA_SIZE];		//32byte
};
typedef struct ef_body ef_body;

struct ef_header{
	uchar type;					//1byte
	uint16 size;				//2byte
	uint16 FID;					//2byte
	uchar structure;			//1byte
	uchar status;				//1byte 	invalidate	/	validate
	uchar access_rw;			//1byte
	uchar access_inc;			//1byte
	uchar access_ri;			//1byte
	uchar record_size;			//1byte
	uchar padding[9];			//9byte
	uint16 ef_key;				//2byte
	uint16 parent;				//2byte
	uint16 sibling;				//2byte
	uint16 child;				//2byte
	uint16 next;				//2byte
};

struct df_header{
	uchar type;					//1byte
	uint16 size;				//2byte
	uint16 FID;					//2byte
	uchar name[16];				//16byte
	uchar name_length;			//1byte
	uint16 parent;				//2byte
	uint16 sibling;				//2byte
	uint16 child;				//2byte
	uchar number_of_df;			//1byte
	uchar number_of_ef;			//1byte
};

typedef struct ef_header ef_header;
typedef struct df_header df_header;
#endif


#define file_write_cluster(x, no) { \
			char * stream = (char *)x;	\
			uint16 i; \
			ef_body *crcfile = (ef_body *)stream;\
			crcfile->crc = crcFast(stream, CRC_SIZE); \
			ioman_seek((no * CLUSTER_SIZE) + ALLOCATION_DATA_OFFSET); \
			for(i=0;i<CLUSTER_SIZE;i++) { \
				ioman_write(*(stream+i)); \
			} \
		}

#define file_read_cluster(x, cluster_no) { \
			char * str = x;	\
			uint16 i; \
			ef_body *crcfile = (ef_body *)str;\
			if(crcfile->crc == crcFast(crcfile->data, CRC_SIZE)) { \
				ioman_seek((cluster_no * CLUSTER_SIZE) + ALLOCATION_DATA_OFFSET); \
				for(i=0;i<CLUSTER_SIZE;i++) { \
					*(str++) = ioman_read(); \
				} \
			} \
		}

#define check_crc(x) { \
			ef_header *h = (ef_header *)x; \
			if(h->crc!=crcFast((uchar *)h, CRC_SIZE)) { \
				free(h); \
				return APDU_CRC_ERROR; \
			}}

//global access variable
//extern uint16 cur_ptr;
//extern uint16 efk_ptr;
//extern uint16 cur_rcd;
#if ASGARD_VERSION == 1
//header(lama) untuk asgard 1.0
uint16 file_get_last_child(uint16 parent);
void file_add(uint16 prvnode, uint16 sibling, uint16 parent);
ef_header * file_get_header(uint16 cluster_no);
ef_header * file_get_current_header();
uint16 file_create(char *id, uchar type, uchar structure, uint16 parent);
uint16 file_write_bin(ef_header * header, uint16 offset, char *buffer, uint16 size);
uint16 file_read_bin(ef_header * header, uint16 offset, char *buffer, uint16 size);
uint16 file_seek(uint16 file_id);	//mencari seluruh direktori tree sampai ditemukan file dengan id yang bersesuaian
uchar file_remove(char *file_id);
void file_dirlist();
void file_dirlist_nd();
#elsif ASGARD_VERSION == 2
//untuk SCOS (yggdrasil micro kernel), Asgard 2.0
uint16 _select(uint16 fid);
uint16 _readbin(uint16 offset, uchar *buffer, uint16 size);
uint16 _readrec(uint16 rec_no, uchar *buffer, uchar size);
uint16 _readrec_next(uchar *buffer, uchar size);
uint16 _readrec_prev(uchar *buffer, uchar size);
uint16 _writebin(uint16 offset, uchar *buffer, uint16 size);
uint16 _write_new_rec(uint16 rec_no, uchar size, uchar *buffer);
uint16 _writerec(uint16 rec_no, uchar *buffer, uchar size);
uint16 _writerec_next(uchar *buffer, uchar size);
uint16 _writerec_prev(uchar *buffer, uchar size);
uint16 _createfilebin(uint16 fid, uchar ACC_READ, uchar ACC_WRT, uchar ACC_INC, uchar ACC_INV, uchar ACC_RHB, uchar *buffer, uint16 size);
uint16 _createfilerec(uint16 fid, uchar ACC_READ, uchar ACC_WRT, uchar ACC_INC, uchar ACC_INV, uchar ACC_RHB, uchar total_rec, uchar rec_size);
uint16 _createfilecyclic(uint16 fid, uchar ACC_READ, uchar ACC_WRT, uchar ACC_INC, uchar ACC_INV, uchar ACC_RHB, uchar total_rec, uchar rec_size);
uint16 _createdirectory(uint16 fid, uchar *dirname, uchar len_name);
uint16 _remove(uint16 file_id);
uint16 _invalidate();
uint16 _rehabilitate();

#endif
#if ASGARD_VERSION == 4

#define FILE_READ			1
#define FILE_WRITE			2
#define FILE_INCREASE		4
#define FILE_INVALIDATE		6
#define FILE_REHABILITATE	8

struct fs_handle {
#if FS_ADDRESS_WIDTH == AW_32BIT
	uint32 cur_ptr;
	uint32 cur_dir;
	int16 cur_wr_rec;
	int16 cur_rd_rec;
#else
	uint16 cur_ptr;
	uint16 cur_dir;
	int16 cur_wr_rec;
	int16 cur_rd_rec;
#endif
};

typedef struct fs_handle fs_handle;

void * file_get_current_header(fs_handle * handle) _REENTRANT_;

uint16 _select(fs_handle * handle, uint16 fid) _REENTRANT_;
uint16 _readbin(fs_handle * handle, uint16 offset, uchar *buffer, uint16 size) _REENTRANT_;
uint16 _readrec(fs_handle * handle, uint16 rec_no, uchar *buffer, uchar size) _REENTRANT_;
uint16 _readrec_next(fs_handle * handle, uchar *buffer, uchar size) _REENTRANT_;
uint16 _readrec_prev(fs_handle * handle, uchar *buffer, uchar size) _REENTRANT_;
uint16 _writebin(fs_handle * handle, uint16 offset, uchar *buffer, uint16 size) _REENTRANT_;
uint16 _writerec(fs_handle * handle, uint16 rec_no, uchar *buffer, uchar size) _REENTRANT_;
uint16 _writerec_next(fs_handle * handle, uchar *buffer, uchar size) _REENTRANT_;
uint16 _writerec_prev(fs_handle * handle, uchar *buffer, uchar size) _REENTRANT_;
uint16 _seek(fs_handle * handle, uchar type, uchar * buffer, uchar * pattern, uchar size) _REENTRANT_;
uint16 _increase(fs_handle * handle, uint32 ival, uchar * res, uchar size) _REENTRANT_;
uint16 _createfilebin(fs_handle * handle, uint16 fid, uchar acc_rw, uchar acc_io, acc_ri, uint16 size) _REENTRANT_;
uint16 _createfilerec(fs_handle * handle, uint16 fid, uchar acc_rw, uchar acc_io, uchar acc_ri, uchar total_rec, uchar rec_size) _REENTRANT_;
uint16 _createfilecyclic(fs_handle * handle, uint16 fid, uchar acc_rw, uchar acc_io, uchar acc_ri, uchar total_rec, uchar rec_size) _REENTRANT_;
uint16 _createdirectory(fs_handle * handle, uint16 fid, uchar filechar) _REENTRANT_;
uint16 _remove(fs_handle * handle, uint16 file_id) _REENTRANT_ ;
uint16 _invalidate(fs_handle * handle) _REENTRANT_;
uint16 _rehabilitate(fs_handle * handle) _REENTRANT_;
uint16 _check_access(fs_handle * handle, uchar mode) _REENTRANT_ ;

#define file_select _select
#define file_readbin _readbin
#define file_readrec _readrec
#define file_readrec_next _readrec_next
#define file_readrec_prev _readrec_prev
#define file_writebin _writebin
#define file_writerec _writerec
#define file_writerec_next _writerec_next
#define file_writerec_prev _writerec_prev
#define file_seek _seek
#define file_createfilebin _createfilebin
#define file_createfilerec _createfilerec
#define file_createfilecyclic _createfilecyclic
#define file_createdirectory _createdirectory
#define file_remove _remove
#define file_invalidate _invalidate
#define file_rehabilitate _rehabilitate
#define file_check_access _check_access

#define AS_SELECT		_select
#define AS_READBIN		_readbin
#define AS_WRITEBIN		_writebin
#define AS_READREC		_readrec
#define AS_WRITEREC		_writerec


#endif
#if ASGARD_VERSION == 3
//Asgard 3.0
/* internal asgard operation */
#if USE_ENTRY_STACK
void as_push(asgard_handle * handle, fs_file * file);
fs_file * as_pop(asgard_handle * handle);
fs_file * as_peek(asgard_handle * handle);
fs_file * as_peek_parent(asgard_handle * handle);
void as_flush(asgard_handle * handle);
#else

#endif
fs_file * as_get_current_directory(asgard_handle * handle);
void as_set_current_file(asgard_handle * handle, fs_file * file);
fs_file * as_get_current_file(asgard_handle * handle);
asgard_handle * as_create_handle(fs_handle * fs, uchar mode);
/* external asgard operation */
uint16 as_select(asgard_handle * handle, uint16 fid);
uint16 as_read_binary(asgard_handle * handle, uint16 offset, uchar * buffer, uchar size);
uint16 as_write_binary(asgard_handle * handle, uint16 offset, uchar * buffer, uchar size);
uint16 as_read_record(asgard_handle * handle, uint16 rec_no, uchar *buffer, uchar size);
uint16 as_read_record_next(asgard_handle * handle, uchar *buffer, uchar size);
uint16 as_read_record_prev(asgard_handle * handle, uchar *buffer, uchar size);
uint16 as_write_record(asgard_handle * handle, uint16 rec_no, uchar *buffer, uchar size);
uint16 as_write_record_next(asgard_handle * handle, uchar *buffer, uchar size);
uint16 as_write_record_prev(asgard_handle * handle, uchar *buffer, uchar size);
uint16 as_create_binary(asgard_handle * handle, uint16 fid, uchar ACC_READ, uchar ACC_WRT, uchar ACC_INC, uchar ACC_INV, uchar ACC_RHB, uchar * buffer, uint16 size);
uint16 as_create_record(asgard_handle * handle, uint16 fid, uchar ACC_READ, uchar ACC_WRT, uchar ACC_INC, uchar ACC_INV, uchar ACC_RHB, uchar total_rec, uchar rec_size);
uint16 as_create_cyclic(asgard_handle * handle, uint16 fid, uchar ACC_READ, uchar ACC_WRT, uchar ACC_INC, uchar ACC_INV, uchar ACC_RHB, uchar total_rec, uchar rec_size);
uint16 as_create_directory(asgard_handle * handle, uint16 fid);
uint16 as_remove(asgard_handle * handle);
uint16 as_invalidate(asgard_handle * handle);
uint16 as_rehabilitate(asgard_handle * handle);

#if ASGARD30PORT20
//ported version of asgard 30 to asgard 20
//asgard_handle * g_as_handle;
//uint16 _select(uint16 fid);
#define _select(fid) (as_select(g_as_handle, fid)) 
//uint16 _readbin(uint16 offset, uchar *buffer, uint16 size);
#define _readbin(offset, buffer, size) as_read_binary(g_as_handle, offset, buffer, size)
//uint16 _readrec(uint16 rec_no, uchar *buffer, uchar size);
#define _readrec(rec_no, buffer, size) as_read_record(g_as_handle, rec_no, buffer, size)
//uint16 _readrec_next(uchar *buffer, uchar size);
#define _readrec_next(buffer, size) as_read_record_next(g_as_handle, buffer, size)
//uint16 _readrec_prev(uchar *buffer, uchar size);
#define _readrec_prev(buffer, size) as_read_record_prev(g_as_handle, buffer, size)
//uint16 _writebin(uint16 offset, uchar *buffer, uint16 size);
#define _writebin(offset, buffer, size) as_write_binary(g_as_handle, offset, buffer, size)
//uint16 _write_new_rec(uint16 rec_no, uchar size, uchar *buffer);
//uint16 _writerec(uint16 rec_no, uchar *buffer, uchar size);
#define _writerec(rec_no, buffer, size) as_write_record(g_as_handle, rec_no, buffer, size)
//uint16 _writerec_next(uchar *buffer, uchar size);
#define _writerec_next(buffer, size) as_write_record_next(g_as_handle, buffer, size)
//uint16 _writerec_prev(uchar *buffer, uchar size);
#define _writerec_prev(buffer, size) as_write_record_prev(g_as_handle, buffer, size)
//uint16 _createfilebin(uint16 fid, uchar ACC_READ, uchar ACC_WRT, uchar ACC_INC, uchar ACC_INV, uchar ACC_RHB, uchar *buffer, uint16 size);
#define _createfilebin(fid, acc_read, acc_wrt, acc_inc, acc_inv, acc_rhb, buffer, size) as_create_binary(g_as_handle, fid, acc_read, acc_wrt, acc_inc, acc_inv, acc_rhb, buffer, size)
//uint16 _createfilerec(uint16 fid, uchar ACC_READ, uchar ACC_WRT, uchar ACC_INC, uchar ACC_INV, uchar ACC_RHB, uchar total_rec, uchar rec_size);
#define _createfilerec(fid, acc_read, acc_wrt, acc_inc, acc_inv, acc_rhb, total_rec, rec_size) as_create_record(g_as_handle, fid, acc_read, acc_wrt, acc_inc, acc_inv, acc_rhb, total_rec, rec_size)
//uint16 _createfilecyclic(uint16 fid, uchar ACC_READ, uchar ACC_WRT, uchar ACC_INC, uchar ACC_INV, uchar ACC_RHB, uchar total_rec, uchar rec_size);
#define _createfilecyclic(fid, acc_read, acc_wrt, acc_inc, acc_inv, acc_rhb, total_rec, rec_size) as_create_cyclic(g_as_handle, fid, acc_read, acc_wrt, acc_inc, acc_inv, acc_rhb, total_rec, rec_size)
//uint16 _createdirectory(uint16 fid, uchar *dirname, uchar len_name);
#define _createdirectory(fid, dirname, len) as_create_directory(g_as_handle, fid)
//uint16 _remove(uint16 file_id);
#define _remove(fid) as_remove(g_as_handle)
//uint16 _invalidate();
#define _invalidate() as_invalidate(g_as_handle)
//uint16 _rehabilitate();
#define _rehabilitate() as_rehabilitate(g_as_handle)
#endif

#endif

#define _FILE_H
#endif

