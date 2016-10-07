#include <rtthread.h>
#include <components.h>
#include <stdio.h>

#include "common.h"

#include "ppp_service.h"
#include "modem_serial.h"
#include "netif/ppp/ppp.h"
#include "modem.h"
#include "lwip_raw_api_test.h"
#include "lwip_raw_api_tcp_template.h"
#include "XMPP/xmpp.h"
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
#include "uart4_cam_drv.h"
#include "dcmi_ov2640.h"
#include "ledctrlapp.h"
#include "easyiot.h"
#include "w25q16.h"
#include "spiflash_config.h"

extern rt_err_t rt_thread_sleep(rt_tick_t tick);

#if 0
/* Error codes. */
#define PPPERR_NONE      0 /* No error. */
#define PPPERR_PARAM    -1 /* Invalid parameter. */
#define PPPERR_OPEN     -2 /* Unable to open PPP session. */
#define PPPERR_DEVICE   -3 /* Invalid I/O device for PPP. */
#define PPPERR_ALLOC    -4 /* Unable to allocate resources. */
#define PPPERR_USER     -5 /* User interrupt. */
#define PPPERR_CONNECT  -6 /* Connection lost. */
#define PPPERR_AUTHFAIL -7 /* Failed authentication challenge. */
#define PPPERR_PROTOCOL -8 /* Failed to meet protocol. */
#endif

extern int lwip_system_init(void);
extern void TCP_Client_Routing(void);
static void linkStatusCB(void *ctx, int errCode, void *arg);

//int pppd_status = PPPD_STATUS_NONE;						/*PPP 状态机*/
static int pppd_pd = -1;															/*PPP线程文件描述符*/

void __test_modem_err (void)
{
	privd.pppd_status = PPPD_STATUS_PROTO_ERROR;
}

struct PRIVATEDATA privd;

static unsigned int last_rv_serial_data_jiff = 0;			/*串口接收数据定时器*/
void recv_serial_data_fn(void *p)
{
	last_rv_serial_data_jiff = app_timer_data.app_timer_second;
}
// 检查MCU与GPRS模块间的数据通讯是否正常
//static void check_ppp_timeout(void)
//{
//	
//	#define SERIAL_COMM_TIMEOUT 60*10
//		
//	if (last_rv_serial_data_jiff == 0)
//		last_rv_serial_data_jiff = app_timer_data.app_timer_second;
//	
//	rt_kprintf("recv ppp serial data timep %d \r\n",app_timer_data.app_timer_second - last_rv_serial_data_jiff);
//	
//	//如果PPP 
//	switch(pppd_status)
//	{
//		case PPPD_STATUS_PROTO_SUCCESS:
//		{
//			//如果是在PPP正常连接状态下超过 SERIAL_COMM_TIMEOUT 秒未收到串口数据则断开当前PPP进行重新连接
//			if ((app_timer_data.app_timer_second - last_rv_serial_data_jiff) > SERIAL_COMM_TIMEOUT)
//			{
//				pppd_status = PPPD_STATUS_PROTO_TIMEOUT;
//				rt_kprintf("Serial Timeout \r\n");
//				last_rv_serial_data_jiff = app_timer_data.app_timer_second;
//			}
//			break;
//		}
//		default:
//			last_rv_serial_data_jiff = app_timer_data.app_timer_second;
//			break;
//	}
//	
//	//
//}


// ??MCU?GPRS????????????
static void check_ppp_timeout(void)
{
	
	int SERIAL_COMM_TIMEOUT = 60*10;
		
	if (last_rv_serial_data_jiff == 0)
		last_rv_serial_data_jiff = app_timer_data.app_timer_second;
	
	DEBUGL->debug("recv ppp serial data timep %d \r\n",app_timer_data.app_timer_second - last_rv_serial_data_jiff);
	
	//??PPP 
	switch(privd.pppd_status)
	{
		case PPPD_STATUS_PROTO_SUCCESS:
		{
			//????PPP????????? SERIAL_COMM_TIMEOUT ?????????????PPP??????
			//??XMPP ?????,??4???3G???????????
			if (tcp_client_buffer[XMPP_TCP_CLIENT].active_status == TCP_CONNECTED)
				SERIAL_COMM_TIMEOUT = 60 * 10;	//10??
			else	
				SERIAL_COMM_TIMEOUT = 60 * 4;	//4??
			
			if ((app_timer_data.app_timer_second - last_rv_serial_data_jiff) > SERIAL_COMM_TIMEOUT)
			{
				privd.pppd_status = PPPD_STATUS_PROTO_TIMEOUT;
				DEBUGL->debug("Serial Timeout \r\n");
				last_rv_serial_data_jiff = app_timer_data.app_timer_second;
			}
			break;
		}
		default:
			last_rv_serial_data_jiff = app_timer_data.app_timer_second;
			break;
	}
	
	//
}

extern void write_ppp(unsigned char *data , int len);
extern void recv_serial_data_fn(void *p);
static void serial_vport_recv_ppp_data(unsigned char *data , int len)
{
	//led_on(2);
	recv_serial_data_fn(0);
	write_ppp(data ,len);
	//led_off(2);
}

static int dial(void)
{
	
	int i=0;
	int j=0;
	char atcmd_buf[64];
	int at_cmd_ret_code;
	char apn_cnt = 0;
	char apn_cnt_index = 0;
	const APN_TABLE *atb;
	char successflag = 0;
	
	DEBUGL->debug("GET APN NAME %d %s \r\n",network_code,uconfig.APNINFO.APN);
	snprintf(atcmd_buf,sizeof(atcmd_buf),"AT+CGDCONT=1,\"IP\",\"%s\"\r\n",uconfig.APNINFO.APN);
	cmux_at_command(1,atcmd_buf,AT_CGDCONT,200,&at_cmd_ret_code);
	cmux_at_command_wait(1,"ATD*99***1#\r\n",AT_ATD,6000,&at_cmd_ret_code);
	if (at_cmd_ret_code == AT_RESP_OK)
	{
		pppSetAuth(PPPAUTHTYPE_PAP,uconfig.APNINFO.USERNAME,uconfig.APNINFO.PASSWORD);
	} else {
		return -1;
	}
	
	cmux_ctrl(1,VPORT_RAWDATA_TYPE,serial_vport_recv_ppp_data);
	//
}

static void ppp_routing(void *p)
{
	int at_cmd_ret_code;
	check_ppp_timeout();
	switch(privd.pppd_status)
	{
		case PPPD_STATUS_NONE:
			privd.pppd_status = PPPD_STATUS_INITMODEM;
			break;
		case PPPD_STATUS_INITMODEM:
			
			if (pppd_pd < 0)
				pppd_pd = pppOpen(rt_device_find("uart3"),linkStatusCB,0);
			else
			{
				//close tcp
				pppReOpen(pppd_pd);
			}
			dial();
			privd.pppd_status = PPPD_STATUS_PROTO_START;
			break;
		case PPPD_STATUS_PROTO_STOP:
			//privd.pppd_status = PPPD_STATUS_NONE;
			break;
		case PPPD_STATUS_PROTO_TIMEOUT:
		case PPPD_STATUS_PROTO_ERROR:
			
			//比较极端的做法，直接复位系统，现在看来效果不错
			//rt_reset_system();
		
			//当出现错误，或者 PPP 超时的时候将销毁当前PPP 链接
			LOCK_TCPIP_CORE();
			disconn_tcp_client_buf();
			pppClose(pppd_pd);
			UNLOCK_TCPIP_CORE();
			privd.pppd_status = PPPD_STATUS_PROTO_STOP;
			privd.system_status = SYS_PPPERROR_STATUS;
			break;
		case PPPD_STATUS_PROTO_START:
			break;
		case PPPD_STATUS_PROTO_SUCCESS:
			rout_tcp_client_buf();
			//privd.system_status = SYS_PPPSUCCESS_STATUS;
			break;
		default:
			break;
	}
//
}


static void linkStatusCB(void *ctx, int errCode, void *arg)
{
	rt_kprintf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ PPP STATUS %d\n",errCode);
	switch(errCode)
	{
		case -5:
		case -8:
			privd.pppd_status = PPPD_STATUS_PROTO_ERROR;
			
			break;
		case -6:
			break;
		case 0:
			privd.pppd_status = PPPD_STATUS_PROTO_SUCCESS;
		default:
			break;
	
	}
}

void system_info(void* parameter)
{
		rt_uint32_t meminfo_total, meminfo_used, meminfo_maxused;
		DEBUGL->debug("\r\n");
		DEBUGL->debug("\r\n");
		DEBUGL->debug("\r\n");
		DEBUGL->debug("#####################################\r\n");
		DEBUGL->debug("SYSTEM INFO BuildDate %s\r\n",__DATE__);
		DEBUGL->debug("#####################################\r\n");
		DEBUGL->debug("\r\n");
		rt_memory_info(&meminfo_total,&meminfo_used,&meminfo_maxused);
		DEBUGL->debug("stm32 meminfo  jiff:%u total:%u used:%u maxused:%u\n",1,meminfo_total,meminfo_used,meminfo_maxused);
		DEBUGL->debug("XMPP PING_CNT:%d \r\n",xmpp_pma.recv_ping_cnt);
		DEBUGL->debug("System runnning tim %d \r\n",app_timer_data.app_timer_second);
		DEBUGL->debug("System STATUS SYS:%d PPP:%d APP:%d \r\n",privd.system_status,privd.pppd_status,privd.app_status); //privd.pppconnect_cnt ++;
		DEBUGL->debug("System privd.pppconnect_cnt %d \r\n",privd.pppconnect_cnt);
		DEBUGL->debug("\r\n");
		DEBUGL->debug("\r\n");
		DEBUGL->debug("\r\n");
}

void system_check(void *p)
{
	rt_uint32_t meminfo_total, meminfo_used, meminfo_maxused;
	rt_memory_info(&meminfo_total,&meminfo_used,&meminfo_maxused);
	rt_kprintf("stm32 meminfo  jiff:%u total:%u used:%u maxused:%u\n",1,meminfo_total,meminfo_used,meminfo_maxused);
}

//定期执行，负责检查系统状态
static void checksys_tmr_task(void *p)
{
	feed_watchdog(0);
	DEBUGL->debug("FEED WATCHDOG !\r\n");
	if (read_modem_status)
	{
		DEBUGL->debug("SIM800C ACTIVE !\r\n");
	}else {
		DEBUGL->debug("SIM800C OFFFFFFFFFFF !\r\n");
	}
	//led 针对不同的系统状态，对led做出不同的判断
	//
	
	system_info(0);
	system_check(0);
}



static void upload_signal_val(void *p)
{
	//+SIG=
	char out[32];
	extern unsigned char gsm_signal;
	int signal_mapping[] = {0,3,6,10,13,16,19,23,26,29,32,36,39,42,43,45,48,52,55,61,65,58,71,74,78,81,84,87,90,94,97,100 };
	if( gsm_signal >= 99 )
		gsm_signal = 0;
	if(gsm_signal > 31)
		gsm_signal = 31;
			
//	snprintf(out,32,"+SIG=%d",signal_mapping[gsm_signal]);
//	xmpp_set_presence("online",out);
	snprintf(out,32,"+INFO=%d,%d",signal_mapping[gsm_signal],EASYIOT_FW_VERSION_NUM);
	xmpp_set_presence("online",out);
	//snprintf(out,RESP_BUFFER_LEN,"OK",signal_mapping[gsm_signal]);
}

void mainloop(void *p)
{
	int at_cmd_ret_code;
	int returncode;
	char stream = 0;
	memset(&privd,0x0,sizeof(privd));
	
	rt_kprintf("@@@@@@@@@@@@@@@@@@@@@@@@ BUILD DATE %s TIME %s \r\n",__DATE__,__TIME__);
	

	SPI_Flash_Init();
	
	init_gpio();
	init_led_hw();
	init_ledctrlapp();
	init_modem_gpio();
	__read_uconfig();
	
	//初始化协议栈
	lwip_system_init();
	pppInit();
	// read config
  pppSetAuth(PPPAUTHTYPE_PAP,NETWORK_PPP_USERNAME,NETWORK_PPP_PASSWORD);
	init_tcp_client_buf();

	
	//打开ISP
	init_isp_serial_port("uart2");
	create_isp_thread();
	
	//打开GPRS串口
	open_modem_serial_port("uart3");
	
	easyiot_init();
	config_uart4();
	
	__init_cmux_thread();
	
	{	//启动一个定时器
		//alloc_timer
		EI_TIMER *_ota_tmr = alloc_timer();
		_ota_tmr->ticks = 60;
		_ota_tmr->timeout = upload_signal_val;
		_ota_tmr->timeout_arg = 0;
		timer_start(_ota_tmr); //?????
	}
	
	{	//启动一个定时器
		//alloc_timer
		EI_TIMER *_ota_tmr = alloc_timer();
		_ota_tmr->ticks = 2;
		_ota_tmr->timeout = checksys_tmr_task;
		_ota_tmr->timeout_arg = 0;
		timer_start(_ota_tmr); //?????
	}
	

	for(;;)
	{
		static char routting_counter = 0;
		routting_counter ++;
		
		switch(privd.system_status)
		{
			case SYS_NORMAL_STATUS:
				privd.system_status = SYS_INITMODEM_STATUS;
				break;
			case SYS_INITMODEM_STATUS:
			{
				for(;;)
				{
					__pause_cmux_thread(1);
					init_modem();
					if (__restart_cmux() == RT_EOK)
						break;
					
				}
				
				privd.pppconnect_cnt ++;
				
				init_xmpp_tcp();
				init_isp_http_cli();
				
				
				privd.system_status = SYS_INITPPP_STATUS;
				privd.pppd_status = PPPD_STATUS_NONE;
				break;
			}
			
			case SYS_INITPPP_STATUS:
			{
				if ((routting_counter % 100) == 0)
				{
					ppp_routing(0);
				}
				if (privd.pppd_status == PPPD_STATUS_PROTO_SUCCESS)
				{
					
					privd.system_status = SYS_PPPSUCCESS_STATUS;
				}
				break;
			}
			
			case SYS_PPPERROR_STATUS:
			{
				DEBUGL->debug("PPP ERROR \r\n");
				privd.system_status = SYS_INITMODEM_STATUS;
				break;
			}
			
			case SYS_PPPSUCCESS_STATUS:
			{
				if ((routting_counter % 100) == 0)
				{
					ppp_routing(0);
					isp_http_tcp_routing(0);
					
					//如果发现N次CSQ没有反应，说明模块挂了,重启并重新连接网络
					cmux_at_command(2,"AT+CSQ\r\n",AT_CSQ,200,&at_cmd_ret_code);
					if (at_cmd_ret_code != AT_RESP_OK)
					{
						static unsigned char atcsq_err_cnt = 0;
						atcsq_err_cnt ++;
						if (atcsq_err_cnt > 5)
						{
							privd.pppd_status = PPPD_STATUS_PROTO_ERROR;
							//
						}
					}
					
				}
				break;
			}
			
			
		}
		
		
		if (routting_counter >= 100)
			routting_counter = 0;
		rt_thread_sleep(RT_TICK_PER_SECOND/100);
	}
	
	return ;
}




