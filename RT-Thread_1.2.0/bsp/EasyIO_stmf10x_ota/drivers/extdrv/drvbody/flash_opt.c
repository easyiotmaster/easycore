#include "flash_opt.h"
#include "common.h"
#define FALSE 0
#define TRUE 1

static u32 ApplicationAddress = 0x801C000;      //APP程序首地址
static u32 ApplicationSize		= 1024*200;          //程序预留空间

#define FLASH_ProgramHalfWord(...)


/*
********************************************************************************
*                           
*                              程序下载部分
*
* File          : Download.C
* Version       : V1.0
* By            : whq
*
* For           : Stm32f10x
* Mode          : Thumb2
* Toolchain     : 
*                   RealView Microcontroller Development Kit (MDK)
*                   Keil uVision
* Description   : 
*
*
* Date          : 2013.2.20
*******************************************************************************/



//static u32 m_EraseCounter = 0;


/*******************************************************************************
* Function Name :unsigned int FLASH_PagesMask(volatile unsigned int Size)
* Description   :计算所要擦除 的页数
* Input         :
* Output        :
* Other         :
* Date          :2013.02.24
*******************************************************************************/
static u32 FLASH_PagesMask(vu32 Size)
{
    u32 pagenumber = 0x0;
    u32 size = Size;

    if ((size % PAGE_SIZE) != 0)
    {
        pagenumber = (size / PAGE_SIZE) + 1;
    }
    else
    {
        pagenumber = size / PAGE_SIZE;
    }
    return pagenumber;
}


/*******************************************************************************
* Function Name :u32 FLASH_WriteBank(u32 *pData, u32 addr, u32 size)
* Description   :写入一块数据
* Input         :pData:数据；addr:数据的地址；size:长度
* Output        :TRUE:成功，FALSE:失败。
* Other         :
* Date          :2013.02.24
*******************************************************************************/
u32 FLASH_WriteBank(u8 *pData, u32 addr, u16 size)
{
    vu16 *pDataTemp = (vu16 *)pData;
    vu32 temp = addr;
//    FLASH_Status FLASHStatus = FLASH_COMPLETE;
//    u32 NbrOfPage = 0;
//
//    NbrOfPage = FLASH_PagesMask(addr + size - ApplicationAddress);
//    for (; (m_EraseCounter < NbrOfPage) && (FLASHStatus == FLASH_COMPLETE); m_EraseCounter++)
//    {
//        FLASHStatus = FLASH_ErasePage(ApplicationAddress + (PAGE_SIZE * m_EraseCounter));
//    }

    for (; temp < (addr + size); pDataTemp++, temp += 2)
    {
				//写数据数据
        FLASH_ProgramHalfWord(temp, *pDataTemp);
        if (*pDataTemp != *(vu16 *)temp)
        {
            return FALSE;
        }
    }

    return TRUE;
}


static u8 buffer[2];
static u8 buffer_index = 0;
static u32 buffer_addr = 0;
u32 FLASH_AppendOneByte(u8 Data)
{
	buffer[buffer_index++] = Data;
	if (buffer_index >= sizeof(buffer))
	{
		FLASH_WriteBank(buffer,buffer_addr,sizeof(buffer));
		buffer_addr += sizeof(buffer);
		buffer_index = 0;
	}
	//
}

u32 FLASH_AppendBuffer(u8 *Data , u32 size)
{
	u32 i=0;
	for(i=0;i<size;i++)
	{
		FLASH_AppendOneByte(Data[i]);
	}
	
	//
}

void FLASH_AppendEnd(void)
{
	if (buffer_index > 0)
	{
		FLASH_WriteBank(buffer,buffer_addr,sizeof(buffer));
		buffer_addr += sizeof(buffer);
	}
	//
}
/*******************************************************************************
* Function Name :void FLASH_ProgramDone(void)
* Description   :烧写启动
* Input         :
* Output        :
* Other         :
* Date          :2013.02.24
*******************************************************************************/
void FLASH_ProgramStart(u32 addr , u32 size)
{
    FLASH_Status FLASHStatus = FLASH_COMPLETE;
    u32 NbrOfPage = 0;
    vu32 EraseCounter = 0;

    FLASH_Unlock();
//    m_EraseCounter = 0;
	
		ApplicationAddress = addr;
		ApplicationSize = size;
	
		buffer_index = 0;
		buffer_addr = ApplicationAddress;

    NbrOfPage = FLASH_PagesMask(ApplicationSize);
    for (; (EraseCounter < NbrOfPage) && (FLASHStatus == FLASH_COMPLETE); EraseCounter++)
    {
        FLASHStatus = FLASH_ErasePage(ApplicationAddress + (PAGE_SIZE * EraseCounter));
    }
}


/*******************************************************************************
* Function Name :void FLASH_ProgramDone(void)
* Description   :烧写结束
* Input         :
* Output        :
* Other         :
* Date          :2013.02.24
*******************************************************************************/
void FLASH_ProgramDone(void)
{
    FLASH_Lock();
}






/********************************* END ****************************************/




//--------------------------------------------------------------------------------

#define printf(...)


int __write_flash(u32 StartAddr,u16 *buf,u16 len)
{
	#if 0
        volatile FLASH_Status FLASHStatus;
        u32 FlashAddr;
        len=(len+1)/2;
        FLASH_Unlock();
        FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGAERR | FLASH_FLAG_WRPERR);
        FlashAddr=StartAddr;
        FLASH_ErasePage(StartAddr);
        while(len--){
                FLASHStatus = FLASH_ProgramHalfWord(FlashAddr,*buf++);
                if (FLASHStatus != FLASH_COMPLETE){
                        printf("FLSH :Error %08X\n\r",FLASHStatus);
                        return -1;
                }
                FlashAddr += 2;
        }
        FLASH_Lock();
				#endif
        return 0;
}
int __read_flash(u32 StartAddr,u16 *buf,u16 len)
{
        u16 *p=(u16 *)StartAddr;
        len=(len+1)/2;
        while(len--){
                *buf++=*p++;
        }
        return 0;
}