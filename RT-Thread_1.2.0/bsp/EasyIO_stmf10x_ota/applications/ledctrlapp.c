#include "ledctrlapp.h"
#include "led.h"
#include <rtthread.h>

#include "XMPP/xmpp.h"
#include "ppp_service.h"

static struct rt_timer flicker_led_tmr;
struct LED_FLICKER led_flicker[3];

static void flick_led_tmr_timeout(void * arg)
{
	int i=0;

	//下面是新的LED控制规则
	for(i=0;i<3;i++)
	{
		switch(led_flicker[i].mode)
		{
			case LED_WORK_MOD_FLICKER:
				led_off(i+1);
				if (led_flicker[i].idle_ms_max == 0)
					break;
				led_flicker[i].idle_ms += 50;
				if (led_flicker[i].idle_ms >= led_flicker[i].idle_ms_max)
				{
					led_flicker[i].idle_ms = 0;
					led_flicker[i].mode = 1;
				}
				break;
			case 1:
				led_on(i+1);
				if (led_flicker[i].busy_ms_max == 0)
					break;
				led_flicker[i].busy_ms += 50;
				if (led_flicker[i].busy_ms >= led_flicker[i].busy_ms_max)
				{
					led_flicker[i].busy_ms = 0;
					led_flicker[i].mode = LED_WORK_MOD_FLICKER;
				}
				break;
				
			case LED_WORK_MOD_ON:
				led_on(i+1);
				break;
			case LED_WORK_MOD_OFF:
				led_off(i+1);
				break;
			case LED_WORK_MOD_CUSTOME:
				break;
			default:
				break;
		}
	}
	
	//////////////////////////针对不用的状态，做不同的处理
	
	switch(privd.pppd_status)
	{
		case PPPD_STATUS_PROTO_SUCCESS:
		{
			//判断当前xmpp的状态
			switch(xmpp_pma.xmpp_status)
			{
				case XMPP_BINDSUCC:
					LED_ON_MOD(led_flicker[0]);
					break;
				default:
					LED_FAST_FLICKER(led_flicker[0]);
					break;
			}
	
			break;
		}
		
		default:
		{
			LED_SLOW_FLICKER(led_flicker[0]);
			break;
		}
		//
	}
	

	return;
	
	//
}


static void init_flicker_led_tmr(void)
{
	memset(&led_flicker,0x0,sizeof(led_flicker));
	rt_timer_init(&flicker_led_tmr,"fl_tmr",flick_led_tmr_timeout,0,RT_TICK_PER_SECOND/20,RT_TIMER_FLAG_PERIODIC);
  rt_timer_start(&flicker_led_tmr);
	//
}



void init_ledctrlapp(void)
{
	init_flicker_led_tmr();
	
	LED_OFF_MOD(led_flicker[0]);
	LED_CUSTOME_MOD(led_flicker[1]);
	LED_OFF_MOD(led_flicker[2]);
	//
}