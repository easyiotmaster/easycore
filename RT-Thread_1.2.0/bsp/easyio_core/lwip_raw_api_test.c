#include "lwip/tcp.h"
#include "lwip/dns.h"

#include "lwip_raw_api_test.h"

#include "global_env.h"
#include "XMPP/xmpp.h"
#include "my_stdc_func/debugl.h"

enum{
	TCP_INIT,
	TCP_DNS_START,
	TCP_DNS_ERR,
	TCP_DNS_FINISH,
	TCP_CONNECTING,
	TCP_CONNECTED,
	
	
	TCP_CONNECT_ERR,
	TCP_DISCONNECT,
	TCP_PAUSE,//暂停
};


struct TcpClientCon{
	struct tcp_pcb *pcb;
	struct ip_addr ipaddr;
	long port;
	char status_base;
	char hostname[64];
};

static struct TcpClientCon TcpClient;


static err_t
tcp_client_recv_cb(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t err)
{
	char *recv_buf;
	DEBUGL->debug("tcp client recv errcode %d \r\n",err);
  if(err == ERR_OK && p != NULL) {
    /* Inform TCP that we have taken the data. */
    tcp_recved(pcb, p->tot_len);
		
		DEBUGL->debug("recv data len %d \r\n",p->tot_len);
		recv_buf = rt_malloc(p->tot_len+1);
		if (recv_buf > 0)
		{
			recv_buf[p->tot_len] = 0x0;
			pbuf_copy_partial(p,recv_buf,p->tot_len,0);
			
			process_recv_data(recv_buf,p->tot_len+1);
			//
			rt_free(recv_buf);
		}
		
		//tcp_write(pcb,TCP_TestData,sizeof(TCP_TestData),0);   
  }
  else if(err == ERR_OK && p == NULL) {
    TCP_Client_stop();
  }
	if (p > 0)
		pbuf_free(p);
  return ERR_OK;
}


static err_t tcp_sent_cb(void *arg, struct tcp_pcb *tpcb,
                              u16_t len)
{
	return 0;
	//
}

//连接成功
static err_t
TcpCli_Connected(void *arg,struct tcp_pcb *pcb,err_t err) 
{ 
  tcp_recv(pcb,tcp_client_recv_cb);
	tcp_sent(pcb, tcp_sent_cb);
  TcpClient.status_base=TCP_CONNECTED;
	connected_xmpp();
  return ERR_OK;
} 

//连接失败
static
void ClientConError(void *arg, err_t err)
{
	DEBUGL->debug("connect error %d \r\n",err);
	TcpClient.status_base=TCP_CONNECT_ERR;
}



static
void my_found(const char *name, ip_addr_t *ipaddr, void *arg)
{
	ip_addr_t result;
	unsigned char  ip[4];
	
	if (ipaddr <= 0)
	{
		DEBUGL->debug("dns error !\r\n");
		TcpClient.status_base = TCP_DNS_ERR;
		return ;
	}
	
	result = *ipaddr;
	
	
	ip[0] = result.addr>>24; 
	ip[1] = result.addr>>16;
	ip[2] = result.addr>>8; 
	ip[3] = result.addr; 
	
	memcpy(&TcpClient.ipaddr,ipaddr,sizeof(TcpClient.ipaddr));

	DEBUGL->debug("DNS SUCCESS :%d.%d.%d.%d \r\n",ip[3], ip[2], ip[1], ip[0]); 
	
	TcpClient.status_base = TCP_DNS_FINISH;
	
	
}

void
TCP_Client_Init(unsigned char *hostname , long port)
{
	memset(&TcpClient,0x0,sizeof(TcpClient));
	TcpClient.port=port;
	snprintf(TcpClient.hostname,sizeof(TcpClient.hostname),"%s",hostname);
	//
}

void 
TCP_Client_Routing(void)
{ 
	err_t err_ret;
	switch(TcpClient.status_base)
	{
	
		case TCP_INIT:
			//开始进行DNS解析
			err_ret = dns_gethostbyname(TcpClient.hostname,&TcpClient.ipaddr,my_found,0);
			DEBUGL->debug("######## err_ret %d \r\n",err_ret);
			if (err_ret == ERR_OK)
				TcpClient.status_base = TCP_DNS_FINISH;
			else if (err_ret == ERR_INPROGRESS)
				TcpClient.status_base = TCP_DNS_START;
			break;
		case TCP_DNS_START:
			break;
		case TCP_DNS_ERR:
			TcpClient.status_base = TCP_INIT;
			break;
		case TCP_DNS_FINISH:
		{
			DEBUGL->debug("Start connecting ... \r\n");
			
			TcpClient.pcb=tcp_new();   /*  建立通信的TCP 控制块(Clipcb)                */

			tcp_bind(TcpClient.pcb,IP_ADDR_ANY,0);/*  绑定本地IP 地址和端口号*/ 
			tcp_err(TcpClient.pcb, ClientConError); //连接失败
			
			if (tcp_connect(TcpClient.pcb,&TcpClient.ipaddr,TcpClient.port,TcpCli_Connected) != 0)
			{
				DEBUGL->debug("Local tcp connect error ! \r\n");
				//TcpClient.status_base = 0
				TcpClient.status_base = TCP_INIT;
				return ;
			}else
			{
				DEBUGL->debug("Local tcp connect success ! \r\n");
				TcpClient.status_base = TCP_CONNECTING;
			}
		}
		break;
		case TCP_CONNECTING:
			break;
		case TCP_CONNECT_ERR:
		case TCP_DISCONNECT:
			TcpClient.status_base = TCP_INIT;
			break;
		case TCP_CONNECTED:
		{
			static unsigned int connected_cnt = 0;
			connected_cnt ++;
			if (connected_cnt > 15)
			{
				make_presence_status_str();
				xmpp_set_presence("NULL",presence_status_str);
				
				
				connected_cnt = 0;
				//
			}
			break;
		}
		
		case TCP_PAUSE:
			break;
		default:
			break;
	}
	
} 

void TCP_Client_stop(void)
{
	if (TcpClient.status_base == TCP_CONNECTED)
	{
		tcp_close(TcpClient.pcb);
		TcpClient.status_base = TCP_DISCONNECT;
		disconn_xmpp();
	}
	//
}

void TCP_Client_start(void)
{
	TcpClient.status_base = TCP_INIT;
	//
}

void TCP_Client_pause(void)
{
	TCP_Client_stop();
	TcpClient.status_base = TCP_PAUSE;
}

err_t TCP_Client_write(unsigned char *buffer , int len)
{
	return tcp_write(TcpClient.pcb,buffer,len,TCP_WRITE_FLAG_COPY); 
	//
}

err_t __tcp_send_char(char *str)
{
	return TCP_Client_write((char*)str,strlen(str));
}



