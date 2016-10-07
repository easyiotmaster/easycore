#ifndef __ppp_service__
#define __ppp_service__

typedef enum {
	SYS_NORMAL_STATUS,		//正常态
	SYS_INITMODEM_STATUS,	
	SYS_INITPPP_STATUS,
	SYS_PPPERROR_STATUS,
	SYS_PPPSUCCESS_STATUS,
	
}SYSTEM_STATUS;

typedef enum {
	APP_NORMAL_STATUS,
	
}APP_STATUS;

typedef enum {
	PPPD_STATUS_NONE=0,
	PPPD_STATUS_INIT,
	PPPD_STATUS_INITMODEM,
	PPPD_STATUS_PROTO_START,
	PPPD_STATUS_PROTO_SUCCESS,
	PPPD_STATUS_PROTO_STOP,
	PPPD_STATUS_PROTO_ERROR,
	PPPD_STATUS_PROTO_TIMEOUT = 1000,
}PPPSTATUS;
struct PRIVATEDATA {
	SYSTEM_STATUS system_status;		//系统状态机
	PPPSTATUS pppd_status;
	APP_STATUS app_status;
	char pppconnect_cnt;
};

extern struct PRIVATEDATA privd;




void mainloop(void *p);
extern struct PRIVATEDATA priv;

#endif