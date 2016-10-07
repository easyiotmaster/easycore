#ifndef __easyiot__
#define __easyiot__

typedef enum {
	EASYIOT_NORMAL_STATUS,
	EASYIOT_XMPP_REG_STATUS,
}EASYIOT_STATUS;
	
struct EASYIOT_DATA {
	EASYIOT_STATUS status;
};

extern struct EASYIOT_DATA easyiot_data;

void easyiot_init(void);
void easyiot_routing(void);


#define EASYCORE_VERSION_STR "{\"version_name\":\"V0.8\" , \"version\":41}"

	
#endif