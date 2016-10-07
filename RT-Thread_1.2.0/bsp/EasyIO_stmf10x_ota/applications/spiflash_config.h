#ifndef __spi_flash_zone__
#define __spi_flash_zone__





/*

SPI Flash 分区表

+---------------+-----1024*1024*2
+Free Zone 			+
+---------------+-----1024*512;
+Download Zone	+
+---------------+-----0x0
*/



#define DOWNLOAD_ZONE_ADDRESS							0x0
#define DOWNLOAD_ZONE_SIZE								(1024*512)

/**
	开始获取SPIFLASH download 数据，返回DOWNLOAD ZONE filesize
*/
unsigned int start_get_down_zone_spiflash(void);

/**
	获取File 内容，每次返回获取的长度，如果获取的长度为 是 0 说明已经到了文件末尾
*/
short get_down_zone_buffer(unsigned char *out , short out_size);

/**
	按照偏移量来获取文件内容，ISP下载的时候就用到了这种方式
*/
int get_down_zone_buffer_offset(unsigned int offset , unsigned char *out , short out_size);

/**
	获取DOWNLOAD ZONE 中的文件MD5值
*/
void check_down_zone_buffer_md5(char *md5_out);

#endif
