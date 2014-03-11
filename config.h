#ifndef _CONFIG__H

#define YGGDRASIL_OS_CODE			3
#define YGGDRASIL_KERNEL_VERSION	2 //version 2 (gsm specific), 0 prototype (not used), 1 general user application	
#define YGGDRASIL_FS_TYPE			0xA			//asgard file system 
#define ASGARD_VERSION				4

/* OS Configuration */
#define _YGGDRASIL_MICRO_KERNEL
#define MIDGARD_HEAP_SIZE			0x100

/* Authentication Support Configuration */
#define AUTH_USE_COMP128_1			1
#define AUTH_USE_COMP128_2			2
#define AUTH_USE_COMP128_3			3
#define AUTH_GSM_MODE				AUTH_USE_COMP128_1

#define LIQUID_ALLOCATED			1

/* Data Coding Scheme Configuration */
#define UNICODE_SUPPORT 			0
#define COMPACT_7BIT_SUPPORT 		1

/* DES Cryptography Configuration */
#define DES_OPERATION_FILE			1		//needed to decode command packet (long concatenated message)
#define DES_OPERATION_MEMORY		1		//needed to encode response packet

/* VAS Configuration */	
#define VAS_ALLOCATED				1
#define VAS_ACTIVATED				1

/* Redundancy Check Configuration */
#define CRC32_PROCESS_FILE			1		//needed for decode command packet
#define CRC32_PROCESS_MEMORY		1		//needed for encode response packet

/* SAT Processing */
#define SAT_MAX_RESPONSE_SIZE		192
#define SAT_USE_TEMP_FILE			0

/* SAT Configuration */
#define SAT_CONFIG_CELLTICK			0
#define SAT_CELLTICK_TIMER			0
#define SAT_MENU_VAS				4
#define SAT_MENU_LIQUID				3
//#define SAT_MENU_MODE				SAT_MENU_VAS		//SAT_MENU_VAS		//use VAS, else use Liquid


/* VAS Configuration */
#define IVAS_SUBMIT_ALLOCATED							1
#define IVAS_DISPLAY_TEXT_ALLOCATED						1
#define IVAS_GET_INPUT_ALLOCATED						1
#define IVAS_SELECT_ITEM_ALLOCATED						1
#define IVAS_SKIP_ALLOCATED								1
#define IVAS_EXIT_ALLOCATED								1
#define IVAS_PLUGIN_ALLOCATED							1
#define IVAS_SET_ALLOCATED								1
#define IVAS_PROVIDE_LOCAL_INFORMATION_ALLOCATED		1
#define IVAS_PLAY_TONE_ALLOCATED						0
#define IVAS_SETUP_IDLE_MODE_TEXT_ALLOCATED				0
#define IVAS_REFRESH_ALLOCATED							1
#define IVAS_SETUP_CALL_ALLOCATED						0
#define IVAS_NEW_CONTEXT_ALLOCATED						1
#define IVAS_SET_RETURN_TAR_VALUE_ALLOCATED				1
#define IVAS_SEND_USSD_ALLOCATED						1
#define IVAS_SEND_SM_ALLOCATED							1
#define IVAS_SET_EXTENDED_ALLOCATED						1
#define IVAS_BRANCH_ON_VAR_VALUE_ALLOCATED				1
#define IVAS_LAUNCH_BROWSER_ALLOCATED					0
#define IVAS_CHECK_TERMINAL_PROFILE_ALLOCATED			0
#define IVAS_SUBSTRING_ALLOCATED						1
#define IVAS_DISPLAY_TEXT_CLEAR_AFTER_DELAY_ALLOCATED	1
#define IVAS_EXECUTE_LOCAL_SCRIPT_ALLOCATED				1
#define IVAS_SUBMIT_EXTENDED_ALLOCATED					1
#define IVAS_LAUNCH_BROWSER_EXTENDED_ALLOCATED			1
#define IVAS_ADD_SUBSTRACT_ALLOCATED					0
#define IVAS_CONVERT_VARIABLE_ALLOCATED					1
#define IVAS_GROUP_UNGROUP_VARIABLE_ALLOCATED			0
#define IVAS_SETUP_CALL_EXTENDED_ALLOCATED				0
#define IVAS_DISPLAY_TEXT_EXTENDED_ALLOCATED			1
#define IVAS_SETUP_IDLE_MODE_TEXT_EXTENDED_ALLOCATED	0
#define IVAS_SEND_SM_EXTENDED_ALLOCATED					1
#define IVAS_SWAP_NIBBLES_ALLOCATED						0
#define IVAS_TIMER_MANAGEMENT_ALLOCATED					0
/* VAS Plug-ins Configuration */
#define VAS_DIL_ALLOCATED			0
#define VAS_VDIL_ALLOCATED			VAS_DIL_ALLOCATED
#define VAS_MDIL_ALLOCATED			VAS_DIL_ALLOCATED
#define VAS_DITR_ALLOCATED			0
#define VAS_MASL_ALLOCATED			0 
#define VAS_ENCR_DECR_ALLOCATED		0
#define VAS_DECR_ALLOCATED			VAS_ENCR_DECR_ALLOCATED
#define VAS_ENCR_ALLOCATED			VAS_ENCR_DECR_ALLOCATED
#define VAS_ICCID_ALLOCATED			0


/* ATR Configuration */
//BYTEC	ATR[]= {0x3B, 0x1A, 0x95, 0x00, 0x17, 0xBD, 0x10, 0x59, 0x43, 0x4F, 0x53, 0x31, 0x00}; 
					
#define ATR_PPS						0x95
#define ATR_CHIP_ID					0x17
#define ATR_CHIP_TYPE				0xBD
#define ATR_CHIP_VER				0x10
#define ATR_OS_NAME					'Y'
#define ATR_OS_YEAR					'C'
#define ATR_OS_CUSTOMER				'O'
#define ATR_OS_REV					'S'
#define ATR_OS_VER					((YGGDRASIL_OS_CODE << 4) | YGGDRASIL_KERNEL_VERSION)
#define ATR_FS_VER					((YGGDRASIL_FS_TYPE << 4) | ASGARD_VERSION)
#define ATR_APP_FRAMEWORKS			(VAS_ALLOCATED << 5) | (LIQUID_ALLOCATED << 4) | AUTH_GSM_MODE
#define ATR_WIB_PLUGINS				(VAS_ENCR_DECR_ALLOCATED << 0) | (VAS_DITR_ALLOCATED << 1) | (VAS_MASL_ALLOCATED << 2) | (VAS_DITR_ALLOCATED << 3) | (VAS_ICCID_ALLOCATED << 4) | (VAS_DIL_ALLOCATED << 7)

#if VAS_ALLOCATED
#define ATR_CHECKSUM				(ATR_OS_NAME + ATR_OS_YEAR + ATR_OS_CUSTOMER + ATR_OS_REV) + ATR_PPS + ATR_CHIP_ID + ATR_CHIP_TYPE	+ ATR_CHIP_VER + ATR_OS_VER + ATR_FS_VER + ATR_APP_FRAMEWORKS + ATR_WIB_PLUGINS
#define ATR_LENGTH					0x1E
#else
#define ATR_CHECKSUM				(ATR_OS_NAME + ATR_OS_YEAR + ATR_OS_CUSTOMER + ATR_OS_REV) + ATR_PPS + ATR_CHIP_ID + ATR_CHIP_TYPE	+ ATR_CHIP_VER + ATR_OS_VER + ATR_FS_VER + ATR_APP_FRAMEWORKS
#define ATR_LENGTH					0x1D
#endif

#define _CONFIG__H
#endif