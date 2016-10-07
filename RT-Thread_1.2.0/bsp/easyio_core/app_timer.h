#ifndef __app_timer__
#define __app_timer__

#include <rtthread.h>
#include "common.h"

#define SYSTEM_TIMER_COUNTER_CNT 2

struct APP_TIMER {
	unsigned int app_timer_second;
	unsigned int app_timer_ms;
};

extern struct APP_TIMER app_timer_data;
extern void init_app_timer(void);

void app_timer_add(struct APP_TIMER *tmr , unsigned int ms);
int app_timer_cmp(struct APP_TIMER *tmr1 , struct APP_TIMER *tmr2); //tmr1 > tmr2 == 1 ; tmr1 < tmr2 == -1
int app_timer_copy(struct APP_TIMER *tmr1 , struct APP_TIMER *tmr2); //tmr2 copy tmr1 会有误差

extern unsigned short system_timer_counter[SYSTEM_TIMER_COUNTER_CNT];  //系统时间计数器，用于ms级别的判断，精度为10ms

typedef struct {
	
	//setting
	unsigned char type;					//类型
	unsigned int ticks;				//周期时间
	
	//private:
	unsigned char num;
	unsigned long ticksleft;
	unsigned int his_time;

	//public:
	void (*timeout)(void *arg);
	void *timeout_arg;

	//Atomic:
	unsigned char enable;
	unsigned char alloc;
}EI_TIMER;


extern EI_TIMER ei_timer[EI_TIMER_COUNT];

EI_TIMER *alloc_timer(void);
void timer_start(EI_TIMER *timer);
void destory_timer(EI_TIMER *timer);

#define EI_TIMER_TYPE_ONE 1


#endif
