#include "easyiot.h"
#include "ppp_service.h"
#include <string.h>

struct EASYIOT_DATA easyiot_data;

void easyiot_init(void)
{
	memset((unsigned char*)&easyiot_data,0x0,sizeof(easyiot_data));
	easyiot_data.status = EASYIOT_NORMAL_STATUS;
	//
}

void easyiot_routing(void){
	
	switch(easyiot_data.status)
	{
		case EASYIOT_NORMAL_STATUS:
			easyiot_data.status = EASYIOT_XMPP_REG_STATUS;
			break;
		case EASYIOT_XMPP_REG_STATUS:
		{
			//…Í«Î
			
			break;
		}
		default:
			break;
	}
	
	switch(privd.pppd_status)
	{
		//
	}
	//
}