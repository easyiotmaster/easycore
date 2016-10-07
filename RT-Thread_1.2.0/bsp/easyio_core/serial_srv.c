#include <rtthread.h>
#include "serial_srv.h"
#include "my_stdc_func/debugl.h"


static rt_device_t serial_dev[SERIAL_COUNT] = {0x0};
static struct rt_semaphore serial_rx_sem[SERIAL_COUNT];
static rt_uint8_t serial_working_buffer[SERIAL_COUNT][1024];
static rt_uint8_t poll_thread_stack[SERIAL_COUNT][1024];
static struct rt_thread poll_thread[SERIAL_COUNT];

SERIAL_CB serial_handle[SERIAL_COUNT];

static rt_err_t usart_rx_ind(rt_device_t dev, rt_size_t size)
{

	int i = 0;
	for(i=0;i<SERIAL_COUNT;i++)
	{
		if (serial_dev[i] == dev)
		{
			rt_sem_release(&serial_rx_sem[i]);//???????????
		}
	}

	return RT_EOK;
}
static int open_serial(char *serial1_name , char *serial2_name)
{
	
	if (serial1_name > 0)
	{
		serial_dev[0] = rt_device_find(serial1_name);
		if (serial_dev[0] != RT_NULL && rt_device_open(serial_dev[0], RT_DEVICE_OFLAG_RDWR) == RT_EOK)
		{
			rt_sem_init(&(serial_rx_sem[0]), "uart1_sem", 0, 0);
			rt_device_set_rx_indicate(serial_dev[0], usart_rx_ind);
		}
	}
	
	#if SERIAL_COUNT == 2
	if (serial2_name > 0)
	{
		serial_dev[1] = rt_device_find(serial2_name);
		if (serial_dev[1] != RT_NULL && rt_device_open(serial_dev[1], RT_DEVICE_OFLAG_RDWR) == RT_EOK)
		{
			rt_sem_init(&(serial_rx_sem[1]), "uart2_sem", 0, 0);
			rt_device_set_rx_indicate(serial_dev[1], usart_rx_ind);
		}
	}
	#endif

	return 0;
}

static void poll_uart(void * arg)
{

	int num;
	int index = 0; char ch;
	static int take_ret;
	num = (int)arg;
	for(;;)
	{
		index = 0;
		memset(serial_working_buffer[num],0x0,sizeof(serial_working_buffer[num])); //clear buffer
		take_ret = rt_sem_take(&serial_rx_sem[num], RT_WAITING_FOREVER);//RT_ETIMEOUT
		do
		{
			while (rt_device_read(serial_dev[num], 0, &ch, 1) == 1)
			{
				if(index == sizeof(serial_working_buffer[num]))
					continue;
				serial_working_buffer[num][index++] = ch;
			}
		}while((rt_sem_take(&serial_rx_sem[num], 10) == RT_EOK));

		
		if (serial_handle[num] > 0)
		{
			serial_handle[num](serial_working_buffer[num],index);
		}else
		{
			//
		}
		//todo


	}

}

rt_err_t init_user_serial_service(char *serial1_name , char *serial2_name)
{
	rt_err_t result;
	open_serial(serial1_name,serial2_name);
	
	
	if (serial1_name > 0)
	{

		result = rt_thread_init(&poll_thread[0],
				"pollt0",
				poll_uart,(void*)0,
				(rt_uint8_t*)poll_thread_stack[0], sizeof(poll_thread_stack[0]), 22, 5);


		//
		if (result == RT_EOK)
		{
			rt_thread_startup(&poll_thread[0]);
		}
		else
		{
			return result;
		}
	}

	#if SERIAL_COUNT == 2
	if (serial2_name > 0)
	{

		result = rt_thread_init(&poll_thread[1],
				"pollt1",
				poll_uart,(void*)1,
				(rt_uint8_t*)poll_thread_stack[1], sizeof(poll_thread_stack[1]), 22, 5);


		//
		if (result == RT_EOK)
		{
			rt_thread_startup(&poll_thread[1]);
		}
		else
		{
			return result;
		}
	}
	#endif

	return result;
}
