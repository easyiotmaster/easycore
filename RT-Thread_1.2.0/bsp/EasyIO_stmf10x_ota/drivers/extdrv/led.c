#include "led.h"
#include <stm32f10x.h>

void init_gpio_hw(void)
{
	//
}


void init_led_hw(void)
{

	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(LED1_RCC,ENABLE);
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin   = (LED1_PIN|LED2_PIN|LED3_PIN);
	GPIO_Init(LED1_PORT, &GPIO_InitStructure);
	
	led_off(1);
	led_off(2);
	led_off(3);
	
}



void led_on(unsigned char index)
{

	switch(index)
	{
		case 1:
			GPIO_ResetBits(LED1_PORT, LED1_PIN);
			break;
		case 2:
			GPIO_ResetBits(LED2_PORT, LED2_PIN);
			break;
		case 3:
			GPIO_ResetBits(LED3_PORT, LED3_PIN);
			break;
		default:
			break;
	}

	//
}

void led_off(unsigned char index)
{

	switch(index)
	{
		case 1:
			GPIO_SetBits(LED1_PORT, LED1_PIN);
			break;
		case 2:
			GPIO_SetBits(LED2_PORT, LED2_PIN);
			break;
		case 3:
			GPIO_SetBits(LED3_PORT, LED3_PIN);
			break;
		default:
			break;
	}

	//
}


void led_trog(unsigned char index)
{

	switch(index)
	{
		case 1:
			GPIO_SetBits(LED1_PORT, LED1_PIN);
			break;
		case 2:
			GPIO_SetBits(LED2_PORT, LED2_PIN);
			break;
		case 3:
			GPIO_SetBits(LED3_PORT, LED3_PIN);
			break;
		default:
			break;
	}

}

