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
 * format standar header dengan type-length/size-value encoding, untuk direktori size/length = 0
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

//#if ASGARD_VERSION == 4

#define FILE_READ			1
#define FILE_WRITE			2
#define FILE_INCREASE		4
#define FILE_INVALIDATE		6
#define FILE_REHABILITATE	8

void * file_get_current_header(void);

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

//#endif

#define _FILE_H
#endif

