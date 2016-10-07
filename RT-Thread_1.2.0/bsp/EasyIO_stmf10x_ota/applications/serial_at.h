#ifndef __serial_at__
#define __serial_at__

#include "json/cJSON.h"

struct SERIAL_AT {
	char at[128];
	void (*cb)(char*);
	void *p;
};

struct SERIAL_JSON {
	char name[128];
	void (*cb)(cJSON *json_in , cJSON * json_out);
};

struct SERIAL_JSON_RET {
	char retcode;
	char retstr[64];
};

char *serial_at_str_handle(char *atstr , int length);
char *xmpp_at_str_handle(const char *atstr , int length);
char *json_xmpp_msg(char *from , char *msg);
char *at_xmpp_msg(char *from , char *msg);


#endif
