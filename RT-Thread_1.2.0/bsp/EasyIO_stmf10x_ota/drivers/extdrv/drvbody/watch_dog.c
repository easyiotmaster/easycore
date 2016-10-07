#include "watch_dog.h"

#include <rtthread.h>
#include "common.h"

void watch_dog_config(void)
{
   	/* Enable write access to IWDG_PR and IWDG_RLR registers */
 	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);	
	/* IWDG counter clock: 32KHz(LSI) / 32 = 1KHz */
	IWDG_SetPrescaler(IWDG_Prescaler_128);	
	/* Set counter reload value T=(fn(????)/4)*0.1*RLR(?????)  */
	IWDG_SetReload(0x0fff);	  //500ms
	/* Reload IWDG counter?? */
	IWDG_ReloadCounter();	
	//IWDG_Enable();

}

void enable_wg()
{
	
	watch_dog_config();
	IWDG_Enable();
}

void feed_watchdog(void *p)
{
	IWDG_ReloadCounter();
	//
}
