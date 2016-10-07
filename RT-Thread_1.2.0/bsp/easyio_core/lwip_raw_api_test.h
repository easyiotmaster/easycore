#ifndef __lwip_raw_api_test_h__
#define __lwip_raw_api_test_h__


void TCP_Client_Init(unsigned char *hostname , long port);
void TCP_Client_Routing(void);
void TCP_Client_start(void);
void TCP_Client_pause(void);
void TCP_Client_stop(void);


#endif
