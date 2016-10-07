#ifndef __ota_h__
#define __ota_h__

enum {
	OTA_STATUS_NORMAL,
	OTA_STATUS_READY,
	OTA_STATUS_UPDATING,
	OTA_STATUS_FINISH,
};

struct OTA_CONFIG{
	unsigned char ota_status;							//OTA����״̬
	unsigned int fw_spiflash_address;			//�̼��洢��ַ
};

#endif