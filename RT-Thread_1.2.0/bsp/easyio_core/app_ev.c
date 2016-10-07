#include "app_ev.h"
#include <rtthread.h>
#include "app_timer.h"
ALIGN(RT_ALIGN_SIZE)

struct APP_EVENT  app_event[APP_EVENT_BUFFER];

static char mainloop_mb_pool[128];
static struct rt_mailbox mainloop_mb;


static rt_err_t init_app_event(void)
{
	int i = 0 ;
	for(i=0;i<APP_EVENT_BUFFER;i++)
	{
		app_event[i].enable = 0;
	}
	return -RT_EOK;
}

static struct APP_EVENT * alloc_app_event(void)
{
	int i = 0 ;
	for(i=0;i<APP_EVENT_BUFFER;i++)
	{
		if (app_event[i].enable == 0)
		{
			app_event[i].enable = 1;
			return &app_event[i];
		}
	}
	
	return 0;
}

static void destory_app_event(struct APP_EVENT *ev)
{
	ev->enable = 0;
}


rt_err_t post_hw_int_event(void * func , void * arg , void * func_resp , void *arg_resp)
{
	rt_err_t res;
	struct APP_EVENT * ev = alloc_app_event();
	if (ev > 0)
	{
		ev->type = APP_EV_HW_INT_TYPE;
		ev->simple_data[0] = (unsigned int)func;
    ev->simple_data[1] = (unsigned int)arg;
		ev->simple_data[2] = (unsigned int)func_resp;
    ev->simple_data[3] = (unsigned int)arg_resp;
		
		res = rt_mb_send(&mainloop_mb,(rt_uint32_t)ev);
		if (res != RT_EOK)
		{
			destory_app_event(ev);
		}
		return res;
		
	}
	
	return -RT_ERROR;
}

rt_err_t post_default_event(void * func , void * arg , void * func_resp , void *arg_resp)
{
	rt_err_t res;
	struct APP_EVENT * ev = rt_malloc(sizeof(struct APP_EVENT));
	if (ev > 0)
	{
		ev->type = APP_EV_DEFAULT_TYPE;
		ev->simple_data[0] = (unsigned int)func;
    ev->simple_data[1] = (unsigned int)arg;
		ev->simple_data[2] = (unsigned int)func_resp;
    ev->simple_data[3] = (unsigned int)arg_resp;
		
		res = rt_mb_send(&mainloop_mb,(rt_uint32_t)ev);
		if (res != RT_EOK)
		{
			//destory_app_event(ev);
			rt_free(ev);
		}
		return res;
		
	}
	
	return -RT_ERROR;
}

rt_err_t post_default_event_wait(void * func , void * arg , void * func_resp , void *arg_resp , unsigned int timeout)
{
	rt_err_t res;
	struct APP_EVENT * ev = rt_malloc(sizeof(struct APP_EVENT));
	if (ev > 0)
	{
		ev->type = APP_EV_DEFAULT_TYPE;
		ev->simple_data[0] = (unsigned int)func;
    ev->simple_data[1] = (unsigned int)arg;
		ev->simple_data[2] = (unsigned int)func_resp;
    ev->simple_data[3] = (unsigned int)arg_resp;
		
		res = rt_mb_send_wait(&mainloop_mb,(rt_uint32_t)ev,timeout);
		if (res != RT_EOK)
		{
			//destory_app_event(ev);
			rt_free(ev);
		}
		return res;
		
	}
	
	return -RT_ERROR;
}

rt_err_t post_hw_int_event_wait(void * func , void * arg , void * func_resp , void *arg_resp , unsigned int timeout)
{
	//
}


void appev_thread_entry(void* parameter)
{
	rt_err_t result;
	rt_uint32_t tmp;
	rt_kprintf("mainloop_init\r\n");
  result = rt_mb_init(&mainloop_mb,
        "mainloop_mb",
        &mainloop_mb_pool[0],
        sizeof(mainloop_mb_pool)/4,
        RT_IPC_FLAG_FIFO);
  if (result != RT_EOK)
  {
    rt_kprintf("init mailbox failed.\n");
		return ;
	}else
	{
		rt_kprintf("init mailbox success.\n");
	}
	
	init_app_event();
	init_app_timer();
	
	do {
		//DEBUGL->debug("mainloop thread : try to recv a mail\n");
        if (rt_mb_recv(&mainloop_mb, (rt_uint32_t*)&tmp, RT_WAITING_FOREVER) == RT_EOK)
        {
					
					struct APP_EVENT *ev = (struct APP_EVENT *)tmp;
					if (ev > 0)
					{
						//TODO
						
						switch(ev->type)
						{
							case APP_EV_DEFAULT_TYPE:
							{
								EVENT_FUNC_CB cb = (EVENT_FUNC_CB)ev->simple_data[0];
								EVENT_FUNC_CB cb_reply = (EVENT_FUNC_CB)ev->simple_data[2];
								if (cb > 0)
									cb((void*)ev->simple_data[1]);
								if (cb_reply > 0)
									cb_reply((void*)ev->simple_data[3]);
								rt_free(ev);
								break;
							}
							case APP_EV_HW_INT_TYPE:
							{
								EVENT_FUNC_CB cb = (EVENT_FUNC_CB)ev->simple_data[0];
								EVENT_FUNC_CB cb_reply = (EVENT_FUNC_CB)ev->simple_data[2];
								if (cb > 0)
									cb((void*)ev->simple_data[1]);
								if (cb_reply > 0)
									cb_reply((void*)ev->simple_data[3]);
								destory_app_event(ev);
								break;
							}
							default:
								break;
						}
						
						
					}
        }
		//
	}
	while(1);
}
