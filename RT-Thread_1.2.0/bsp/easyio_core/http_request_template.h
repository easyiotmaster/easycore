#ifndef __http_request_temp__
#define __http_request_temp__

#include "lwip_raw_api_tcp_template.h"
#include "app_timer.h"

enum {
	HTTPREQUEST_INIT,
	HTTPREQUEST_START,
	HTTPREQUEST_DOWN,
	HTTPREQUEST_END,
};

enum {
	HTTP_CTX_CONNECTED,
	HTTP_CTX_DISCONNECT,
	HTTP_CTX_CONNERR,
	HTTP_CTX_RECV,
	HTTP_CTX_TIMEOUT,
};

struct ctx_data_hdr {
	int ctx_type;
	unsigned char *data;
	int data_length;
};

struct http_request_obj{
	int http_req_status;
	struct TcpClientCon *tcpcli;
	void (*callback)(struct ctx_data_hdr *ctx);
	
	char hostaddr[64];
	char hostpath[128];
	int hostport;
	
	unsigned int requeset_time;
	unsigned int timeout;
	//
};


#define HTTP_REQUEST_OBJ_COUNT 1
extern struct http_request_obj http_obj[HTTP_REQUEST_OBJ_COUNT];

/**
	初始化一个Httprequeset 对象
*/
extern void init_http_obj(struct http_request_obj *obj , struct TcpClientCon *cli);

/**
	执行一个Http请求
*/
extern void http_request(struct http_request_obj *obj , char *webaddress , int timeout);

/**
	结束一个Http请求
*/
extern void stop_request(struct http_request_obj *obj);

#endif

