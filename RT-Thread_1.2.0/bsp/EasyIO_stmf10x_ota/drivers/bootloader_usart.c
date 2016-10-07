/*
 * File      : usart.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006-2013, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 * 2010-03-29     Bernard      remove interrupt Tx and DMA Rx mode
 * 2013-05-13     aozima       update for kehong-lingtai.
 */

#include "stm32f10x.h"
//#include "stm32l1xx.h"
#include "usart.h"
#include "board.h"

#include <rtdevice.h>

/* USART1 */
#define UART1_GPIO_TX		GPIO_Pin_9
#define UART1_GPIO_RX		GPIO_Pin_10
#define UART1_GPIO			GPIOA

/* USART2 */
#define UART2_GPIO_TX	    GPIO_Pin_2
#define UART2_GPIO_RX	    GPIO_Pin_3
#define UART2_GPIO	    	GPIOA

/* USART3_REMAP[1:0] = 00 */
#define UART3_GPIO_TX		GPIO_Pin_10
#define UART3_GPIO_RX		GPIO_Pin_11
#define UART3_GPIO_CTS		GPIO_Pin_13
#define UART3_GPIO_RTS		GPIO_Pin_14
#define UART3_GPIO			GPIOB

/* STM32 uart driver */
struct stm32_uart
{
    USART_TypeDef* uart_device;
    IRQn_Type irq;
};


struct SERIAL_CFG{
	unsigned char flag;
	struct rt_serial_device *device;
	struct serial_configure *cfg;
};

struct SERIAL_CFG serial_cfg[3] = {0,0,0,0,0,0,0,0,0};
static rt_err_t stm32_configure(struct rt_serial_device *serial, struct serial_configure *cfg)
{
    struct stm32_uart* uart;
    USART_InitTypeDef USART_InitStructure;

    RT_ASSERT(serial != RT_NULL);
    RT_ASSERT(cfg != RT_NULL);

    uart = (struct stm32_uart *)serial->parent.user_data;
	
		if (uart->uart_device == USART1)
		{
			serial_cfg[0].flag = 1;
			serial_cfg[0].device = serial;
			serial_cfg[0].cfg = cfg;
			
		}
		if (uart->uart_device == USART2)
		{
			serial_cfg[1].flag = 1;
			serial_cfg[1].device = serial;
			serial_cfg[1].cfg = cfg;
		}
		if (uart->uart_device == USART3)
		{
			serial_cfg[2].flag = 1;
			serial_cfg[2].device = serial;
			serial_cfg[2].cfg = cfg;
		}

    USART_InitStructure.USART_BaudRate = cfg->baud_rate;

    if (cfg->data_bits == DATA_BITS_8)
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;

    if (cfg->stop_bits == STOP_BITS_1)
        USART_InitStructure.USART_StopBits = USART_StopBits_1;
    else if (cfg->stop_bits == STOP_BITS_2)
        USART_InitStructure.USART_StopBits = USART_StopBits_2;
		
		//addbyliudan
		USART_InitStructure.USART_Parity = USART_Parity_No;
		
////		//����Ǵ���2��.. ��Ϊ ISP �������Ҫ����Ϊ����ģʽ.
//		if (uart->uart_device == USART2)
//		{
//			USART_InitStructure.USART_Parity = USART_Parity_Even;
//			USART_InitStructure.USART_WordLength = USART_WordLength_9b;
//		}
		
		//��Ϊ���ģ�飬������Ϊ����żУ�飬������ʱ��git ��Ϊ���ģʽ
		//if (uart->uart_device == USART3)
		//{
			//USART_InitStructure.USART_Parity = USART_Parity_Odd;
		//}
		
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(uart->uart_device, &USART_InitStructure);

    /* Enable USART */
    USART_Cmd(uart->uart_device, ENABLE);
    /* enable interrupt */
    USART_ITConfig(uart->uart_device, USART_IT_RXNE, ENABLE);

    return RT_EOK;
}

void __set_uart2_mod(char mod)
{
	switch(mod)
	{
		case 0:
		{
			break;
		}
		
		case 1:
		{
			break;
		}
	}
		
}

static rt_err_t stm32_control(struct rt_serial_device *serial, int cmd, void *arg)
{
    struct stm32_uart* uart;

    RT_ASSERT(serial != RT_NULL);
    uart = (struct stm32_uart *)serial->parent.user_data;

    switch (cmd)
    {
    case RT_DEVICE_CTRL_CLR_INT:
        /* disable rx irq */
        UART_DISABLE_IRQ(uart->irq);
        break;
    case RT_DEVICE_CTRL_SET_INT:
        /* enable rx irq */
        UART_ENABLE_IRQ(uart->irq);
        break;
    }

    return RT_EOK;
}

static int stm32_putc(struct rt_serial_device *serial, char c)
{
    struct stm32_uart* uart;

    RT_ASSERT(serial != RT_NULL);
    uart = (struct stm32_uart *)serial->parent.user_data;

    while (!(uart->uart_device->SR & USART_FLAG_TXE));
    uart->uart_device->DR = c;

    return 1;
}

static int stm32_getc(struct rt_serial_device *serial)
{
    int ch;
    struct stm32_uart* uart;

    RT_ASSERT(serial != RT_NULL);
    uart = (struct stm32_uart *)serial->parent.user_data;

    ch = -1;
    if (uart->uart_device->SR & USART_FLAG_RXNE)
    {
        ch = uart->uart_device->DR & 0xff;
    }

    return ch;
}

static const struct rt_uart_ops stm32_uart_ops =
{
    stm32_configure,
    stm32_control,
    stm32_putc,
    stm32_getc,
};

#if defined(RT_USING_UART1)
/* UART1 device driver structure */
struct serial_ringbuffer uart1_int_rx;
struct stm32_uart uart1 =
{
    USART1,
    USART1_IRQn,
};
struct rt_serial_device serial1;

void USART1_IRQHandler(void)
{
    struct stm32_uart* uart;

    uart = &uart1;

    /* enter interrupt */
    rt_interrupt_enter();
    if(USART_GetITStatus(uart->uart_device, USART_IT_RXNE) != RESET)
    {
        rt_hw_serial_isr(&serial1);
        /* clear interrupt */
        USART_ClearITPendingBit(uart->uart_device, USART_IT_RXNE);
    }
    if (USART_GetITStatus(uart->uart_device, USART_IT_TC) != RESET)
    {
        /* clear interrupt */
        USART_ClearITPendingBit(uart->uart_device, USART_IT_TC);
    }

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif /* RT_USING_UART1 */

#if defined(RT_USING_UART2)
/* UART1 device driver structure */
struct serial_ringbuffer uart2_int_rx;
struct stm32_uart uart2 =
{
    USART2,
    USART2_IRQn,
};
struct rt_serial_device serial2;

void USART2_IRQHandler(void)
{
    struct stm32_uart* uart;

    uart = &uart2;

    /* enter interrupt */
    rt_interrupt_enter();
    if(USART_GetITStatus(uart->uart_device, USART_IT_RXNE) != RESET)
    {
        rt_hw_serial_isr(&serial2);
        /* clear interrupt */
        USART_ClearITPendingBit(uart->uart_device, USART_IT_RXNE);
    }
    if (USART_GetITStatus(uart->uart_device, USART_IT_TC) != RESET)
    {
        /* clear interrupt */
        USART_ClearITPendingBit(uart->uart_device, USART_IT_TC);
    }

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif /* RT_USING_UART2 */

#if defined(RT_USING_UART3)
/* UART1 device driver structure */
struct serial_ringbuffer uart3_int_rx;
struct stm32_uart uart3 =
{
    USART3,
    USART3_IRQn,
};
struct rt_serial_device serial3;

//extern void __parser_cmux_pkg(unsigned char data);
void USART3_IRQHandler_x(void)
{
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
	{
		unsigned char rx_char = USART_ReceiveData(USART3);
		USART_ClearITPendingBit(USART3,USART_IT_RXNE); //
		//__parser_cmux_pkg(rx_char);
		
	}
	//
}

void USART3_IRQHandler(void)
{
    struct stm32_uart* uart;
	
    uart = &uart3;

    /* enter interrupt */
    rt_interrupt_enter();
    if(USART_GetITStatus(uart->uart_device, USART_IT_RXNE) != RESET)
    {
        rt_hw_serial_isr(&serial3);
        /* clear interrupt */
        USART_ClearITPendingBit(uart->uart_device, USART_IT_RXNE);
    }
    if (USART_GetITStatus(uart->uart_device, USART_IT_TC) != RESET)
    {
        /* clear interrupt */
        USART_ClearITPendingBit(uart->uart_device, USART_IT_TC);
    }

    /* leave interrupt */
    rt_interrupt_leave();
}
#endif /* RT_USING_UART3 */

static void RCC_Configuration(void)
{
#ifdef RT_USING_UART1
    /* Enable UART GPIO clocks */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    /* Enable UART clock */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
#endif /* RT_USING_UART1 */

#ifdef RT_USING_UART2
    /* Enable UART GPIO clocks */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    /* Enable UART clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
#endif /* RT_USING_UART2 */

#ifdef RT_USING_UART3
    /* Enable UART GPIO clocks */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    /* Enable UART clock */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
#endif /* RT_USING_UART3 */
}

static void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;

#ifdef RT_USING_UART1
    /* Configure USART Rx/tx PIN */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = UART1_GPIO_RX;
    GPIO_Init(UART1_GPIO, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = UART1_GPIO_TX;
    GPIO_Init(UART1_GPIO, &GPIO_InitStructure);
#endif /* RT_USING_UART1 */

#ifdef RT_USING_UART2
    /* Configure USART Rx/tx PIN */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = UART2_GPIO_RX;
    GPIO_Init(UART1_GPIO, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = UART2_GPIO_TX;
    GPIO_Init(UART2_GPIO, &GPIO_InitStructure);
#endif /* RT_USING_UART2 */

#ifdef RT_USING_UART3
    /* Configure USART Rx/tx PIN */
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Pin = UART3_GPIO_RX;
    GPIO_Init(UART3_GPIO, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = UART3_GPIO_TX;
    GPIO_Init(UART3_GPIO, &GPIO_InitStructure);
#endif /* RT_USING_UART3 */
}

static void NVIC_Configuration(struct stm32_uart* uart)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enable the USART1 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = uart->irq;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void rt_hw_usart_init(void)
{
    struct stm32_uart* uart;
    struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
		struct serial_configure config2 = RT_SERIAL_CONFIG_DEFAULT;

    RCC_Configuration();
    GPIO_Configuration();

#ifdef RT_USING_UART1
    uart = &uart1;
#ifdef EASYIO_BOARD_2
    config.baud_rate = BAUD_RATE_9600;
#endif
#ifdef EASYIO_BOARD_1
		config.baud_rate = BAUD_RATE_115200;
#endif
#ifdef EASYIO_UBLOX_U2
		config.baud_rate = BAUD_RATE_115200;
#endif
	#ifdef EASYIO_SIM800C_BRD
		config.baud_rate = BAUD_RATE_115200;
#endif

    serial1.ops    = &stm32_uart_ops;
    serial1.int_rx = &uart1_int_rx;
    serial1.config = config;

    NVIC_Configuration(&uart1);

    /* register UART1 device */
    rt_hw_serial_register(&serial1, "uart1",
                          RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX | RT_DEVICE_FLAG_STREAM,
                          uart);
#endif /* RT_USING_UART1 */

#ifdef RT_USING_UART2
    uart = &uart2;

#ifdef EASYIO_BOARD_2
    config.baud_rate = BAUD_RATE_115200;
#endif
#ifdef EASYIO_BOARD_1
		config.baud_rate = BAUD_RATE_115200;
#endif
#ifdef EASYIO_UBLOX_U2
		config.baud_rate = BAUD_RATE_115200;
#endif


    serial2.ops    = &stm32_uart_ops;
    serial2.int_rx = &uart2_int_rx;
    serial2.config = config;

    NVIC_Configuration(&uart2);

    /* register UART1 device */
    rt_hw_serial_register(&serial2, "uart2",
                          RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
                          uart);
#endif /* RT_USING_UART2 */

#ifdef RT_USING_UART3
    uart = &uart3;
		
#if  (defined EASYIO_BOARD_1) || (defined EASYIO_BOARD_2)
		config.baud_rate = BAUD_RATE_115200;
#endif

#ifdef EASYIO_UBLOX_U2
    config.baud_rate = BAUD_RATE_115200;
#endif


    serial3.ops    = &stm32_uart_ops;
    serial3.int_rx = &uart3_int_rx;
    serial3.config = config;

    NVIC_Configuration(&uart3);

    /* register UART1 device */
    rt_hw_serial_register(&serial3, "uart3",
                          RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
                          uart);
#endif /* RT_USING_UART3 */
}