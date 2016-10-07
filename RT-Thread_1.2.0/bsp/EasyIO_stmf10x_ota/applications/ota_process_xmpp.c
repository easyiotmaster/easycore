
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <rtthread.h>

#include "gnss.h"
#include "global_env.h"
#include "nmea/generate.h"
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

static void resp_getgprmc(char *from , char *in , char *out)
{
	#if 1
	get_gps_line("$GPRMC",(char*)tmp_resp_buffer,(unsigned int)RESP_BUFFER_LEN,(char*)gps_buffer_valid);
	#endif
	//nmea_gen_GPRMC((char*)tmp_resp_buffer,RESP_BUFFER_LEN,&gps_info.gprmc);
	snprintf(out,RESP_BUFFER_LEN,"getgprmc=%s",tmp_resp_buffer);
}

static void resp_getgprmc2(char *from , char *in , char *out)
{
	#if 1
	get_gps_line("$GPRMC",(char*)tmp_resp_buffer,(unsigned int)RESP_BUFFER_LEN,(char*)gps_buffer_valid);
	#endif
	//nmea_gen_GPRMC((char*)tmp_resp_buffer,RESP_BUFFER_LEN,&gps_info.gprmc);
	snprintf(out,RESP_BUFFER_LEN,"getgprmc=%s",tmp_resp_buffer);
}

static void resp_getgsv(char *from , char *in , char *out)
{
	
	//nmea_gen_GPGSV((char*)tmp_resp_buffer,RESP_BUFFER_LEN,&gps_info.gpgsv[0]);
	get_gps_line("$GPGSV",(char*)tmp_resp_buffer,(unsigned int)RESP_BUFFER_LEN,(char*)gps_buffer_tmp);
	snprintf(out,RESP_BUFFER_LEN,"getgsv=%s",tmp_resp_buffer);
}

static void resp_getgsa(char *from , char *in , char *out)
{
	
	
	//nmea_gen_GPGSA((char*)tmp_resp_buffer,RESP_BUFFER_LEN,&gps_info.gpgsa);
	
	#if 1
	get_gps_line("$GPGSA",(char*)tmp_resp_buffer,(unsigned int)RESP_BUFFER_LEN,(char*)gps_buffer_tmp);
	if (strlen((char*)tmp_resp_buffer) <= 0)
	{
		get_gps_line("$GNGSA",(char*)tmp_resp_buffer,(unsigned int)RESP_BUFFER_LEN,(char*)gps_buffer_tmp);
		if (strlen((char*)tmp_resp_buffer) > 0)
		{
			for(;;)
			{
				char *tmp = strstr(tmp_resp_buffer,"$GNGSA");
				if (tmp <= 0)
					break;
				tmp[2] = 'P';
			}
			//tmp_resp_buffer[2] = 'P';
			//
		}
	}
	#endif
	snprintf(out,RESP_BUFFER_LEN,"getgsa=%s",tmp_resp_buffer);
	//
}

static void resp_setbroadcastgps(char *from , char *in , char*out)
{
	int p_len = get_parma_count(in);
	char *tmp;
	if (p_len == 1)
	{
		tmp = (char *)get_parma(0,in);
		sscanf(tmp,"%d",&global_user_config.broadcastgps_time);
		
		rt_kprintf("set global_broadcastgps_time = %d\r\n",global_user_config.broadcastgps_time);
		
		snprintf(out,RESP_BUFFER_LEN,"setbroadcastgps=OK");
	}
	//
}

static void resp_setgpspower(char *from , char *in , char*out)
{
	int p_len = get_parma_count(in);
	
	if (p_len == 1)
	{
		snprintf(out,RESP_BUFFER_LEN,"setgpspower=OK");
	}
	//
}

static void resp_getgpsstatus(char *from , char *in , char*out)
{
	snprintf(out,RESP_BUFFER_LEN,"getgpsstatus=%d,%d,%d",0,1,2);
	//
}

static void resp_powersaving(char *from , char *in , char*out)
{
	
	int p_len = get_parma_count(in);
	
	if (p_len == 1)
	{
		snprintf(out,RESP_BUFFER_LEN,"powersaving=OK");
	}
	//
}

static void resp_getpowersaving(char *from , char *in , char*out)
{
	snprintf(out,RESP_BUFFER_LEN,"getpowersaving=%d",0);
	//
}


static void resp_reboot(char *from , char *in , char*out)
{
	char *tmp;
	int p_len = get_parma_count(in);
	if (p_len >= 1)
	{
		tmp = (char *)get_parma(0,in);
		sscanf(tmp,"%d",&global_reboot_time);
	}
	//global_reboot_time
	snprintf(out,RESP_BUFFER_LEN,"OK");
	//
}

static void resp_where(char *from , char *in , char*out)
{
	snprintf(out,RESP_BUFFER_LEN,"where=https://www.google.com/maps/preview?q=%f,%f",gps_info.google_LAT,gps_info.google_LON);
	//
}
#include "isp_http_tcp.h"
#include "ota_http_tcp.h"
static void resp_down(char *from , char *in , char*out)
{
	
	int p_len = get_parma_count(in);
	char *tmp;
	if (p_len == 1)
	{
		tmp = (char *)get_parma(0,in);
		//tmp 是要下载的文件名字
		snprintf(out,RESP_BUFFER_LEN,"DOWN [%s] OK",tmp);
		isp_stop_http_download();
		if (isp_start_http_download(tmp,from) == 0)
		{
			snprintf(out,RESP_BUFFER_LEN,"Restart program fw !");
			isp_restart_http_download();
		}else
		{
			snprintf(out,RESP_BUFFER_LEN,"Start program fw !");
		}
		
	}else
	{
		snprintf(out,RESP_BUFFER_LEN,"CMD ERROR");
	}
	
	//
}

static void resp_program(char *from , char *in , char*out)
{
	
	//#ifdef ENABLE_OTA
	//snprintf(out,RESP_BUFFER_LEN,"Start update fw !");
	#if 1
	if (isp_start_http_download(in,from) == 0)
	{
		snprintf(out,RESP_BUFFER_LEN,"Restart program fw !");
		isp_restart_http_download();
	}else
	{
		snprintf(out,RESP_BUFFER_LEN,"Start program fw !");
	}
	#endif

	
	
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
	
	if (strstr(in,"en_wd"))
	{
		
	}
	//
}

static void isp(char *from , char *in , char*out)
{
	
	//set_isp_step(ISP_HANDSHAKE_STM32);
	snprintf(out,RESP_BUFFER_LEN,"set isp step %d",ISP_HANDSHAKE_STM32);

	
}

struct XMPP_MSG_PM xmpp_msg_pm[] = {
	{"hello",resp_hello,0},
	{"down=",resp_down,0},
	{"prog",resp_program,0},
	{"isp",isp,0},
	{"test=",test,0},

};

char *process_xmpp_msg(char *from , char *msg)
{
	int i=0;
	static char resp_str[RESP_BUFFER_LEN];
	for(i=0;i<((sizeof(xmpp_msg_pm))/(sizeof(struct XMPP_MSG_PM)));i++)
	{
		if (strstr(msg,xmpp_msg_pm[i].msg) == msg)
		{
			xmpp_msg_pm[i].cb(from,msg,resp_str);
			rt_kprintf("## resp str: [%s]",resp_str);
			return resp_str;
		}
	}
	resp_str[0] = 0x0;
	
	return resp_str;
}