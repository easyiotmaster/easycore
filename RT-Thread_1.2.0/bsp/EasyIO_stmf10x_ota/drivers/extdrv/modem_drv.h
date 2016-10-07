#ifndef __modem_drv__
#define __modem_drv__

#include <rtthread.h>

extern void init_modem_pin(void);
extern void init_modem_serial(void);

extern void pwr_modem(void);
extern void rst_modem(void);

#endif