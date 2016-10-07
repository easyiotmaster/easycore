#include "http_request_template.h"
#include "my_stdc_func/my_stdc_func.h"

struct http_request_obj http_obj[HTTP_REQUEST_OBJ_COUNT];


static int send_httpget_pkg(char *path , char *host)
{
	char *buffer = rt_malloc(128);
	if (buffer > 0)
	{
		
		snprintf(buffer,128,"GET /%s HTTP/1.1\r\nHOST: %s\r\nConnection: keep-alive\r\n\r\n",path,host);
		rt_kprintf("REQUEST : %s",buffer);
		MY_TCP_Client_write(&tcp_client_buffer[HTTP_TCP_CLIENT],(unsigned char*)buffer,strlen(buffer));
		rt_free(buffer);
		
		return 0;
	}
	
	return -1;
}


static void recv(void *arg,unsigned char *buf , int len)
{
	struct ctx_data_hdr hdr;
	struct http_request_obj *obj = (struct http_request_obj *)arg;
	hdr.ctx_type = HTTP_CTX_RECV;
	hdr.data = buf;
	hdr.data_length = len;
	obj->callback(&hdr);
}

static void connected(void *arg)
{
	struct ctx_data_hdr hdr;
	struct http_request_obj *obj = (struct http_request_obj *)arg;
	hdr.ctx_type = HTTP_CTX_CONNECTED;
	obj->callback(&hdr);
	
	send_httpget_pkg(obj->hostpath,obj->hostaddr);
	
}
static void disconn(void *arg)
{
	struct ctx_data_hdr hdr;
	struct http_request_obj *obj = (struct http_request_obj *)arg;
	hdr.ctx_type = HTTP_CTX_DISCONNECT;
	obj->callback(&hdr);
	stop_request(obj);
}
static void connerr(void *arg)
{
	struct ctx_data_hdr hdr;
	struct http_request_obj *obj = (struct http_request_obj *)arg;
	hdr.ctx_type = HTTP_CTX_CONNERR;
	obj->callback(&hdr);
	stop_request(obj);
}
static void routing(void *arg)
{
	struct http_request_obj *obj = (struct http_request_obj *)arg;
}

//¶¨ÆÚ¼ì²é
static void http_request_routing(void *p)
{
	int i=0;
	for(;i<HTTP_REQUEST_OBJ_COUNT;i++)
	{
		if (http_obj[i].http_req_status > HTTPREQUEST_INIT)
		{
			if ((app_timer_data.app_timer_second - http_obj[i].requeset_time) > http_obj[i].timeout)
			{
				struct ctx_data_hdr hdr;
				//struct http_request_obj *obj = (struct http_request_obj *)arg;
				hdr.ctx_type = HTTP_CTX_TIMEOUT;
				http_obj[i].callback(&hdr);
				//
			}
			//
		}
		//
	}
	//
}


void init_http_obj(struct http_request_obj *obj , struct TcpClientCon *cli)
{
	memset(obj,0x0,sizeof(struct http_request_obj));
	obj->tcpcli = cli;
	obj->tcpcli->arg = (void*)obj;
	obj->http_req_status = HTTPREQUEST_INIT;
	
	//MY_TCP_Client_Init(cli,http_host,htp_port,0);
	INIT_TCPCLI_PTR_CB_FUNC_(cli);
	
	ei_timer[HTTP_TEMPLATE_TIMR].ticks = 1;
	ei_timer[HTTP_TEMPLATE_TIMR].timeout = http_request_routing;
	ei_timer[HTTP_TEMPLATE_TIMR].enable = 1;
	
	//
}

void http_request(struct http_request_obj *obj ,char *web , int timeout)
{
	
	if (HTTPREQUEST_INIT == obj->http_req_status)
	{
		obj->timeout = timeout;
		obj->http_req_status = HTTPREQUEST_START;
		obj->http_req_status = app_timer_data.app_timer_second;
		GetHost(web,obj->hostaddr,obj->hostpath,&obj->hostport);
		MY_TCP_Client_Init(obj->tcpcli,obj->hostaddr,obj->hostport,0,1);
		MY_TCP_Client_start(obj->tcpcli);
	}
	//
}

void stop_request(struct http_request_obj *obj)
{
	obj->http_req_status = HTTPREQUEST_INIT;
	MY_TCP_Client_stop(obj->tcpcli);
	//
}


