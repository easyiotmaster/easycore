#ifndef __isp_private__
#define __isp_private__

enum {
	ISP_STAT_NORMAL,
	ISP_STAT_NULL,
};
struct PRIV_DATA {
	char isp_sys_state;
};

extern struct PRIV_DATA privdata;
void init_private_data(void);



#endif