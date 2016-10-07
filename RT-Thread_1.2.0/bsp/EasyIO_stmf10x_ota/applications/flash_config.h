#ifndef __flash_config__
#define __flash_config__

#include "flash_opt.h"

enum {
	OTAFLAG_NORMAL = 0,
	OTAFLAG_READY_UPDATA,
	OTAFLAG_READY_AUTO,
	OTAFLAG_READY_MANUAL,
	OTAFLAG_PROGING,
	OTAFALG_PROGFINISH,
	OTAFALG_PROGERROR,
};

struct SIMCARD_APN {
	char CCID[28];
	char APN[32];
	char USERNAME[32];
	char PASSWORD[32];
};

//最大不能超过1K
struct USER_CONFIG {
	
	char 			DISABLE_WATCHDOG;
	
	//串口信息
	short			serialbaud;				//串口波特率
	char			presence_str[64];			//presence字符串
	//tcp 信息
	char			tcp_host[64];
	short			tcp_port;
	
	//openfire 服务器信息
	char			openfire_hostname[64];
	char			openfire_domain[16];
	short			openfire_port;
	
	char			openfire_username[32];
	char			openfire_password[32];
	
	//APNINFO
	struct SIMCARD_APN	APNINFO;
	
	//OTA
	char			ota_flag;
  unsigned int ota_newfw_address;
	unsigned int short ota_newfw_size;
	char			ota_target_fw_md5[32+8];

	char bootloader_ver_a;
	char bootloader_ver_b;
};

extern struct USER_CONFIG uconfig;

#define INIT_USER_CONFIG memset(&uconfig,0x0,sizeof(uconfig));

extern void __read_uconfig(void);
extern void __write_uconfig(void);


#define READ_USER_CONFIG			__read_uconfig()
#define WRITE_USER_CONFIG			__write_uconfig()

#endif