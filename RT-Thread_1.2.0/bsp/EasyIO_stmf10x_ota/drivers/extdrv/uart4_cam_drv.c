#include "uart4_cam_drv.h"
#include <stm32f10x.h>
#include <rtthread.h>
#include "app_ev.h"

/* USART4_REMAP[1:0] = 00 */
#define UART4_GPIO_TX		GPIO_Pin_10
#define UART4_GPIO_RX		GPIO_Pin_11
#define UART4_GPIO			GPIOC

unsigned char uartrx_buffer[UART_BUFFER_LENGTH];
unsigned int uartrx_buffer_index = 0;


//post_hw_int_event

void UART4_IRQHandler(void)
{
	unsigned char ch = -1;
    /* enter interrupt */
    //rt_interrupt_enter();
    if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)
    {
        /* clear interrupt */
        USART_ClearITPendingBit(UART4, USART_IT_RXNE);
				ch = UART4->DR & 0xff;
			
				uartrx_buffer[uartrx_buffer_index ++] = ch;
			
    }
    if (USART_GetITStatus(UART4, USART_IT_TC) != RESET)
    {
        /* clear interrupt */
        USART_ClearITPendingBit(UART4, USART_IT_TC);
    }

    /* leave interrupt */
    //rt_interrupt_leave();
}

void config_uart4(void)
{
	USART_InitTypeDef tUSART;
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	memset(&GPIO_InitStructure,0x0,sizeof(GPIO_InitStructure));
	memset(&NVIC_InitStructure,0x0,sizeof(NVIC_InitStructure));
	memset(&tUSART,0x0,sizeof(tUSART));
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	/* Configure USART Rx/tx PIN */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = UART4_GPIO_RX;
	GPIO_Init(UART4_GPIO, &GPIO_InitStructure);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = UART4_GPIO_TX;
	GPIO_Init(UART4_GPIO, &GPIO_InitStructure);
	
	/* config USART2 clock */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);

	/* UART4 mode config */
	tUSART.USART_BaudRate = 38400;
	tUSART.USART_WordLength = USART_WordLength_8b;
	tUSART.USART_StopBits = USART_StopBits_1;
	tUSART.USART_Parity = USART_Parity_No;
	tUSART.USART_HardwareFlowControl = USART_HardwareFlowControl_None;		
	tUSART.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_Init(UART4, &tUSART);
	USART_Cmd(UART4, ENABLE);
	USART_LINCmd(UART4, DISABLE);	
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);	 //??????
	USART_ITConfig(UART4, USART_IT_TXE, DISABLE);  

	USART_Cmd(UART4, ENABLE);		 //??UART
	
	/* Enable the USART4 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
		
	USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);
	
	
		
}

void write_uart4_buffer(unsigned char *buffer , int len)
{
	int i=0;
	for(i=0;i<len;i++)
	{
		USART_SendData(UART4, (uint8_t) buffer[i]);
		/* Loop until the end of transmission */
		while (USART_GetFlagStatus(UART4, USART_FLAG_TC) == RESET)
		{}
	}
	//
}


	/*************************************************************************************
	1 ��λָ��:56 00 26 00  ���أ�76 00 26 00
	2 ����ָ��:56 00 36 01 00 ���أ�76 00 36 00 00
	3 ������ͼƬ����ָ��:56 00 34 01 00 ���أ�76 00 34 00 04 00 00 XX YY
	 XX YY -------ͼƬ���ݳ��ȣ�XXΪ��λ�ֽڣ�YYΪ��λ�ֽ�
	4 ��ȡ����ͼƬ����ָ��:56 00 32 0C 00 0A 00 00 XX XX 00 00 YY YY ZZ ZZ
	����:76 00 32 00 00�����ʱ�䣩FF D8.......FF D9�����ʱ�䣩76 00 32 00 00
	00 00 XX XX ------��ʼ��ַ ����ʼ��ַ������8�ı�����һ����00 00��
	00 00 YY YY ------ͼƬ���ݳ��ȣ��ȸ�λ�ֽڣ����λ�ֽڣ�
	ZZ ZZ       --------���ʱ�䣨= XX XX*0.01���� �������С����00 0A��
	ע�⣺JPEGͼƬ�ļ�һ������FF D8��ʼ��FF D9����
	5 ֹͣ����ָ��:56 00 36 01 03���أ�76 00 36 00 00
	6 ��������ͼƬѹ����ָ��:56 00 31 05 01 01 12 04 XX  ���أ�76 00 31 00 00
	XXһ��ѡ36  ����Χ:00 ----FF��
	7 ��������ͼƬ��Сָ��:
	56 00 31 05 04 01 00 19 11 ��320*240�����أ�76 00 31 00 00
	56 00 31 05 04 01 00 19 00 ��640*480��
	8 �޸Ĵ�������:56 00 24 03 01 XX XX    ���أ�76 00 24 00 00
	XX XX        ����
	AE C8         9600
	56  E4        19200
	2A F2         38400
	*************************************************************************************/
	


unsigned short imglen = 0;
unsigned short imgidx = 0;

int getImg(unsigned short block_size)
{
	unsigned char cam_cmd_3[] =  {0x56 ,0x00 ,0x32 ,0x0C ,0x00 ,0x0A ,0x00 ,0x00 ,0xAA ,0xAA ,0x00 ,0x00 ,0xBB ,0xBB ,0xCC ,0xCC};	//��ȡͼƬ
	unsigned short BUFFER_LENGTH = block_size;
	unsigned short READLEN;
	if (imgidx < imglen)
	{
		

		cam_cmd_3[8] = ((unsigned char*)&imgidx)[1];
		cam_cmd_3[9] = ((unsigned char*)&imgidx)[0];
		
		if ((imglen - imgidx) > BUFFER_LENGTH)
		{
			READLEN = BUFFER_LENGTH ;
			cam_cmd_3[12] = ((unsigned char*)&READLEN)[1];
			cam_cmd_3[13] = ((unsigned char*)&READLEN)[0];
			

		} else {
			
			READLEN = imglen - imgidx;
			cam_cmd_3[12] = ((unsigned char*)&READLEN)[1];
			cam_cmd_3[13] = ((unsigned char*)&READLEN)[0];
			//
		}
		
		
		
		//��ʱ
		cam_cmd_3[14] = 0x00;
		cam_cmd_3[15] = 0x0a;
		
		uartrx_buffer_index = 0;
		write_uart4_buffer(cam_cmd_3,sizeof(cam_cmd_3));
		rt_thread_sleep(30);
		
		//У�鷵�ؽ��
		if (READLEN == uartrx_buffer_index - 0x0A)
		{
			imgidx += READLEN;
			return READLEN;
		} else {
			return -1;
		}
		
	}
	
	return 0;
	
	
}


int xxpaizhao(void)
{
	#define UART_CMD_DELAY 100

	int i=0;
	
	unsigned char cam_cmd_0[] =  {0x56 ,0x00 ,0x31 ,0x05 ,0x01 ,0x01 ,0x12 ,0x04 ,0x36};	//ѹ����
	unsigned char cam_cmd_1[] =  {0x56 ,0x00 ,0x36 ,0x01 ,0x00};	//����
	unsigned char cam_cmd_2[] =  {0x56 ,0x00 ,0x34 ,0x01 ,0x00};	//��ȡ����
	unsigned char cam_cmd_4[] =  {0x56 ,0x00 ,0x31 ,0x05 ,0x04 ,0x01 ,0x00 ,0x19 ,0x11}; //����320��40

	
//	uartrx_buffer_index = 0;
//	write_uart4_buffer(cam_cmd_0,sizeof(cam_cmd_0));
//	rt_thread_sleep(UART_CMD_DELAY);
//	if ((uartrx_buffer[1] != 0x00) && (uartrx_buffer[0] != 0x76))
//	{
//		return -1;
//	}
	
	uartrx_buffer_index = 0;
	write_uart4_buffer(cam_cmd_4,sizeof(cam_cmd_4));
	rt_thread_sleep(UART_CMD_DELAY);
	
	if ((uartrx_buffer[1] != 0x00) || (uartrx_buffer[0] != 0x76))
	{
		return -1;
	}
	
	uartrx_buffer_index = 0;
	write_uart4_buffer(cam_cmd_1,sizeof(cam_cmd_1));
	rt_thread_sleep(UART_CMD_DELAY);
	if ((uartrx_buffer[1] != 0x00) || (uartrx_buffer[0] != 0x76))
	{
		return -1;
	}
	
	
	uartrx_buffer_index = 0;
	write_uart4_buffer(cam_cmd_2,sizeof(cam_cmd_2));
	rt_thread_sleep(UART_CMD_DELAY);
	
	if ((uartrx_buffer_index == 9) && (uartrx_buffer[0] == 0x76))
	{
		
		imglen = uartrx_buffer[7]*256 + uartrx_buffer[8];
		imgidx = 0;
		
		return imglen;

	}
	
	return 0;
		
}

int tingzhipaizhao(void)
{
	unsigned char cam_cmd_5[] =  {0x56 ,0x00 ,0x36 ,0x01 ,0x03}; //ֹͣ����
	uartrx_buffer_index = 0;
	write_uart4_buffer(cam_cmd_5,sizeof(cam_cmd_5));
	rt_thread_sleep(UART_CMD_DELAY);
	if ((uartrx_buffer[1] != 0x00) || (uartrx_buffer[0] != 0x76))
	{
		return -1;
	}
	return 0;
	
}

int resetcam(void)
{
	unsigned char resetcmd[] = {0x56 ,0x00 ,0x26 ,0x00};
	uartrx_buffer_index = 0;
	write_uart4_buffer(resetcmd,sizeof(resetcmd));
	rt_thread_sleep(UART_CMD_DELAY);
	if ((uartrx_buffer[1] != 0x00) || (uartrx_buffer[0] != 0x76))
	{
		return -1;
	}
	return 0;
}