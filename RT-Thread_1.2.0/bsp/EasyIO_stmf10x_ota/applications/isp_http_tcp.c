#include "isp_http_tcp.h"
#include "lwip_raw_api_tcp_template.h"
#include "MD5/md5sum.h"
#include "flash_opt.h"
#include "flash_config.h"
#include "XMPP/xmpp.h"
#include "my_stdc_func/my_stdc_func.h"
#include "extdrv/gpio.h"
#include <rtthread.h>
#include "my_stdc_func/debugl.h"
#include "serial_at.h"
#include "app_timer.h"
#include "json/cJSON.h"
#include "spiflash_config.h"
#include "w25q16.h"
#include <stdio.h>
#include "usart.h"
#include "app_timer.h"

//#define TEST_AVR_PROG

static char 						DOWNLOAD_TYPE									= 2;
static unsigned int 		STORAGE_SIZE									= DOWNLOAD_ZONE_SIZE;
static unsigned int 		STORAGE_ADDRESS								= DOWNLOAD_ZONE_ADDRESS;

struct HTTP_OPT httpopt;

enum{
	HTTP_STATUS_INIT,
	HTTP_STATUS_START,
	HTTP_STATUS_STARTED,
	
	HTTP_STATUS_UPLOADING,
	HTTP_STATUS_UPLOADING_END,
	HTTP_STATUS_UPLOAD_FINISH,
	HTTP_STATUS_UPLOAD_DONE,
	
	HTTP_DOWNLOADING,
	HTTP_DOWNLOAD_FINISH,
	
	
	HTTP_ERROR,
	HTTP_FINISH,
	HTTP_SOCKET_BUSY,
};

enum{
	HTTP_GET_CONFIG,
	HTTP_GET_DOWNLOADFILE,
	HTTP_POST_UPLOADPIC,
};

static md5_state_t state;
static md5_byte_t digest[16];
static char hex_output[16*2 + 1];

static int http_tcp_status = HTTP_STATUS_INIT;
//static int http_get_status = HTTP_GET_DOWNLOADFILE;
static int http_task_type = HTTP_GET_DOWNLOADFILE;

static unsigned int upload_size;		//当前上传的长度
static unsigned int upload_content_length; //总长度

static unsigned int download_size;	//当前下载的长度
static unsigned int download_content_length; //总长度

static unsigned http_active_timep;		//http最后活动时间

#define SET_HTTP_ACTIVE http_active_timep = app_timer_data.app_timer_second;
#define GET_HTTP_ACTIVE (app_timer_data.app_timer_second - http_active_timep)


static char xmpp_sender[64];
static char http_host[64] = {0x0};
static char http_path[128] = {0x0};
static int http_port = 80;

static unsigned int priv_send_tmr = 0;

void set_xmpp_sender(char *sender)
{
	snprintf(xmpp_sender,sizeof(xmpp_sender),"%s",sender);
}


static void xmpp_send_msg_delay(char *from , char *msg , unsigned int tmr)
{
	
	if ((app_timer_data.app_timer_second - priv_send_tmr) < tmr)
	{
		return;
		//
	}
	
	//
	xmpp_send_msg(from,msg);
	priv_send_tmr = app_timer_data.app_timer_second;
	//
}

static void send_httpget_pkg(char *path , char *host)
{
	char *buffer = rt_malloc(128);
	if (buffer > 0)
	{
		
		snprintf(buffer,128,"GET /%s HTTP/1.1\r\nHOST: %s\r\nConnection: keep-alive\r\n\r\n",path,host);
		DEBUGL->debug("REQUEST : %s",buffer);
		MY_TCP_Client_write(&tcp_client_buffer[HTTP_TCP_CLIENT],(unsigned char*)buffer,strlen(buffer));
		rt_free(buffer);
	}else
	{
		http_tcp_status = HTTP_ERROR;
		
	}
}

static void send_httppost_pkg(char *path , char *host , unsigned char *data , int length)
{
	int post_header_length = 0;
	char *buffer = rt_malloc(512);
	if (buffer > 0)
	{
		char *tmpstr = buffer + 500;
		buffer[0] = 0x0;
		tmpstr[0] = 0x0;
		
		strcat(buffer,"POST /upload/upload.php HTTP/1.1\r\n");
		strcat(buffer,"Host: www.easy-iot.cc\r\n");
		strcat(buffer,"Connection: keep-alive\r\n");
		//strcat(buffer,"Content-Length: 10664\r\n");   //== body + img + end 10664
		strcat(buffer,"Content-Length: ");
		sprintf(tmpstr,"%d",length + (10664 - 10479));
		strcat(buffer,tmpstr);
		strcat(buffer,"\r\n");
		strcat(buffer,"Content-Type: multipart/form-data; boundary=----WebKitFormBoundaryeCGbABkhlBu3xcId\r\n");
		strcat(buffer,"Referer: http://www.easy-iot.cc/upload/\r\n");
		strcat(buffer,"Accept-Encoding: gzip, deflate\r\n");
		strcat(buffer,"Accept-Language: zh-CN,zh;q=0.8\r\n\r\n");
		
		post_header_length = strlen(buffer);
		
		strcat(buffer,"------WebKitFormBoundaryeCGbABkhlBu3xcId\r\n");
		strcat(buffer,"Content-Disposition: form-data; name=\"pic\"; filename=\"__EasyIOT.jpg\"\r\n");
		strcat(buffer,"Content-Type: image/jpg\r\n\r\n");
		
		rt_kprintf("=====[%s]\r\n",buffer);
		
		post_header_length = strlen(buffer);
		
		//发送header
		LOCK_TCPIP_CORE();
		tcp_write(tcp_client_buffer[HTTP_TCP_CLIENT].pcb,(unsigned char*)buffer,strlen(buffer),TCP_WRITE_FLAG_COPY);
		UNLOCK_TCPIP_CORE();
		
		rt_free(buffer);
		
		upload_content_length = length;
		upload_size = 0;
		
		
		//tcp_write(cli->pcb,buffer,len,TCP_WRITE_FLAG_COPY);
		
		//标记上传状态
		http_tcp_status = HTTP_STATUS_UPLOADING;
		
	}else
	{
		http_tcp_status = HTTP_ERROR;
		
	}
}



static int HTTP_GetContentLength(char *revbuf)
{
    char *p1 = NULL, *p2 = NULL;
    int HTTP_Body = 0;

    p1 = strstr(revbuf,"Content-Length");
    if(p1 == NULL)
        return -1;
    else
    {
        p2 = p1+strlen("Content-Length")+ 2; 
        HTTP_Body = atoi(p2);
        return HTTP_Body;
    }

}


static void recv_http_buf(unsigned char *buf , int len)
{
	//if (content_length)
	
	switch(http_task_type)
	{
		case HTTP_GET_DOWNLOADFILE:
		{
			
			char str[64];
			//snprintf(str,sizeof(str),"FINISHED:%d KB TOTAL:%d KB",download_size ,download_content_length,(int)(((float)download_size/(float)download_content_length)*100));
			snprintf(str,sizeof(str),"+DOWN=0,%d,%d",download_size ,download_content_length);
			xmpp_send_msg_delay(xmpp_sender,str,5);
			
			md5_append(&state, (const md5_byte_t *)buf,len);
			
			if (DOWNLOAD_TYPE == 1)
			{
				FLASH_AppendBuffer((u8*)buf,len);
			}else if (DOWNLOAD_TYPE == 2)
			{
				//写入数据
				append_w25x_append_buffer((unsigned char*)buf,len);

			}
			//
			break;
		}
		case HTTP_GET_CONFIG:
		{
			//json
			char *out;cJSON *json,*json_value;
	
			json=cJSON_Parse((char *)buf);
			if (!json) {rt_kprintf("Error before: [%s]\n",cJSON_GetErrorPtr());}
			else
			{
				
				json_value = cJSON_GetObjectItem( json , "filename");
				if( json_value->type == cJSON_String )
				{
					DEBUGL->info("filename:%s\r\n",json_value->valuestring);
				}
				
				json_value = cJSON_GetObjectItem( json , "version_name");
				if( json_value->type == cJSON_String )
				{
					DEBUGL->info("version_name:%s\r\n",json_value->valuestring);
				}
				
				json_value = cJSON_GetObjectItem( json , "checksum");
				if( json_value->type == cJSON_String )
				{
					DEBUGL->info("checksum:%s\r\n",json_value->valuestring);
				}
				
				json_value = cJSON_GetObjectItem( json , "mode");
				if( json_value->type == cJSON_String )
				{
					 DEBUGL->info("mode:%s\r\n",json_value->valuestring);  
				}
				
				json_value = cJSON_GetObjectItem( json , "size");
				if( json_value->type == cJSON_Number )
				{
					DEBUGL->info("filesize:%d\r\n",json_value->valueint);  
				}
				
				json_value = cJSON_GetObjectItem( json , "version");
				if( json_value->type == cJSON_Number )
				{
					DEBUGL->info("version:%d\r\n",json_value->valueint);  
				}
				
				cJSON_Delete(json);
				
			}
			
			DEBUGL->info("CONFIG BODY: %s",buf);
			break;
		}
	}
	
}

static void recv(void *arg,unsigned char *buf , int len)
{
	
	SET_HTTP_ACTIVE;
	
	switch(http_tcp_status)
	{
		case HTTP_STATUS_UPLOADING:
		case HTTP_STATUS_UPLOADING_END:
		case HTTP_STATUS_UPLOAD_FINISH:
			if (httpopt.done > 0)
			{
				httpopt.done(HTTP_UPLOADFINISH);
			}
		
			DEBUGL->debug("%s response : %s \r\n",buf);
			{
				
				char *jstr = strstr((char*)buf,"{\"");
				cJSON *json,*json_value;
				json=cJSON_Parse((char *)jstr);
				if (!json) {rt_kprintf("Error before: [%s]\n",cJSON_GetErrorPtr());}
				else
				{
					json_value = cJSON_GetObjectItem( json , "imgurl");
					if( json_value->type == cJSON_String )
					{
						char *buffer = rt_malloc(128);
						if (buffer > 0)
						{
							snprintf(buffer,128,"+PIC=\"http://easy-iot.cc/upload/%s\"",json_value->valuestring+1);
							xmpp_set_presence("online",buffer);
							rt_free(buffer);
						}
						
						cJSON_Delete(json);
						//DEBUGL->info("version_name:%s\r\n",json_value->valuestring);
					}
					////
				}
					
				
//				char str[64];
//				snprintf(str,sizeof(str),"Upload PIC SUCCESS!");
				//xmpp_send_msg(xmpp_sender,str);
			}
			http_tcp_status = HTTP_STATUS_UPLOAD_DONE;
			break;
		case HTTP_STATUS_STARTED:
		{
			//检查报文头
			DEBUGL->debug("%s response : %s \r\n",buf);
			download_content_length = HTTP_GetContentLength((char*)buf);
			
			//判断下载文件的合法性
			if ((download_content_length > 0) && (download_content_length < STORAGE_SIZE))
			{
				char *data_pos = (strstr((char*)buf,"\r\n\r\n"));
				if (data_pos > 0)
				{
					data_pos += 4;
					download_size = 0;
					
					switch(DOWNLOAD_TYPE)
					{
						case 1:
							break;
						case 2:
						{
							//download_content_length
							//前4个字节写明文件长度
							append_w25x_append_buffer((unsigned char*)&download_content_length,sizeof(download_content_length));
							//w25x_append_end();
							break;
						}
						default:
							break;
					}
					

					http_tcp_status = HTTP_DOWNLOADING;
					
					download_size = (unsigned int)len - ((unsigned int)data_pos - (unsigned int)buf);
					recv_http_buf((u8*)data_pos,(unsigned int)len - ((unsigned int)data_pos - (unsigned int)buf));
					
					
					if (download_size == download_content_length)
					{	
						http_tcp_status = HTTP_DOWNLOAD_FINISH;
					}
				}
				//
			}else
			{
				http_tcp_status = HTTP_ERROR;
			}
			break;
		}
		
		case HTTP_DOWNLOADING:
			download_size += len;
			recv_http_buf((u8*)buf,len);
			
			if (download_size == download_content_length)
			{
				http_tcp_status = HTTP_DOWNLOAD_FINISH;
			}
			break;
		
		default:
			break;
	
	}
	
}

static void connected(void *arg)
{
	DEBUGL->debug("http server connected !!!\r\n");
	
	switch(http_task_type)
	{
		case HTTP_POST_UPLOADPIC:
			send_httppost_pkg(http_path,http_host,(unsigned char*)0,httpopt.uploadlenth);
			break;
		case HTTP_GET_DOWNLOADFILE:
			DEBUGL->debug("@@@@@@@@@@@@@@@@@@@@@@@@@ downloading fw ...\r\n");
			//send_httpget_pkg("rtthread.bin","www.easy-io.net");
			send_httpget_pkg(http_path,http_host);
			//send_httpget_pkg("/rthread.bin","www.easy-io.net");
			md5_init(&state);
		
			if (DOWNLOAD_TYPE == 1)
			{
				FLASH_ProgramStart(CHIP_DOWNLOAD_ADDR,CHIP_DOWNLOAD_SIZE);
			}else if (DOWNLOAD_TYPE == 2)
			{
				init_write_flash(DOWNLOAD_ZONE_ADDRESS,0x0);
			}
		
			break;
		case HTTP_GET_CONFIG:
			DEBUGL->debug("@@@@@@@@@@@@@@@@@@@@@@@@@ downloading config ...\r\n");
			//send_httpget_pkg("/fw_config","www.easy-io.net");
			send_httpget_pkg(http_path,http_host);
			break;
	}
}
static void disconn(void *arg)
{
	
	
	unsigned int di;
	if (httpopt.done > 0)
	{
		//httpopt.done(HTTP_DISCONNECT);
	}
	
	if (http_task_type == HTTP_GET_DOWNLOADFILE)
	{
		char str[64];
		
		if (DOWNLOAD_TYPE == 1)
		{
			FLASH_AppendEnd();
			FLASH_ProgramDone();
			snprintf(str,sizeof(str),"+DOWN=1,\"");
			//md5 recode;
			md5_init(&state);
			md5_append(&state, (const md5_byte_t *)CHIP_DOWNLOAD_ADDR,download_content_length);
			md5_finish(&state, digest);
			for (di = 0; di < 16; ++di)
			{
				char md5str[4];
				snprintf(md5str,sizeof(md5str),"%02x",digest[di]);
				strcat(str,md5str);
			}
			strcat(str,"\"");
		
		}else if (DOWNLOAD_TYPE == 2)
		{
			w25x_append_end();
			snprintf(str,sizeof(str),"+DOWN=1,\"");
			check_down_zone_buffer_md5(str+9);
			strcat(str,"\"");
		}
		
		xmpp_send_msg_delay(xmpp_sender,str,0);
	
	}
	
	http_tcp_status = HTTP_FINISH;
	//
}
static void connerr(void *arg)
{
	http_tcp_status = HTTP_ERROR;
	
}
static void routing(void *arg)
{
}



//////////////////////////  ISP

#define ISPPIN_BT_PORT  GPIO_Pin_4
#define ISPPIN_RT_PORT  GPIO_Pin_3

unsigned int isp_address_offset = 0x0;
unsigned int isp_program_size = 0x0;

static char serial_buffer[128];
static	int serial_buffer_index = 0;

static ISP_STATUS_STEP isp_step = ISP_NULL; 

#define STM32_ISP_HANDSHAKE_COUNT 20
#define STM32_ISP_ERASFLASH_COUNT 20
static unsigned char stm32isp_handshake_cnt = 0;
static unsigned char stm32isp_ack_cnt = 0;

static unsigned short max_uart_idle_time = 0;


#define RUNNING_ISP			set_boot_h();
#define RUNNING_FLASH		set_boot_l();

//static void isp_init_gpio(void)
//{
//	GPIO_InitTypeDef GPIO_InitStructure;
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
//	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//	GPIO_InitStructure.GPIO_Pin   = (ISPPIN_BT_PORT);
//	GPIO_Init(GPIOC, &GPIO_InitStructure);
//	
//	GPIO_ResetBits(GPIOC, GPIO_Pin_4); //BT
//}

static void set_rst_h(void)
{
	
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_3;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_SetBits(GPIOC, GPIO_Pin_3);//RST
	//
}

static void set_rst_a(void)
{
	
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_3;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	//GPIO_ResetBits(GPIOC, GPIO_Pin_3);//RST
	//
}

static void set_rst_l(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_3;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOC, GPIO_Pin_3);//RST
	//
}

static void set_boot_h(void)
{
	
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_SetBits(GPIOC, GPIO_Pin_4);//RST
	//
}

static void set_boot_l(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOC, GPIO_Pin_4);//RST
	//
}

//static void isp_set_gpio(unsigned char num)
//{
//	switch(num)
//	{
//		case 0:
//			GPIO_SetBits(GPIOC,ISPPIN_RT_PORT);
//			break;
//		case 1:
//			GPIO_SetBits(GPIOC,ISPPIN_BT_PORT);
//			break;
//	}
//	//
//}

//static void isp_reset_gpio(unsigned char num)
//{
//	switch(num)
//	{
//		case 0:
//			GPIO_ResetBits(GPIOC,ISPPIN_RT_PORT);
//			break;
//		case 1:
//			GPIO_ResetBits(GPIOC,ISPPIN_BT_PORT);
//			break;
//	}
//	//
//}



void reset_target_mcu(void)
{
	int i=0;

	set_rst_l();
	rt_thread_sleep(50);
	set_rst_h();
}

static void avr_rest(void)
{
	int i=0;
	
	NVIC_InitTypeDef		NVIC_InitStructure;
	USART_InitTypeDef		USART_InitStructure;
	GPIO_InitTypeDef		GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);//??GPIOA???,?????,??????,????,???????????

	//GPIO
	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed= GPIO_Speed_50MHz; //?????,IO???????
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //??????;??????????GPIO_Mode_Out_PP????????,??????,?????????;
	GPIO_Init(GPIOC,&GPIO_InitStructure);
	GPIO_ResetBits(GPIOC,GPIO_Pin_3);
	
	rt_thread_sleep(10);
//	//GPIO
	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed= GPIO_Speed_50MHz; //?????,IO???????
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; //??????;??????????GPIO_Mode_Out_PP????????,??????,?????????;
	GPIO_Init(GPIOC,&GPIO_InitStructure);
	GPIO_SetBits(GPIOC,GPIO_Pin_3);
	
	
}



static rt_device_t isp_serial_fd = RT_NULL ;
static struct rt_semaphore isp_serial_rx_sem;

static rt_err_t usart_rx_ind(rt_device_t dev, rt_size_t size)
{
	rt_sem_release(&isp_serial_rx_sem);//当有数据的时候将锁打开
	return RT_EOK;
}

rt_err_t open_isp_serial_port(char *portname)
{
	isp_serial_fd = rt_device_find("uart2");
	if (isp_serial_fd != RT_NULL && rt_device_open(isp_serial_fd, RT_DEVICE_OFLAG_RDWR) == RT_EOK)
	{
		rt_sem_init(&isp_serial_rx_sem, "isp_sem", 0, 0);
		rt_device_set_rx_indicate(isp_serial_fd, usart_rx_ind);
		//
	}else
	{
		return -RT_ERROR;
	}
	
	return -RT_EOK;
	//
}


//阻塞读串口数据
static int read_isp_serial_data(unsigned char *buf , int len , rt_int32_t time)
{
	//只等待1秒吧
	rt_sem_take(&isp_serial_rx_sem, time) ;//RT_ETIMEOUT
	return rt_device_read(isp_serial_fd, 0, buf, len);
}

static unsigned char clearbuffer(void)
{
	unsigned char buffer[50];
	while(read_isp_serial_data(buffer,50,1) > 0){} //wait 10ms
	return 0;
	
	//
}

static unsigned char write_isp_buffer_ack(unsigned char *buffer , int len)
{
	int size;
	clearbuffer();
	isp_serial_rx_sem.value = 0;
	rt_device_write(isp_serial_fd,0,buffer,len);
	buffer[0] = 0x0;
	size = read_isp_serial_data(buffer,100,200);
	return buffer[0];
	//
}

static int write_isp_buffer_ack_v2(unsigned char *buffer , int len , unsigned char *rbuffer , int rlen , int tim)
{
	int size=0;
	int rbuf_index = 0;
	int ret=0;
	
	rt_device_write(isp_serial_fd,0,buffer,len);
	buffer[0] = 0x0;
	
	while(rt_sem_take(&isp_serial_rx_sem, tim) == RT_EOK)
	{
		tim = RT_TICK_PER_SECOND/50; //只等待20ms
		size += rt_device_read(isp_serial_fd, 0, rbuffer + size, rlen-rbuf_index);
	}
	
	return size;
	//
}

static void write_isp_buffer(unsigned char *buffer , int len)
{

	rt_device_write(isp_serial_fd,0,buffer,len);
	
	//
}

static void write_resp_at(char *resp_at)
{
	//只有当有数据的时候才会发送
	if (strlen(resp_at) > 0)
		rt_device_write(isp_serial_fd,0,resp_at,strlen(resp_at));
	else
		return;
}

void write_to_isp_serial(char *buf , int length)
{
	if (isp_step == ISP_NULL)
	{
		rt_device_write(isp_serial_fd,0,buf,length);
	}
}

void put2ispuart(unsigned char *buf , int len)
{
	if (isp_step == ISP_NULL)
	{
		rt_device_write(isp_serial_fd,0,buf,len);
	}
}


void set_isp_step(ISP_STATUS_STEP step)
{
	isp_step = step;
	stm32isp_handshake_cnt = 0;
}
ISP_STATUS_STEP get_isp_step(void)
{
	return isp_step;
}

const unsigned char avrfw[] =
{
   0x0C, 0x94, 0x35, 0x00, 0x0C, 0x94, 0x5D, 0x00, 0x0C, 0x94, 0x5D, 0x00, 0x0C, 0x94, 0x5D, 
   0x00, 0x0C, 0x94, 0x5D, 0x00, 0x0C, 0x94, 0x5D, 0x00, 0x0C, 0x94, 0x5D, 0x00, 0x0C, 0x94, 0x5D, 
   0x00, 0x0C, 0x94, 0x5D, 0x00, 0x0C, 0x94, 0x5D, 0x00, 0x0C, 0x94, 0x5D, 0x00, 0x0C, 0x94, 0x5D, 
   0x00, 0x0C, 0x94, 0x5D, 0x00, 0x0C, 0x94, 0x5D, 0x00, 0x0C, 0x94, 0x5D, 0x00, 0x0C, 0x94, 0x5D, 
   0x00, 0x0C, 0x94, 0xC2, 0x02, 0x0C, 0x94, 0x5D, 0x00, 0x0C, 0x94, 0xB9, 0x01, 0x0C, 0x94, 0xEB, 
   0x01, 0x0C, 0x94, 0x5D, 0x00, 0x0C, 0x94, 0x5D, 0x00, 0x0C, 0x94, 0x5D, 0x00, 0x0C, 0x94, 0x5D, 
   0x00, 0x0C, 0x94, 0x5D, 0x00, 0x0C, 0x94, 0x5D, 0x00, 0x1B, 0x02, 0x11, 0x24, 0x1F, 0xBE, 0xCF, 
   0xEF, 0xD8, 0xE0, 0xDE, 0xBF, 0xCD, 0xBF, 0x11, 0xE0, 0xA0, 0xE0, 0xB1, 0xE0, 0xE2, 0xEE, 0xF6, 
   0xE0, 0x02, 0xC0, 0x05, 0x90, 0x0D, 0x92, 0xAE, 0x33, 0xB1, 0x07, 0xD9, 0xF7, 0x21, 0xE0, 0xAE, 
   0xE3, 0xB1, 0xE0, 0x01, 0xC0, 0x1D, 0x92, 0xA4, 0x3E, 0xB2, 0x07, 0xE1, 0xF7, 0x10, 0xE0, 0xCA, 
   0xE6, 0xD0, 0xE0, 0x04, 0xC0, 0x22, 0x97, 0xFE, 0x01, 0x0E, 0x94, 0x6B, 0x03, 0xC8, 0x36, 0xD1, 
   0x07, 0xC9, 0xF7, 0x0E, 0x94, 0xB3, 0x02, 0x0C, 0x94, 0x6F, 0x03, 0x0C, 0x94, 0x00, 0x00, 0x26, 
   0xE0, 0x40, 0xE0, 0x52, 0xEC, 0x61, 0xE0, 0x70, 0xE0, 0x8E, 0xE3, 0x91, 0xE0, 0x0E, 0x94, 0x57, 
   0x01, 0x60, 0xE0, 0x71, 0xE0, 0x8E, 0xE3, 0x91, 0xE0, 0x0C, 0x94, 0xA0, 0x02, 0x60, 0xE0, 0x71, 
   0xE0, 0x8E, 0xE3, 0x91, 0xE0, 0x0C, 0x94, 0xA0, 0x02, 0xFC, 0x01, 0x81, 0x8D, 0x22, 0x8D, 0x90, 
   0xE0, 0x80, 0x5C, 0x9F, 0x4F, 0x82, 0x1B, 0x91, 0x09, 0x8F, 0x73, 0x99, 0x27, 0x08, 0x95, 0xFC, 
   0x01, 0x91, 0x8D, 0x82, 0x8D, 0x98, 0x17, 0x31, 0xF0, 0x82, 0x8D, 0xE8, 0x0F, 0xF1, 0x1D, 0x85, 
   0x8D, 0x90, 0xE0, 0x08, 0x95, 0x8F, 0xEF, 0x9F, 0xEF, 0x08, 0x95, 0xFC, 0x01, 0x91, 0x8D, 0x82, 
   0x8D, 0x98, 0x17, 0x61, 0xF0, 0x82, 0x8D, 0xDF, 0x01, 0xA8, 0x0F, 0xB1, 0x1D, 0x5D, 0x96, 0x8C, 
   0x91, 0x92, 0x8D, 0x9F, 0x5F, 0x9F, 0x73, 0x92, 0x8F, 0x90, 0xE0, 0x08, 0x95, 0x8F, 0xEF, 0x9F, 
   0xEF, 0x08, 0x95, 0x81, 0xE1, 0x92, 0xE0, 0x89, 0x2B, 0x49, 0xF0, 0x80, 0xE0, 0x90, 0xE0, 0x89, 
   0x2B, 0x29, 0xF0, 0x0E, 0x94, 0x11, 0x02, 0x81, 0x11, 0x0C, 0x94, 0x00, 0x00, 0x08, 0x95, 0xFC, 
   0x01, 0x84, 0x8D, 0xDF, 0x01, 0xA8, 0x0F, 0xB1, 0x1D, 0xA3, 0x5A, 0xBF, 0x4F, 0x2C, 0x91, 0x84, 
   0x8D, 0x90, 0xE0, 0x01, 0x96, 0x8F, 0x73, 0x99, 0x27, 0x84, 0x8F, 0xA6, 0x89, 0xB7, 0x89, 0x2C, 
   0x93, 0xA0, 0x89, 0xB1, 0x89, 0x8C, 0x91, 0x80, 0x64, 0x8C, 0x93, 0x93, 0x8D, 0x84, 0x8D, 0x98, 
   0x13, 0x06, 0xC0, 0x02, 0x88, 0xF3, 0x89, 0xE0, 0x2D, 0x80, 0x81, 0x8F, 0x7D, 0x80, 0x83, 0x08, 
   0x95, 0xCF, 0x93, 0xDF, 0x93, 0xEC, 0x01, 0x88, 0x8D, 0x88, 0x23, 0xC9, 0xF0, 0xEA, 0x89, 0xFB, 
   0x89, 0x80, 0x81, 0x85, 0xFD, 0x05, 0xC0, 0xA8, 0x89, 0xB9, 0x89, 0x8C, 0x91, 0x86, 0xFD, 0x0F, 
   0xC0, 0x0F, 0xB6, 0x07, 0xFC, 0xF5, 0xCF, 0x80, 0x81, 0x85, 0xFF, 0xF2, 0xCF, 0xA8, 0x89, 0xB9, 
   0x89, 0x8C, 0x91, 0x85, 0xFF, 0xED, 0xCF, 0xCE, 0x01, 0x0E, 0x94, 0xAF, 0x00, 0xE7, 0xCF, 0xDF, 
   0x91, 0xCF, 0x91, 0x08, 0x95, 0xCF, 0x92, 0xDF, 0x92, 0xFF, 0x92, 0x0F, 0x93, 0x1F, 0x93, 0xCF, 
   0x93, 0xDF, 0x93, 0x1F, 0x92, 0xCD, 0xB7, 0xDE, 0xB7, 0x6C, 0x01, 0x81, 0xE0, 0xD6, 0x01, 0x58, 
   0x96, 0x8C, 0x93, 0x58, 0x97, 0x5B, 0x96, 0x9C, 0x91, 0x5B, 0x97, 0x5C, 0x96, 0x8C, 0x91, 0x5C, 
   0x97, 0x98, 0x13, 0x07, 0xC0, 0x50, 0x96, 0xED, 0x91, 0xFC, 0x91, 0x51, 0x97, 0x80, 0x81, 0x85, 
   0xFD, 0x2E, 0xC0, 0xF6, 0x01, 0x03, 0x8D, 0x10, 0xE0, 0x0F, 0x5F, 0x1F, 0x4F, 0x0F, 0x73, 0x11, 
   0x27, 0xF0, 0x2E, 0xF6, 0x01, 0x84, 0x8D, 0xF8, 0x12, 0x11, 0xC0, 0x0F, 0xB6, 0x07, 0xFC, 0xF9, 
   0xCF, 0xD6, 0x01, 0x50, 0x96, 0xED, 0x91, 0xFC, 0x91, 0x51, 0x97, 0x80, 0x81, 0x85, 0xFF, 0xF1, 
   0xCF, 0xC6, 0x01, 0x69, 0x83, 0x0E, 0x94, 0xAF, 0x00, 0x69, 0x81, 0xEB, 0xCF, 0x83, 0x8D, 0xE8, 
   0x0F, 0xF1, 0x1D, 0xE3, 0x5A, 0xFF, 0x4F, 0x60, 0x83, 0xD6, 0x01, 0x5B, 0x96, 0x0C, 0x93, 0x5B, 
   0x97, 0x52, 0x96, 0xED, 0x91, 0xFC, 0x91, 0x53, 0x97, 0x80, 0x81, 0x80, 0x62, 0x0C, 0xC0, 0xD6, 
   0x01, 0x56, 0x96, 0xED, 0x91, 0xFC, 0x91, 0x57, 0x97, 0x60, 0x83, 0x50, 0x96, 0xED, 0x91, 0xFC, 
   0x91, 0x51, 0x97, 0x80, 0x81, 0x80, 0x64, 0x80, 0x83, 0x81, 0xE0, 0x90, 0xE0, 0x0F, 0x90, 0xDF, 
   0x91, 0xCF, 0x91, 0x1F, 0x91, 0x0F, 0x91, 0xFF, 0x90, 0xDF, 0x90, 0xCF, 0x90, 0x08, 0x95, 0xBF, 
   0x92, 0xCF, 0x92, 0xDF, 0x92, 0xEF, 0x92, 0xFF, 0x92, 0xCF, 0x93, 0xDF, 0x93, 0xEC, 0x01, 0x6A, 
   0x01, 0x7B, 0x01, 0xB2, 0x2E, 0xE8, 0x89, 0xF9, 0x89, 0x82, 0xE0, 0x80, 0x83, 0x41, 0x15, 0x81, 
   0xEE, 0x58, 0x07, 0x61, 0x05, 0x71, 0x05, 0xA1, 0xF0, 0x60, 0xE0, 0x79, 0xE0, 0x8D, 0xE3, 0x90, 
   0xE0, 0xA7, 0x01, 0x96, 0x01, 0x0E, 0x94, 0x47, 0x03, 0x21, 0x50, 0x31, 0x09, 0x41, 0x09, 0x51, 
   0x09, 0x56, 0x95, 0x47, 0x95, 0x37, 0x95, 0x27, 0x95, 0x21, 0x15, 0x80, 0xE1, 0x38, 0x07, 0x98, 
   0xF0, 0xE8, 0x89, 0xF9, 0x89, 0x10, 0x82, 0x60, 0xE8, 0x74, 0xE8, 0x8E, 0xE1, 0x90, 0xE0, 0xA7, 
   0x01, 0x96, 0x01, 0x0E, 0x94, 0x47, 0x03, 0x21, 0x50, 0x31, 0x09, 0x41, 0x09, 0x51, 0x09, 0x56, 
   0x95, 0x47, 0x95, 0x37, 0x95, 0x27, 0x95, 0xEC, 0x85, 0xFD, 0x85, 0x30, 0x83, 0xEE, 0x85, 0xFF, 
   0x85, 0x20, 0x83, 0x18, 0x8E, 0xEC, 0x89, 0xFD, 0x89, 0xB0, 0x82, 0xEA, 0x89, 0xFB, 0x89, 0x80, 
   0x81, 0x80, 0x61, 0x80, 0x83, 0xEA, 0x89, 0xFB, 0x89, 0x80, 0x81, 0x88, 0x60, 0x80, 0x83, 0xEA, 
   0x89, 0xFB, 0x89, 0x80, 0x81, 0x80, 0x68, 0x80, 0x83, 0xEA, 0x89, 0xFB, 0x89, 0x80, 0x81, 0x8F, 
   0x7D, 0x80, 0x83, 0xDF, 0x91, 0xCF, 0x91, 0xFF, 0x90, 0xEF, 0x90, 0xDF, 0x90, 0xCF, 0x90, 0xBF, 
   0x90, 0x08, 0x95, 0x1F, 0x92, 0x0F, 0x92, 0x0F, 0xB6, 0x0F, 0x92, 0x11, 0x24, 0x2F, 0x93, 0x8F, 
   0x93, 0x9F, 0x93, 0xEF, 0x93, 0xFF, 0x93, 0xE0, 0x91, 0x4E, 0x01, 0xF0, 0x91, 0x4F, 0x01, 0x80, 
   0x81, 0xE0, 0x91, 0x54, 0x01, 0xF0, 0x91, 0x55, 0x01, 0x82, 0xFD, 0x12, 0xC0, 0x90, 0x81, 0x80, 
   0x91, 0x57, 0x01, 0x8F, 0x5F, 0x8F, 0x73, 0x20, 0x91, 0x58, 0x01, 0x82, 0x17, 0x51, 0xF0, 0xE0, 
   0x91, 0x57, 0x01, 0xF0, 0xE0, 0xE2, 0x5C, 0xFE, 0x4F, 0x95, 0x8F, 0x80, 0x93, 0x57, 0x01, 0x01, 
   0xC0, 0x80, 0x81, 0xFF, 0x91, 0xEF, 0x91, 0x9F, 0x91, 0x8F, 0x91, 0x2F, 0x91, 0x0F, 0x90, 0x0F, 
   0xBE, 0x0F, 0x90, 0x1F, 0x90, 0x18, 0x95, 0x1F, 0x92, 0x0F, 0x92, 0x0F, 0xB6, 0x0F, 0x92, 0x11, 
   0x24, 0x2F, 0x93, 0x3F, 0x93, 0x4F, 0x93, 0x5F, 0x93, 0x6F, 0x93, 0x7F, 0x93, 0x8F, 0x93, 0x9F, 
   0x93, 0xAF, 0x93, 0xBF, 0x93, 0xEF, 0x93, 0xFF, 0x93, 0x8E, 0xE3, 0x91, 0xE0, 0x0E, 0x94, 0xAF, 
   0x00, 0xFF, 0x91, 0xEF, 0x91, 0xBF, 0x91, 0xAF, 0x91, 0x9F, 0x91, 0x8F, 0x91, 0x7F, 0x91, 0x6F, 
   0x91, 0x5F, 0x91, 0x4F, 0x91, 0x3F, 0x91, 0x2F, 0x91, 0x0F, 0x90, 0x0F, 0xBE, 0x0F, 0x90, 0x1F, 
   0x90, 0x18, 0x95, 0x8E, 0xE3, 0x91, 0xE0, 0x0E, 0x94, 0x74, 0x00, 0x21, 0xE0, 0x89, 0x2B, 0x09, 
   0xF4, 0x20, 0xE0, 0x82, 0x2F, 0x08, 0x95, 0x10, 0x92, 0x41, 0x01, 0x10, 0x92, 0x40, 0x01, 0x88, 
   0xEE, 0x93, 0xE0, 0xA0, 0xE0, 0xB0, 0xE0, 0x80, 0x93, 0x42, 0x01, 0x90, 0x93, 0x43, 0x01, 0xA0, 
   0x93, 0x44, 0x01, 0xB0, 0x93, 0x45, 0x01, 0x80, 0xE2, 0x91, 0xE0, 0x90, 0x93, 0x3F, 0x01, 0x80, 
   0x93, 0x3E, 0x01, 0x85, 0xEC, 0x90, 0xE0, 0x90, 0x93, 0x4B, 0x01, 0x80, 0x93, 0x4A, 0x01, 0x84, 
   0xEC, 0x90, 0xE0, 0x90, 0x93, 0x4D, 0x01, 0x80, 0x93, 0x4C, 0x01, 0x80, 0xEC, 0x90, 0xE0, 0x90, 
   0x93, 0x4F, 0x01, 0x80, 0x93, 0x4E, 0x01, 0x81, 0xEC, 0x90, 0xE0, 0x90, 0x93, 0x51, 0x01, 0x80, 
   0x93, 0x50, 0x01, 0x82, 0xEC, 0x90, 0xE0, 0x90, 0x93, 0x53, 0x01, 0x80, 0x93, 0x52, 0x01, 0x86, 
   0xEC, 0x90, 0xE0, 0x90, 0x93, 0x55, 0x01, 0x80, 0x93, 0x54, 0x01, 0x10, 0x92, 0x57, 0x01, 0x10, 
   0x92, 0x58, 0x01, 0x10, 0x92, 0x59, 0x01, 0x10, 0x92, 0x5A, 0x01, 0x08, 0x95, 0xCF, 0x92, 0xDF, 
   0x92, 0xEF, 0x92, 0xFF, 0x92, 0x0F, 0x93, 0x1F, 0x93, 0xCF, 0x93, 0xDF, 0x93, 0x7C, 0x01, 0x6A, 
   0x01, 0xEB, 0x01, 0x00, 0xE0, 0x10, 0xE0, 0x0C, 0x15, 0x1D, 0x05, 0x71, 0xF0, 0x69, 0x91, 0xD7, 
   0x01, 0xED, 0x91, 0xFC, 0x91, 0x01, 0x90, 0xF0, 0x81, 0xE0, 0x2D, 0xC7, 0x01, 0x09, 0x95, 0x89, 
   0x2B, 0x19, 0xF0, 0x0F, 0x5F, 0x1F, 0x4F, 0xEF, 0xCF, 0xC8, 0x01, 0xDF, 0x91, 0xCF, 0x91, 0x1F, 
   0x91, 0x0F, 0x91, 0xFF, 0x90, 0xEF, 0x90, 0xDF, 0x90, 0xCF, 0x90, 0x08, 0x95, 0x61, 0x15, 0x71, 
   0x05, 0x81, 0xF0, 0xDB, 0x01, 0x0D, 0x90, 0x00, 0x20, 0xE9, 0xF7, 0xAD, 0x01, 0x41, 0x50, 0x51, 
   0x09, 0x46, 0x1B, 0x57, 0x0B, 0xDC, 0x01, 0xED, 0x91, 0xFC, 0x91, 0x02, 0x80, 0xF3, 0x81, 0xE0, 
   0x2D, 0x09, 0x94, 0x80, 0xE0, 0x90, 0xE0, 0x08, 0x95, 0x6C, 0xE2, 0x71, 0xE0, 0x0C, 0x94, 0x86, 
   0x02, 0x0F, 0x93, 0x1F, 0x93, 0xCF, 0x93, 0xDF, 0x93, 0xEC, 0x01, 0x0E, 0x94, 0x86, 0x02, 0x8C, 
   0x01, 0xCE, 0x01, 0x0E, 0x94, 0x9C, 0x02, 0x80, 0x0F, 0x91, 0x1F, 0xDF, 0x91, 0xCF, 0x91, 0x1F, 
   0x91, 0x0F, 0x91, 0x08, 0x95, 0x08, 0x95, 0x0E, 0x94, 0x0C, 0x03, 0x0E, 0x94, 0xB2, 0x02, 0x0E, 
   0x94, 0x5F, 0x00, 0xC1, 0xEA, 0xD0, 0xE0, 0x0E, 0x94, 0x6E, 0x00, 0x20, 0x97, 0xE1, 0xF3, 0x0E, 
   0x94, 0xA1, 0x00, 0xF9, 0xCF, 0x1F, 0x92, 0x0F, 0x92, 0x0F, 0xB6, 0x0F, 0x92, 0x11, 0x24, 0x2F, 
   0x93, 0x3F, 0x93, 0x8F, 0x93, 0x9F, 0x93, 0xAF, 0x93, 0xBF, 0x93, 0x80, 0x91, 0xDC, 0x01, 0x90, 
   0x91, 0xDD, 0x01, 0xA0, 0x91, 0xDE, 0x01, 0xB0, 0x91, 0xDF, 0x01, 0x30, 0x91, 0xDB, 0x01, 0x23, 
   0xE0, 0x23, 0x0F, 0x2D, 0x37, 0x20, 0xF4, 0x01, 0x96, 0xA1, 0x1D, 0xB1, 0x1D, 0x05, 0xC0, 0x26, 
   0xE8, 0x23, 0x0F, 0x02, 0x96, 0xA1, 0x1D, 0xB1, 0x1D, 0x20, 0x93, 0xDB, 0x01, 0x80, 0x93, 0xDC, 
   0x01, 0x90, 0x93, 0xDD, 0x01, 0xA0, 0x93, 0xDE, 0x01, 0xB0, 0x93, 0xDF, 0x01, 0x80, 0x91, 0xE0, 
   0x01, 0x90, 0x91, 0xE1, 0x01, 0xA0, 0x91, 0xE2, 0x01, 0xB0, 0x91, 0xE3, 0x01, 0x01, 0x96, 0xA1, 
   0x1D, 0xB1, 0x1D, 0x80, 0x93, 0xE0, 0x01, 0x90, 0x93, 0xE1, 0x01, 0xA0, 0x93, 0xE2, 0x01, 0xB0, 
   0x93, 0xE3, 0x01, 0xBF, 0x91, 0xAF, 0x91, 0x9F, 0x91, 0x8F, 0x91, 0x3F, 0x91, 0x2F, 0x91, 0x0F, 
   0x90, 0x0F, 0xBE, 0x0F, 0x90, 0x1F, 0x90, 0x18, 0x95, 0x78, 0x94, 0x84, 0xB5, 0x82, 0x60, 0x84, 
   0xBD, 0x84, 0xB5, 0x81, 0x60, 0x84, 0xBD, 0x85, 0xB5, 0x82, 0x60, 0x85, 0xBD, 0x85, 0xB5, 0x81, 
   0x60, 0x85, 0xBD, 0xEE, 0xE6, 0xF0, 0xE0, 0x80, 0x81, 0x81, 0x60, 0x80, 0x83, 0xE1, 0xE8, 0xF0, 
   0xE0, 0x10, 0x82, 0x80, 0x81, 0x82, 0x60, 0x80, 0x83, 0x80, 0x81, 0x81, 0x60, 0x80, 0x83, 0xE0, 
   0xE8, 0xF0, 0xE0, 0x80, 0x81, 0x81, 0x60, 0x80, 0x83, 0xE1, 0xEB, 0xF0, 0xE0, 0x80, 0x81, 0x84, 
   0x60, 0x80, 0x83, 0xE0, 0xEB, 0xF0, 0xE0, 0x80, 0x81, 0x81, 0x60, 0x80, 0x83, 0xEA, 0xE7, 0xF0, 
   0xE0, 0x80, 0x81, 0x84, 0x60, 0x80, 0x83, 0x80, 0x81, 0x82, 0x60, 0x80, 0x83, 0x80, 0x81, 0x81, 
   0x60, 0x80, 0x83, 0x80, 0x81, 0x80, 0x68, 0x80, 0x83, 0x10, 0x92, 0xC1, 0x00, 0x08, 0x95, 0xA1, 
   0xE2, 0x1A, 0x2E, 0xAA, 0x1B, 0xBB, 0x1B, 0xFD, 0x01, 0x0D, 0xC0, 0xAA, 0x1F, 0xBB, 0x1F, 0xEE, 
   0x1F, 0xFF, 0x1F, 0xA2, 0x17, 0xB3, 0x07, 0xE4, 0x07, 0xF5, 0x07, 0x20, 0xF0, 0xA2, 0x1B, 0xB3, 
   0x0B, 0xE4, 0x0B, 0xF5, 0x0B, 0x66, 0x1F, 0x77, 0x1F, 0x88, 0x1F, 0x99, 0x1F, 0x1A, 0x94, 0x69, 
   0xF7, 0x60, 0x95, 0x70, 0x95, 0x80, 0x95, 0x90, 0x95, 0x9B, 0x01, 0xAC, 0x01, 0xBD, 0x01, 0xCF, 
   0x01, 0x08, 0x95, 0xEE, 0x0F, 0xFF, 0x1F, 0x05, 0x90, 0xF4, 0x91, 0xE0, 0x2D, 0x09, 0x94, 0xF8, 
   0x94, 0xFF, 0xCF, 0x41, 0x53, 0x43, 0x49, 0x49, 0x20, 0x54, 0x61, 0x62, 0x6C, 0x65, 0x20, 0x7E, 
   0x20, 0x43, 0x68, 0x61, 0x72, 0x61, 0x63, 0x74, 0x65, 0x72, 0x20, 0x4D, 0x61, 0x70, 0x00, 0x00, 
   0x00, 0x00, 0x00, 0xF2, 0x00, 0x5E, 0x02, 0x74, 0x00, 0x8D, 0x00, 0x7F, 0x00, 0xD0, 0x00, 0x0D, 
   0x0A, 0x00, 0x6E, 0x61, 0x6E, 0x00, 0x69, 0x6E, 0x66, 0x00, 0x6F, 0x76, 0x66, 0x00, 0x2E, 0x00, 
   0x00, 0xFF, 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF , 0xFF ,
};

static void isp_usart_recv_timer(void *p)
{
	if (isp_step != ISP_NULL)
		return ;
	
	if(max_uart_idle_time >= 700) //700ms
	{
			max_uart_idle_time = 0;
			system_timer_counter[0] = 0;
			__enable_isp_serial();

		//
	}else{
		//关闭串口
		__disable_isp_serial();
		DEBUGL->debug("DISABLE ISP UARTRECV . r\\n");
		max_uart_idle_time = 0xFFFF;
		
		//
	}
	
	
}

static void isp_pargarm_entry(void *p)
{
	
	#define ISP_BUFFER_LEN 32
	#define ACK_OK 0x79
	#define ACK_NO 0x1F
	
	int error_code = 0;
	static unsigned char buffer[ISP_BUFFER_LEN];

	//启动一个定时器，用于UART负载控制
	{
		EI_TIMER *tmr = alloc_timer();
		tmr->ticks = 1;
		tmr->timeout = isp_usart_recv_timer;
		timer_start(tmr);
	}
	for(;;)
	{
		
		//RESET MCU TO ISP MOD
		switch(isp_step)
		{
			
			case ISP_NULL:
			{
				
				//设置串口模式
				
				//读取串口数据
				int serial_buffer_length = 0;
				memset(serial_buffer,0x0,sizeof(serial_buffer));
				serial_buffer_index = 0;
				
				serial_buffer_length = read_isp_serial_data(&serial_buffer[serial_buffer_index],sizeof(serial_buffer),RT_TICK_PER_SECOND/10);
				while(serial_buffer_length > 0)
				{
					serial_buffer_index += serial_buffer_length;
					
					if (serial_buffer_length >= sizeof(serial_buffer))
						break;
					
					serial_buffer_length = read_isp_serial_data(&serial_buffer[serial_buffer_index],sizeof(serial_buffer) - serial_buffer_index,RT_TICK_PER_SECOND/10);
				}
				
				if (serial_buffer_index > 0)
				{
					system_timer_counter[0] = 0; //每次接收到串口数据就把它清 0
					DEBUGL->debug("recv isp uart data %d \r\n",serial_buffer_index);
					write_resp_at(serial_at_str_handle(serial_buffer , serial_buffer_index));
				}else{
					//如果发现超过最长的空闲时间，则赋值
					if (system_timer_counter[0] > max_uart_idle_time){max_uart_idle_time = system_timer_counter[0];};
				}
				//serial_buffer_index

				
				//stm32isp_handshake_cnt = 0;
				
				//判断输出类型是不是字符串
				//判断真的是不是字符串
				//
				//readserial_data;
				//xmpp_send_msg_delay
				
				//is_string();
				//isp_step = ISP_HANDSHAKE_STM32;
				break;
			}
			//握手
			case ISP_HANDSHAKE_STM32:
			{
				
				//设置串口模式
				__config_to_stm32_isp_serial_mod();
				//连接STM32
				DEBUGL->debug("handshake!\r\n");

				DEBUGL->debug("reset target mcu!\r\n");
				RUNNING_ISP;
				reset_target_mcu();
				DEBUGL->debug("reset finish\r\n");
				
				rt_thread_sleep(100);
				stm32isp_ack_cnt = 0;
				isp_step = ISP_HANDSHAKE_STM32_2;
				break;
			}
			
			case ISP_HANDSHAKE_STM32_2:
			{
				stm32isp_handshake_cnt ++;
				stm32isp_ack_cnt ++;

				memset(buffer,0x0,ISP_BUFFER_LEN);
				buffer[0] = 0x7f;
				if (write_isp_buffer_ack(buffer,1) == ACK_OK)
				{
					char str[64];
					stm32isp_handshake_cnt = 0;
					isp_step = ISP_ERASE_FLASH_FLAG;
				}else
				{
					char str[64];
					
					if(stm32isp_ack_cnt > STM32_ISP_HANDSHAKE_COUNT)
					{
						stm32isp_handshake_cnt ++;
						if (stm32isp_handshake_cnt > STM32_ISP_HANDSHAKE_COUNT)
						{
							
							snprintf(str,sizeof(str),"+ISP=2");
							xmpp_send_msg(xmpp_sender,str);
							stm32isp_handshake_cnt = 0;
							goto program_err;
						}else{
							isp_step = ISP_HANDSHAKE_STM32;
						}
					}
				}
				
				
				break;
			}
			//擦除
			case ISP_ERASE_FLASH_FLAG:
			{
				//擦除FLASH
				
				DEBUGL->debug("erase flash\r\n");
				memset(buffer,0x0,ISP_BUFFER_LEN);
				buffer[0] = 0x43;
				buffer[1] = 0xbc;
				if (write_isp_buffer_ack(buffer,2) == ACK_OK)
				{
					isp_step = ISP_ERASE_FLAHH_ALL;
				}else
				{
					isp_step = ISP_HANDSHAKE_STM32;
				}
				
				break;
			}
			//全部擦除
			case ISP_ERASE_FLAHH_ALL:
			{
				//全部擦除
				DEBUGL->debug("erase flash all\r\n");
				memset(buffer,0x0,ISP_BUFFER_LEN);
				buffer[0] = 0xff;
				buffer[1] = 0x00;
				if (write_isp_buffer_ack(buffer,2) == ACK_OK)
				{
					isp_step = ISP_PROGRAM;
				}else
				{
					isp_step = ISP_HANDSHAKE_STM32;
				}
				break;
			}
			//编程
			case ISP_PROGRAM:
			{

				#define FLASH_BASEADDR (0x8000000)
				unsigned int FileLen = isp_program_size ; //DOWNLOAD_SIZE;//1024*100; //100k 下载目标文件的长度
				unsigned int sendnum = 0;
				unsigned int Address = 0x0;
				unsigned char *Adr;
				unsigned char AdrXOR;
				unsigned char datnum = 0;
				unsigned char Dat;
				unsigned char DatXOR;
				unsigned char i,j,k;
				//unsigned char *ProgramPtr = (unsigned char*)0x08000000;//DOWNLOAD_ADDR; //下载目标文件的地址
				const unsigned char *ProgramPtr = (unsigned char*)CHIP_DOWNLOAD_ADDR; //下载目标文件的地址
				unsigned int pread = 0;
				unsigned char error_cnt = 0;
		
				char str[64];
				
				//如果下载内容过大，则放弃
				if (FileLen > CHIP_DOWNLOAD_SIZE)
				{
					FileLen = CHIP_DOWNLOAD_SIZE;
				}
				
				if (DOWNLOAD_TYPE == 2)
				{
					FileLen = start_get_down_zone_spiflash();
				}
				
				for(;;)
				{
					
					//如果错误的太多了
					if (error_cnt >= 10)
						isp_step = ISP_PROGRAM_ERROR;
					
					memset(buffer,0x0,ISP_BUFFER_LEN);
					buffer[0] = 0x31;
					buffer[1] = 0xce;
					if (write_isp_buffer_ack(buffer,2) == ACK_OK)
					{
					}else
					{
						error_cnt++;
						DEBUGL->debug("0x31ce error\r\n");
						continue;
						//失败
						//goto program_err;
					}
					
					
					if(sendnum == FileLen)
					{
						isp_step = ISP_PROGRAM_DONE;
						break;
					}
				
					//计算当前要写入的地址
					Address = FLASH_BASEADDR + sendnum;
					Adr = (unsigned char *)&Address;
				
					AdrXOR = Adr[0];
//					for(k = 1; k < 4; k ++)
//					{
//						Address >>= 8;
//						Adr[k]=(u8)Address;
//						AdrXOR ^= Adr[k];
//					}
//					
					AdrXOR = Adr[3];
					AdrXOR ^= Adr[2];
					AdrXOR ^= Adr[1];
					AdrXOR ^= Adr[0];
					
					//发送地址和校验位
					memset(buffer,0x0,ISP_BUFFER_LEN);
					buffer[0] = Adr[3];
					buffer[1] = Adr[2];
					buffer[2] = Adr[1];
					buffer[3] = Adr[0];
					buffer[4] = AdrXOR;
					//地址发送成功
					if (write_isp_buffer_ack(buffer,5) == ACK_OK)
					{
					}else
					{
						
						DEBUGL->debug("write address error\r\n");
						error_cnt++;
						continue;
					}
					
					//开始进入下载流程
					#define WRITE_LEN 64*4 - 8
					if((FileLen-sendnum) >= WRITE_LEN)//????250???
					{
						datnum = WRITE_LEN-1;						
					}
					else
					{
						datnum = (unsigned char)(FileLen - sendnum - 1);												
					}
					
					//发送长度
					memset(buffer,0x0,ISP_BUFFER_LEN);
					buffer[0] = datnum;
					write_isp_buffer(buffer,1);
					
					
					DatXOR = datnum;
					
					
					for (j= 0; j <= datnum; j++)
					{
						
	
						
						if (DOWNLOAD_TYPE == 2)
						{
							//int get_down_zone_buffer_offset(unsigned int offset , unsigned char *out , short out_size)
							get_down_zone_buffer_offset(pread,&Dat,1);
						}else if (DOWNLOAD_TYPE == 1){
							Dat = ProgramPtr[pread];
						}
						
						memset(buffer,0x0,ISP_BUFFER_LEN);
						buffer[0] = Dat;
						write_isp_buffer(buffer,1);
						
						sendnum ++;
						pread ++;
						
						DatXOR ^= Dat;

					}
					//发送校验
					memset(buffer,0x0,ISP_BUFFER_LEN);
					buffer[0] = DatXOR;
					if (write_isp_buffer_ack(buffer,1) == ACK_OK)
					{
						
						char str[64];
						snprintf(str,sizeof(str),"+ISP=0,%d,%d",sendnum,FileLen);
						xmpp_send_msg_delay(xmpp_sender,str,5);
						
						DEBUGL->debug("write dat success %d\r\n",sendnum);
						
						//rt_thread_sleep(50);
						
					}else
					{
						//失败
						sendnum = sendnum - datnum - 1;// ????
						pread = pread - datnum - 1;
						
						DEBUGL->debug("write dat error\r\n");
						error_cnt++;
						continue;
					}
						
				}
				
				break;
			}
			//完成
			
			case ISP_PROGRAM_ERROR:
			{
				char str[64];
				//snprintf(str,sizeof(str),"Program ERR.");
				//xmpp_send_msg(xmpp_sender,str);
				DEBUGL->debug("ProgramERR !!!\r\n");
				snprintf(str,sizeof(str),"+ISP=2");
				xmpp_send_msg(xmpp_sender,str);
				goto program_err;
				break;
			}
			case ISP_PROGRAM_DONE:
			{
				//
				//程序下载完毕
				char str[64];
				DEBUGL->debug("ProgramDone !!!\r\n");
				RUNNING_FLASH;
				reset_target_mcu();
				
				snprintf(str,sizeof(str),"+ISP=1");
				xmpp_send_msg_delay(xmpp_sender,str,0);
				
				goto program_done;
				break;
			
			}
			
			case ISP_HANDSHAKE_AVR:
			{
				int ret,i;
				char send_index = 0;
				unsigned int fs = sizeof(avrfw);
				//write to spiflash
				
//				void init_write_flash(unsigned int addr , unsigned char *spiflash_buffer);
//				void append_w25x_byte(uint32_t BaseAddr , unsigned char data);
//				void append_w25x_append_buffer(unsigned char *buffer , int length);
//				void w25x_append_end(void);
//				void w25x_append_end_check(void);
				
//				init_write_flash(DOWNLOAD_ZONE_ADDRESS,0x0);
//				append_w25x_append_buffer(((unsigned char*)&fs),4);
//				append_w25x_append_buffer((unsigned char*)avrfw,sizeof(avrfw));
//				w25x_append_end();
				
				
				__config_to_default_serial_mod();
				for(i=0;i<10;i++)
				{
					//avr_rest();
					reset_target_mcu();
					rt_thread_sleep(50);
					
					send_index = 0;
					buffer[send_index++] = 0x30;
					buffer[send_index++] = 0x20;
					
					ret = write_isp_buffer_ack_v2(buffer,send_index,buffer,sizeof(buffer),200); //wait 20ms
					if (ret >= 2)
					{
						if ((buffer[ret-2] == 0x14))
						{
							isp_step = ISP_PROG_AVR;
							break;
						}
					}
					
				}
				if(isp_step == ISP_HANDSHAKE_AVR)
				{
					char str[64];
					snprintf(str,sizeof(str),"+ISP=2");
					xmpp_send_msg(xmpp_sender,str);
					goto program_err;
					//
				}
				break;
			}
			case ISP_PROG_AVR:
			{
				unsigned char *tmpbuf;
				unsigned short crc;
				int FileLen;
				unsigned short address;
				char buffer_index = 0;
				unsigned int write_address = 0x0;
				int ret;
				//unsigned char buffer[8];			// = {0x55,((unsigned char *)&address)[0],((unsigned char *)&address)[1],0x20};
				unsigned short fw_size = 0;
				unsigned short write_count = 0;

				FileLen = start_get_down_zone_spiflash();
				//FileLen = 0x777;
				#ifdef TEST_AVR_PROG
				FileLen = sizeof(avrfw);//1024 * 5;
				#endif
				fw_size = FileLen;

				// 判斷 FileLen 的長度
				
				for(;;)
				{
					
					#define SENDCMD_TIMEOUT 20
					unsigned char pkgsize = 0;
					address = write_address;
					
					if ((fw_size - write_count) >= 128)
					{
						pkgsize = 128;
					}else{
						pkgsize = fw_size - write_count;
					}
					
					
					buffer_index = 0;
					buffer[buffer_index++] = 0x55;
					buffer[buffer_index++] = ((unsigned char *)&address)[0];
					buffer[buffer_index++] = ((unsigned char *)&address)[1];
					buffer[buffer_index++] = 0x20;
					clearbuffer();
					ret = write_isp_buffer_ack_v2(buffer,buffer_index,buffer,sizeof(buffer),SENDCMD_TIMEOUT); //wait 20ms
					
					if (ret >= 2)
					{
						if (buffer[ret-2] == 0x14)
						{
							rt_kprintf("write address succ %x \r\n",address);
							//寫入地址完成
						}else{
							rt_kprintf("write address err %x \r\n",address);
							continue;
						}
					
						
					}else
					{
						rt_kprintf("write address err2 %02x  %d\r\n",buffer[0],ret);
						continue;
					}
					
					#ifndef TEST_AVR_PROG
					tmpbuf = rt_malloc(128);
					if (tmpbuf > 0)
					{
						
						get_down_zone_buffer_offset(write_count,tmpbuf,pkgsize);
					
						//開始少些Flash
						crc = sdk_stream_crc16_calc(tmpbuf,pkgsize);
						
						buffer_index = 0;
						buffer[buffer_index++] = 0x64;
						buffer[buffer_index++] = ((unsigned char*)&crc)[0];
						buffer[buffer_index++] = ((unsigned char*)&crc)[1];
						buffer[buffer_index++] = pkgsize;
						
						
						//寫入協議頭
						rt_device_write(isp_serial_fd,0,buffer,buffer_index);


						
						//判断FW剩余字节数
						if ((fw_size - write_count) >= 128)
						{
							//avrfw
							//get_down_zone_buffer_offset(write_count,tmpbuf,128);
							rt_device_write(isp_serial_fd,0,tmpbuf,128);
						}else{
							int i=0;
							int surplus_size = fw_size - write_count;
							//get_down_zone_buffer_offset(write_count,tmpbuf,surplus_size);
							rt_device_write(isp_serial_fd,0,tmpbuf,surplus_size);
							//不足的后面补FF
							for(i=0;i<(128 - surplus_size);i++)
							{
								unsigned char XFF = 0xFF;
								rt_device_write(isp_serial_fd,0,&XFF,1);
							}
						}
						
						rt_free(tmpbuf);
					}else{
						//申请失败
						isp_step = ISP_PROG_AVR_ERR;
						continue;
						
					}
#else					
					//判断FW剩余字节数
					if ((fw_size - write_count) >= 128)
					{
						//avrfw
						rt_device_write(isp_serial_fd,0,avrfw+write_count,128);
					}else{
						int i=0;
						int surplus_size = fw_size - write_count;
						
						rt_device_write(isp_serial_fd,0,avrfw+write_count,surplus_size);
						//不足的后面补FF
						for(i=0;i<(128 - surplus_size);i++)
						{
							unsigned char XFF = 0xFF;
							rt_device_write(isp_serial_fd,0,&XFF,1);
						}
					}
#endif
					
					clearbuffer();
					
					//發送結束符
					{
						unsigned char X20 = 0x20;
						rt_device_write(isp_serial_fd,0,&X20,1);
					}
					
					ret = write_isp_buffer_ack_v2(buffer,buffer_index,buffer,sizeof(buffer),SENDCMD_TIMEOUT); //wait 20ms
					
					if (ret >= 2)
					{
						if (buffer[ret-2] == 0x14)
						{
							//ack ok
							//
							
							rt_kprintf("write block succ %x \r\n",write_count);
							
							//isp_step = ISP_PROG_AVR;
						}else{
							rt_kprintf("write block err %x \r\n",write_count);
							continue;
						}
						
					}else
					{
						rt_kprintf("write block err2 %02x  %d\r\n",buffer[0],ret);
						continue;
					}
					
					write_count += 128;		//固件寫入字節計數器
					write_address += 64;	//扇區計數器
					
					
					
					//烧录完成后，检查写入是否成功
					if(write_count >= fw_size)
					{
						//ISP完成了
						
						buffer_index = 0;
						buffer[buffer_index++] = 0x40;
						buffer[buffer_index++] = 0x20;
						clearbuffer();
						
						ret = write_isp_buffer_ack_v2(buffer,buffer_index,buffer,sizeof(buffer),SENDCMD_TIMEOUT); //wait 20ms
						
						if (ret >= 2)
						{
							if (buffer[ret-2] == 0x14)
							{
								rt_kprintf("done prog succ %x \r\n",0);
								isp_step = ISP_PROG_AVR_A;
								break;
								//寫入地址完成
							}else{
								rt_kprintf("done prog err %x \r\n",0);
								isp_step = ISP_PROG_AVR_ERR;
								break;
							}
						
							
						}else
						{
							rt_kprintf("done prog err2 %02x  %d\r\n",buffer[0],0);
							isp_step = ISP_PROG_AVR_ERR;
							break;
						}
						
						break;
					} else {
						char str[64];
						snprintf(str,sizeof(str),"+ISP=0,%d,%d",write_count,FileLen);
						xmpp_send_msg_delay(xmpp_sender,str,5);
					}
					
				}
				
				break;
			}
			
			case ISP_PROG_AVR_A:
			{
				int ret;
				int i=0;
				int send_index;
				unsigned char buffer[32];
				for(i=0;i<5;i++)
				//for(;;)
				{
					//avr_rest();
					//reset_target_mcu();
					rt_thread_sleep(50);
					
					send_index = 0;
					buffer[send_index++] = 0x50;
					buffer[send_index++] = 0x20;
					
					ret = write_isp_buffer_ack_v2(buffer,send_index,buffer,sizeof(buffer),10); //wait 20ms
					if (ret >= 2)
					{
						if (buffer[ret-2] == 0x14)
						{
							rt_kprintf("ISP FINISH !!!!!!!!!!!!!!!!!!!!!!\r\n");
							isp_step = ISP_PROG_AVR_DONE;
							break;
						}
						else if (buffer[ret-2] == 0x15)
						{
							char str[64];
							snprintf(str,sizeof(str),"+ISP=0,%d,%d",99,100);
							xmpp_send_msg_delay(xmpp_sender,str,5);
							rt_kprintf("ISP PROG !!!!!!!!!!!!!!!!!!!!!!\r\n");
							i=0;
							continue;
						}
						else if (buffer[ret-2] == 0x16)
						{
							rt_kprintf("ISP ERROR !!!!!!!!!!!!!!!!!!!!!!\r\n");
							isp_step = ISP_PROG_AVR_ERR;
							break;
						}	
						
					}
				}
				//isp_step = ISP_PROG_AVR_ERR;
				
				
				break;
			}
			
			case ISP_PROG_AVR_DONE:
			{
				char str[64];
				snprintf(str,sizeof(str),"+ISP=1");
				xmpp_send_msg_delay(xmpp_sender,str,0);
				goto program_done;
				break;
			}
			case ISP_PROG_AVR_ERR:
			{
				char str[64];
				snprintf(str,sizeof(str),"+ISP=2");
				xmpp_send_msg(xmpp_sender,str);
				goto program_err;
				break;
			}
			
			default:
				break;
			
			
		}
		
		continue;
		
		//错误处理
		program_done:
		program_err:
		isp_step = ISP_NULL;
		__config_to_default_serial_mod();
		
	}
	
	
	return;
}

#include <rtthread.h>
static struct rt_thread isp_thread;
static rt_uint8_t isp_stack[ 1024 ];
void create_isp_thread(void)
{
	rt_err_t result;
	//初始化PPP服务
	result = rt_thread_init(&isp_thread,
													"mloop",
													isp_pargarm_entry,
													RT_NULL,
													(rt_uint8_t*)&isp_stack[0],
													sizeof(isp_stack),
													20,
													5);
	if (result == RT_EOK)
	{
			rt_thread_startup(&isp_thread);
	}
	
	//isp_init_gpio();
}

void init_isp_serial_port(char *name)
{
	open_isp_serial_port("uart2");
	//isp_pargarm_entry(0);
	
	
	//
}

static unsigned char rout_upload_cb_run_flag = 0;

static void rout_upload_cb(void *ctx)
{
	switch(http_tcp_status) 
	{
		case HTTP_STATUS_UPLOADING:
		{
			char str[64];
			
			#define BLOCK_SIZE 512
			if (upload_content_length > upload_size)
			{
				//如果缓冲是空的，就获取
				if (httpopt.buf == 0)
				{
					httpopt.get_upload_data(&httpopt);
				}
				
				if (httpopt.buflen > 0)
				{
					if (tcp_write(tcp_client_buffer[HTTP_TCP_CLIENT].pcb,(unsigned char*)httpopt.buf,httpopt.buflen,TCP_WRITE_FLAG_COPY) == ERR_OK)
					{
						SET_HTTP_ACTIVE;

						upload_size += httpopt.buflen;
						DEBUGL->debug("XXXXXXXXXXXXXXXXXXXXXXXXXXXXX %d %d \r\n",upload_size,upload_content_length);
						httpopt.buflen = 0;
						httpopt.buf  = 0;
					}
				}

				snprintf(str,sizeof(str),"+GETPIC=0,%d,%d",upload_size ,upload_content_length);
				xmpp_send_msg_delay(xmpp_sender,str,5);
				
			} else {
				http_tcp_status = HTTP_STATUS_UPLOADING_END;
				//
			}
			break;
		}
		
		case HTTP_STATUS_UPLOADING_END:
		{
			char ebuffer[96] = "";
			strcat(ebuffer,"\r\n");
			strcat(ebuffer,"------WebKitFormBoundaryeCGbABkhlBu3xcId--\r\n");
			//LOCK_TCPIP_CORE();
			if (tcp_write(tcp_client_buffer[HTTP_TCP_CLIENT].pcb,(unsigned char*)ebuffer,strlen(ebuffer),TCP_WRITE_FLAG_COPY) == ERR_OK)
			{
					http_tcp_status = HTTP_STATUS_UPLOAD_FINISH;
			}
			//UNLOCK_TCPIP_CORE();
			break;
		}
		
	}
	
	rout_upload_cb_run_flag = 0;
			
}
static void rout_upload(void)
{
	err_t ret;
	ret = tcpip_callback_with_block(rout_upload_cb,0,1);
	if (ret == ERR_OK)
	{
		rout_upload_cb_run_flag = 1;
		//
	}
}

void init_isp_http_cli(void)
{
	MY_TCP_Client_Init(&tcp_client_buffer[HTTP_TCP_CLIENT],http_host,http_port,0,1);
	INIT_TCPCLI_CB_FUNC(HTTP_TCP_CLIENT);
	//
}

void isp_http_tcp_routing(void *p)
{
	
	if (http_tcp_status != HTTP_STATUS_INIT)
	{
		if (GET_HTTP_ACTIVE > 30)
		{
			if (httpopt.done > 0)
			{
				httpopt.done(HTTP_TIMEOUT);
			}
			isp_stop_http();
			DEBUGL->debug("HTTP DOWNLOAD TIMEOUT ............ QUIT !!!! \r\n");
		}
	}
	
	switch(http_tcp_status)
	{
		case HTTP_SOCKET_BUSY:
			break;
		case HTTP_STATUS_INIT:
			
			//NULL
			break;
		case HTTP_STATUS_START:
			MY_TCP_Client_start(&tcp_client_buffer[HTTP_TCP_CLIENT]);
			http_tcp_status = HTTP_STATUS_STARTED;
			break;
		case HTTP_STATUS_STARTED:
			break;
		case HTTP_STATUS_UPLOADING:
		case HTTP_STATUS_UPLOADING_END:
			rout_upload();
			break;
		case HTTP_STATUS_UPLOAD_FINISH:
			break;
		case HTTP_STATUS_UPLOAD_DONE:
			isp_stop_http();
			break;
		case HTTP_DOWNLOADING:
			break;
		case HTTP_DOWNLOAD_FINISH:
			isp_stop_http();
			break;
		case HTTP_ERROR:
			http_tcp_status = HTTP_STATUS_START;
			break;
		case HTTP_FINISH:
			
		
			//如果当前是下载CONFIG
			if (http_task_type == HTTP_GET_CONFIG)
			{
				http_tcp_status = HTTP_STATUS_START;
				http_task_type = HTTP_GET_DOWNLOADFILE;
				//
			}else if (http_task_type == HTTP_GET_DOWNLOADFILE){
				isp_stop_http();
			}
			break;
		
	}
	//
}


void isp_stop_http(void)
{
	LOCK_TCPIP_CORE();
	MY_TCP_Client_stop(&tcp_client_buffer[HTTP_TCP_CLIENT]);
	UNLOCK_TCPIP_CORE();
	http_tcp_status = HTTP_STATUS_INIT;
	//
}

char isp_start_http_upload(char *web , char *sender)
{
	//__test_upload_pic_serialcam();
	//__test_upload_http();
	//sprintf(web,"http://www.easy-io.net/rtthread.bin");
	if (http_tcp_status == HTTP_STATUS_INIT)
	{
		if(strlen(web) > sizeof(http_path))
			return 0;
		
		http_task_type = HTTP_POST_UPLOADPIC;
		snprintf(xmpp_sender,sizeof(xmpp_sender),sender);
		
		GetHost(web,http_host,http_path,&http_port);
		
		sprintf(http_host,"www.easy-iot.cc");
		http_port = 80;
		init_isp_http_cli();
		//snprintf(download_from,sizeof(download_from),web);
		http_tcp_status = HTTP_STATUS_START;
		SET_HTTP_ACTIVE
		return 1;
	}
	return 0;
}


char isp_start_http_download(char *web , char *sender)
{
	
	//sprintf(web,"http://115.28.44.147/rtthread.bin");
	if (http_tcp_status == HTTP_STATUS_INIT)
	{
		if(strlen(web) > sizeof(http_path))
			return 0;
		
		http_task_type = HTTP_GET_DOWNLOADFILE;
		
		snprintf(xmpp_sender,sizeof(xmpp_sender),sender);

		GetHost(web,http_host,http_path,&http_port);
		init_isp_http_cli();
		//snprintf(download_from,sizeof(download_from),web);
		http_tcp_status = HTTP_STATUS_START;
		SET_HTTP_ACTIVE;
		return 1;
	}
	return 0;
}

//上传部分功能

#include "uart4_cam_drv.h"
#include "dcmi_ov2640.h"

static unsigned short JpegBuffer_put_total = 0;
static int get_upload_data(struct HTTP_OPT *opt)
{
	int retlength = 0;
	#define PUT_IMG_BLOCK_SIZE 512
	
	rt_kprintf("################### %d %d\r\n",JpegDataCnt,JpegBuffer_put_total);
	
	if ((JpegDataCnt - JpegBuffer_put_total) >= PUT_IMG_BLOCK_SIZE)
	{
		retlength = PUT_IMG_BLOCK_SIZE;
		
		opt->buflen = PUT_IMG_BLOCK_SIZE;
		opt->buf = JpegBuffer + JpegBuffer_put_total;
		JpegBuffer_put_total += retlength;

	} else {
		opt->buflen = (JpegDataCnt - JpegBuffer_put_total);
		retlength = opt->buflen;
		opt->buf = JpegBuffer + (JpegDataCnt - JpegBuffer_put_total);
		JpegBuffer_put_total = JpegDataCnt;
	}
	
	debug_buf("PIC BLOCK ",opt->buf,opt->buflen);
	
	return retlength;
}
void __test_upload_http(void)
{
	JpegBuffer_put_total = 0;
	memset(&httpopt,0x0,sizeof(httpopt));
	httpopt.uploadlenth = JpegDataCnt;
	httpopt.get_upload_data = get_upload_data;
	//
}

static int get_upload_serialcam_data(struct HTTP_OPT *opt)
{
	int length = 0;
	length = getImg(512);
	opt->buf = uartrx_buffer + 5;
	opt->buflen = length;
	debug_buf("PIC BLOCK ",opt->buf,opt->buflen);
	return length;
}


void __test_upload_pic_serialcam(int pic_length)
{
	//int pic_length = xxpaizhao();
	JpegBuffer_put_total = 0;
	memset(&httpopt,0x0,sizeof(httpopt));
	httpopt.uploadlenth = pic_length;
	DEBUGL->debug(" pic length %d \r\n",pic_length);
	httpopt.get_upload_data = get_upload_serialcam_data;
	//
}

