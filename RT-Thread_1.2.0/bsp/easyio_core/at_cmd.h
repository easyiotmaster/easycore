#ifndef __at_cmd__
#define __at_cmd__

enum {
	AT_RESP_LOCK = -2,
	AT_RESP_EMPTY = -1,
	AT_RESP_ERROR = 0,
	AT_RESP_OK,
//	AT_RESP_CREG_0_1,
//	AT_RESP_CREG_0_5,
//	AT_RESP_CONNECT,
};

#define MODEM_IMEI_LEN 16
#define SIMCARD_CCID_LEN 20
extern char modem_imei[MODEM_IMEI_LEN];
extern char modem_card_id[MODEM_IMEI_LEN+1];
extern char simcard_ccid[SIMCARD_CCID_LEN];
extern unsigned int network_code;
extern unsigned char gsm_signal;

typedef struct __APN_TABLE {
	unsigned int code;
	char apn[32];
	char username[32];
	char password[32];
}APN_TABLE;
extern const APN_TABLE *get_apn(unsigned int code,char index);
extern const char get_apn_cnt(unsigned int code);

int AT_AT(const char *resp , int len);
int AT_CGDCONT(const char *resp , int len);
int AT_ATD(const char *resp , int len);
int AT_PLUSPLUSPLUS(const char *resp , int len);
int AT_CSQ(const char *resp , int len);
int AT_GSN(const char *resp , int len);
int AT_CREG(const char *resp , int len);
int AT_COPS(const char *resp , int len);
int AT_CPIN(const char *resp , int len);
int AT_UPINCNT(const char *resp , int len);
int AT_CGATT(const char *resp , int len);
int AT_CMGS(const char *resp , int len);
int AT_CCID(const char *resp , int len);
int AT_ULSTFILE(const char *resp , int len);
int AT_UPSND(const char *resp , int len);


int AT_INTERRUPT(const char *input_str , int len);

extern void (*smsrecv)(const char *phone_num , const char *body , const char *at);
extern void (*__ATINT)(const char *input_str , int len);

//at_command("AT+CMGF=1\r\n",AT_AT,200,&at_cmd_ret_code);
//#define ENABLE_SMS_TEXTMOD 	at_command("AT+CNMI=1,1\r\n",AT_AT,200,&at_cmd_ret_code);



#endif
