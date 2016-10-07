#ifndef __app_ev__
#define __app_ev__

#include <rtthread.h>

#define APP_EVENT_BUFFER 10

#define APP_EV_DEFAULT_TYPE 0
#define APP_EV_HW_INT_TYPE 1

struct APP_EVENT {
        unsigned char enable; //atom
        unsigned char type;
        unsigned int simple_data[8];
};
typedef void (*EVENT_FUNC_CB)(void*);


extern void appev_thread_entry(void* parameter);
extern rt_err_t post_default_event(void * func , void * arg , void * func_resp , void *arg_resp);
extern rt_err_t post_hw_int_event(void * func , void * arg , void * func_resp , void *arg_resp);
extern rt_err_t post_default_event_wait(void * func , void * arg , void * func_resp , void *arg_resp , unsigned int timeout);
extern rt_err_t post_hw_int_event_wait(void * func , void * arg , void * func_resp , void *arg_resp , unsigned int timeout);


#endif
