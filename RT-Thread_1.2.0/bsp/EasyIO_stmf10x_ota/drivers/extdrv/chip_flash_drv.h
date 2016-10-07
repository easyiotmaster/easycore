#ifndef __chip_flash_drv__
#define __chip_flash_drv__

#include <rtthread.h>

void FLASH_ProgramStart(u32 addr , u32 size);
u32 FLASH_AppendOneByte(u8 Data);
u32 FLASH_AppendBuffer(u8 *Data , u32 size);
void FLASH_AppendEnd(void);
u32 FLASH_WriteBank(u8 *pData, u32 addr, u16 size);
void FLASH_ProgramDone(void);


#endif