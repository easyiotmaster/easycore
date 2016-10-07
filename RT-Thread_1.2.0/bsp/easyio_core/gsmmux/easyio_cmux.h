#ifndef __easyio_cmux__
#define __easyio_cmux__

#include <rtthread.h>


#define VPORT_AT_TYPE					0
#define VPORT_RAWDATA_TYPE		1
#define VPORT_PPP_TYPE				2

#define CMUX_AT_RESP_LENGTH 64

typedef void (*CMUX_RCV_DATA_CB)(unsigned char port , unsigned char *data , int len);

struct cmux_vport_type {
		unsigned char vport_typel;				/*虚拟串口工作类型*/
		unsigned char at_resp_buf[CMUX_AT_RESP_LENGTH];		/*作为AT命令的缓冲*/
		unsigned char *data;
		int data_len;
		void (*recv_data_cb)(unsigned char *data , int len);	/*数据接收的回调函数*/
		struct rt_semaphore sem;					/*信号量，互斥用的*/
};

struct cmux_private_data {
	CMUX_RCV_DATA_CB cmux_data_cb;
	int serial_fd;
	struct cmux_vport_type vport[4];
	
};


void recv_vport_data(unsigned char port , unsigned char *data , int len);

rt_err_t start_cmux(void);
rt_err_t init_modem_serial_thread(void);
rt_err_t cmux_write(char port , unsigned char *data , int len);
rt_err_t cmux_at_command(char port , char *at_cmd ,int(*at_cb)(const char*resp,int resplen) , int to , int *atcmd_respcode);
rt_err_t cmux_at_command_wait(char port , char *at_cmd ,int(*at_cb)(const char*resp,int resplen) , int to , int *atcmd_respcode);
int cmux_at_command_check_resp(char port , char *at_cmd ,char *respstr, int to);
rt_err_t cmux_ctrl(char port , unsigned char type , void (*recv_data_cb)(unsigned char *data , int len));

rt_err_t __init_cmux_thread(void);
rt_err_t __restart_cmux(void);
void __pause_cmux_thread(unsigned char pause);

extern int make_cmux_pkg(unsigned char *buf , int len , unsigned char channel);



#endif
