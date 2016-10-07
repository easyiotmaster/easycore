#ifndef __MMA845X_H__
#define __MMA845X_H__

#include <stm32f10x.h>
#include "app_timer.h"

void init_mma845x(void);
void start_systimer(void);
void init_mma8452_int(void);

extern unsigned int mma8452_moving_time ;
extern unsigned int mma8452_stay_time;


//静止或停留的时间
#define DEVICE_MOV_TIME ((app_timer_data.app_timer_second - mma8452_moving_time)%app_timer_data.app_timer_second)
#define DEVICE_STAY_TIME ((app_timer_data.app_timer_second - mma8452_stay_time)%app_timer_data.app_timer_second)


void mma845x_routing(void *p);
void init_mma8452_int(void);
void init_mma845x(void);


#define MMA845X_IS_MOVING 1
//#define MMA845X_IS_MOVING GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13)

#endif