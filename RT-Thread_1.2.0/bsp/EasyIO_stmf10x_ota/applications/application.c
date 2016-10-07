/*
 * File      : application.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 * 2013-07-12     aozima       update for auto initial.
 */

/**
 * @addtogroup STM32
 */
/*@{*/

#include <board.h>
#include <rtthread.h>
#include "ppp_service.h"
#include "app_ev.h"


ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t system_service_stack[ 1024 * 2 ];
static rt_uint8_t mainloop_stack[ 1024 ];

static struct rt_thread system_service_thread;
static struct rt_thread mainloop_thread;




#include "dcmi_ov2640.h"
	
int rt_application_init(void)
{
    rt_thread_t init_thread;

    rt_err_t result;
	
		//初始化系统服务
    result = rt_thread_init(&system_service_thread,
                            "appev",
                            appev_thread_entry,
                            RT_NULL,
                            (rt_uint8_t*)&system_service_stack[0],
                            sizeof(system_service_stack),
                            20,
                            5);
    if (result == RT_EOK)
    {
        rt_thread_startup(&system_service_thread);
    }
		
		//初始化PPP服务
		result = rt_thread_init(&mainloop_thread,
                            "mloop",
                            mainloop,
                            RT_NULL,
                            (rt_uint8_t*)&mainloop_stack[0],
                            sizeof(mainloop_stack),
                            20,
                            5);
    if (result == RT_EOK)
    {
        rt_thread_startup(&mainloop_thread);
    }
		
		
		return 0;

}

/*@}*/
