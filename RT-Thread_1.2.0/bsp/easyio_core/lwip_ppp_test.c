#include <rtthread.h>
#include <lwip/sys.h>
#include <netif/ethernetif.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>
#include <lwip/tcp.h>

#define RECV_TCP_BUFFER_LEN 2048


extern unsigned char ppp_connectok_cnt;

static unsigned int connect_id = 0;
static int sock;
void tcpclient(const char* url, int port)
{
	char *recv_data;
	struct hostent *host;
	int bytes_received,bytes_sent;
	struct sockaddr_in server_addr;
		
	fd_set readset;
	fd_set writeset;
	struct timeval timeout;
	
	timeout.tv_sec = 1;
	timeout.tv_usec = 0;


	/* 通过函数入口参数url获得host地址（如果是域名，会做域名解析） */
	host = gethostbyname(url);
	if (host <= 0)
	{
		rt_kprintf("No gethostbyname\n");
			return ;
	}

	/* 分配用于存放接收数据的缓冲 */
	recv_data = rt_malloc(RECV_TCP_BUFFER_LEN);
	if (recv_data == RT_NULL)
	{
		rt_kprintf("No memory\n");
		return;
	}

	/* 创建一个socket，类型是SOCKET STREAM，TCP类型 */
	if ((sock = lwip_socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		/* 创建socket失败 */
		rt_kprintf("Socket error\n");
		/* 释放接收缓冲 */
		rt_free(recv_data);
		return;
	}
	else
	{
	}


	/* 初始化预连接的服务端地址 */
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr = *((struct in_addr *)host->h_addr);
	rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

	/* 连接到服务端 */
	if (lwip_connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
	{
		/* 连接失败 */
		rt_kprintf("Connect error\n");

		/*释放接收缓冲 */
		rt_free(recv_data);

		/**/
		lwip_close(sock);
		return;
		//
	}
	else
	{
			connect_id = ppp_connectok_cnt;
			rt_kprintf("CONNECT OK \r\n");
			connected_xmpp();//连接XMPP 成功，发送连接成功XML
	};
	
	//lwip_select
#if 1
	for(;;)
	{
		int ret;
		int recv_len = 0;
		FD_ZERO(&readset);
		FD_ZERO(&writeset);
		FD_SET(sock, &readset);
		FD_SET(sock, &writeset);
		
		//每隔1秒超时
		ret = lwip_select(sock+1, &readset, 0, 0, &timeout);
		
		if(ret > 0)
		{
			if (FD_ISSET(sock, &readset))
			{
					recv_len = recv(sock,recv_data,RECV_TCP_BUFFER_LEN-4,0);
					if(recv_len <= 0)
					{
							lwip_close(sock);
							sock = -1;
							return;
					}
					else
					{
						//因为是字符串，所以将最后一个字节设置为0
						recv_data[recv_len] = 0x0;
						process_recv_data(recv_data,recv_len);
						
					}
			}
			/*
				else if(FD_ISSET(remotefd, &writeset))
				{
				send(remotefd,"this is time to send!\r\n",25,0);
				}
			*/
		}
		else if (ret == 0)
		{
			rt_kprintf("Read socket Timeout !!!! \r\n");
			print_sysinfo();
			if (ppp_connectok_cnt > connect_id)
			{
				lwip_close(sock);
				sock = -1;
				return ;
			}
			//
		}
		else if(ret < 0)
		{
			lwip_close(sock);
			sock = -1;
			return;
		}
		
	}
#endif
#if 0
	while(1)
	{
		static int recv_count = 0;
		/* 从sock连接中接收最大 RECV_TCP_BUFFER_LEN 字节数据 */
		bytes_received = lwip_recv(sock, recv_data, RECV_TCP_BUFFER_LEN, 0);
		if (bytes_received <= 0)
		{
			/* 接收失败，关闭这个连接 */
			lwip_close(sock);
			/* 释放接收缓冲 */
			rt_free(recv_data);
			return ;
		}

		recv_count = bytes_received;
		rt_kprintf("############# %d\r\n",recv_count);
		recv_data[recv_count] = 0x0;
		#ifdef ENABLE_XMPP
		process_recv_data(recv_data,recv_count);
		#endif
	}
#endif
	return;
}

void __abort_socket(void)
{
	//abort_socket = 1;
	//lwip_close(sock);
}

void __tcp_send_char(char *str)
{
	//int strlen = strlen(str);
	//char send_str[] = "hello lwip ppp!";
	lwip_send(sock,str,strlen(str), 0);
}

