#ifndef __spi_flash_zone__
#define __spi_flash_zone__





/*

SPI Flash ������

+---------------+-----1024*1024*2
+Free Zone 			+
+---------------+-----1024*512;
+Download Zone	+
+---------------+-----0x0
*/



#define DOWNLOAD_ZONE_ADDRESS							0x0
#define DOWNLOAD_ZONE_SIZE								(1024*512)

/**
	��ʼ��ȡSPIFLASH download ���ݣ�����DOWNLOAD ZONE filesize
*/
unsigned int start_get_down_zone_spiflash(void);

/**
	��ȡFile ���ݣ�ÿ�η��ػ�ȡ�ĳ��ȣ������ȡ�ĳ���Ϊ �� 0 ˵���Ѿ������ļ�ĩβ
*/
short get_down_zone_buffer(unsigned char *out , short out_size);

/**
	����ƫ��������ȡ�ļ����ݣ�ISP���ص�ʱ����õ������ַ�ʽ
*/
int get_down_zone_buffer_offset(unsigned int offset , unsigned char *out , short out_size);

/**
	��ȡDOWNLOAD ZONE �е��ļ�MD5ֵ
*/
void check_down_zone_buffer_md5(char *md5_out);

#endif
