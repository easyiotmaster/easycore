#ifndef __serial_srv__
#define __serial_srv__

#include <rtthread.h>

#define SERIAL_COUNT 1

typedef void (*SERIAL_CB)(unsigned char *buf , int length);
extern SERIAL_CB serial_handle[SERIAL_COUNT];

rt_err_t init_user_serial_service(char *serial1_name , char *serial2_name);

#endif
