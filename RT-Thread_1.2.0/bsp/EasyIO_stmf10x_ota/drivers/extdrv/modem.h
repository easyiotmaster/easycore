#ifndef __modem_h__
#define __modem_h__


extern void init_modem_gpio(void);
extern void init_modem(void);
extern void disable_dtr(void);
extern void enable_dtr(void);

extern unsigned char read_modem_status(void);




#endif