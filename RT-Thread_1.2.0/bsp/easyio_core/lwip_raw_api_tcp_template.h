#ifndef __lwip_raw_api_template__
#define __lwip_raw_api_template__

#include "lwip/tcp.h"
#include "lwip/dns.h"
#include "lwip/tcpip.h"

#include "common.h"



#define TCP_RUN_TEST //rt_kprintf("#################### RUN_TEST FILE %s FUNC %s LINE %d \r\n",__FILE__,__FUNCTION__,__LINE__);

enum{
	TCP_PAUSE,//暂停
	
	TCP_INIT,
	TCP_DNS_START,
	TCP_DNS_FINISH,
	TCP_CONNECTING,
	TCP_CONNECTED,
	
	TCP_DNS_ERR,
	TCP_CONNECT_ERR,
	TCP_DISCONNECT,
	TCP_RECONNECT_DELAY,
	
};


struct TcpClientCon{
	unsigned char used;
	struct tcp_pcb *pcb;
	struct ip_addr ipaddr;
	long port;
	char active_status;
	char hostname[64];
	char connect_type;
	unsigned int reconnection_time;
	char keepalive;
	
	unsigned int recv_ack_timestamp;
	
	unsigned char connecting_count; //发起连接的次数
	
	void (*recv)(void *arg,unsigned char *buf , int len);		//收到数据的回掉函数
	void (*connected)(void *arg);														//成功连接到tcpserver的回掉函数
	void (*disconn)(void *arg);															//断开tcp连接的回掉函数
	void (*connerr)(void *arg);															//连接错误的回掉函数
	void (*routing)(void *arg);															//
	
	void *arg;																							//用户传入参数
	
};




extern struct TcpClientCon tcp_client_buffer[TCP_CLI_COUNT];
void init_tcp_client_buf(void);
void rout_tcp_client_buf(void);
void disconn_tcp_client_buf(void);

/**
	创建按一个TCP连接 type == 1 时如见检测到tcp连接中断，会自动重新连接 type == 0 时断线后不会重新连接
	keepalive > 0 代表开启，会在tcp空闲时自动发送心跳包确保tcp维持连接
*/
void MY_TCP_Client_Init(struct TcpClientCon *cli , char *hostname , long port , char type , char keepalive);

/**
  TCP状态机检查函数，需要系统定期调用，用户App无需关心
*/
void MY_TCP_Client_Routing(struct TcpClientCon *cli);

/**
  停止一个tcp连接
*/
void MY_TCP_Client_stop(struct TcpClientCon *cli);

/**
  开始一个tcp连接
*/
void MY_TCP_Client_start(struct TcpClientCon *cli);

/**
  暂停一个tcp连接，针对于类型为用户断线的tcp连接而言
*/
void MY_TCP_Client_pause(struct TcpClientCon *cli);

/**
  向一个tcp连接中写入数据
*/
err_t MY_TCP_Client_write(struct TcpClientCon *cli , unsigned char *buffer , int len);

#define INIT_TCPCLI_CB_FUNC(X) tcp_client_buffer[X].connected = connected;	\
	tcp_client_buffer[X].connerr = connerr;	\
	tcp_client_buffer[X].connerr = connerr;	\
	tcp_client_buffer[X].disconn = disconn;	\
	tcp_client_buffer[X].routing = routing;	\
	tcp_client_buffer[X].recv		 = recv;
	
#define INIT_TCPCLI_PTR_CB_FUNC_(X) X->connected = connected;	\
	X->connerr = connerr;	\
	X->connerr = connerr;	\
	X->disconn = disconn;	\
	X->routing = routing;	\
	X->recv		 = recv;

#endif
