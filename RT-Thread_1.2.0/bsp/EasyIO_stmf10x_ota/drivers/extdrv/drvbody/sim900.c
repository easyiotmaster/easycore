#include "sim900.h"
#include "at_cmd.h"
#include "modem_serial.h"
#include "common.h"

#include <rtthread.h>


static void init_sim900_ctrl_gpio(void)
{
	
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
	GPIO_Init(GPIOB, &GPIO_InitStructure); 

	GPIO_ResetBits(GPIOB,GPIO_Pin_1);
	
	
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
	GPIO_Init(GPIOB, &GPIO_InitStructure); 
	GPIO_SetBits(GPIOB,GPIO_Pin_0);
	//GPIO_ResetBits(GPIOB,GPIO_Pin_0);
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
	GPIO_Init(GPIOC, &GPIO_InitStructure); 
	GPIO_SetBits(GPIOC,GPIO_Pin_5);
	
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; 
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
	GPIO_Init(GPIOC, &GPIO_InitStructure); 
	GPIO_SetBits(GPIOC,GPIO_Pin_4);
	
	
}


static void power_sim900(void)
{
	GPIO_ResetBits(GPIOB,GPIO_Pin_1);
	rt_thread_sleep(100); //600ms
	GPIO_SetBits(GPIOB,GPIO_Pin_1);
	
}

static void reset_sim900(void)
{

}

static void powerModem(void)
{
	//
}


static void initModem2(void)
{
	int i=0;
	int at_cmd_ret_code;
	unsigned char atcmd_cnt;
	reset_sim900();
	
	power_sim900();
	
//snprintf(atcmd_buf,sizeof(atcmd_buf),"AT+GSN\r\n");
//at_command(1,"AT+GSN\r\n",AT_GSN,200,&at_cmd_ret_code);
	
//	if (at_cmd_ret_code == AT_RESP_OK)
//	{
//		rt_kprintf("IMEI %s \r\n",modem_imei);
//	}
	
	repower:
	
	
	for(i=0;i<10;i++)
	{
		at_command("AT\r\n",AT_AT,100,&at_cmd_ret_code);
		if (at_cmd_ret_code == AT_RESP_OK)
			return ;
	}
	
	goto repower;
}


static void initModem(void)
{

	int i=0;
	
	char atcmd_buf[64];
	
	int at_cmd_ret_code;
	//initModem2();
	
	snprintf(atcmd_buf,sizeof(atcmd_buf),"AT+CGDCONT=1,\"IP\",\"%s\"\r\n",NETWORK_APN_NAME);
	
	for(;;)
	{
		at_command(atcmd_buf/*"AT+CGDCONT=1,\"IP\",\"cmnet\"\r\n"*/,AT_CGDCONT,200,&at_cmd_ret_code);
		//at_command(atcmd_buf/*"AT+CGDCONT=1,\"IP\",\"cmnet\"\r\n"*/,AT_CGDCONT,200,&at_cmd_ret_code);
		if (at_cmd_ret_code == AT_RESP_OK)
			break;
	}
	
//	for(i=0;i<10;i++)
//	{
//		at_command("AT+CSQ\r\n",AT_CSQ,1000,&at_cmd_ret_code);
//		rt_thread_sleep(100);
//	}
	
	for(;;)
	{
		at_command("ATD*99***1#\r\n",AT_ATD,1000,&at_cmd_ret_code);
		if (at_cmd_ret_code == AT_RESP_CONNECT)
			break;
		
		
	}
}


void init_modem_hw(void)
{
	init_sim900_ctrl_gpio();
	//
}


static int recv_cmux_buf(const char *resp , int len)
{
	return 1;
	//
}

void init_sim900_modem(void)
{
	
	int i,at_cmd_ret_code;
	
	initModem2();
	
	//¶ÁÈ¡IMEI
	for(i=0;i<5;i++)
	{
		at_command("AT+GSN\r\n",AT_GSN,200,&at_cmd_ret_code);
		if (at_cmd_ret_code == AT_RESP_OK)
			break;
	}
	
}