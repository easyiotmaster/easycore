#ifndef __my_stdc_func_h__
#define __my_stdc_func_h__

#define UBYTE unsigned char
#define CHAR char

extern int str_to_int(char * buf , int len);
extern int ascii_2_hex(UBYTE *O_data, UBYTE *N_data, int len);
extern int hex_2_ascii(UBYTE *data, CHAR *buffer, int len);

int get_parmer(unsigned char *str,unsigned char *name,unsigned char *context,unsigned char chr);
int atohl(char *str,char hex);

void get_genstr(char *buffer);


#define SETBIT(x,n) x = ~((~x)&(~(1<<n)))
#define CLEARBIT(x,n) x = x&(~(1<<n))
#define GETBIT(x,n) (x&(1<<n)) >> n

extern void set_4bit_h(unsigned char * _byte , unsigned char val);
extern void set_4bit_l(unsigned char * _byte , unsigned char val);
extern unsigned char get_4bit_h(unsigned char _byte);
extern unsigned char get_4bit_l(unsigned char _byte);

#endif
