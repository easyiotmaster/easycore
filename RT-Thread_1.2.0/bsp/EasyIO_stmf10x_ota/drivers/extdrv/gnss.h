#ifndef __gnss_h__
#define __gnss_h__

#include "nmea/parse.h"

struct GPS_INFO {
	
	//history data:
	float google_LAT;
	float google_LON;
	
	float LAT;
	float LON;
	
	float	SPEED;
	
	float DIS;
	

	//real data:
	nmeaGPRMC gprmc;
	nmeaGPGSA gpgsa;
	nmeaGPGSV gpgsv[8];
	unsigned char gpgsv_count;
	
	nmeaGPRMC his_gprmc;
	nmeaGPGSA his_gpgsa;
	nmeaGPGSV his_gpgsv[8];
	unsigned char his_gpgsv_count;
	
	int satellite_cnt;
	unsigned char valid;
	
	unsigned char valid_sat_count; //有效星数
	unsigned char in_use_sat_count;//正在使用的星数
	unsigned char available_sat_count; //可用的卫星数量

};


extern struct GPS_INFO gps_info;
extern unsigned char gps_buffer_tmp[];
extern unsigned char gps_buffer_valid[];

void init_gnss(void);
void init_gps_device(void);
void put_gps_rawdata(unsigned char *xbuffer , unsigned int size);
int get_gps_line(const char *in , char *out , unsigned int size , char *buffer);

//unsigned char get_valid_sat_count(void);

#endif