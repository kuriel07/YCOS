#ifndef _DEFS_DEFINED
#include "..\defs.h"
#endif
#ifndef _CONFIG__H
#include "..\config.h"
#endif	  
#ifndef __LIQUID_H
#include "..\liquid.h"
#endif	
#ifndef __VAS_H
//#include "..\..\YGGSYS\yggapis.h"
//#include "..\liquid.h"

#define FID_WIB				0x2700
#define FID_WTAR			0x6F1A
#define	FID_WERRORTEXT		0x6F02
#define FID_WBYTECODE		0x6F03
#define FID_WSMSHEADER		0x6F04
#define FID_WSC				0x6F1B
#define FID_W0348CNTR		0x6F06
#define FID_WVERSION		0x6F07
#define FID_WIBCONF			0x6F08
#define FID_WEVTCONF		0x6F0B
#define FID_WTEXT			0x6F1C
#define FID_WMENU			0x6F18
#define FID_WSCRADDR		0x6F1D
#define FID_WMENUTITLE		0x6F1E
#define FID_WSMSHDRLIST	  	0x6FB0

#define VAS_SUBMIT			0x01
#define VAS_DISPLAY_TEXT	0x02
#define VAS_GET_INPUT		0x03
#define VAS_SELECT_ITEM		0x04
#define VAS_SKIP			0x05
#define VAS_EXIT			0x06
#define VAS_PLUGIN			0x07
#define VAS_SET				0x08
#define VAS_PROVIDE_LOCAL_INFORMATION	0x09
#define VAS_PLAY_TONE		0x0A
#define VAS_SETUP_IDLE_MODE_TEXT		0x0B
#define VAS_REFRESH			0x0C
#define VAS_SETUP_CALL		0x0D
#define VAS_NEW_CONTEXT		0x10
#define VAS_SET_RETURN_TAR_VALUE		0x011
#define VAS_SEND_USSD		0x12
#define VAS_SEND_SM			0x13
#define VAS_SET_EXTENDED	0x14
#define VAS_BRANCH_ON_VAR_VALUE		0x15
#define VAS_LAUNCH_BROWSER	0x16
#define VAS_CHECK_TERMINAL_PROFILE	0x17
#define VAS_SUBSTRING		0x18
#define VAS_DISPLAY_TEXT_CLEAR_AFTER_DELAY		0x19
#define VAS_EXECUTE_LOCAL_SCRIPT		0x1A
#define VAS_SUBMIT_EXTENDED		0x1B
#define VAS_LAUNCH_BROWSER_EXTENDED		0x1C
#define VAS_ADD_SUBSTRACT			0x1D
#define VAS_CONVERT_VARIABLE		0x1E
#define VAS_GROUP_UNGROUP_VARIABLE	0x1F
#define VAS_SETUP_CALL_EXTENDED		0x20
#define VAS_DISPLAY_TEXT_EXTENDED	0x21
#define VAS_SETUP_IDLE_MODE_TEXT_EXTENDED		0x22
#define VAS_SEND_SM_EXTENDED		0x23
#define VAS_SWAP_NIBBLES			0x24
#define VAS_TIMER_MANAGEMENT		0x28
/* Administrative Commands */
#define VAS_INSTALL_PLUGIN				0x7E
#define VAS_REMOVE_PLUGIN				0x7D
#define VAS_SET_SCRIPT_TRIGGER_MODE		0x7B
#define VAS_GET_SCRIPT_TRIGGER_MODE		0x7A
#define VAS_GET_MENU					0x79
#define VAS_SCRIPT_INFO					0x78

#define VAS_TAR_PULL					0
#define VAS_TAR_PUSH					1
#define VAS_TAR_ADMIN					2

#define VAS_LOAD_KIC					4
#define VAS_LOAD_KID					8

//vas fetch mode
#define VAS_MODE_SCRIPT					0x00
#define VAS_MODE_PLUGIN					0x80

//vas plugin select
#define VAS_PLUGIN_MDIL					5
#define VAS_PLUGIN_VDIL					7
#define VAS_PLUGIN_DITR					9
#define VAS_PLUGIN_ENCR					11
#define VAS_PLUGIN_DECR					13
#define VAS_PLUGIN_MASL					15	
#define VAS_PLUGIN_ICCID				17

//low nibble
#define VAS_STATE_WAIT_RESPONSE			0x07
#define VAS_STATE_NORMAL				0x04
#define VAS_STATE_MENU					0x01
//high nibble
#define VAS_STATE_WAIT_VARIABLE			0x80
#define VAS_STATE_VAR_TEXT				0x10
#define VAS_STATE_VAR_LOCALINFO			0x20

#define VAS_SELECT_MENU					1
#define VAS_SELECT_EVENT				2
#define VAS_SELECT_PLUGIN				3

#define VAS_SC_OCNTR					0
#define VAS_SC_ICNTR					1
#define VAS_SC_OSPI						2
#define VAS_SC_ISPI						3
#define VAS_SC_KIC						4
#define VAS_SC_KID						5

//replay checking mode
#define VAS_CNTR_X_HIGHER 				0x02
#define VAS_CNTR_1_HIGHER				0x03

#define VAS_STARTUP_EVENT				0x80
 
struct vas_config {
  	//uint16 address;
	uchar type;
	uchar sc_offset;
	uchar ocntr_index;
	uchar icntr_index;
	//uchar key[16];
};

struct vas_variable_header {
	struct vas_variable_header * next;	
	uchar id;
	uchar length;
};

struct vas_variable {  
	struct vas_variable * next;
	uchar id; 
	uchar length;
	uchar buffer[256];
};

struct vas_constant {
	uchar id;
	uchar length;
	struct vas_constant * next;
};

struct vas_sms_header {
	uchar da_address[12];
	uchar sc_address[12];
	uchar pid;
	uchar dcs;
	uchar vp;
};

typedef struct vas_variable vas_variable;
typedef struct vas_variable_header vas_variable_header;
typedef struct vas_constant vas_constant;
typedef struct vas_config vas_config;  							//>>>>--> VAS setup file configuration 
typedef struct vas_sms_header vas_sms_header;

extern uchar vas_state;
extern uchar vas_tar_mode;	
extern uchar vas_mode;	
extern vas_variable * _var_list;
extern vas_variable * _const_list;

uchar VAS_activated(void) _REENTRANT_ ;
/* VAS APIs */
#if VAS_ALLOCATED
uint16 VAS_startup(void) _REENTRANT_ ;
uint16 VAS_event(void) _REENTRANT_ ;
uint16 VAS_menu(void) _REENTRANT_ ;
vas_config * VAS_preprocess(command_packet * cmpkt) _REENTRANT_ ;
vas_variable * set_variable(vas_variable ** list, uchar id, uchar length, uchar * buffer) _REENTRANT_ ;
#define VAS_set_variable(id, len, buffer) set_variable(&_var_list, id, len, buffer)

uchar VAS_set_tar(uchar * tar) _REENTRANT_ ;
uchar VAS_loadkey(vas_config * config, uchar mode, uchar index, uchar * buffer) _REENTRANT_ ;
uchar VAS_loadcntr(vas_config * config, uchar index, uchar * buffer) _REENTRANT_ ; 
uchar VAS_load_text(uint16 fid, uchar id, uchar * buffer, uchar max_len) _REENTRANT_ ;
uchar VAS_invoke(uchar mode, uchar id) _REENTRANT_ ;
uchar VAS_get_security_config(uchar * tar, uchar mode, uchar index, uchar * buffer_out) _REENTRANT_ ;
uchar VAS_replay_check(uchar mode, uchar * dcntr, uchar * scntr) _REENTRANT_ ;
uchar VAS_strcmp(uchar * a, uchar * b, uchar len) _REENTRANT_ ;

uchar VAS_plugin_init(uint16 address) _REENTRANT_ ;
void VAS_exit_plugin(void);
uchar VAS_init(fs_handle * handle, uint16 address, uint16 size) _REENTRANT_ ; 
uchar VAS_file_pop(uint16 i, uchar * tag, uchar * size, uchar * value) _REENTRANT_ ;
uchar VAS_file_push(uint16 i, uchar tag, uchar length, uchar * value) _REENTRANT_ ;
uchar VAS_push_header(uchar offset, uchar cmd, uchar qualifier, uchar target) _REENTRANT_ ;
uint16 VAS_script_fetch(uchar * response, uint16 length) _REENTRANT_ ;
uint16 VAS_fetch(uchar * response, uint16 length) _REENTRANT_ ;
uint16 VAS_execute(uchar * response) _REENTRANT_ ;
void VAS_decode_plugin(uchar * buffer, uchar size) _REENTRANT_ ;
uint16 VAS_decode(void) _REENTRANT_ ;		/* Value Added Service Decoder (WIB)*/
#endif
#define __VAS_H
#endif