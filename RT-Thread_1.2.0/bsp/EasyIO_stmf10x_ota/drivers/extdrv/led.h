#ifndef __led__
#define __led__

#define LED1_PORT				GPIOC
#define LED1_PIN				GPIO_Pin_0
#define LED1_RCC				RCC_APB2Periph_GPIOC

#define LED2_PORT				GPIOC
#define LED2_PIN				GPIO_Pin_1
#define LED2_RCC				RCC_APB2Periph_GPIOC

#define LED3_PORT				GPIOC
#define LED3_PIN				GPIO_Pin_2
#define LED3_RCC				RCC_APB2Periph_GPIOC


void init_led_hw(void);
void led_on(unsigned char index);
void led_off(unsigned char index);

#define DATA_LED_ON			GPIO_ResetBits(LED2_PORT, LED2_PIN);
#define DATA_LED_OFF		LED2_PORT->BSRR = LED2_PIN;

#endif