#ifndef __uart4_cam_drv_h__
#define __uart4_cam_drv_h__
#define UART_BUFFER_LENGTH 1024

extern unsigned char uartrx_buffer[UART_BUFFER_LENGTH];
extern unsigned int uartrx_buffer_index;

void config_uart4(void);
void write_uart4_buffer(unsigned char *buffer , int len);

extern unsigned short imglen;
extern unsigned short imgidx;

int xxpaizhao(void);
int getImg(unsigned short block_size);
int tingzhipaizhao(void);
int resetcam(void);

#endif