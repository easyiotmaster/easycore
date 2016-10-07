#ifndef __flash_opt__
#define __flash_opt__

#include <rtthread.h>
#include <stm32f10x_flash.h>

#if defined (STM32F10X_MD) || defined (STM32F10X_MD_VL)
 #define PAGE_SIZE                         (0x400)    /* 1 Kbyte */
 #define FLASH_SIZE                        (0x20000)  /* 128 KBytes */
#elif defined STM32F10X_CL
 #define PAGE_SIZE                         (0x800)    /* 2 Kbytes */
 #define FLASH_SIZE                        (0x40000)  /* 256 KBytes */
#elif defined STM32F10X_HD
 #define PAGE_SIZE                         (0x800)    /* 2 Kbytes */
 #define FLASH_SIZE                        (0x80000)  /* 512 KBytes */
#elif defined STM32F10X_XL
 #define PAGE_SIZE                         (0x800)    /* 2 Kbytes */
 #define FLASH_SIZE                        (0x100000) /* 1 MByte */
#else 
 #error "Please select first the STM32 device"    
#endif


void FLASH_ProgramStart(u32 addr , u32 size);
u32 FLASH_AppendOneByte(u8 Data);
u32 FLASH_AppendBuffer(u8 *Data , u32 size);
void FLASH_AppendEnd(void);
u32 FLASH_WriteBank(u8 *pData, u32 addr, u16 size);
void FLASH_ProgramDone(void);


int __write_flash(u32 StartAddr,u16 *buf,u16 len);
int __read_flash(u32 StartAddr,u16 *buf,u16 len);

#define TOTAL_FLASH_SIZE						(1024 * 512)

#define USER_CONFIG_FLASH_SIZE			(1024 * 4)
#define USER_CONFIG_FLASH_ADDRESS		(0x8000000 +  1024 * 512 - USER_CONFIG_FLASH_SIZE)

#define BOOTLOADER_SIZE							(1024 * (64+32))					//1C000
#define SYSTEM_APP_SIZE							(1024 * (256 + 128))
#define CHIP_DOWNLOAD_SIZE					(TOTAL_FLASH_SIZE - USER_CONFIG_FLASH_SIZE - SYSTEM_APP_SIZE - BOOTLOADER_SIZE) //204 - 128 k


#define BOOTLOADER_ADDRESS					0x8000000
#define SYSTEM_APP_ADDRESS					0x8000000 + BOOTLOADER_SIZE
#define CHIP_DOWNLOAD_ADDR					0x8000000 + BOOTLOADER_SIZE + SYSTEM_APP_SIZE


#endif