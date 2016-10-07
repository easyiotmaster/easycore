#include "serial_at.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "XMPP/xmpp.h"
#include <rtthread.h>
#include "flash_config.h"
#include "app_timer.h"
#include "BASE64/cbase64.h"
#include "my_stdc_func/my_stdc_func.h"
#include "my_stdc_func/debugl.h"
#include "JSON/cJSON.h"
#include "isp_http_tcp.h"
#include "atparse.h"
#include "conv_hex_str.h"
#include "flash_config.h"

#define RESPAT_STR_LENGTH 64
static char resp_str[RESPAT_STR_LENGTH];

static void at_msg(char *at)
{
	//int parse_at_cmd_string(char *at , struct ATCMD_DATA *ad);
	int ret;
	char count;
	struct ATCMD_DATA ad;
	char *str = 0;
	
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
					xmpp_set_presence("online",str);
					goto atok;
				}

			}
			
		}else if(ad.param_count == 2)
		{
			if ((ad.param[0].type == ATCMD_PARAM_STRING_TYPE) && (ad.param[1].type == ATCMD_PARAM_STRING_TYPE))
			{
				char *str = rt_malloc(256);
				if (str > 0)
				{
					get_at_param_string(&ad,1,str,64);
					get_at_param_string(&ad,0,str+64,256-64);
					xmpp_send_msg(str,str+64);
					goto atok;
				}
				//
			}
		}
		//
	}
	
	aterr:
	snprintf(resp_str,RESPAT_STR_LENGTH,"ERROR\r\n");
	if (str>0)
		rt_free(str);
	return;
	atok:
	snprintf(resp_str,RESPAT_STR_LENGTH,"OK\r\n");
	if (str>0)
		rt_free(str);
	return;
	
}

static void at_sql(char *at)
{
	//int parse_at_cmd_string(char *at , struct ATCMD_DATA *ad);
	int ret;
	char count;
	struct ATCMD_DATA ad;
	
	ret = parse_at_cmd_string(at,&ad);
	
	if (ret == 0) // 表示解析AT成功
	{
		if (ad.param_count == 2)
		{
			// 一个参数
			if ((ad.param[1].type == ATCMD_PARAM_STRING_TYPE) && (ad.param[0].type == ATCMD_PARAM_INTEGER_TYPE))
			{
				//去掉AT字符
				char *tb = rt_malloc(128+64);
				if (tb>0)
				{
					get_at_param_string(&ad,1,tb+64+32,64+32);
					snprintf(tb,128,"+SQL=%d,\"%s\"",get_at_param_int(&ad,0),tb+64+32);
					xmpp_set_presence("online",tb);
					rt_free(tb);
					goto atok;

				}

			}
			
		}
		//
	}
	
	aterr:
	snprintf(resp_str,RESPAT_STR_LENGTH,"ERROR\r\n");
	return;
	atok:
	snprintf(resp_str,RESPAT_STR_LENGTH,"OK\r\n");
	return;
}

static void at_confsrv(char *at)
{
	int ret;
	char *tb = 0;
	char count;
	struct ATCMD_DATA ad;
	
	ret = parse_at_cmd_string(at,&ad);
	
	if (ret == 0) // 表示解析AT成功
	{
		char *tb = rt_malloc(128);
		if (tb > 0)
		{
			if (ad.param_count == 0)
			{
				goto atok;
			}
			else if (ad.param_count == 5)
			{
				//hostname
				get_at_param_string(&ad,0,tb,128);
				snprintf(uconfig.openfire_hostname,sizeof(uconfig.openfire_hostname),"%s",tb);
				
				//port
				ret = get_at_param_int(&ad,1);
				uconfig.openfire_port = ret;
				
				get_at_param_string(&ad,2,tb,128);
				snprintf(uconfig.openfire_domain,sizeof(uconfig.openfire_domain),"%s",tb);
				
				get_at_param_string(&ad,3,tb,128);
				snprintf(uconfig.openfire_username,sizeof(uconfig.openfire_username),"%s",tb);
				
				get_at_param_string(&ad,4,tb,128);
				snprintf(uconfig.openfire_password,sizeof(uconfig.openfire_password),"%s",tb);
				
				__write_uconfig();
				
				goto atok;
				
			}
		}
	}
	aterr:
	snprintf(resp_str,RESPAT_STR_LENGTH,"ERROR\r\n");
	if (tb>0){rt_free(tb);};
	return;
	atok:
	snprintf(resp_str,RESPAT_STR_LENGTH,"+CONFSRV=%s,%d,%s,%s,%s\r\nOK\r\n",uconfig.openfire_hostname,uconfig.openfire_port,uconfig.openfire_domain,uconfig.openfire_hostname,uconfig.openfire_password);;
	if (tb>0){rt_free(tb);};
	return;
	//
}

static void at_at(char *at)
{
	snprintf(resp_str,sizeof(resp_str),"OK\r\n");
	//
}

static void at_status(char *at)
{
	snprintf(resp_str,sizeof(resp_str),"+STATUS=%d\r\nOK\r\n",xmpp_pma.xmpp_status);
}

static struct SERIAL_AT serial_at_func[] = {
	{"AT+MSG=",at_msg,0},										//发送message消息,如果不带有目的地址则认为是广播作为Presence格式发送出去
	{"AT+SQL=",at_sql,0},
	{"AT+CONFSRV",at_confsrv,0},
	{"AT\r",at_at,0},												//AT测试命令

};



static char *process_serial_at(const char *at,int length)
{
	int i=0;
	char *buf;
	
	
	
	for(i=0;i<((sizeof(serial_at_func))/(sizeof(struct SERIAL_AT)));i++)
	{
		if (strstr(at,serial_at_func[i].at) == at)
		{
			serial_at_func[i].cb((char*)at);
			return resp_str;
		}
	}
	resp_str[0] = 0x0;
	
//	filter_str((char*)at);
//	xmpp_set_presence("online",at);
	
	
	#define JSONBASE64_BUFFER_LEN 256
	buf = rt_malloc(JSONBASE64_BUFFER_LEN);
	
	if (buf > 0)
	{
		
		snprintf(buf,JSONBASE64_BUFFER_LEN,"+HEX=\"");
		//BufferToHex
		BufferToHex((unsigned char*)at,length,buf+6,JSONBASE64_BUFFER_LEN-8);
		strcat(buf,"\"");
		//if (tcp_client_buffer[XMPP_TCP_CLIENT])
		//xmpp_set_presence("online",buf);
		
		rt_free(buf);
		//
	}
	

	return resp_str;
}



char *serial_at_str_handle(char *atstr , int length)
{
	//return process_serial_json(atstr);
	char *p = process_serial_at(atstr,length);
	write_to_isp_serial(p,strlen(p));
	//
}


char *xmpp_at_str_handle(const char *atstr , int length)
{
	return process_serial_at(atstr,length);
	//
}

char *at_xmpp_msg(char *from , char *msg)
{
	char *str = rt_malloc(256);
	if (str > 0)
	{
		snprintf(str,256,"+MSG=\"%s\",\"%s\"\r\n",msg,from);
		write_to_isp_serial(str,strlen(str));
		rt_free(str);
	}
	return 0;
}
