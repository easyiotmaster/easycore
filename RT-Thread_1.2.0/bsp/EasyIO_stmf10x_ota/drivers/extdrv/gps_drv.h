#ifndef __gps_drv__
#define __gps_drv__

#include <rtthread.h>

extern void init_gps_pin(void);
extern void init_gps_serial(void);

extern void pwr_gps(void);
extern void rst_gps(void);

#endif