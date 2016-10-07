#include "at_cmd.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "my_stdc_func/debugl.h"
#include "my_stdc_func/my_stdc_func.h"
#include "app_timer.h"


void (*__ATINT)(const char *input_str , int len) = 0;




const APN_TABLE apntable[] = {

	
{46692,"internet","",""},
{46692,"emome","",""},

{46601,"internet","",""},
{46601,"f3prepaid","",""},

{46697,"internet","",""},
{46697,"twm","",""},

{46689,"internet","",""},
{46689,"viboone","",""},
{46689,"vibo","",""},



//HK
{45400,"mobile","",""},
{45400,"NWMOBILE","",""},

{45402,"mobile","",""},
{45402,"NWMOBILE","",""},

{45403,"mobile.lte.three.com.hk","",""},
{45403,"mobile.three.com.hk","",""},

{45404,"mms-g.three.com.hk","",""},
{45404,"web-g.three.com.hk","",""},

{45406,"smartone","",""},

{45408,"truphone.com","",""},

{45410,"hkcsl","",""},

{45412,"cmhk","",""},

{45413,"cmhk","",""},

{45415,"smartone","",""},

{45416,"pccw","",""},

{45417,"smartone","",""},

{45418,"mobile","",""},
{45418,"NWMOBILE","",""},

{45419,"pccw","",""},


//CN
{46000,"cmnet","",""},
{46000,"cmwap","",""},

{46001,"3gnet","",""},
{46001,"3gwap","",""},
{46001,"uninet","",""},
{46001,"uniwap","",""},
{46001,"","",""},

{46002,"cmnet","",""},
{46002,"cmwap","",""},

{46007,"cmnet","",""},
{46007,"cmwap","",""},


//UK
{23401,"internet","",""},

{23402,"mobile.o2.co.uk","",""},
{23402,"payandgo.o2.co.uk","",""},
{23402,"wap.o2.co.uk","",""},

{23408,"internet.btonephone.com","",""},
{23408,"mms.btonephone.com","",""},

{23410,"payandgo.o2.co.uk","vertigo","vertigo"},
{23410,"mobile.o2.co.uk","",""},
{23410,"wap.o2.co.uk","",""},
{23410,"prepay.tesco-mobile.com","",""},
{23410,"giffgaff.com","",""},

{23411,"mobile.o2.co.uk","",""},
{23411,"payandgo.o2.co.uk","",""},
{23411,"wap.o2.co.uk","",""},
{23411,"mobile.o2.co.uk","",""},
{23411,"payandgo.o2.co.uk","",""},
{23411,"wap.o2.co.uk","",""},

{23415,"internet","",""},
{23415,"wap.vodafone.co.uk","wap","wap"},
{23415,"pp.vodafone.co.uk","wap","wap"},
{23415,"asdamobiles.co.uk","",""},
{23415,"payg.talkmobile.co.uk","",""},
{23415,"talkmobile.co.uk","",""},
{23415,"mobile.talktalk.co.uk","",""},
{23415,"btmobile.bt.com","",""},
{23415,"payg.mobilebysainsburys.co.uk","",""},
{23415,"uk.lebara.mobi","",""},

{23420,"three.co.uk","",""},
{23420,"3hotspot","",""},

{23426,"data.lycamobile.co.uk","",""},

{23430,"goto.virginmobile.uk","",""},
{23430,"everywhere","",""},
{23430,"eezone","",""},
{23430,"internet.btonephone.com","",""},
{23430,"mms.btonephone.com","",""},
{23430,"btmobile.bt.com","",""},
{23430,"mms.bt.com","",""},
{23430,"btmobile.bt.com","",""},
{23430,"mms.bt.com","",""},

{23431,"everywhere","",""},
{23431,"eezone","",""},

{23432,"everywhere","",""},
{23432,"eezone","",""},

{23433,"everywhere","",""},
{23433,"eezone","",""},
{23433,"tslpaygnet","",""},
{23433,"tslmms","",""},

{23434,"everywhere","",""},
{23434,"eezone","",""},
{23434,"orangeinternet","",""},
{23434,"orangemms","",""},

{23425,"truphone.com","",""},

{23450,"mms","",""},
{23450,"pepper","",""},

{23455,"internet","",""},
{23455,"mms","",""},

{23458,"3gpronto","",""},
{23458,"mms.manxpronto.net","",""},
{23458,"mms.prontogo.net","",""},
{23458,"web.manxpronto.net","",""},

{23486,"everywhere","",""},
{23486,"eezone","",""},


//GM
{26201,"internet.t-mobile","t-mobile","tm"},
{26201,"internet.t-mobile","tm","tm"},
{26201,"","",""},
{26201,"internet.telekom","",""},

{26202,"web.vodafone.de","Vodafone","Vodafone"},
{26202,"web.vodafone.de","",""},
{26202,"event.vodafone.de","",""},
{26202,"","",""},

{26203,"internet.eplus.de","nettokom","nettokom"},
{26203,"internet.eplus.de","",""},
{26203,"mms.eplus.de","",""},

{26207,"internet","",""},
{26207,"pinternet.interkom.de","",""},
{26207,"internet.partner1","",""},
{26207,"pinternet.interkom.de","",""},
{26207,"webmobil1","",""},

{26242,"truphone.com","",""},


//FR
{20801,"free","",""},
{20801,"orange","",""},
{20801,"orange.acte","",""},
{20801,"orange-mib","",""},
{20801,"orange.fr","",""},
{20801,"ofnew.fr","",""},
{20801,"orange.acte","",""},

{20809,"sl2sfr","",""},
{20809,"websfr","",""},

{20810,"","",""},
{20810,"sl2sfr","",""},
{20810,"websfr","",""},
{20810,"fnetnrj","",""},
{20810,"mmsnrj","",""},
{20810,"internetnrj","",""},
{20810,"wap65","",""},
{20810,"mms65","",""},
{20810,"wap66","",""},
{20810,"mms66","",""},
{20810,"fnetcoriolis","",""},
{20810,"mmscoriolis","",""},
{20810,"internetcoriolis","",""},
{20810,"sl2sfr","",""},
{20810,"wap68","",""},
{20810,"mms68","",""},
{20810,"internet68","",""},
{20810,"wap68","",""},

{20815,"free","",""},
{20815,"mmsfree","",""},

{20820,"mmsbouygtel.com","",""},

{20823,"virgin-mobile.fr","",""},

{20826,"fnetnrj","",""},
{20826,"mmsnrj","",""},


//AU
{50501,"telstra.internet","",""},
{50501,"telstra.iph","",""},
{50501,"telstra.wap","",""},
{50501,"telstra.mms","",""},
{50501,"internet","",""},

{50502,"yesinternet","",""},
{50502,"yesbusiness","",""},
{50502,"internet","",""},
{50502,"mms","",""},
{50502,"truphone.com","",""},
{50502,"virgininternet","",""},
{50502,"virginmms","",""},

{50503,"live.vodafone.com","",""},

{50506,"3services","",""},

{50507,"live.vodafone.com","",""},
{50507,"vfinternet.au","",""},

{50511,"Telstra.mms","",""},
{50511,"Telstra.wap","",""},

{50512,"3netaccess","",""},
{50512,"3services","",""},

{50571,"Telstra.mms","",""},
{50571,"Telstra.wap","",""},

{50572,"Telstra.mms","",""},
{50572,"Telstra.wap","",""},

{50588,"live.vodafone.com","",""},
{50588,"vfinternet.au","",""},

{50590,"internet","",""},
{50590,"mms","",""},

{50599,"live.vodafone.com","",""},


//PL
{26001,"plus","",""},
{26001,"internet","",""},
{26001,"mms","",""},

{26002,"","",""},
{26002,"internet","",""},
{26002,"mms","",""},
{26002,"heyah.pl","",""},
{26002,"heyahmms","",""},

{26003,"internet","",""},
{26003,"Internet","internet","internet"},
{26003,"Internet","Internet","Internet"},
{26003,"mms","mms","mms"},

{26006,"internet","",""},
{26006,"mms","",""},

{26033,"truphone.com","",""},


//MM
{41401,"mptnet","",""},
{41401,"default","",""},


//KH
{45601,"default","",""},

{45602,"default","",""},
{45602,"smart","",""},

{45604,"default","",""},

{45605,"default","",""},

{45606,"default","",""},
{45606,"smart","",""},

{45608,"default","",""},

{45609,"default","",""},

{45611,"default","",""},

{45618,"default","",""},


//MO
{45500,"smartweb","",""},
{45500,"smartgprs","",""},

{45501,"ctm-mobile","",""},
{45501,"ctmmms","",""},
{45501,"ctmprepaid","",""},

{45503,"mobile.three.com.mo","",""},

{45504,"ctm-mobile","",""},
{45504,"ctmmms","",""},
{45504,"ctmprepaid","",""},

{45505,"mobile.three.com.mo","",""},

//yindu
//aircelgprs
{405800,"aircelgprs","",""},

//SG
{52501,"e-ideas","",""},
{52501,"hicard","",""},

{52502,"e-ideas","",""},
{52502,"hicard","",""},

{52503,"sunsurf","",""},
{52503,"miworld","65","user123"},

{52504,"sunsurf","",""},
{52504,"miworld","65","user123"},

{52505,"shwap","",""},
{52505,"shmms","65","user123"},


//NL
{20402,"internet.tele2.nl","",""},

{20404,"truphone.com","",""},
{20404,"live.vodafone.com","vodafone","vodafone"},
{20404,"VZWINTERNET","",""},
{20404,"VZWADMIN","",""},
{20404,"VZWIMS","",""},
{20404,"VZWAPP","",""},
{20404,"internet.cs4glte.com","Uniroam@inet.cs.com","cs3g"},
{20404,"admin.cs4glte.com","",""},

{20408,"KPN4G.nl","",""},
{20408,"portalmmm.nl","",""},
{20408,"rabo","",""},

{20412,"internet","",""},
{20412,"","",""},

{20416,"internet","",""},
{20416,"mms","tmobilemms","tmobilemms"},
{20416,"internet.ben","",""},
{20416,"basic.internet.ben.data","",""},
{20416,"mms.ben","",""},

{20420,"","",""},
{20420,"internet","",""},
{20420,"mms","tmobilemms","tmobilemms"},

};


static unsigned int history_code = 0;
static unsigned char code_index = 0;

const char get_apn_cnt(unsigned int code)
{
	char cnt = 0;
	int i=0;
	for(i=0;i<(sizeof(apntable)/sizeof(APN_TABLE));i++)
	{
		if (apntable[i].code == code)
		{
			cnt ++;
		}
	}
	
	return cnt;
	
}

const APN_TABLE *get_apn(unsigned int code ,char index)
{
	int i=0;
	char cnt = 0;
	for(i=0;i<(sizeof(apntable)/sizeof(APN_TABLE));i++)
	{
		if (apntable[i].code == code)
		{
			if(cnt == index)
			{
				return &apntable[i];
			}

		}
	}
	
	return 0;
	
}



int AT_AT(const char *resp , int len)
{
	if (strstr(resp,"OK"))
		return AT_RESP_OK;
	else
		return AT_RESP_ERROR;
	//
}

int AT_CREG(const char *resp , int len)
{
	//REG: 0,1
	if (strstr(resp,"REG: 0,1"))
	{
		return AT_RESP_OK;
	}
	else 
	if (strstr(resp,"REG: 0,5"))
	{
		return AT_RESP_OK;
	}
	else
		return AT_RESP_ERROR;
}

//AT+CGDCONT
int AT_CGDCONT(const char *resp , int len)
{
	if (strstr(resp,"OK"))
		return AT_RESP_OK;
	else
		return AT_RESP_ERROR;
	//
}

int AT_ATD(const char *resp , int len)
{
	if (strstr(resp,"CONNECT"))
		return AT_RESP_OK;
	else
		return AT_RESP_EMPTY;
	//
}

unsigned char gsm_signal = 0;
int AT_CSQ(const char *resp , int len)
{
	if (strstr(resp,"OK"))
	{
		int i=0;
		char *h = strstr(resp,"+CSQ: ");
		char *a = strstr(resp,",");
		
		h += 6;
		a[0] = 0x0;
		
		sscanf(h,"%d",&gsm_signal);
		
		return AT_RESP_OK;
	}
	else
		return AT_RESP_ERROR;
	//
}

int AT_CMGS(const char *resp , int len)
{
	if (strstr(resp,"<"))
	{
		return AT_RESP_OK;
	}
	return AT_RESP_OK;
	//
}

char modem_card_id[MODEM_IMEI_LEN+1] = {0x0};
int get_card_id(char *imei,char *out){

	char keySet[15][2]={{14,0},{3,1},{5,2},{7,3},{8,4},{2,5},{1,6},{12,7},{10,8},{4,9},{11,10},{13,11},{6,12},{9,13},{0,14}};
	int keyNum=7431;
	long long carid;
	int i;
	char temp[16+1] = {0x0};
	unsigned int sum = 0;
	
	if(strlen(imei) != 15)
		return 0;
	
	sscanf(imei,"%lld",&carid);
	carid+=keyNum;
	snprintf(temp,sizeof(temp),"%lld",carid);

	for(i=0;i<MODEM_IMEI_LEN-1;i++){
		out[keySet[i][1]] = temp[keySet[i][0]];
		sum+=temp[keySet[i][1]]-'0';
	 }
	 
	out[15] = (char)(sum%10)+'0';
	out[16] = 0x0;
	DEBUGL->debug("Device Card ID: [%s]\r\n",out);
	return 1;
}


char modem_imei[MODEM_IMEI_LEN] = {0x0};
int AT_GSN(const char *resp , int len)
{
	if (strstr(resp,"ERROR"))
	{
		modem_imei[0] = 0x0;
		return AT_RESP_ERROR;
	}
	
	else if (strstr(resp,"OK"))
	{
		
		char *_imei = strstr(resp,"OK");
		
		//判断后面是否还有字符
		char *tmp = strstr(&_imei[1],"OK");
		if (tmp != 0)
		{		
			return AT_RESP_ERROR;
		}
		
		_imei -= (15+4);
		_imei[15] = 0x0;
		snprintf(modem_imei,sizeof(modem_imei),"%s",_imei);
		if ( strlen(modem_card_id) <= 0) {
			get_card_id(modem_imei,modem_card_id);
		}
		
		DEBUGL->debug("modem_imei : [%s]\r\n",modem_imei);
		return AT_RESP_OK;
	}
	
	return AT_RESP_ERROR;
}

char simcard_ccid[SIMCARD_CCID_LEN] = {0x0};
int AT_CCID(const char *resp , int len)
{
	if (strstr(resp,"ERROR"))
	{
		modem_imei[0] = 0x0;
		return AT_RESP_ERROR;
	}
	
	else if (strstr(resp,"OK"))
	{
		
		char *_ccid = strstr(resp,"OK");
		
		//判断后面是否还有字符
		char *tmp = strstr(&_ccid[1],"OK");
		if (tmp != 0)
		{		
			return AT_RESP_ERROR;
		}
		
		_ccid -= (20+4);
		_ccid[20] = 0x0;
		snprintf(simcard_ccid,sizeof(modem_imei),"%s",_ccid);
		//DEBUGL->debug("_imei : [%s]\r\n",_imei);
		return AT_RESP_OK;
	}
	
	return AT_RESP_ERROR;
}

int AT_ULSTFILE(const char *resp , int len)
{
	//检查警报声档名是否存在
	if (strstr(resp,"sensor.wav"))
	{
		return AT_RESP_OK;
	}

	return AT_RESP_ERROR;
}
int AT_UPSND(const char *resp , int len)
{
	if (strstr(resp,"0.0.0.0") || strstr(resp,"ERROR"))
	{
		return AT_RESP_ERROR;
	}

	return AT_RESP_OK;
}

unsigned int network_code = 0;
int AT_COPS(const char *resp , int len)
{
	unsigned int mycode;
	char code_str[32];
	
	if (strstr(resp,"OK"))
	{
		if (get_str_fragment(resp,"\"","\"",code_str,28) == 0)
		{
			sscanf(code_str,"%d",&network_code);
			DEBUGL->debug("NET CODE: %d \r\n",network_code);
			return AT_RESP_OK;
		}
	}
	return AT_RESP_ERROR;
	//
}

int AT_CPIN(const char *resp , int len)
{
	
	if (strstr(resp,"READY"))
	{
		return AT_RESP_OK;
	}else if (strstr(resp,"SIM PIN") ){
		//SIM Lock. 
		return AT_RESP_LOCK;
	}
	return AT_RESP_ERROR;

}

int AT_UPINCNT(const char *resp , int len)
{
	//SIM解锁次数剩余3次
	if (strstr(resp,"+UPINCNT: 3"))
	{
		return AT_RESP_OK;
	}
	return AT_RESP_ERROR;
}

int AT_CGATT(const char *resp , int len)
{
	
	if (strstr(resp,"+CGATT: 1"))
	{
		return AT_RESP_OK;
	}
	return AT_RESP_ERROR;

}
int AT_PLUSPLUSPLUS(const char *resp , int len)
{
	return AT_RESP_OK;
	//
}


static void cmti(void *p)
{
	//8613010888500 
	char i;
	char Z = 0x1A;
	char phoneNUM[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	char *smsBody;
	char *p1,*p2,*p3;
	char *recv_str = (char*)p;
	int at_cmd_ret_code;
	
	smsBody = rt_malloc(64+32);
	if (smsBody <= 0)
		return ;
	
	smsBody[0] = 0x0;
	
	//rt_kprintf("STR %s \r\n",recv_str);
	p1 = strstr(recv_str,"\"");
	p2 = strstr(p1+1,"\"");
	
	if (p1 && p2)
	{
		for(i=1;i<(p2-p1);i++)
		{
			phoneNUM[i-1] = p1[i];
		}
	}
	
	//phoneNUM[i] = 0x0;
	
	//rt_kprintf("Phone NUM %s \r\n",phoneNUM);
	
	//rt_kprintf("SMS recv_str [%s] \r\n",recv_str);
	
	p3 = strstr(recv_str,"+CMT:");
	//rt_kprintf("SMS p3 [%s] \r\n",p3);
	p1 = strstr(p3,"\n");
	//rt_kprintf("SMS p1 [%s] \r\n",p1);
	p2 = strstr(p1+1,"\r");
	//rt_kprintf("SMS p2 [%s] \r\n",p2);
	if (p1 && p2)
	{
		for(i=1;i<(p2-p1);i++)
		{
			smsBody[i-1] = p1[i];
		}
	}
	
	
	recv_str[0] = 0;

	//rt_kprintf("SMS Body [%s] \r\n",smsBody);
	
	if ((smsrecv > 0) && (strlen(smsBody) > 0))
	{
		smsrecv(phoneNUM,smsBody,recv_str);
	}
	
	
	/**
	+CMT: "+8613821783003","","16/03/02,15:04:37+32"
	qqqqqhhhshdg 
	*/
//	cmux_at_command(2,"AT+CMGS=\"13821783003\"\r\n",AT_CMGS,10,&at_cmd_ret_code);
//	make_cmux_pkg((unsigned char*)"hello",5,2);
//	make_cmux_pkg((unsigned char*)&Z,1,2);
	
	rt_free(smsBody);
	
}

int AT_INTERRUPT(const char *input_str,int len)
{
	
	static unsigned int CNT_INTERRUPT_TIM = 0;
	static char parser_sms_buffer[128] = "";
	
	
	//rt_kprintf("###############################LEN [%d] INT [%s] \r\n",len,input_str);
	
	if (__ATINT > 0)
	{
			__ATINT(input_str,len);
	}
	
//	if (strstr(input_str,"RING"))
//	{
//		//rt_kprintf("RING !!!!!!!!!!!!!!!!!!!!!!!!!");
//		if (user_app_conf.autohello == 1)
//			post_default_event(ata,0,0,0);
//		else
//			post_default_event(ath,0,0,0);
//		
//		//
//	}
	
	
	if (strstr(input_str,"+CMT"))
	{
		int input_len;
		char *atbuffer;
		CNT_INTERRUPT_TIM = app_timer_data.app_timer_second;
		
		parser_sms_buffer[0] = 0x0;
		
		input_len = strlen(input_str);
		
		if (input_len < 128)
		{
			strncat(parser_sms_buffer,input_str,128);
			DEBUGL->debug("parser_sms_buffer1 : [%s] \r\n",parser_sms_buffer);
			if (post_default_event(cmti,(void*)parser_sms_buffer,0,0)!=RT_EOK);
			{
				//rt_free(atbuffer);
			}
		}
		
		//
	}else
	if ((app_timer_data.app_timer_second - CNT_INTERRUPT_TIM) < 2)
	{
		strncat(parser_sms_buffer,input_str,128-strlen(parser_sms_buffer));
		DEBUGL->debug("parser_sms_buffer2 : [%s] \r\n",parser_sms_buffer);
		if (post_default_event(cmti,(void*)parser_sms_buffer,0,0)!=RT_EOK);
		{
			//rt_free(atbuffer);
		}
		//
	}
	
	
	
	return AT_RESP_OK;
	//
}

void (*smsrecv)(const char *phone_num , const char *body , const char *at) = 0;
