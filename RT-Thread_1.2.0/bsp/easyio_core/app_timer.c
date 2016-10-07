#include "app_timer.h"
#include "app_ev.h"
#include "common.h"
#include <rtthread.h>
#include "watch_dog.h"
#include "my_stdc_func/debugl.h"

static struct rt_timer app_timer;
struct APP_TIMER app_timer_data;
unsigned short system_timer_counter[SYSTEM_TIMER_COUNTER_CNT];

#define TIMER_PREIOD  10  //定时器周期，单位为 毫秒


void app_timer_add(struct APP_TIMER *tmr , unsigned int ms)
{
	unsigned int sec;
	unsigned int mms;
	
	mms = ms + tmr->app_timer_ms;
	
	sec = mms/1000;
	tmr->app_timer_second += sec;
	tmr->app_timer_ms = mms%1000;
	//
}

int app_timer_cmp(struct APP_TIMER *tmr1 , struct APP_TIMER *tmr2)
{
	if (tmr1->app_timer_second > tmr2->app_timer_second)
		return 1;
	else if (tmr1->app_timer_second < tmr2->app_timer_second)
		return -1;
	else if (tmr1->app_timer_second == tmr2->app_timer_second)
	{
		
		if (tmr1->app_timer_ms > tmr2->app_timer_ms)
			return 1;
		else if (tmr1->app_timer_ms < tmr2->app_timer_ms)
			return -1;
			
		if (tmr1->app_timer_ms == tmr2->app_timer_ms)
			return 0;
	}
	//
	
	return 0;
}

int app_timer_copy(struct APP_TIMER *tmr1 , struct APP_TIMER *tmr2)
{
	tmr1->app_timer_second = tmr2->app_timer_second;
	tmr1->app_timer_ms = tmr2->app_timer_ms;
	return 0;
}

static void __timerout_in(void)
{
	unsigned int i=0;
	for(i=0;i<EI_TIMER_COUNT;i++)
	{
		if (ei_timer[i].enable == 1)
		{
			if ((app_timer_data.app_timer_second - ei_timer[i].his_time) > ei_timer[i].ticks )
			{
				ei_timer[i].his_time = app_timer_data.app_timer_second;
				ei_timer[i].timeout(ei_timer[i].timeout_arg);
				
				if ( ei_timer[i].type == EI_TIMER_TYPE_ONE)
				{
					ei_timer[i].enable = 0;
				}
				
			}
		}
		//
	}
}

static void second_timer(void *p)
{
	__timerout_in();
}

static void app_tmr_timeout(void *arg)
{
	char i;
	static unsigned int feed_watchdog_cnt = 0;
//	static unsigned int rout_mma8452_cnt = 0;
	// system timer counter
	app_timer_data.app_timer_ms += TIMER_PREIOD;
	if (app_timer_data.app_timer_ms >= 1000)
	{
		app_timer_data.app_timer_second ++;
		app_timer_data.app_timer_ms = 0;
		post_hw_int_event(second_timer,0,0,0);
	}
	//
	
	//watchdog feed
	feed_watchdog_cnt += TIMER_PREIOD;
	if (feed_watchdog_cnt > 2000)		//每2000毫秒喂狗
	{
		post_hw_int_event(feed_watchdog,0,0,0);
		feed_watchdog_cnt = 0;
	}
	
	for(i=0;i<SYSTEM_TIMER_COUNTER_CNT;i++)
	{
		system_timer_counter[i] += TIMER_PREIOD;
	}
}

void init_app_timer(void)
{
	memset(&app_timer_data,0x0,sizeof(struct APP_TIMER));
	rt_timer_init(&app_timer,"app_tmr",app_tmr_timeout,0,1,RT_TIMER_FLAG_PERIODIC);
  rt_timer_start(&app_timer);
	
	memset(&ei_timer,0x0,sizeof(EI_TIMER)*EI_TIMER_COUNT);
	
}

EI_TIMER ei_timer[EI_TIMER_COUNT];


EI_TIMER *alloc_timer(void){
	int i=0;
	for(i=0;i<EI_TIMER_COUNT;i++)
	{
		if(ei_timer[i].alloc == 0)
		{
			ei_timer[i].alloc = 1;
			return &ei_timer[i];
		}
	}
	
	return 0;
}

void timer_start(EI_TIMER *timer)
{
	timer->his_time = app_timer_data.app_timer_second;
	timer->enable = 1;
}

void destory_timer(EI_TIMER *timer){
	timer->alloc = 0;
	memset(timer,0x0,sizeof(EI_TIMER));
}