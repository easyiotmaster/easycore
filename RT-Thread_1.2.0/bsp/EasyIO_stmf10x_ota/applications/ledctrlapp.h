#ifndef __led_ctrl_app_h__
#define __led_ctrl_app_h__

struct LED_FLICKER{
	
	//public
	unsigned char mode;
	unsigned short busy_ms_max;
	unsigned short idle_ms_max;
	
	//private:
	unsigned short busy_ms;
	unsigned short idle_ms;

};

enum {
	LED_WORK_MOD_FLICKER=0,		//��˸ģʽ
	LED_WORK_MOD_ON = 3,			//����ģʽ
	LED_WORK_MOD_OFF,					//����ģʽ
	LED_WORK_MOD_CUSTOME,			//�û��������ģʽ
};

extern struct LED_FLICKER led_flicker[3];

extern void init_ledctrlapp(void);

#define LED_FAST_FLICKER(EX)		/*EX.mode=LED_WORK_MOD_FLICKER;*/if(EX.mode>=LED_WORK_MOD_ON){EX.mode=LED_WORK_MOD_FLICKER;};EX.busy_ms_max=200;EX.idle_ms_max=200;
#define LED_SLOW_FLICKER(EX)		/*EX.mode=LED_WORK_MOD_FLICKER;*/if(EX.mode>=LED_WORK_MOD_ON){EX.mode=LED_WORK_MOD_FLICKER;};EX.busy_ms_max=200;EX.idle_ms_max=1000;
#define LED_OFF_MOD(EX)					EX.mode=LED_WORK_MOD_OFF;
#define LED_ON_MOD(EX)					EX.mode=LED_WORK_MOD_ON;
#define LED_CUSTOME_MOD(EX)			EX.mode=LED_WORK_MOD_CUSTOME;

#endif