#ifndef __modem_serial__
#define __modem_serial__

#include <rtthread.h>

extern rt_device_t modem_serial_fd;
//rt_device_write(modem_serial_fd,0,at_cmd,cmdlen);
int read_serial_data(unsigned char *buf , int len , rt_int32_t time);
rt_err_t open_modem_serial_port(char *portname);
rt_err_t at_command(char *at_cmd ,int(*at_cb)(const char*resp,int resplen) , int to , int *atcmd_respcode);
rt_err_t at_command_wait(char *at_cmd ,int(*at_cb)(const char*resp,int resplen) , int to , int *atcmd_respcode);
rt_err_t cmux_command(unsigned char *cmux_cmd , int len  ,int(*at_cb)(const char*resp,int resplen) , int to , int *atcmd_respcode);
#endif
