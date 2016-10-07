#include <rtthread.h>
#include <components.h>
#include <stdio.h>

#include "common.h"

#include "ppp_service.h"
#include "modem_serial.h"
#include "netif/ppp/ppp.h"
#include "sim900.h"
#include "lwip_raw_api_test.h"
#include "lwip_raw_api_tcp_template.h"
//#include "XMPP/xmpp.h"
#include "app_ev.h"
#include "xmpp_tcp.h"
#include "serial_srv.h"
#include "IOI2C.h"
#include "MMA845X.h"
#include "gnss.h"
#include "led.h"
#include "isp_http_tcp.h"
#include "app_timer.h"
#include "watch_dog.h"
#include "gsmmux/easyio_cmux.h"
#include "at_cmd.h"
#include "my_stdc_func/my_stdc_func.h"
#include "extdrv/gpio.h"
#include "flash_config.h"
#include "w25q16.h"
#include "spiflash_config.h"
#include "MD5/md5sum.h"

#include "../fw.h"

extern rt_err_t rt_thread_sleep(rt_tick_t tick);


typedef  void (*pFunction)(void);
static void JumpToApp(u32 appAddr)				
{
	pFunction JumpToApplication;
	u32 JumpAddress;
	
	//appAddr = CPU2_FW_ADDRESS;
	
	JumpAddress = *(u32*) (appAddr + 4);
	JumpToApplication = (pFunction)JumpAddress;
  /* Initialize user application's Stack Pointer */
  __set_MSP(*(vu32*) appAddr);
  JumpToApplication();
}

void mainloop(void *p)
{
	
	int i=0;
	int di =0;
	char str[32] = {0x0};
	md5_state_t state;
	md5_byte_t digest[16];
	char hex_output[16*2 + 8];
	
	
	int at_cmd_ret_code;
	
	__read_uconfig();
	

	rt_kprintf("@@@@@@@@@@@@@@@@@@@@@@@@ BUILD DATE %s TIME %s \r\n",__DATE__,__TIME__);
	
	if (uconfig.DISABLE_WATCHDOG != 1)
	{
		enable_wg();
	}
	
	SPI_Flash_Init();
	SPI_Flash_Init();
	
	/**
	enum {
	OTAFLAG_NORMAL = 0,
	OTAFLAG_READY_AUTO,
	OTAFLAG_READY_MANUAL,
	OTAFLAG_PROGING,
	OTAFALG_PROGFINISH,
	};
	*/
	

	
	switch(uconfig.ota_flag)
	{
		case OTAFLAG_READY_UPDATA:
		{
			static char md5out[40];
			check_down_zone_buffer_md5(md5out);
			
			if (strstr(md5out,uconfig.ota_target_fw_md5))
			{
				//匹配
				//
				int size;
				static unsigned char tmpbuffer[1024];
				
				rt_kprintf("BOOTLOADER DBG : Check FW MD5 PASS . \r\n");
				
				feed_watchdog(0);
				
				
				size = start_get_down_zone_spiflash();
				
				rt_kprintf("BOOTLOADER DBG :SPI Flash FW size %d . \r\n",size);
				
				if (size < 1024 * 100)
				{
					
					uconfig.ota_flag = OTAFALG_PROGERROR;
					__write_uconfig();
					rt_kprintf("BOOTLOADER DBG : FW SIZE ERROR < 100KB . \r\n");
					goto RUN_APP;
					
				}
				
				FLASH_ProgramStart(SYSTEM_APP_ADDRESS,SYSTEM_APP_SIZE);
				
				for(;;)
				{
					int l = get_down_zone_buffer(tmpbuffer,1024);
					if (l<=0)
						break;
					feed_watchdog(0);
					rt_kprintf("BOOTLOADER DBG :SPI Flash write size %d . \r\n",l);
					FLASH_AppendBuffer(tmpbuffer,l);
					
				}
				
				FLASH_AppendEnd();
				FLASH_ProgramDone();
				
				
				//校验写入的是否正确
				hex_output[0] = 0x0;
				md5_init(&state);
				md5_append(&state,(unsigned char*)SYSTEM_APP_ADDRESS,size);
				md5_finish(&state, digest);
				for (di = 0; di < 16; ++di)
				{
					char md5str[4];
					snprintf(md5str,sizeof(md5str),"%02x",digest[di]);
					strcat(str,md5str);
					strcat(hex_output,md5str);
				}
				
				if (strstr(hex_output,uconfig.ota_target_fw_md5))
				{
					uconfig.ota_flag = OTAFALG_PROGFINISH;
					__write_uconfig();
					//直接运行
					goto RUN_APP;
					
				}else{
					//仅仅是写入失败，所以，重新写
					goto RUN_APP;
					//RESET
				}
				
				
				
				
				
				
				
			}else
			{
				uconfig.ota_flag = OTAFALG_PROGERROR;
				__write_uconfig();
				rt_kprintf("BOOTLOADER DBG : Check FW MD5 ERROR . \r\n");
				goto RUN_APP;
				//不匹配
			}
			
			break;
		}
		
		default:
			break;
	}


	RUN_APP:
	feed_watchdog(0);
	JumpToApp(SYSTEM_APP_ADDRESS);
	
	return ;
}

