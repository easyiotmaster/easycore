#include "my_stdc_func.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
int str_to_int(char * buf , int len)
{
	char xbuf[10]; int ret = -1;
	memset(xbuf,0x0,sizeof(buf));
	memcpy(xbuf,buf,len);
	sscanf(xbuf,"%d",&ret);
	return ret;

	//
}


int ascii_2_hex(UBYTE *O_data, UBYTE *N_data, int len) 
{ 
	int i,j,tmp_len;
	UBYTE tmpData; 
	UBYTE *O_buf = O_data;
	UBYTE *N_buf = N_data;
	for(i = 0; i < len; i++)
	{
		if ((O_buf[i] >= '0') && (O_buf[i] <= '9')) 
		{ 
			tmpData = O_buf[i] - '0'; 
		} 
		else  if ((O_buf[i] >= 'A') && (O_buf[i] <= 'F'))  //A....F 
		{ 
			tmpData = O_buf[i] - 0x37; 
		} 
		else if((O_buf[i] >= 'a') && (O_buf[i] <= 'f'))  //a....f 
		{ 
			tmpData = O_buf[i] - 0x57; 
		} 
		else 
		{ 
			return -1; 
		} 
		O_buf[i] =  tmpData; 
	} 
	for(tmp_len = 0,j = 0; j < i; j+=2) 
	{ 
		N_buf[tmp_len++] = (O_buf[j]<<4) | O_buf[j+1]; 
	} 
	return tmp_len;  
}

int hex_2_ascii(UBYTE *data, CHAR *buffer, int len) 
{ 
    const CHAR  ascTable[17] = {"0123456789ABCDEF"}; 
    CHAR *tmp_p = buffer; 
    int i, pos; 
    pos = 0; 
    for(i = 0; i < len; i++) 
    { 
         tmp_p[pos++] = ascTable[data[i] >> 4]; 
         tmp_p[pos++] = ascTable[data[i] & 0x0f]; 
 	} 
    tmp_p[pos] = '\0'; 
    return pos;               
}


int ftoa(char *str, float num, int n)        //n是转换的精度，即是字符串'.'后有几位小数
{
    int     sumI;
    float   sumF;
    int     sign = 0;
    int     temp;
    int     count = 0;

    char *p;
    char *pp;

    if(str == NULL) return -1;
    p = str;

    /*Is less than 0*/
    if(num < 0)
    {
        sign = 1;
        num = 0 - num;
    }

    sumI = (int)num;    //sumI is the part of int
    sumF = num - sumI;  //sumF is the part of float

    /*Int ===> String*/
    do
    {
        temp = sumI % 10;
        *(str++) = temp + '0';
    }while((sumI = sumI /10) != 0);


    /*******End*******/
 

    if(sign == 1)
    {
        *(str++) = '-';
    }

    pp = str;
    
    pp--;
    while(p < pp)
    {
        *p = *p + *pp;
        *pp = *p - *pp;
        *p = *p -*pp;
        p++;
        pp--;
    }

    *(str++) = '.';     //point

    /*Float ===> String*/
    do
    {
        temp = (int)(sumF*10);
        *(str++) = temp + '0';

        if((++count) == n)
            break;
    
        sumF = sumF*10 - temp;

    }while(!(sumF > -0.000001 && sumF < 0.000001));

    *str = '/0';

    return 0;

}


int get_parmer(unsigned char *str,unsigned char *name,unsigned char *context,unsigned char chr)
{
	int cnt=0;
	unsigned char *parm;
	while(*context){
		if((*(context-1)!=chr)&&(*(context-1)!='?')){
			context++;
			continue;
		}
		if((memcmp(context,name,strlen((const char *)name))==0)&&
			(*(context+strlen((const char *)name))=='=')){//名字匹配而且以‘=’开始参数
			parm=context+strlen((const char *)name)+1;//"name=
			context+=(strlen((const char *)name)+1);
			while((*context!=chr)&&(*context!='\0')){//&标示参数结束
				cnt++;
				context++;
			}
			memcpy(str,parm,cnt);//copy parm to str
			*(str+cnt)='\0';
			return 0;
		}
		else
			context++;	
	}
	return -1;//cann't find the parm
}

int atohl(char *str,char hex)
{
        char bit_cnt=0;
        int index=1;
        int num=0;
        char chr;
        while(*str++){
                bit_cnt++;              
        }
        str-=2;
        while(bit_cnt--){
                if(!hex){
                        chr=*str--;
                        if((chr>'0'-1)&&(chr<'9'+1)){
                                num+=(chr-'0')*index;
                                index*=10;
                        }
                        else
                                break;
                }
                else{
                        chr=*str--;
                        if((chr>'0'-1)&&(chr<'9'+1)){
                                num+=(chr-'0')*index;
                                index*=16;
                        }
                        else if((chr>'a'-1)&&(chr<'f'+1)){
                                num+=((chr-'a')+10)*index;
                                index*=16;
                        }
                        else if((chr>'A'-1)&&(chr<'F'+1)){
                                num+=((chr-'A')+10)*index;
                                index*=16;
                        }
                        else
                                break;
                }
        }
        return num;
}

int change_str(char * input)
{
	int i;
	int n = strlen(input);
	for (i=0;i<n;i++)
	{
		if (input[i] == '\r' || input[i] == '\n')
		{
			input[i] = '\0';
			return 0;
		}
	}
	
	return 0;
}


static char ch(char c)
{
	if(c>=97&&c<=122)
		c-=32;
	return c;
}
void str_conv(char *str1)
{
	while(*str1)
	{
		if(*str1>='A'&&*str1<='Z')
		{
			*str1+=('a'-'A');
		}
		str1++;
	}
}

void  checkstr(char *input)
{

        int i = 0;
        int len = strlen(input);
        for(i=0;i<len;i++)
        {
                if ((input[i] < 33 ) || (input[i] > 126 ))
                {
                        input[i] = 0x0;
                        return;
                }
                //
        }

}

int get_parma_count(char *str)
{
	int i = 0;int len;
	int ret = 1;
	char *body =
		strstr(str,"=");
	if (body <= 0)
		return -1;

	body += 1;
	len = strlen(body);
	for(i=0;i<len;i++)
	{
		if (body[i] == ',' )
			ret ++;
		//
	}
	return ret;
}


char* get_parma(int index , char *instr)
{

	char *parma;int i;
	static char in[128];
	//char *in ;
	char *body ;

	//in = rt_malloc(128);
	if (in <=0 ) {return 0;};

	snprintf(in,sizeof(in),"%s",instr);
	body = strstr(in,"=");
	if (body < 0)
	{
		//rt_free(in);
		return 0;
	}
	body = body ++;

	parma = strtok(body,",");
	if (index == 1)
	{
		checkstr(parma);
		//rt_free(in);
		return parma;
	}

	for(i=1;i<index;i++)
	{
		parma = strtok(0,",");
	}

	checkstr(parma);

	//rt_free(in);
	return parma;

}

#if 0
#include <stdio.h>
#include <string.h>
char s[]="direct usb://Canon/MX420%20series?serial=312E7C&interface=1 \"Canon MX420 series\" \"Canon MX420 series\" \"MFG:Canon;CMD:BJL,BJRaster3,BSCCe,NCCe,IVEC,IVECPLI;SOJ:BJNP2,BJNPe;MDL:MX420 series;CLS:PRINTER;DES:Canon MX420　series;　VER:1.020;STA:10;FSI:04;HRI:CHN;MSI:DAT,E3,HASF;PDR:7;PSE:LFBA05000;";
char *p;
char MFG[40];
char MDL[40];
char DES[40];
int main() {
    p=strstr(s,"MFG:");if (p && 1==sscanf(p,"MFG:%39[^;]",MFG)) printf("MFG=[%s]\n",MFG);
    p=strstr(s,"MDL:");if (p && 1==sscanf(p,"MDL:%39[^;]",MDL)) printf("MDL=[%s]\n",MDL);
    p=strstr(s,"DES:");if (p && 1==sscanf(p,"DES:%39[^;]",DES)) printf("DES=[%s]\n",DES);
    return 0;
}
//MFG=[Canon]
//MDL=[MX420 series]
//DES=[Canon MX420　series]
//
#endif
char* get_parma_2(int index , char *instr) //这个函数再取参数的时候不会去掉特殊字符
{

	char *parma;int i;
	static char in[128];
	//char *in ;
	char *body ;

	//in = rt_malloc(128);
	if (in <=0 ) {return 0;};

	snprintf(in,sizeof(in),"%s",instr);
	body = strstr(in,"=");
	if (body < 0)
	{
		//rt_free(in);
		return 0;
	}
	body = body ++;

	parma = strtok(body,",");
	if (index == 1)
	{
		return parma;
	}

	for(i=1;i<index;i++)
	{
		parma = strtok(0,",");
	}

	//rt_free(in);
	return parma;

}

char *parser_multi_parameter_str(int index , char *str_in , char sp, char *str_out , unsigned char out_lmt)
{
	int i=0,j=0,k=0;
	int len = strlen(str_in);
	int p_idx = 0;
	for(i=0;i<len;i++)
	{
		if (str_in[i] == sp) p_idx ++;
		if (p_idx == index)
		{
			if (str_in[i] != sp)
			{
				str_out[j++] = str_in[i];
				if (j>=(out_lmt-1))
				{
					goto out;
				}
			}
			//
		}

		if (p_idx > index)
			goto out;
	}

out:
	str_out[j] = 0x0;
	return str_out;
}

/*
#include <stdio.h>

int main()
{
        char ex[] = "AJOISLDHFWUEHDSFSJKLSJDFKJS";

        int i = 0;
        for(;i<sizeof(ex);i++)
        {
                printf("buffer[%d] = 0x%x;\n",i,ex[i]);
                //
        }
}
*/
void get_genstr(char *buffer)
{
	//AJOISLDHFWUEHDSFSJKLSJDFKJS
	buffer[0] = 0x41;
	buffer[1] = 0x4a;
	buffer[2] = 0x4f;
	buffer[3] = 0x49;
	buffer[4] = 0x53;
	buffer[5] = 0x4c;
	buffer[6] = 0x44;
	buffer[7] = 0x48;
	buffer[8] = 0x46;
	buffer[9] = 0x57;
	buffer[10] = 0x55;
	buffer[11] = 0x45;
	buffer[12] = 0x48;
	buffer[13] = 0x44;
	buffer[14] = 0x53;
	buffer[15] = 0x46;
	buffer[16] = 0x53;
	buffer[17] = 0x4a;
	buffer[18] = 0x4b;
	buffer[19] = 0x4c;
	buffer[20] = 0x53;
	buffer[21] = 0x4a;
	buffer[22] = 0x44;
	buffer[23] = 0x46;
	buffer[24] = 0x4b;
	buffer[25] = 0x4a;
	buffer[26] = 0x53;
	buffer[27] = 0x0;
}


char *get_p(char *str_in , char *str_out , unsigned char out_lmt)
{
	int i=0;
	int len = strlen(str_in);
	unsigned char status = 0;
	unsigned char str_out_index = 0;

	for(i=0;i<len;i++)
	{
		switch(status)
		{
			case 0:
				if ((str_in[i] == '\"') || (str_in[i] == '\''))
				{
					status = 1; //第一次找到了引号
					//
				}
				break;
			case 1:
				if ((str_in[i] == '\"') || (str_in[i] == '\''))
				{
					status = 2; //第二次找到了引号
				}else{
					/* 第一次找到了引号，后就进行数据处理 */
					str_out[str_out_index++] = str_in[i];
					if (str_out_index >= (out_lmt-1))
						status = 3; //没有空间进行数据输出了
				}
				break;
			case 2:
			case 3:
				goto __out;
			default:
				break;
		}
	}
__out:
	str_out[str_out_index] = 0x0; //末尾一个字节为0X0
	if (status == 2)
		return str_in;
	else
		return 0;

}

/**
	example : 
	char input[] = "abc=abc,\"123\"";
	char output[16];
	get_str_fragment(intput,"\"","\"",output,16);
	printf : 123
*/
int get_str_fragment(const char * in_str , const char *start_str  , const char *end_str , char *out_buf , int buflen)
{
        //
        int i;
        unsigned int j;
        char * str1,*str2,*str3;
        int keywordlen = strlen(start_str);
        int inlen = strlen(in_str);

        str1 = strstr(in_str,start_str);
        str2 = strstr(str1+keywordlen,end_str);

        //printf("str1 : %s \r\n",str1);
        //printf("str2 : %s \r\n",str2);

        i=j=0;
        if ((str1>0) && (str2>0) && (str2 > str1))
        {
                j = ((unsigned int)str2 - (unsigned int)str1) - keywordlen;
					
								if ((j<=0) || ( j >= buflen))
								{
									out_buf[0] = 0x0;
									return 0;
								}
					
                //printf("str2-str1 : %d\r\n",j);
                for(i=0;i<j;i++)
                {
                        out_buf[i] = str1[keywordlen + i];
                }
                out_buf[i] = 0x0;
								
								return 0;
                //
        }

        return -1;
}



void set_4bit_h(unsigned char * _byte , unsigned char val)
{
        unsigned char val_tmp,val_tmp2;
        val_tmp = val * 16;
        val_tmp2 =  *_byte * 0x0F;
        *_byte = val_tmp + val_tmp2;
        //
}

void set_4bit_l(unsigned char * _byte , unsigned char val)
{
        unsigned char val_tmp;
        val_tmp =  *_byte & 0xF0;
        val_tmp += val;
        *_byte = val_tmp;
        //
}

unsigned char get_4bit_h(unsigned char _byte)
{
        return _byte >> 4;
}

unsigned char get_4bit_l(unsigned char _byte)
{
        unsigned char val_tmp;
        val_tmp =  _byte << 4;
        return val_tmp >> 4;
}


void GetHost(char * src, char * web, char * file, int * port) {
        char * pA;
        char * pB;
        memset(web, 0, sizeof(web));
        memset(file, 0, sizeof(file));
        *port = 0;
        if(!(*src)) return;
        pA = src;
        if(!strncmp(pA, "http://", strlen("http://"))) pA = src+strlen("http://");
        else if(!strncmp(pA, "https://", strlen("https://"))) pA = src+strlen("https://");
        pB = strchr(pA, '/');
        if(pB) {
                memcpy(web, pA, strlen(pA) - strlen(pB));
                if(pB+1) {
                        memcpy(file, pB + 1, strlen(pB) - 1);
                        file[strlen(pB) - 1] = 0;
                }
        }
        else memcpy(web, pA, strlen(pA));
        if(pB) web[strlen(pA) - strlen(pB)] = 0;
        else web[strlen(pA)] = 0;
        pA = strchr(web, ':');
        if(pA) *port = atoi(pA + 1);
        else *port = 80;
}


int is_string(unsigned char * buf , int len)
{
	int i=0;
	int stringlen =strlen(buf);
	if (stringlen > len)
	{
		return 0;
	}
	
	for(i=0;i<stringlen;i++)
	{
		//if(buf[i].writer[i]>'z'&&buf[i].writer[i]>'a' || buf[i].writer[i]>'Z'&&buf[i].writer[i]>'A')
		if (((buf[i]<='Z')&&(buf[i]>='A'))||((buf[i]<='z')&&(buf[i]>='a')))
		{
		}else
		{
			return 0;
		}
	}
	
	return 1;
	//
}

#include "debugl.h"

void debug_buf(char *name , unsigned char *buf , int len)
{
	int i=0;
	DEBUGL->debug("DEBUG BUFFER (%s):\r\n",name);
	//for(i=0;i<len;i++)
	//{
	//	DEBUGL->debug("%02X ",buf[i]);
	//}
	//DEBUGL->debug("\r\n");
}





#if 1
#define CRC_INIT 0x3AA3
const unsigned short crc_tab16[] =
{
0x0000, 0xc0c1, 0xc181, 0x0140, 0xc301, 0x03c0, 0x0280, 0xc241,
0xc601, 0x06c0, 0x0780, 0xc741, 0x0500, 0xc5c1, 0xc481, 0x0440,
0xcc01, 0x0cc0, 0x0d80, 0xcd41, 0x0f00, 0xcfc1, 0xce81, 0x0e40,
0x0a00, 0xcac1, 0xcb81, 0x0b40, 0xc901, 0x09c0, 0x0880, 0xc841,
0xd801, 0x18c0, 0x1980, 0xd941, 0x1b00, 0xdbc1, 0xda81, 0x1a40,
0x1e00, 0xdec1, 0xdf81, 0x1f40, 0xdd01, 0x1dc0, 0x1c80, 0xdc41,
0x1400, 0xd4c1, 0xd581, 0x1540, 0xd701, 0x17c0, 0x1680, 0xd641,
0xd201, 0x12c0, 0x1380, 0xd341, 0x1100, 0xd1c1, 0xd081, 0x1040,
0xf001, 0x30c0, 0x3180, 0xf141, 0x3300, 0xf3c1, 0xf281, 0x3240,
0x3600, 0xf6c1, 0xf781, 0x3740, 0xf501, 0x35c0, 0x3480, 0xf441,
0x3c00, 0xfcc1, 0xfd81, 0x3d40, 0xff01, 0x3fc0, 0x3e80, 0xfe41,
0xfa01, 0x3ac0, 0x3b80, 0xfb41, 0x3900, 0xf9c1, 0xf881, 0x3840,
0x2800, 0xe8c1, 0xe981, 0x2940, 0xeb01, 0x2bc0, 0x2a80, 0xea41,
0xee01, 0x2ec0, 0x2f80, 0xef41, 0x2d00, 0xedc1, 0xec81, 0x2c40,
0xe401, 0x24c0, 0x2580, 0xe541, 0x2700, 0xe7c1, 0xe681, 0x2640,
0x2200, 0xe2c1, 0xe381, 0x2340, 0xe101, 0x21c0, 0x2080, 0xe041,
0xa001, 0x60c0, 0x6180, 0xa141, 0x6300, 0xa3c1, 0xa281, 0x6240,
0x6600, 0xa6c1, 0xa781, 0x6740, 0xa501, 0x65c0, 0x6480, 0xa441,
0x6c00, 0xacc1, 0xad81, 0x6d40, 0xaf01, 0x6fc0, 0x6e80, 0xae41,
0xaa01, 0x6ac0, 0x6b80, 0xab41, 0x6900, 0xa9c1, 0xa881, 0x6840,
0x7800, 0xb8c1, 0xb981, 0x7940, 0xbb01, 0x7bc0, 0x7a80, 0xba41,
0xbe01, 0x7ec0, 0x7f80, 0xbf41, 0x7d00, 0xbdc1, 0xbc81, 0x7c40,
0xb401, 0x74c0, 0x7580, 0xb541, 0x7700, 0xb7c1, 0xb681, 0x7640,
0x7200, 0xb2c1, 0xb381, 0x7340, 0xb101, 0x71c0, 0x7080, 0xb041,
0x5000, 0x90c1, 0x9181, 0x5140, 0x9301, 0x53c0, 0x5280, 0x9241,
0x9601, 0x56c0, 0x5780, 0x9741, 0x5500, 0x95c1, 0x9481, 0x5440,
0x9c01, 0x5cc0, 0x5d80, 0x9d41, 0x5f00, 0x9fc1, 0x9e81, 0x5e40,
0x5a00, 0x9ac1, 0x9b81, 0x5b40, 0x9901, 0x59c0, 0x5880, 0x9841,
0x8801, 0x48c0, 0x4980, 0x8941, 0x4b00, 0x8bc1, 0x8a81, 0x4a40,
0x4e00, 0x8ec1, 0x8f81, 0x4f40, 0x8d01, 0x4dc0, 0x4c80, 0x8c41,
0x4400, 0x84c1, 0x8581, 0x4540, 0x8701, 0x47c0, 0x4680, 0x8641,
0x8201, 0x42c0, 0x4380, 0x8341, 0x4100, 0x81c1, 0x8081, 0x4040,
};

static unsigned short crc16_update(unsigned short crc, unsigned char ch)
{
	unsigned short tmp;
	unsigned short msg;

	msg = 0x00ff & (unsigned short)ch;
	tmp = crc       ^ msg;
	crc = (crc >> 8) ^ crc_tab16[tmp & 0xff];

	return crc;
}


unsigned short sdk_stream_crc16_calc(const unsigned char* pMsg, unsigned int nLen)
{
	unsigned int i;
	unsigned short wCRC = CRC_INIT;

	for (i = 0; i < nLen; i++)
	{
		wCRC = crc16_update(wCRC, pMsg[i]);
	}

	return wCRC;
}

void filter_str(char *str)
{
	int i=0;
	int len = strlen(str);
	
	for(i=0;i<len;i++)
	{
		if ((str[i] >= 48) && (str[i] <= 57))		//0~9
			continue;
		if ((str[i] >= 65) && (str[i] <= 90))		//大写
			continue;
		if ((str[i] >= 97) && (str[i] <= 122))	//小写
			continue;
		if ((str[i] >= 97) && (str[i] <= 122))	//小写
			continue;
		if (str[i] == ' ')
			continue;
		if (str[i] == '|')
			continue;
		if (str[i] == '[')
			continue;
		if (str[i] == ']')
			continue;
		if (str[i] == '#')
			continue;
		if (str[i] == '$')
			continue;
		if (str[i] == '(')
			continue;
		if (str[i] == ')')
			continue;
		if (str[i] == '-')
			continue;
		if (str[i] == '_')
			continue;
		if (str[i] == '+')
			continue;
		if (str[i] == '!')
			continue;
		if (str[i] == '`')
			continue;
		if (str[i] == '~')
			continue;
		if (str[i] == '&')
			continue;
		if (str[i] == '{')
			continue;
		if (str[i] == '}')
			continue;
		if (str[i] == ',')
			continue;
		if (str[i] == '=')
			continue;
		if (str[i] == '.')
			continue;
		if (str[i] == ':')
			continue;
		if (str[i] == '*')
			continue;
		if (str[i] == '?')
			continue;
		
		str[i] = '*';
		
		
	}
}


#endif


