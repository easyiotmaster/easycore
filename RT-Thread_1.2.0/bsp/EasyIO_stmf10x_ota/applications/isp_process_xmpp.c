
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <rtthread.h>

#include "XMPP/xmpp.h"

#include "gnss.h"
#include "app_ev.h"
//#include "nmea/generate.h"
#include "my_stdc_func/my_stdc_func.h"
#include "easyiot.h"
#include "atparse.h"
#include "serial_at.h"
#include "conv_hex_str.h"
#include "isp_http_tcp.h"
#include "flash_config.h"
#include "spiflash_config.h"
#include "w25q16.h"
#include "gsmmux/easyio_cmux.h"
#include "dcmi_ov2640.h"

#define RESP_BUFFER_LEN 512

struct XMPP_MSG_PM {
	char msg[64];
	void (*cb)(char*,char*,char*);
	void *p;
};
static unsigned char tmp_resp_buffer[RESP_BUFFER_LEN];

static void resp_hello(char *from , char *in , char *out)
{
	snprintf(out,RESP_BUFFER_LEN,"hi");
}


static void resp_hex(char *from , char *at , char*out)
{
	int ret;
	short convlen;
	char count;
	struct ATCMD_DATA ad;
	char tmp;
	char *str = 0;
	
	rt_kprintf("#####[%s]\r\n",at);
	
	ret = parse_at_cmd_string(at,&ad);
	
	if (ret == 0) // 表示解析AT成功
	{
		if (ad.param_count == 1)
		{
			if (ad.param[0].type == ATCMD_PARAM_STRING_TYPE)
			{
				str = rt_malloc(256);
				get_at_param_string(&ad,0,str,128);
				//HexToBuffer(
				convlen = HexToBuffer(str,(unsigned char*)str+128,128);
				//长度
				//writ
				
				put2ispuart((unsigned char*)str+128,convlen);
				
				goto atok;
				//
			}
			//
		}
	}
	
	aterr:
	snprintf(out,RESP_BUFFER_LEN,"ERROR");
	if (str > 0)
		rt_free(str);
	return;
	atok:
	snprintf(out,RESP_BUFFER_LEN,"OK");
	if (str > 0)
		rt_free(str);
	return;
	
	
}


static void resp_down(char *from , char *at , char*out)
{
	
	
		//int parse_at_cmd_string(char *at , struct ATCMD_DATA *ad);
	int ret;
	char count;
	struct ATCMD_DATA ad;
	char tmp;
	char *str = 0;
	
	rt_kprintf("#####[%s]\r\n",at);
	
	ret = parse_at_cmd_string(at,&ad);
	
	if (ret == 0) // 表示解析AT成功
	{
		if (ad.param_count == 1)
		{
			// 一个参数
			if (ad.param[0].type == ATCMD_PARAM_STRING_TYPE)
			{
				
				str = rt_malloc(256);
				if (str > 0)
				{
					get_at_param_string(&ad,0,str,256);
					
					ret = isp_start_http_download(str,from);
					if (ret == 1)
					{
						goto atok;
					} else {
						isp_stop_http();
						goto aterr;
					}
				
					goto atok;
				}

			}
			
		}
		//
	}
	
	aterr:
	snprintf(out,RESP_BUFFER_LEN,"ERROR");
	if (str > 0)
		rt_free(str);
	return;
	atok:
	snprintf(out,RESP_BUFFER_LEN,"OK");
	if (str > 0)
		rt_free(str);
	return;
	
	
	#if 0
	char ret;
	int p_len = get_parma_count(in);
	char *tmp;
	if (p_len == 1)
	{
		tmp = (char *)get_parma(0,in);
		//tmp 是要下载的文件名字
		snprintf(out,RESP_BUFFER_LEN,"DOWN [%s] OK",tmp);
		ret = isp_start_http_download(tmp,from);
		if (ret == 1)
		{
			snprintf(out,RESP_BUFFER_LEN,"OK");
		} else {
			isp_stop_http();
			snprintf(out,RESP_BUFFER_LEN,"ERROR");
		}
		
	}else
	{
		snprintf(out,RESP_BUFFER_LEN,"ERROR");
	}
	
	#endif
	
	//
}

//resp_ota
static void resp_ota(char *from , char *at , char*out)
{
	
	
		//int parse_at_cmd_string(char *at , struct ATCMD_DATA *ad);
	int ret;
	char count;
	struct ATCMD_DATA ad;
	char tmp;
	char *str = 0;
	
	rt_kprintf("#####[%s]\r\n",at);
	
	ret = parse_at_cmd_string(at,&ad);
	
	if (ret == 0) // 表示解析AT成功
	{
		if (ad.param_count == 1)
		{
			// 一个参数
			if (ad.param[0].type == ATCMD_PARAM_STRING_TYPE)
			{
				
				str = rt_malloc(256);
				if (str > 0)
				{
					get_at_param_string(&ad,0,str,256);
					
					uconfig.ota_flag = OTAFLAG_READY_UPDATA;
					snprintf(uconfig.ota_target_fw_md5,sizeof(uconfig.ota_target_fw_md5),"%s",str);
					__write_uconfig();
//					ret = isp_start_http_download(str,from);
//					if (ret == 1)
//					{
//						goto atok;
//					} else {
//						isp_stop_http();
//						goto aterr;
//					}
				
					goto atok;
				}

			}
			
		}
		//
	}
	
	aterr:
	snprintf(out,RESP_BUFFER_LEN,"ERROR");
	if (str > 0)
		rt_free(str);
	return;
	atok:
	snprintf(out,RESP_BUFFER_LEN,"OK");
	if (str > 0)
		rt_free(str);
	return;
	
	
	#if 0
	char ret;
	int p_len = get_parma_count(in);
	char *tmp;
	if (p_len == 1)
	{
		tmp = (char *)get_parma(0,in);
		//tmp 是要下载的文件名字
		snprintf(out,RESP_BUFFER_LEN,"DOWN [%s] OK",tmp);
		ret = isp_start_http_download(tmp,from);
		if (ret == 1)
		{
			snprintf(out,RESP_BUFFER_LEN,"OK");
		} else {
			isp_stop_http();
			snprintf(out,RESP_BUFFER_LEN,"ERROR");
		}
		
	}else
	{
		snprintf(out,RESP_BUFFER_LEN,"ERROR");
	}
	
	#endif
	
	//
}

static void wait_rst(void *p)
{
	NVIC_SystemReset();
		//rt_reset_system();
}
static void resp_rest(char *from , char *at , char*out)
{
	
	logout_xmpp();
	post_default_event_wait(wait_rst,0,0,0,500);
	snprintf(out,RESP_BUFFER_LEN,"OK");
	//
}

//resp_sqlresp
static void resp_sqlresp(char *from , char *at , char*out)
{
			//int parse_at_cmd_string(char *at , struct ATCMD_DATA *ad);
	int ret;
	char count;
	struct ATCMD_DATA ad;
	char tmp;
	char *str = 0;
	
	rt_kprintf("#####[%s]\r\n",at);
	
	ret = parse_at_cmd_string(at,&ad);
	
	if (ret == 0) // 表示解析AT成功
	{
		if (ad.param_count == 2)
		{
			// 一个参数
			if ((ad.param[0].type == ATCMD_PARAM_INTEGER_TYPE) && (ad.param[1].type == ATCMD_PARAM_INTEGER_TYPE))
			{
				str = rt_malloc(128);
				if (str>0)
				{
					snprintf(str,128,"+SQL=%d,%d\r\n",get_at_param_int(&ad,0),get_at_param_int(&ad,1));
					put2ispuart((unsigned char*)str,strlen(str));
					goto atok;
				}
				//
			}
			
		}
		//
	}
	
	aterr:
	snprintf(out,RESP_BUFFER_LEN,"");
	if (str > 0)
		rt_free(str);
	return;
	atok:
	snprintf(out,RESP_BUFFER_LEN,"");
	if (str > 0)
		rt_free(str);
	return;
	
	//snprintf(out,RESP_BUFFER_LEN,"OK");
	//
}



static void test(char *from , char *in , char*out)
{
	if (strstr(in,"crash"))
	{
		volatile char dummy = 0;                                                  \
    rt_kprintf("assert failed at %s:%d \n" , __FUNCTION__, __LINE__);\
    while (dummy == 0);
	}
	
	if (strstr(in,"upload"))
	{
		//__test_upload_pic_serialcam();
		//201600000000.jpg
		//snprintf(out,RESP_BUFFER_LEN,"http://115.28.44.147/upload/upload/201600000000.jpg");
		//isp_start_http_upload(0,from);
	}
	
	if (strstr(in,"paizhao"))
	{
		xxpaizhao();
//		ov2640_flashlight_power(1);
//		rt_thread_sleep(50);
//		OV_DISABLE_IT;
//		readimg();
//		OV_ENABLE_IT;
//		ov2640_flashlight_power(0);
		snprintf(out,RESP_BUFFER_LEN,"paizhaook");
	}
	
	if (strstr(in,"ota"))
	{
		NVIC_SystemReset();
		
		//
	}
	//
}

static void resp_isp(char *from , char *in , char*out)
{

	int ret;
	char count;
	struct ATCMD_DATA ad;
	char tmp;
	char *str = 0;
	int devid;
	
	ret = parse_at_cmd_string(in,&ad);

	if (ret == 0) // 表示解析AT成功
	{
		if (ad.param_count == 2)
		{
			
			str = rt_malloc(256);
			if (str > 0)
			{
				devid = get_at_param_int(&ad,1);
				//根據不同的設備類型，進行不同的更新流程
				switch(devid)
				{
					case 0:
						set_xmpp_sender(from);
						set_isp_step(ISP_HANDSHAKE_STM32);
						goto atok;
						break;
					case 1:
						set_xmpp_sender(from);
						set_isp_step(ISP_HANDSHAKE_AVR);
						goto atok;
						break;
				}
				
			}
			
			//
		}else{
			goto aterr;
		}
	}
	
	aterr:
	snprintf(out,RESP_BUFFER_LEN,"ERROR");
	if (str>0)
	{
		rt_free(str);
	}
	return;
	atok:
	snprintf(out,RESP_BUFFER_LEN,"OK");
	if (str>0)
	{
		rt_free(str);
	}
	return;
	
	
	
	
}

static void resp_sig(char *from , char *in , char*out)
{
	//+SIG=
	extern unsigned char gsm_signal;
	int signal_mapping[] = {0,3,6,10,13,16,19,23,26,29,32,36,39,42,43,45,48,52,55,61,65,58,71,74,78,81,84,87,90,94,97,100 };
		if( gsm_signal >= 99 )
				gsm_signal = 0;
			if(gsm_signal > 31)
				gsm_signal = 31;
			
	snprintf(out,RESP_BUFFER_LEN,"+SIG=%d",signal_mapping[gsm_signal]);
	xmpp_send_msg(from,out);
	snprintf(out,RESP_BUFFER_LEN,"OK",signal_mapping[gsm_signal]);
}

static void resp_version(char *from , char *in , char*out)
{
	//
	//AT+CSQ
	//BOOTLOADER 版本，固件版本
	snprintf(out,RESP_BUFFER_LEN,"%d.%d %d.%d",1,1,1,1,1);
}


void uploadpic(char *from , char *in , char *out)
{
	//rt_kprintf("init read img\r\n");

	//int parse_at_cmd_string(char *at , struct ATCMD_DATA *ad);
	int ret,devid;
	char count;
	struct ATCMD_DATA ad;
	char tmp;
	char *str = 0;
	
	//rt_kprintf("#####[%s]\r\n",at);
	
	ret = parse_at_cmd_string(in,&ad);
	
	if (ret == 0) // 表示解析AT成功
	{
		if (ad.param_count == 1)
		{
			// 一个参数
			if (ad.param[0].type == ATCMD_PARAM_INTEGER_TYPE)
			{
				
				str = rt_malloc(256);
				if (str > 0)
				{
					devid = get_at_param_int(&ad,0);
					
					if (devid == 0)
					{

						char i=0;
						int pic_length;
						for(i=0;i<3;i++)
						{
							tingzhipaizhao();
							pic_length = xxpaizhao();
							if (pic_length > 0)
								break;
						}
						
						if (pic_length > 0)
						{
							__test_upload_pic_serialcam(pic_length);
							//debug_buf("PIC",JpegBuffer,JpegDataCnt);
							if (isp_start_http_upload(0,from) == 1)
							{
								goto atok;
								//snprintf(out,RESP_BUFFER_LEN,"OK");
							}
							else
							{
								goto aterr;
								//snprintf(out,RESP_BUFFER_LEN,"ERROR");
							}
						}else{
							goto aterr;
							//snprintf(out,RESP_BUFFER_LEN,"ERROR");
						}
						
						
						
						
					}
					
				}

			}
			
		}
		//
	}
	
	aterr:
	snprintf(out,RESP_BUFFER_LEN,"ERROR");
	if (str>0)
	{
		rt_free(str);
	}
	return;
	atok:
	snprintf(out,RESP_BUFFER_LEN,"OK");
	if (str>0)
	{
		rt_free(str);
	}
	return;
	
	
	
	
	DEVID_0:
	
	
	return ;
}



struct XMPP_MSG_PM xmpp_msg_pm[] = {
	{"hello",resp_hello,0},
	
	{"AT+HEX=",resp_hex,0},
	{"AT+DOWN=",resp_down,0},
	{"AT+ISP=",resp_isp,0},
	{"AT+GETPIC=",uploadpic,0},
	{"AT+OTA=",resp_ota,0},
	{"AT+REST",resp_rest,0},
	{"AT+SQLRESP",resp_sqlresp,0},
	{"AT+SIG",resp_sig,0},
	{"AT+VER",resp_version,0},
	{"test=",test,0},

};

#include "at_cmd.h"
#include "gsmmux/easyio_cmux.h"
static char *resp;
static int XMPPAT(const char *in , int len)
{
	snprintf(resp,64,"%s",in);
	if (strstr(resp,"\r\n"))
		return AT_RESP_OK;
	else
		return AT_RESP_ERROR;
	//
	
	
}

char *process_xmpp_msg(char *from , char *msg)
{
	int i=0;
	char *str;
	static char resp_str[RESP_BUFFER_LEN];
	
	at_xmpp_msg(from,msg);
	
	for(i=0;i<((sizeof(xmpp_msg_pm))/(sizeof(struct XMPP_MSG_PM)));i++)
	{
		if (strstr(msg,xmpp_msg_pm[i].msg) == msg)
		{
			xmpp_msg_pm[i].cb(from,msg,resp_str);
			rt_kprintf("## resp str: [%s]",resp_str);
			return resp_str;
		}
	}
	
	//check at cmd:
	if (strstr(msg,"AT+") == msg)
	{
		int at_cmd_ret_code;
		strcat(msg,"\r\n");
		resp = rt_malloc(64);
		memset(resp,0x0,64);
		
		if (resp <= 0)
		{
			goto ret;
		}
		
		cmux_at_command(2,msg,XMPPAT,RT_TICK_PER_SECOND,&at_cmd_ret_code);
		snprintf(resp_str,RESP_BUFFER_LEN,"%s",resp);
		
		//??
		rt_free(resp);
		return resp_str;
	}
	
	ret:
	
	resp_str[0] = 0x0;
	
	return resp_str;
}