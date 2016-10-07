#include <rtthread.h>
#include <string.h>
#include "my_stdc_func/debugl.h"
#include "modem_serial.h"
rt_device_t modem_serial_fd = RT_NULL ;
static struct rt_semaphore modem_serial_rx_sem;

static rt_err_t usart_rx_ind(rt_device_t dev, rt_size_t size)
{
	rt_sem_release(&modem_serial_rx_sem);//当有数据的时候将锁打开
	return RT_EOK;
}

rt_err_t open_modem_serial_port(char *portname)
{
	modem_serial_fd = rt_device_find(portname);
		if (modem_serial_fd != RT_NULL && rt_device_open(modem_serial_fd, RT_DEVICE_OFLAG_RDWR) == RT_EOK)
	{
		rt_sem_init(&modem_serial_rx_sem, "p_s_sem", 0, 0);
		rt_device_set_rx_indicate(modem_serial_fd, usart_rx_ind);
		//
	}else
	{
		return -RT_ERROR;
	}
	
	return -RT_EOK;
	//
}

//阻塞读串口数据
int read_serial_data(unsigned char *buf , int len , rt_int32_t time)
{
	//只等待1秒吧
	rt_sem_take(&modem_serial_rx_sem, time) ;//RT_ETIMEOUT
	return rt_device_read(modem_serial_fd, 0, buf, len);
}

static rt_err_t priv_at_command(unsigned char mod , unsigned char *at_cmd ,int cmdlen, int(*at_cb)(const char*resp,int resplen) , int to , int *atcmd_respcode)
{
	int i=0;
	char at_resp_buf[64];
	int at_resp_buf_index = 0;
	
	rt_err_t result,result2;
	unsigned char ch;
	
	*atcmd_respcode = -1;
	
	//写数据
	rt_device_write(modem_serial_fd,0,at_cmd,cmdlen);
	//如果是字符AT
	if (mod == 0)
	{
		DEBUGL->debug("SEND AT:\r\n");
		for(i=0;i<cmdlen;i++)
		{
				DEBUGL->debug("%c",at_cmd[i]);
		}
		DEBUGL->debug("\r\n");
	}else{
		DEBUGL->debug("SEND BUF:\r\n");
		for(i=0;i<cmdlen;i++)
		{
				DEBUGL->debug("0x%02X ",at_cmd[i]);
		}
		DEBUGL->debug("\r\n");
	}

	memset(at_resp_buf,0x0,sizeof(at_resp_buf));
	result = rt_sem_take(&modem_serial_rx_sem, to) ;//RT_ETIMEOUT
	
	if (result == -RT_EOK)
	{
		
		for(;;)
		{
			while (rt_device_read(modem_serial_fd, 0, &ch, 1) == 1)
			{
				if (at_resp_buf_index < sizeof(at_resp_buf)- 4)
				{
					//if ((ch >= 32) && (ch <= 127) || (ch == 0x0d) || (ch == 0x0a)) 
					at_resp_buf[at_resp_buf_index++] = ch;
				}
				
			}
			
			result2 = rt_sem_take(&modem_serial_rx_sem, 50); //如果100ms后还没有AT命令则认为这条AT命令接受完毕
			if (result2 == -RT_ETIMEOUT)
			{
				break;
			}
		}
	}
	
	if (mod == 0)
	{
		DEBUGL->debug("RESP:%s\r\n",at_resp_buf);
	}else
	{
		DEBUGL->debug("RESP:\r\n");
		for(i=0;i<at_resp_buf_index;i++)
		{
				DEBUGL->debug("0x%02X ",at_resp_buf[i]);
		}
		DEBUGL->debug("\r\n");
		
	}
	if (at_cb > 0)
	{
		*atcmd_respcode = at_cb(at_resp_buf,at_resp_buf_index);
	}
	return result;
}

static rt_err_t priv_at_command_wait(unsigned char *at_cmd ,int cmdlen, int(*at_cb)(const char*resp,int resplen) , int to , int *atcmd_respcode)
{
	int i=0;
	char at_resp_buf[64];
	int at_resp_buf_index = 0;
	
	rt_err_t result,result2;
	unsigned char ch;
	
	*atcmd_respcode = -1;
	
	//写数据
	rt_device_write(modem_serial_fd,0,at_cmd,cmdlen);

	memset(at_resp_buf,0x0,sizeof(at_resp_buf));
	result = rt_sem_take(&modem_serial_rx_sem, to) ;//RT_ETIMEOUT
	
	if (result == -RT_EOK)
	{
		
		for(;;)
		{
			while (rt_device_read(modem_serial_fd, 0, &ch, 1) == 1)
			{
				if (at_resp_buf_index < sizeof(at_resp_buf)- 4)
				{
					//if ((ch >= 32) && (ch <= 127) || (ch == 0x0d) || (ch == 0x0a)) 
					at_resp_buf[at_resp_buf_index++] = ch;
				}
				
			}
			
			if (at_cb > 0)
			{
				*atcmd_respcode = at_cb(at_resp_buf,at_resp_buf_index);
				if (*atcmd_respcode > 0)
					break;
			}
			
			result2 = rt_sem_take(&modem_serial_rx_sem, to); //如果100ms后还没有AT命令则认为这条AT命令接受完毕
			if (result2 == -RT_ETIMEOUT)
			{
				break;
			}
		}
	}
	
	return result;
}


rt_err_t at_command(char *at_cmd ,int(*at_cb)(const char*resp,int resplen) , int to , int *atcmd_respcode)
{
	return priv_at_command(0,(unsigned char*)at_cmd,strlen(at_cmd),at_cb,to,atcmd_respcode);
}

rt_err_t at_command_wait(char *at_cmd ,int(*at_cb)(const char*resp,int resplen) , int to , int *atcmd_respcode)
{
	return priv_at_command_wait((unsigned char*)at_cmd,strlen(at_cmd),at_cb,to,atcmd_respcode);
}

rt_err_t cmux_command(unsigned char *cmux_cmd , int len  ,int(*at_cb)(const char*resp,int resplen) , int to , int *atcmd_respcode)
{
	return priv_at_command(1,(unsigned char*)cmux_cmd,len,at_cb,to,atcmd_respcode);
}


