#include <rtthread.h>

#include <string.h>
#include <stdio.h>

#include <math.h>

//#include "global_env.h"

#include "nmea/nmea.h"
#include "UBLOX_GPS.h"
#include "MMA845X.h"
#include "gnss.h"

#include "lowpower.h"

//#define rt_kprintf(...)

struct GPS_INFO gps_info;
#define global_gps_info gps_info


const char gps_valid_data[] = "\
$GPRMC,082211.00,A,3905.80078,N,11704.92993,E,0.001,,120914,,,A*75\r\n\
$GPVTG,,T,,M,0.001,N,0.002,K,A*20\r\n\
$GPGGA,082211.00,3905.80078,N,11704.92993,E,1,05,3.71,47.1,M,-5.9,M,,*74 \r\n\
$GPGSA,A,3,23,02,06,13,10,,,,,,,,5.47,3.71,4.02*01\r\n\
$GPGSV,2,1,08,02,56,305,43,06,64,039,45,09,35,080,41,10,87,168,35*78\r\n\
$GPGSV,2,2,08,13,30,076,45,17,33,138,,23,20,050,36,25,17,312,*7E\r\n\
$GPGLL,3905.80078,N,11704.92993,E,082211.00,A,A*62\r\n\
";


#define BUFFER_LEN 1024
unsigned char gps_buffer_tmp[BUFFER_LEN] = {0x0};
unsigned char gps_buffer_valid[BUFFER_LEN] = {0x0};

static float gps2m (float lat_a, float lng_a, float lat_b, float lng_b);
static int gprmcToGoogleMap( float lat, float lon , float *out_lat , float *out_lon);

/**
Example:
$GPRMC,082211.00,A,3905.80078,N,11704.92993,E,0.001,,120914,,,A*75
$GPVTG,,T,,M,0.001,N,0.002,K,A*20
$GPGGA,082211.00,3905.80078,N,11704.92993,E,1,05,3.71,47.1,M,-5.9,M,,*74
$GPGSA,A,3,23,02,06,13,10,,,,,,,,5.47,3.71,4.02*01
$GPGSV,2,1,08,02,56,305,43,06,64,039,45,09,35,080,41,10,87,168,35*78
$GPGSV,2,2,08,13,30,076,45,17,33,138,,23,20,050,36,25,17,312,*7E
$GPGLL,3905.80078,N,11704.92993,E,082211.00,A,A*62
*/

/**
字符串解析函数 ，用于提取GPS字段信息
*/

int get_gps_line(const char *in , char *out , unsigned int outsize , char *buffer)
{
	//
	char *tmp,*tmp2;
	int i=0,j=0,m,n;

	tmp = strstr(buffer,in);
	if (tmp > 0)
	{
		m = strlen(tmp);
		for(i=0;i<m;i++)
		{
			if (tmp[i] == '\n')
			{
				//判断后面是否还有字符
				tmp2 = strstr(&tmp[i],in);
				if (tmp2 == 0)
				{
					goto xxout;
				}
			}
			out[i] = tmp[i];
			if (i >= (outsize-1))
			{goto xxout;}
		}
	}else
	{
	}

	xxout:
	out[i] = 0x0;
	return i;
}
/**
将GPS坐标转换为Google坐标
*/
static int gprmcToGoogleMap( float lat, float lon , float *out_lat , float *out_lon)
{

	int dd;
	float mm;
	
	dd = (int) (lat/100);
	mm = (float) (lat-dd*100);
	lat= dd + (mm/60);
	
	dd = (int) (lon/100);
	mm = (float) (lon-dd*100);
	lon= dd + (mm/60);

	*out_lat = lat;//(int)(lat*1E6);
	*out_lon = lon;//(int)(lon*1E6);
	
	return 0;
}

/**
解析NEMA_GPGSV字符串，并做缓存处理
*/

static int handler_gpsdata_GPGSV(char *line_str)
{
	unsigned int i=0,j=0;
	int parse_ret = 0;
	unsigned int valid_sat_count = 0;
	nmeaGPGSV gpgsv;
	parse_ret = nmea_parse_GPGSV(line_str,strlen(line_str),&gpgsv);
	
	
	if (parse_ret > 0)
	{
		if (gpgsv.pack_count <= 8)
		{
			memcpy(&global_gps_info.gpgsv[gpgsv.pack_index],&gpgsv,sizeof(nmeaGPGSV));
		}
		
		global_gps_info.gpgsv_count = gpgsv.pack_count;
		
		//获取有效星数，并保存
		for(i=0;i<gpgsv.pack_count;i++)
		{
			for(j=0;j<NMEA_SATINPACK;j++)
			{
				if (global_gps_info.gpgsv[i].sat_data[j].id > 0)
				{
					//判断DB大于 10的为有效值
					if (global_gps_info.gpgsv[i].sat_data[j].sig > 10)
					{
						valid_sat_count ++;
						//
					}
					//
				}
				//
			}
			//
		}
		
		global_gps_info.valid_sat_count = valid_sat_count;
		rt_kprintf("GPGSV visual satellite pkg index %d sat cnt:%d\r\n",gpgsv.pack_index,gpgsv.sat_count);
	}
	//nmeaGPGSV
	
	return parse_ret;
	
}

/**
解析NEMA_GPGSA字符串，并做缓存处理
*/
static int handler_gpsdata_GPGSA(char *line_str)
{
//	char NUM_STR[8];
//	
//	parser_multi_parameter_str(3,line_str,',',NUM_STR,sizeof(NUM_STR));	 //取得卫星数量
//	sscanf(NUM_STR,"%d",&(global_gps_info.satellite_cnt)); //将卫星数量存入
//
	int parse_ret;
	unsigned int i =0;
	unsigned int count = 0;
	nmeaGPGSA gpgsa;
	parse_ret = nmea_parse_GPGSA(line_str,strlen(line_str),&gpgsa);
	
	if (parse_ret > 0)
	{
		memcpy(&global_gps_info.gpgsa,&gpgsa,sizeof(nmeaGPGSA));
		
		rt_kprintf("GPGSAINFO:\r\n");
		for(i=0 ; i<12 ; i++)
		{
			rt_kprintf("INVILED SAT NUM:%d\r\n",gpgsa.sat_prn[i]);
			if (gpgsa.sat_prn[i] > 0)
				count ++;
		}
		
		global_gps_info.in_use_sat_count = count;
		
	
	}
	
	return parse_ret;
	
	
}

/**
获取有效定位卫星数
*/

static unsigned char get_avai_sat_count(void)
{
	unsigned char ret;
	if (global_gps_info.in_use_sat_count >= 6)
	{
		ret = 0;
		//
	} else {
		if (global_gps_info.valid_sat_count < global_gps_info.in_use_sat_count)
		{
			ret = 0;
		}else {
			ret =  global_gps_info.valid_sat_count - global_gps_info.in_use_sat_count;
		}
	}
	
	global_gps_info.available_sat_count = ret;
	
	rt_kprintf("valid sta count : %d \r\n",ret);
	return ret;
}

static int handler_gpsdata_GPRMC(char *line_str)
{
	int parse_ret;
	char *tmp;char tmp_str[16];
	float speed , lat,lon,google_lat,google_lon;
	float gps_dis;
	
	//nmeaGPRMC gprmc;
	parse_ret = nmea_parse_GPRMC(line_str,strlen(line_str),&global_gps_info.gprmc);
	return parse_ret;
	
}

static void update_his_gps_info(void)
{
	char *tmp;char tmp_str[16];
	float speed , lat,lon,google_lat,google_lon;
	float gps_dis;
	
	//判断定位是否成功
	if (global_gps_info.gprmc.status == 'A')
	{
		//标记GPS 状态为 A
		//global_gps_loc_status = GPS_STATUS_A;
		
		lat = global_gps_info.gprmc.lat;
		lon = global_gps_info.gprmc.lon;
		speed = global_gps_info.gprmc.speed * 1.852;
		//global_gps_info.gprmc.heig
		//转换为 GOOGLE 坐标
		gprmcToGoogleMap(lat,lon,&google_lat,&google_lon);
		
		//memcpy(&global_gps_info.gprmc,&gprmc,sizeof(nmeaGPRMC));
		
		
		//第一次开机，只要能够定位到，就保存GPS 信息
		if(gps_buffer_valid[0] == 0)
		{
			//google坐标并存储在 gps info 中
				global_gps_info.LAT = lat;
				global_gps_info.LON = lon;
				global_gps_info.google_LAT = google_lat;
				global_gps_info.google_LON = google_lon;
				global_gps_info.SPEED = speed;
				
				//存储原始信息
				//memcpy(&global_gps_info.gprmc,&gprmc,sizeof(nmeaGPRMC));
				snprintf((char*)gps_buffer_valid,sizeof(gps_buffer_valid),"%s",gps_buffer_tmp);
			
				return ;
		}
		

		//判断是否移动
		if (DEVICE_MOV_TIME > 0)
		{
			
			//判断GPS是否移动超过10米
			gps_dis = gps2m(google_lat,google_lon,global_gps_info.google_LAT,global_gps_info.google_LON);
			
			global_gps_info.DIS = gps_dis;
			//缓存数据
			if (fabs(gps_dis) > 10)
			{
				//google坐标并存储在 gps info 中
				global_gps_info.LAT = lat;
				global_gps_info.LON = lon;
				global_gps_info.google_LAT = google_lat;
				global_gps_info.google_LON = google_lon;
				global_gps_info.SPEED = speed;
				
				memcpy(&global_gps_info.gprmc,&global_gps_info.his_gprmc,sizeof(nmeaGPRMC));
			}
		}
		
	}
	
	
}

static void handler_gpsdata2(char *buffer , unsigned int buffer_len)
{
	char *tmpdata = rt_malloc(128);
	char *tmpdata2;
	unsigned char tmpdata_index = 0;
	unsigned int i=0;
	//判断动态内存是否分配成功
	if (tmpdata > 0)
	{
		
		tmpdata2 = rt_malloc(buffer_len);
		if (tmpdata2 <=0)
		{
			rt_free(tmpdata);
			return ;
		}
		
		snprintf(tmpdata2,buffer_len,"");
		
		//判断是否到缓冲结束
		while(buffer[i] != 0x00)
		{
			if (i>=buffer_len)
				break;
			
			if (buffer[i] > 127)
				break;
				
			//判断是否到行末
			if ((buffer[i] == '\r') || (buffer[i] == '\n'))
			{
				//判断是否为空行
				if (tmpdata_index == 0)
				{//ignore
				}
				else
				{
					//添加行结束位
					tmpdata[tmpdata_index++] = 0x0;
					rt_kprintf("GPS_LINE: %s \r\n",tmpdata);
					if(tmpdata_index > 10)
					{
						if ((tmpdata[4] == 'S')&&(tmpdata[5] == 'A'))//GPGSA
						{
							tmpdata[2] = 'P';
							if (handler_gpsdata_GPGSA(tmpdata) > 0)
								strcat(tmpdata2,tmpdata);
							//
						}else
						if ((tmpdata[4] == 'M')&&(tmpdata[5] == 'C'))//GPRMC
						{
							tmpdata[2] = 'P';
							if (handler_gpsdata_GPRMC(tmpdata) > 0)
								strcat(tmpdata2,tmpdata);
							//
						}else
						if ((tmpdata[4] == 'S')&&(tmpdata[5] == 'V'))//GPGSV
						{
							if (handler_gpsdata_GPGSV(tmpdata) > 0)
								strcat(tmpdata2,tmpdata);
							//
						}else
						//if ((tmpdata[4] == 'S')&&(tmpdata[5] == 'A'))//GPGSA
						{
							//handler_gpsdata_GPGSA(tmpdata);
						}
					}
					
					//清楚一行数据
					tmpdata_index = 0;
					//
				}
				
			}else
			{
				tmpdata[tmpdata_index++] = buffer[i];
			}
			i++;
		}
		
		//保存解析后的字符串
		snprintf((char*)gps_buffer_valid,sizeof(gps_buffer_valid),"%s",tmpdata2);
		
		//释放内存
		rt_free(tmpdata2);
		rt_free(tmpdata);
	}
	
	//
	update_his_gps_info();
	//计算可用卫星数
	get_avai_sat_count();
	
}

/**
计算GPS位移
*/
#include <math.h>
static float gps2m (float lat_a, float lng_a, float lat_b, float lng_b)
{
	#define PI 3.1415926
	#define EARTH_RADIUS 6378137.0
	float radLat1 = (lat_a * PI / 180.0);
	float radLat2 = (lat_b * PI / 180.0);
	float a = radLat1 - radLat2;
	float b = (lng_a - lng_b) * PI / 180.0;
	float s = 2 * asin(sqrt(pow(sin(a / 2), 2)
	+ cos(radLat1) * cos(radLat2)
	* pow(sin(b / 2), 2)));
	s = s * EARTH_RADIUS;
	s = round(s * 10000) / 10000;
	return s;
	//
}

/**
GPS 串口 调用接口
*/
void put_gps_rawdata(unsigned char *xbuffer , unsigned int size)
{
	static unsigned char input_data_loopcode = 0;
	return;
	switch(input_data_loopcode)
	{
		case 0:
		case 2:
			break;
		case 1:
			snprintf((char*)gps_buffer_tmp,sizeof(gps_buffer_tmp),"%s",(char*)xbuffer); //缓存数据
			break;
		case 3:
			handler_gpsdata2((char*)xbuffer,size);	//解析数据
			
			input_data_loopcode = 0;
			break;
		default:
			input_data_loopcode = 0;
			break;
	}
	input_data_loopcode++;
}


/**
GPS信息测试接口
*/
void test_gps_buffer(void)
{
	put_gps_rawdata((unsigned char*)gps_valid_data,sizeof(gps_valid_data));
	//
}

void init_gnss(void)
{
	memset(&gps_info,0x0,sizeof(gps_info));
	gps_info.DIS = -1;
}