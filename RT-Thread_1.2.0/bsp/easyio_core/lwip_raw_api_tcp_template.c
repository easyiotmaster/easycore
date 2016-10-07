#include "lwip_raw_api_tcp_template.h"
#include <stdlib.h>
#include <stdio.h>
#include "app_timer.h"
#include "my_stdc_func/debugl.h"
#include "board.h"
//#include "my_stdc_func/debugl.h"

static err_t
tcp_client_recv_cb(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
	unsigned char *recv_buf;
	struct TcpClientCon *cli = (struct TcpClientCon *)arg;
	DEBUGL->debug("tcp client recv errcode %d \r\n",err);
  if(err == ERR_OK && p != NULL) {
    /* Inform TCP that we have taken the data. */
    tcp_recved(pcb, p->tot_len);
		
		//DEBUGL->debug("recv data len %d \r\n",p->tot_len);
		recv_buf = rt_malloc(p->tot_len+1);
		if (recv_buf > 0)
		{
			recv_buf[p->tot_len] = 0x0;
			pbuf_copy_partial(p,recv_buf,p->tot_len,0);
			if (cli->recv > 0)
			{
				cli->recv(cli->arg,recv_buf,p->tot_len);
			}
			rt_free(recv_buf);
		}
		//tcp_write(pcb,TCP_TestData,sizeof(TCP_TestData),0);   
  }
  else if(err == ERR_OK && p == NULL) {
		TCP_RUN_TEST;
    //MY_TCP_Client_stop(cli);
		tcp_close(cli->pcb);
		//tcp_abort(cli->pcb);
		cli->active_status = TCP_DISCONNECT;
		if (cli->disconn > 0)
			cli->disconn(cli->arg);
		TCP_RUN_TEST;
  }
	if (p > 0)
		pbuf_free(p);
  return ERR_OK;
}


static err_t tcp_sent_cb(void *arg, struct tcp_pcb *tpcb,
                              u16_t len)
{
	struct TcpClientCon *cli = (struct TcpClientCon *)arg;
	return 0;
	//
}

//连接成功
static err_t TcpCli_Connected(void *arg,struct tcp_pcb *pcb,err_t err) 
{
	
	struct TcpClientCon *cli = (struct TcpClientCon *)arg;
  tcp_recv(pcb,tcp_client_recv_cb);
	tcp_sent(pcb, tcp_sent_cb);
  cli->active_status=TCP_CONNECTED;
	if (cli->connected > 0)
		cli->connected(cli->arg);
  return ERR_OK;
} 

//连接失败
static void ClientConError(void *arg, err_t err)
{
	struct TcpClientCon *cli = (struct TcpClientCon *)arg;
	DEBUGL->debug("connect error %d \r\n",err);
	cli->active_status=TCP_CONNECT_ERR;
	if (cli->connerr > 0)
		cli->connerr(cli->arg);
}



static
void my_found(const char *name, ip_addr_t *ipaddr, void *arg)
{
	ip_addr_t result;
	unsigned char  ip[4];
	
	struct TcpClientCon *cli = (struct TcpClientCon *)arg;
	
	if (ipaddr <= 0)
	{
		DEBUGL->debug("dns error !\r\n");
		cli->active_status = TCP_DNS_ERR;
		if (cli->connerr > 0)
			cli->connerr(cli->arg);
		return ;
	}
	
	result = *ipaddr;
	
	
	ip[0] = result.addr>>24; 
	ip[1] = result.addr>>16;
	ip[2] = result.addr>>8; 
	ip[3] = result.addr; 
	
	memcpy(&cli->ipaddr,ipaddr,sizeof(cli->ipaddr));

	DEBUGL->debug("DNS SUCCESS :%d.%d.%d.%d \r\n",ip[3], ip[2], ip[1], ip[0]); 
	
	cli->active_status = TCP_DNS_FINISH;
	
	
}

//如果收到 ACK 则 更新apck
extern void (*tcp_rcv_ack_callback)(struct tcp_pcb *pcb);
void __tcp_rcv_ack_callback(struct tcp_pcb *pcb)
{
	char i=0;
	//recv_ack_timestamp
	for(i=0;i<TCP_CLI_COUNT;i++)
	{
		if (pcb == tcp_client_buffer[i].pcb)
		{
			 tcp_client_buffer[i].recv_ack_timestamp = app_timer_data.app_timer_second;
		}
	}
	//
}

struct TcpClientCon tcp_client_buffer[TCP_CLI_COUNT];
void init_tcp_client_buf(void)
{
	memset(&tcp_client_buffer,0x0,sizeof(tcp_client_buffer));
	tcp_rcv_ack_callback = __tcp_rcv_ack_callback;
	
	//
}
void rout_tcp_client_buf(void)
{
	int i=0;
	for(i=0;i<TCP_CLI_COUNT;i++)
	{
		if (tcp_client_buffer[i].used == 1)
		{
			MY_TCP_Client_Routing(&tcp_client_buffer[i]);
			DEBUGL->debug("TCP INDEX [%d] STATUS [%d]\r\n",i,tcp_client_buffer[i].active_status);
		}
	}
}

void disconn_tcp_client_buf(void)
{
	int i=0;
	for(i=0;i<TCP_CLI_COUNT;i++)
	{
		if (tcp_client_buffer[i].used == 1)
		{
			MY_TCP_Client_stop(&tcp_client_buffer[i]);
		}
	}
}

void MY_TCP_Client_Init(struct TcpClientCon *cli , char *hostname , long port , char type , char keepalive)
{
	
	memset(cli,0x0,sizeof(struct TcpClientCon));
	cli->used = 1;
	cli->port=port;
	snprintf(cli->hostname,64,"%s",hostname);
	cli->connect_type = type;
	cli->reconnection_time = 0;
	cli->keepalive = keepalive;
	//
}

static unsigned char dns_wait_count = 0; 

void  MY_TCP_Client_Routing(struct TcpClientCon *cli)
{ 
	err_t err_ret;
	switch(cli->active_status)
	{
	
		case TCP_INIT:
			//开始进行DNS解析
			err_ret = dns_gethostbyname(cli->hostname,&cli->ipaddr,my_found,(void*)cli);
			DEBUGL->debug("######## TCP INIT err_ret %d \r\n",err_ret);
			if (err_ret == ERR_OK){
				dns_wait_count = 0;
				cli->active_status = TCP_DNS_FINISH;
			}
			else if (err_ret == ERR_INPROGRESS){
				cli->active_status = TCP_DNS_START;
			}
			break;
		case TCP_DNS_START:
			//Wait DNS Query by my_found()
			dns_wait_count++;
			if ( dns_wait_count>= 10 ){
				dns_wait_count = 0;
				cli->active_status = TCP_INIT;
			}
			break;
		case TCP_DNS_FINISH:
		{
			DEBUGL->debug("Start connecting ... \r\n");
			
			cli->connecting_count ++ ;	/* 每次重新连接就加1 */
			cli->pcb=tcp_new();   /*  建立通信的TCP 控制块(Clipcb) */
			
			//打开keepalive
			if (cli->keepalive > 0)
			{
				cli->pcb->so_options |= SOF_KEEPALIVE;
			}
			
			if (cli->pcb > 0)
			{
				tcp_arg(cli->pcb,(void*)cli); //设置回调函数的参数

				tcp_bind(cli->pcb,IP_ADDR_ANY,0);/*  绑定本地IP 地址和端口号*/ 
				tcp_err(cli->pcb, ClientConError); //连接失败
				
				if (tcp_connect(cli->pcb,&cli->ipaddr,cli->port,TcpCli_Connected) != 0)
				{
					DEBUGL->debug("Local tcp connect error ! \r\n");
					//TcpClient.active_status = 0
					cli->active_status = TCP_INIT;
					return ;
				}else
				{
					DEBUGL->debug("Local tcp connect success ! \r\n");
					cli->active_status = TCP_CONNECTING;
				}
			}else
			{
				cli->active_status=TCP_CONNECT_ERR;
				if (cli->connerr > 0)
					cli->connerr(cli->arg);
			}
		}
		break;
		case TCP_DNS_ERR:
		case TCP_CONNECT_ERR:
		case TCP_DISCONNECT:
			//如果是长连接，那么断线后继续连接
			if (cli->connect_type == 1)
			{
				if(cli->connecting_count > 100 ){
					//
					rt_reset_system();
				}else{
					//delay 一段时间再进行重连
					cli->active_status = TCP_RECONNECT_DELAY;
				}
				
			}
			break;
		case TCP_RECONNECT_DELAY:
		{
			if (app_timer_data.app_timer_second > cli->reconnection_time)
			{
				cli->active_status = TCP_INIT;
			}else{
				DEBUGL->debug("###Reconnection TCP Delay:%d\r\n",app_timer_data.app_timer_second - cli->reconnection_time);
			}
			break;
		}
		case TCP_CONNECTING:
			break;
		case TCP_CONNECTED:
			cli->connecting_count = 0;
			break;
		case TCP_PAUSE:
			break;
		default:
			break;
	}
	
	if (cli->routing > 0)
		cli->routing(cli->arg);
	
} 

void MY_TCP_Client_stop(struct TcpClientCon *cli)
{

	if (cli->active_status == TCP_CONNECTED)
	{
		//tcp_close(cli->pcb);'
		tcp_abort(cli->pcb);
		
		cli->active_status = TCP_DISCONNECT;
		if (cli->disconn > 0)
			cli->disconn(cli->arg);
	}
	//
}


void MY_TCP_Client_start(struct TcpClientCon *cli)
{
	cli->active_status = TCP_INIT;
	//
}

void MY_TCP_Client_pause(struct TcpClientCon *cli)
{
	MY_TCP_Client_stop(cli);
	cli->active_status = TCP_PAUSE;
}

extern err_t
tcpip_callback_with_block(tcpip_callback_fn function, void *ctx, u8_t block);

struct __WDATA{
	struct TcpClientCon *cli;
	unsigned char *buf;
	int len;
};

static struct __WDATA wd = {0,0,0};
static err_t tcp_write_err = 99;

static void __WDATA_FUNC(void *ctx)
{
	tcp_write_err = tcp_write(wd.cli->pcb,wd.buf,wd.len,TCP_WRITE_FLAG_COPY);
	DEBUGL->debug("Shu ju bei  yi bu fa song [%d]!\r\n",tcp_write_err);
	rt_free(wd.buf);
	wd.buf = 0;
}

err_t MY_TCP_Client_write2(struct TcpClientCon *cli , unsigned char *buffer , int len)
{
	int i=0,j=0;
	err_t ret = 0;
	if (wd.buf == 0)
	{
		wd.buf = rt_malloc(len);
		if (wd.buf > 0)
		{
			memcpy(wd.buf,buffer,len);
			//重复发送10次
			for(i=0;i<5;i++)
			{
				wd.cli = cli;
				wd.len = len;
				tcp_write_err = 99;
				ret = tcpip_callback_with_block(__WDATA_FUNC,0,0);
				if (ret == 0)
				{
					//如果发送成功
					//wait return
					for(j=0;j<10;j++)
					{
						if (tcp_write_err != 99)
						{
							return tcp_write_err;
						}
						rt_thread_sleep(RT_TICK_PER_SECOND/10);
					}
					
					//如果等待1秒还没有返回，说明超时了
					return ERR_TIMEOUT;
				}
				
				rt_thread_sleep(RT_TICK_PER_SECOND/10);
				
			}
			
			rt_free(wd.buf);
			wd.buf = 0;
			
			return ret;
			//
		}
		
		return ret;
		
	}
	
	return ERR_MEM;
}

static struct rt_thread * tcpip = 0;
err_t MY_TCP_Client_write(struct TcpClientCon *cli , unsigned char *buffer , int len)
{
	if (tcpip == 0)
	{
		tcpip = rt_thread_find("tcpip");
		//
	}
	
	if (tcpip == rt_thread_self())
	{
		DEBUGL->debug("Send PKG for my self \r\n");
		return tcp_write(cli->pcb,buffer,len,TCP_WRITE_FLAG_COPY);
	}else
	{
		return MY_TCP_Client_write2(cli,buffer,len);
	}
	
}



