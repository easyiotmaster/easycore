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
	
	unsigned char valid_sat_count; //��Ч����
	unsigned char in_use_sat_count;//����ʹ�õ�����
	unsigned char available_sat_count; //���õ���������

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