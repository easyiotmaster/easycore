#include "lwip_raw_api_tcp_template.h"
#include "XMPP/xmpp.h"
#include "lwip/tcpip.h"
#include "SHA1/sha1_password.h"
#include "gnss.h"
#include "led.h"
#include "MMA845X.h"
#include "app_timer.h"
#include "common.h"
#include "app_timer.h"
#include "my_stdc_func/my_stdc_func.h"
#include <stdio.h>
#include "isp_http_tcp.h"
#include "flash_config.h"


#define xmpp_tcp_client tcp_client_buffer[XMPP_TCP_CLIENT]
#define PRESENCE_STATUS_STRLEN 512

static unsigned int last_recv_vaild_pkg_timep = 0;;

static void __xmpp_srv_connected(void *arg)
{
	
	last_recv_vaild_pkg_timep = app_timer_data.app_timer_second;
	connected_xmpp_event();
	//
}

static void __xmpp_srv_disconnect(void *arg)
{
	disconn_xmpp_event();
	//
}

static void __xmpp_srv_recv(void *arg , unsigned char *buf , int len)
{
	recv_data_event((char*)buf,len);
	//
}


static void __xmpp_srv_routing(void *arg)
{
	
	LOCK_TCPIP_CORE();
	//如果是连接状态的时候就检查多久没连接了
	if (TCP_CONNECTED == xmpp_tcp_client.active_status)
	{
		
		
		if (xmpp_pma.xmpp_status == XMPP_BINDSUCC)
		{
			//如果正常登陆到了服务器
			if ((app_timer_data.app_timer_second - last_recv_vaild_pkg_timep) > 60*10)
			{
				xmpp_send_ping();
				//
			}
			
			if ((app_timer_data.app_timer_second - last_recv_vaild_pkg_timep) > 60*11)
			{
				MY_TCP_Client_stop(&xmpp_tcp_client);
				//
			}
			
		}else
		{
			//如果未正常登陆到服务器
			//
			
			if ((app_timer_data.app_timer_second - last_recv_vaild_pkg_timep) > 20)
			{
				MY_TCP_Client_stop(&xmpp_tcp_client);
			}
			
		}
	}
	
	UNLOCK_TCPIP_CORE();
	return ;

}

static void xmpp_disconn(void)
{
	MY_TCP_Client_pause(&xmpp_tcp_client);
}

static int xmpp_sendbuf(unsigned char *buf , int len)
{
	return MY_TCP_Client_write(&xmpp_tcp_client,buf,len);
	//
}

static void xmpp_rcvmsg(char *from , char *body)
{
	char *buffer;
	int bufferlen;
	xmpp_send_msg(from,(char*)process_xmpp_msg(from,body));
	
//send to uart
//	bufferlen = strlen(body) + strlen(from) + 32;
//	buffer = rt_malloc(bufferlen);
//	if (buffer > 0)
//	{
//		snprintf(buffer,bufferlen,"+MSG=\"%s\",\"%s\"\r\n",from,body);
//		put2ispuart((unsigned char*)buffer,strlen(buffer));
//		rt_free(buffer);
//	}
	
}

#include "dcmi_ov2640.h"
static void xmpp_login(void)
{

#ifdef DQSC
	if(READ_PIN_FANGCHAI == 0)
		xmpp_set_presence("online","alarm-0");
	else
		xmpp_set_presence("online","alarm-1");
#else
	
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
//	//snprintf(uconfig.presence_str,sizeof(uconfig.presence_str),"%s",presence);
//	if (strlen(uconfig.presence_str) > 0)
//	{
//		xmpp_set_presence("online",uconfig.presence_str);
//	}else{
//		
//	}
	
#endif
	
	//
}
static void recv_package(char *xml_pkg)
{
	last_recv_vaild_pkg_timep = app_timer_data.app_timer_second;
	return ;
}

#include "at_cmd.h"

#define XMPP_TCP_INIT 0

static char xmpp_tcp_status = XMPP_TCP_INIT;

void routing_xmpp_tcp(void)
{
	//
}

void init_xmpp_tcp(void)
{
	char xmpp_uname[32];
	struct XMPP_CB_FUNC func;
	//crtMV0xGJuoUAd3RC0RRtdPSXaw
	char *password = rt_malloc(64);
	if (password <= 0)
		return ;
	
	
	make_sha1_pwd(MODEM_IMEI,password);
	snprintf(xmpp_uname,sizeof(xmpp_uname),"imei%s",MODEM_IMEI);
	
	if((strlen(uconfig.openfire_username) > 0) && (strlen(uconfig.openfire_password) > 0))
	{
		DEBUGL->debug("USERNAME : %s \r\n",uconfig.openfire_username);
		DEBUGL->debug("PASSWORD : %s \r\n",uconfig.openfire_password);
		init_xmpp_cli(uconfig.openfire_username,/*"V0y7cuomRkzR3QEUtVFEC6xd0tM="*/uconfig.openfire_password,XMPP_SERVER_HOSTNAME,XMPP_SERVER_PORT);
		
	} else {
		init_xmpp_cli(XMPP_USERNAME,/*"V0y7cuomRkzR3QEUtVFEC6xd0tM="*/XMPP_PASSWORD,XMPP_SERVER_HOSTNAME,XMPP_SERVER_PORT);
		//init_xmpp_cli("test",/*"V0y7cuomRkzR3QEUtVFEC6xd0tM="*/"test",XMPP_SERVER_HOSTNAME,XMPP_SERVER_PORT);
		
		DEBUGL->debug("USERNAME : %s \r\n",XMPP_USERNAME);
		DEBUGL->debug("PASSWORD : %s \r\n",XMPP_PASSWORD);
		
		{
			char *regstr = rt_malloc(128);
			if (regstr > 0)
			{
				snprintf(regstr,128,"{\"USER\":\"%s\" , \"UPWD\":\"%s\"}",XMPP_USERNAME,XMPP_PASSWORD);
				DEBUGL->debug("%s\r\n",regstr);
				rt_free(regstr);
			}
		}
		//reglog#define VERSION_JSON_STR "{\"version_name\":\"V0.8\" , \"version\":45}"
	}
	
	rt_free(password);
	
	//创建一个断线重练的TCP
	MY_TCP_Client_Init(&xmpp_tcp_client,XMPP_SERVER_HOSTNAME,XMPP_SERVER_PORT,1,0);
	
	xmpp_tcp_client.connected = __xmpp_srv_connected;
	xmpp_tcp_client.disconn = __xmpp_srv_disconnect;
	xmpp_tcp_client.recv = __xmpp_srv_recv;
	xmpp_tcp_client.routing = __xmpp_srv_routing;
	
	memset(&func,0x0,sizeof(func));
	
	func.dis_connect = xmpp_disconn;
	func.send_buffer = xmpp_sendbuf;
	func.recv_message = xmpp_rcvmsg;
	func.login = xmpp_login;
	func.recv_package = recv_package;
	
	set_xmpp_cb_func(&func);
	
	MY_TCP_Client_start(&xmpp_tcp_client);
	
}
