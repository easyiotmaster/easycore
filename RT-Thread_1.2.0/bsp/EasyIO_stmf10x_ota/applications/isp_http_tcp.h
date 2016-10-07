#ifndef __isp_http_tcp__
#define __isp_http_tcp__


extern unsigned int isp_address_offset;
extern unsigned int isp_program_size;
typedef enum {
	ISP_NULL,
	ISP_RECV_GEN_DATA,
	ISP_HANDSHAKE_STM32,
	ISP_HANDSHAKE_STM32_2,
	ISP_ERASE_FLASH_FLAG,
	ISP_ERASE_FLAHH_ALL,
	ISP_PROGRAM,
	ISP_PROGRAM_ERROR,
	ISP_PROGRAM_DONE,
	
	ISP_HANDSHAKE_AVR,
	ISP_PROG_AVR,
	ISP_PROG_AVR_A,
	ISP_PROG_AVR_B,
	ISP_PROG_AVR_DONE,
	ISP_PROG_AVR_ERR,
	
}ISP_STATUS_STEP;
void set_isp_step(ISP_STATUS_STEP step);
ISP_STATUS_STEP get_isp_step(void);

void set_xmpp_sender(char *sender);
void init_isp_serial_port(char *port);

//--------------------------------------------------------------------------------------


enum {
	HTTP_TIMEOUT,
	HTTP_ABORT,
	HTTP_UPLOADFINISH,
	HTTP_DOWNLOADINISH,
};

struct HTTP_OPT {
	short uploadlenth;		//目标文件长度
	
	int (*get_upload_data)(struct HTTP_OPT *opt);
	int (*recv_http_response)(struct HTTP_OPT *opt, unsigned char *buf , int lenth);
	void (*done)(int type);
	
	unsigned char *buf;
	int buflen;
};

extern struct HTTP_OPT httpopt;

void init_isp_http_cli(void);
void create_isp_thread(void);
void isp_http_tcp_routing(void *p);
char isp_start_http_download(char *web , char *sender);
char isp_start_http_upload(char *web , char *sender);
void isp_stop_http(void);
void isp_restart_http_download(void);


//--------------------------------------------------------------------------------------



void put2ispuart(unsigned char *buf , int lenth);
void __test_upload_http(void);
void __test_upload_pic_serialcam(int pic_length);
void write_to_isp_serial(char *buf , int length);



#endif
