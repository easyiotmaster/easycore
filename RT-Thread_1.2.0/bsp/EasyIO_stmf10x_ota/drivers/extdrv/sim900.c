#include "modem.h"
#include "at_cmd.h"
#include "modem_serial.h"
#include "common.h"

#include <rtthread.h>
#include <stm32F10x.h>
#include <stdio.h>
#include <string.h>

#include "app_timer.h"
#include "flash_config.h"
#include "my_stdc_func/debugl.h"



#define SET_PWR GPIO_SetBits(GPIOC,GPIO_Pin_8);
#define RESET_PWR GPIO_ResetBits(GPIOC,GPIO_Pin_8);

#define SET_RST
#define RESET_RST

static struct rt_semaphore dtr_sem;

static void config_cts_interrupt(void);

static void init_sim900_ctrl_gpio(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	
	
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	

	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin   = (GPIO_Pin_9);
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
}


static void power_sim900(void)
{
	GPIO_ResetBits(GPIOC,GPIO_Pin_8);
	rt_thread_sleep(100*3);
	GPIO_SetBits(GPIOC,GPIO_Pin_8);

}

static void reset_sim900(void)
{
	
}
void enable_dtr(void)
{

}

void disable_dtr(void)
{
	
	
	//
}



static void powerModem(void)
{
	//
}


static void initModem2(void)
{
	unsigned char __UEXTDCONF_FLAG	= 0;
	unsigned char __USPM						= 0;
	unsigned char __USGC						= 0;
	unsigned char __USGC2						= 0;
	unsigned char __ATD							= 1;
	//AT+USGC?
	
	int i=0;
	int at_cmd_ret_code;
	unsigned char atcmd_cnt;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	DEBUGL->debug("reset_sim900....\r\n");
	reset_sim900();
	power_sim900();
	
	
	repower:
	
	
	
	
	for(i=0;i<1;i++)
	{
		at_command("AT\r\n",AT_AT,100,&at_cmd_ret_code);
		if (at_cmd_ret_code == AT_RESP_OK)
			return ;
	}
	
	
	
	for(i=0;i<1;i++)
	{
		at_command("AT\r\n",AT_AT,100,&at_cmd_ret_code);
		if (at_cmd_ret_code == AT_RESP_OK)
			return ;
	}
	
	DEBUGL->debug("power_sim900....\r\n");
	power_sim900();
	goto repower;
}


void init_modem_gpio(void)
{
	
	//rt_sem_init(&dtr_sem, "dtr_sem", 0, 0);
	init_sim900_ctrl_gpio();
	//
}


static int recv_cmux_buf(const char *resp , int len)
{
	return 1;
	//
}

static unsigned char setting_modem_status = 0;
static int setting_modem(void)
{
	int at_cmd_ret_code;
	switch(setting_modem_status)
	{
		case 0:
		{
			at_command("AT+FLO=0\r\n",AT_AT,200,&at_cmd_ret_code);
			if (at_cmd_ret_code == AT_RESP_OK)
			{
				setting_modem_status ++;
			}
			break;
		}
		case 1:
		{
			at_command("AT&K0\r\n",AT_AT,200,&at_cmd_ret_code);
			if (at_cmd_ret_code == AT_RESP_OK)
			{
				setting_modem_status ++;
			}
			break;
		}		
		case 2:
		{
			at_command("AT+CGMR\r\n",AT_AT,200,&at_cmd_ret_code);
			if (at_cmd_ret_code == AT_RESP_OK)
			{
				setting_modem_status ++;
			}
			break;
		}		
			
	}
}

void init_modem(void)
{
	
	int i,at_cmd_ret_code;
	
	reinit:
	
	initModem2();
	
	
	//AT+CSMS=1
	
	//设置短信直接显示
	at_command("AT+CMGF=1\r\n",AT_AT,200,&at_cmd_ret_code);
	at_command("AT+CNMI=2,2,0,0,0\r\n",AT_AT,200,&at_cmd_ret_code);
//	at_command("AT&W\r\n",AT_AT,200,&at_cmd_ret_code);
//	at_command("AT&K0\r\n",AT_AT,200,&at_cmd_ret_code);
	at_command("AT+COPS=0,2\r\n",AT_AT,200,&at_cmd_ret_code);
	
	for(i=1;i<=10;i++)
	{
		at_command("AT+GSN\r\n",AT_GSN,200,&at_cmd_ret_code);
		
		if (at_cmd_ret_code == AT_RESP_OK)
			break;
		
		if (i == 10)
		{
			goto reinit;
		}
		
	}
	
	for(i=1;i<=10;i++)
	{
		at_command("AT+CCID\r\n",AT_CCID,200,&at_cmd_ret_code);
		
		if (at_cmd_ret_code == AT_RESP_OK)
			break;
		
		if (i == 10)
		{
			goto reinit;
		}
		
	}
	
	
	for(i=1;i<=60;i++)
	{
		at_command("AT+COPS=0,2\r\n",AT_AT,200,&at_cmd_ret_code);
		at_command("AT+COPS?\r\n",AT_COPS,200,&at_cmd_ret_code);
		
		//如果发现网络注册过程中模块挂掉了则重新复位。。。
		if (read_modem_status == 0)
		{
			goto reinit;
		}
		
		if (at_cmd_ret_code == AT_RESP_OK)
			break;
		
		if (i == 60)
		{
			goto reinit;
		}
		
	}
	
	
	
		//??CCID????? SPIFLASH?
	if(strstr(uconfig.APNINFO.CCID,simcard_ccid) <= 0)
	{
		int i=0;
		char apn_cnt = 0;
		char apn_cnt_index = 0;
		const APN_TABLE *atb;
		char atcmd_buf[64];
		apn_cnt = get_apn_cnt(network_code);
		
		for(i=0;i<apn_cnt;i++)
		{
			
			atb = get_apn(network_code,i);
			if (atb > 0)
			{
				DEBUGL->debug("GET APN NAME %d %s \r\n",network_code,atb->apn);
				snprintf(atcmd_buf,sizeof(atcmd_buf),"AT+CGDCONT=1,\"IP\",\"%s\"\r\n",atb->apn);
				at_command(atcmd_buf,AT_CGDCONT,200,&at_cmd_ret_code);
				
				at_command_wait("ATD*99***1#\r\n",AT_ATD,6000,&at_cmd_ret_code);
				if (at_cmd_ret_code == AT_RESP_OK)
				{
					sprintf(uconfig.APNINFO.CCID,"%s",simcard_ccid);
					sprintf(uconfig.APNINFO.APN,"%s",atb->apn);
					sprintf(uconfig.APNINFO.USERNAME,"%s",atb->username);
					sprintf(uconfig.APNINFO.PASSWORD,"%s",atb->password);
					WRITE_USER_CONFIG;
					rt_reset_system();
					break;
				}
				
			} else {
				break;
			}
			
		}
		
		goto reinit;

		
		//
	}
	
	//close_max9860();
	
}

unsigned char read_modem_status(void)
{
	return GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_9);
	//
}

