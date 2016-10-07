#include <stm32f10x.h>
#include <rtthread.h>

#include "IOI2C.h"
#include "MMA845X.h"

#include "app_timer.h"




#define m_define_ic_id 0x2A
#define m_define_device_address_write 0x38
#define m_define_device_address_read 0x39

#define m_define_ctl_reg_1 0x2A
#define m_define_ctl_reg_2 0x2B
#define m_define_ctl_reg_3 0x2C
#define m_define_ctl_reg_4 0x2D
#define m_define_ctl_reg_5 0x2E

#define m_define_ctl_interrupt_status 0x0C
#define m_define_trans_config 0x1D
#define m_define_trans_source 0x1E
#define m_define_trans_threshold 0x1F
#define m_define_trans_count 0x20

#define m_define_xyz_date_config 0x0E
#define m_define_hp_filter_cut_off_config 0x0F

#define m_define_value_ctl_reg_1_standby 0x1C
#define m_define_value_ctl_reg_1_activity 0x1D

#define m_define_value_ctl_reg_2  0x01
#define m_define_value_ctl_reg_3_interrupt_high 0x02
#define m_define_value_ctl_reg_4_trans_mode 0x20
#define m_define_value_ctl_reg_5_pin2 0x00

#define m_define_value_how_much_byte_1 1
#define m_define_value_setting 0x24

#define m_define_value_trans_xyz_date_config 0x10
#define m_define_value_trans_hp_filter_cut_off_config 0x00

#define m_define_value_trans_config 0x0E
#define m_define_value_trans_threshold 0x01

#define m_define_value_trans_count  0x05

unsigned int mma8452_moving_time = 0;
unsigned int mma8452_stay_time = 0;


static void write_i2c_dev(u8 devid , u8 reg , u8 value , u8 num , u8 vs)
{
	IICwriteBytes(devid,reg,1,&value);
}
/**
	初始化MMA8452模块
*/
void init_mma845x(void)
{

	extern unsigned short I2C_Erorr_Count;

	write_i2c_dev(m_define_device_address_write, 
			m_define_ctl_reg_1, 
			m_define_value_ctl_reg_1_standby, 
			m_define_value_how_much_byte_1, 
			m_define_value_setting);

	


	write_i2c_dev(m_define_device_address_write,
			m_define_ctl_reg_2,
			m_define_value_ctl_reg_2,
			m_define_value_how_much_byte_1,
			m_define_value_setting);

	write_i2c_dev(m_define_device_address_write,
			m_define_ctl_reg_3,
			m_define_value_ctl_reg_3_interrupt_high,
			m_define_value_how_much_byte_1,
			m_define_value_setting);

	write_i2c_dev(m_define_device_address_write,
			m_define_ctl_reg_4,
			m_define_value_ctl_reg_4_trans_mode,
			m_define_value_how_much_byte_1,
			m_define_value_setting);

	write_i2c_dev(m_define_device_address_write,
			m_define_ctl_reg_5,
			m_define_value_ctl_reg_5_pin2,
			m_define_value_how_much_byte_1,
			m_define_value_setting);

	write_i2c_dev(m_define_device_address_write,
			m_define_xyz_date_config,
			m_define_value_trans_xyz_date_config,
			m_define_value_how_much_byte_1,
			m_define_value_setting);

	write_i2c_dev(m_define_device_address_write,
			m_define_hp_filter_cut_off_config,
			m_define_value_trans_hp_filter_cut_off_config,
			m_define_value_how_much_byte_1,
			m_define_value_setting);

	write_i2c_dev(m_define_device_address_write,
			m_define_trans_config,
			m_define_value_trans_config,
			m_define_value_how_much_byte_1,
			m_define_value_setting);

	write_i2c_dev(m_define_device_address_write,
			m_define_trans_threshold,
			m_define_value_trans_threshold,
			m_define_value_how_much_byte_1,
			m_define_value_setting);


	write_i2c_dev(m_define_device_address_write,
			m_define_trans_count,
			m_define_value_trans_count,
			m_define_value_how_much_byte_1,
			m_define_value_setting);
			
			
			


	write_i2c_dev(m_define_device_address_write, 
			m_define_ctl_reg_1, 
			m_define_value_ctl_reg_1_standby, 
			m_define_value_how_much_byte_1, 
			m_define_value_setting);

	write_i2c_dev(m_define_device_address_write, 
			m_define_ctl_reg_1, 
			m_define_value_ctl_reg_1_activity, 
			m_define_value_how_much_byte_1, 
			m_define_value_setting);


	write_i2c_dev(m_define_device_address_write, 
			m_define_ctl_reg_1, 
			m_define_value_ctl_reg_1_activity, 
			m_define_value_how_much_byte_1, 
			m_define_value_setting);

			
			
	if (I2C_Erorr_Count == 0)
		rt_kprintf("Init MMA8452Q SUCCESS ! \r\n");
	else
		rt_kprintf("Init MMA8452Q Failed ! \r\n");


}

/**
	初始化MMA8452移动侦测管脚
*/
void init_mma8452_int(void)
{
	
	GPIO_InitTypeDef GPIO_InitStructure;


	/* Enable GPIOC clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	/* Configure PD.03, PC.04, as input floating */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	//GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource13);
	//
}

void mma845x_routing(void *p)
{
	
	//app_timer_data
	static unsigned int start_time = 0;
	static uint8_t status;
	/* 检查MMA8452 是否有移动*/
	status = MMA845X_IS_MOVING;
	if (status == 1)
	{
		//global_gps_mov_status = DEV_MOVING;
		mma8452_stay_time = 0;
		if (mma8452_moving_time == 0)
		{
			mma8452_moving_time = app_timer_data.app_timer_second ;
		}
		//mma8452_moving_time = (app_timer_data.app_timer_second - start_time);
	}else if (status == 0)
	{
		mma8452_moving_time = 0;
		if (mma8452_stay_time == 0)
		{
			mma8452_stay_time = app_timer_data.app_timer_second ;
		}
		//mma8452_stay_time = (app_timer_data.app_timer_second - start_time);
	}
	
	rt_kprintf("mma8452 status MOV:%d STA:%d \r\n",DEVICE_MOV_TIME,DEVICE_STAY_TIME);
}



//static void deal_rising_interrupt(void*p)
//{
//	rt_kprintf("mma85452 Rising interrupt _____\r\n");
//	global_rising_interrupt_time = ApiGetSysSecondCounter();
//	global_falling_interrupt_time = 0;
//	global_gps_mov_status = DEV_MOVING;

//}

//static void deal_falling_interrupt(void*p)
//{
//	rt_kprintf("mma85452 Falling interrupt _____\r\n");
//	global_rising_interrupt_time = 0;
//	global_falling_interrupt_time = ApiGetSysSecondCounter();
//	global_gps_mov_status = DEV_STAY;
//	
//	//global_gps_mov_status = DEV_MOVING;

//}

//static EXTITrigger_TypeDef trigger_type = EXTI_Trigger_Rising;

//static void set_trigger_type(void)
//{
//	EXTI_InitTypeDef EXTI_InitStructure;
//	//设置中断触发状态 
//	EXTI_InitStructure.EXTI_Line = EXTI_Line13 ;
//	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
//	EXTI_InitStructure.EXTI_Trigger = trigger_type;//EXTI_Trigger_Rising;//EXTI_Trigger_Rising_Falling; //EXTI_Trigger_Rising
//	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//	EXTI_Init(&EXTI_InitStructure);
//}

//void EXTI15_10_IRQHandler(void)
//{
//	//改变中断状态
//	
//	if(EXTI_GetITStatus(EXTI_Line13) != RESET)
//	{
//		if (trigger_type == EXTI_Trigger_Rising)
//			ApiAddTask(deal_rising_interrupt,0); // 交给系统去异步处理
//		else
//			ApiAddTask(deal_falling_interrupt,0); // 交给系统去异步处理
//		EXTI_ClearITPendingBit(EXTI_Line13);
//	}
//	
//	if (trigger_type == EXTI_Trigger_Rising)
//	{
//		trigger_type = EXTI_Trigger_Falling;
//	}else
//	{
//		trigger_type = EXTI_Trigger_Rising;
//	}
//	
//	set_trigger_type();
//}


//void config_gpio3_interrupt(void)
//{
//	GPIO_InitTypeDef GPIO_InitStructure;
//	NVIC_InitTypeDef NVIC_InitStructure;
//	EXTI_InitTypeDef EXTI_InitStructure;


//	/* Enable GPIOD clock */
//	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

//	/* Configure PD.03, PC.04, as input floating */
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;

//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
//	GPIO_Init(GPIOC, &GPIO_InitStructure);

//	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource13);


//	/* Enable the EXTI12\3 Interrupt on PC13 */
//	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	NVIC_Init(&NVIC_InitStructure);

//	//设置中断触发状态 
//	EXTI_InitStructure.EXTI_Line = EXTI_Line13 ;
//	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
//	EXTI_InitStructure.EXTI_Trigger = trigger_type;//EXTI_Trigger_Rising;//EXTI_Trigger_Rising_Falling; //EXTI_Trigger_Rising
//	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//	EXTI_Init(&EXTI_InitStructure);

//}

