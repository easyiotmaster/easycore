#include "w25q16.h"




void SPIx_Init(void) 
{ 
    /*定义结构体，下面NSS配置时也要用到GPIO所以这里一起定义*/ 
SPI_InitTypeDef  SPI_InitStructure; 
GPIO_InitTypeDef GPIO_InitStructure; 

/* 使能 SPI1 & GPIOA 时钟 */	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
         RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1|RCC_APB2Periph_AFIO, ENABLE); 

/* Configure SPI1 pins: NSS, SCK, MISO and MOSI */ 
         GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7; 
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; 
GPIO_Init(GPIOA, &GPIO_InitStructure); 

//SPI1 NSS for   W25X16                    
         GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4; 
         GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
         GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
         GPIO_Init(GPIOC, &GPIO_InitStructure); 

GPIO_SetBits(GPIOC, GPIO_Pin_4); 

    //SPI1 NSS for Ethernet 
          GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4; 
          GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
          GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
          GPIO_Init(GPIOA, &GPIO_InitStructure); 

GPIO_SetBits(GPIOA, GPIO_Pin_4);   

/* SPI1 configuration */ 
          SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; //SPI1设置为两线全双工 
SPI_InitStructure.SPI_Mode = SPI_Mode_Master;	                   //设置SPI1为主模式 
SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;                  //SPI发送接收8位帧结构 
SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;	               //串行时钟在不操作时，时钟为高电平 
SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;	               //第二个时钟沿开始采样数据 
SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;	               //NSS信号由软件（使用SSI位）管理 
SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8; //定义波特率预分频的值:波特率预分频值为8 
SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	   //数据传输从MSB位开始 
SPI_InitStructure.SPI_CRCPolynomial = 7;	   //CRC值计算的多项式 
SPI_Init(SPI1, &SPI_InitStructure); 
/* Enable SPI1  */ 
SPI_Cmd(SPI1, ENABLE);   //使能SPI1外设 
} 




////GPIO初始化
//void SPI3_GPIO_Init(void)
//{
//	GPIO_InitTypeDef GPIO_InitStructure;
//	
//	memset(&GPIO_InitStructure,0x0,sizeof(GPIO_InitStructure));
//	
//	
//	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA,ENABLE);
//	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB,ENABLE);
//	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC,ENABLE);
//	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOD,ENABLE);

//	RCC_AHBPeriphClockCmd(W25X_FLASH_CS_GPIO_CLK , ENABLE);
//	RCC_AHBPeriphClockCmd ( W25X_FLASH_SPI_MOSI_GPIO_CLK | W25X_FLASH_SPI_MISO_GPIO_CLK |
//			W25X_FLASH_SPI_SCK_GPIO_CLK, ENABLE);

//	/*!&lt; W25X_FLASH_SPI Periph clock enable */
//	RCC_APB1PeriphClockCmd(W25X_FLASH_SPI_CLK, ENABLE); //
//	
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; 
//  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;  
//	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
//	
//	/*!&lt; Configure W25X_FLASH_SPI pins: SCK */
//	GPIO_InitStructure.GPIO_Pin = W25X_FLASH_SPI_SCK_PIN;
//	GPIO_Init(W25X_FLASH_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

//	/*!&lt; Configure W25X_FLASH_SPI pins: MOSI */
//	GPIO_InitStructure.GPIO_Pin = W25X_FLASH_SPI_MOSI_PIN;
//	GPIO_Init(W25X_FLASH_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

//	/*!&lt; Configure W25X_FLASH_SPI pins: MISO */
//	GPIO_InitStructure.GPIO_Pin = W25X_FLASH_SPI_MISO_PIN;
//	GPIO_Init(W25X_FLASH_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

//	/*!&lt; Configure W25X_FLASH_CS_PIN pin: W25X_FLASH Card CS pin */
//	GPIO_InitStructure.GPIO_Pin = W25X_FLASH_CS_PIN;
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
//	GPIO_Init(W25X_FLASH_CS_GPIO_PORT, &GPIO_InitStructure);

//}


uint8_t SPI_Flash_SendByte(uint8_t byte)
{
	/*! Loop while DR register in not emplty */
	while (SPI_I2S_GetFlagStatus(W25X_FLASH_SPI, SPI_I2S_FLAG_TXE) == RESET);

	/*!Send byte through the SPI1 peripheral */
	SPI_I2S_SendData(W25X_FLASH_SPI, byte);

	/*! Wait to receive a byte */
	while (SPI_I2S_GetFlagStatus(W25X_FLASH_SPI, SPI_I2S_FLAG_RXNE) == RESET);

	/*! Return the byte read from the SPI bus */
	return SPI_I2S_ReceiveData(W25X_FLASH_SPI);
}

//读取字节
uint8_t SPI_Flash_ReadByte(void)
{
	return (SPI_Flash_SendByte(0Xff));
}

#define SPI1_ReadWriteByte SPI_Flash_SendByte

#define W25X_FLASH_CS_HIGH()		GPIO_SetBits(W25X_FLASH_CS_GPIO_PORT,W25X_FLASH_CS_PIN)
#define W25X_FLASH_CS_LOW()			GPIO_ResetBits(W25X_FLASH_CS_GPIO_PORT,W25X_FLASH_CS_PIN);


#include <rtthread.h>
#include "stm32f10x.h"
typedef unsigned short u16;
typedef unsigned char u8;
typedef unsigned int u32;

static void delay_us(rt_uint32_t us)
{
        rt_uint32_t len;
        for (;us > 0; us --)
                for (len = 0; len < 20; len++ );
}

typedef unsigned short u16;

//Mini STM32开发板
//W25X16 驱动函数 
//正点原子@ALIENTEK
//2010/6/13
//V1.0
u16 SPI_FLASH_TYPE;//默认就是25Q64
//4Kbytes为一个Sector
//16个扇区为1个Block
//W25X16
//容量为2M字节,共有32个Block,512个Sector 

//初始化SPI FLASH的IO口
void SPI_Flash_Init(void)
{

	SPI_InitTypeDef SPI_InitStructure;
	
//	GPIO_PinAFConfig(GPIOC,GPIO_PinSource10,GPIO_AF_SPI3);
//	GPIO_PinAFConfig(GPIOC,GPIO_PinSource11,GPIO_AF_SPI3);
//	GPIO_PinAFConfig(GPIOC,GPIO_PinSource12,GPIO_AF_SPI3);
	
	
	SPIx_Init();
	
		/*!&lt; Deselect the FLASH: Chip Select high */
	W25X_FLASH_CS_HIGH();

	/*!&lt; SPI configuration */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;

	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;

	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(W25X_FLASH_SPI, &SPI_InitStructure);

	/*!&lt; Enable the W25X_FLASH_SPI */
	SPI_Cmd(W25X_FLASH_SPI, ENABLE);
	SPI_Flash_SendByte(0xff);
	
	
	SPI_FLASH_TYPE=SPI_Flash_ReadID();//读取FLASH ID.
	
	rt_kprintf("XXXXXXXXXXXXXXXXXXXXXX %02x \r\n",SPI_FLASH_TYPE);
}  

//读取SPI_FLASH的状态寄存器
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:默认0,状态寄存器保护位,配合WP使用
//TB,BP2,BP1,BP0:FLASH区域写保护设置
//WEL:写使能锁定
//BUSY:忙标记位(1,忙;0,空闲)
//默认:0x00
u8 SPI_Flash_ReadSR(void)   
{  
	u8 byte=0;   
	W25X_FLASH_CS_LOW();                          //使能器件   
	SPI1_ReadWriteByte(W25X_ReadStatusReg);    //发送读取状态寄存器命令    
	byte=SPI1_ReadWriteByte(0Xff);             //读取一个字节  
	W25X_FLASH_CS_HIGH();                            //取消片选     
	return byte;   
} 
//写SPI_FLASH状态寄存器
//只有SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)可以写!!!
void SPI_FLASH_Write_SR(u8 sr)   
{   
	W25X_FLASH_CS_LOW();                           //使能器件   
	SPI1_ReadWriteByte(W25X_WriteStatusReg);   //发送写取状态寄存器命令    
	SPI1_ReadWriteByte(sr);               //写入一个字节  
	W25X_FLASH_CS_HIGH();                             //取消片选     	      
}   
//SPI_FLASH写使能	
//将WEL置位   
void SPI_FLASH_Write_Enable(void)   
{
	W25X_FLASH_CS_LOW();                           //使能器件   
    SPI1_ReadWriteByte(W25X_WriteEnable);      //发送写使能  
	W25X_FLASH_CS_HIGH();                            //取消片选     	      
} 
//SPI_FLASH写禁止	
//将WEL清零  
void SPI_FLASH_Write_Disable(void)   
{  
	W25X_FLASH_CS_LOW();                           //使能器件   
   SPI1_ReadWriteByte(W25X_WriteDisable);     //发送写禁止指令    
	W25X_FLASH_CS_HIGH();                            //取消片选     	      
} 			    
//读取芯片ID W25X16的ID:0XEF14
u16 SPI_Flash_ReadID(void)
{
	u16 Temp = 0;	  
	W25X_FLASH_CS_LOW();			    
	SPI1_ReadWriteByte(0x90);//发送读取ID命令	    
	SPI1_ReadWriteByte(0x00); 	    
	SPI1_ReadWriteByte(0x00); 	    
	SPI1_ReadWriteByte(0x00); 	 			   
	Temp|=SPI1_ReadWriteByte(0xFF)<<8;  
	Temp|=SPI1_ReadWriteByte(0xFF);	 
	W25X_FLASH_CS_HIGH(); 				    
	return Temp;
}   		    
//读取SPI FLASH  
//在指定地址开始读取指定长度的数据
//pBuffer:数据存储区
//ReadAddr:开始读取的地址(24bit)
//NumByteToRead:要读取的字节数(最大65535)
void SPI_Flash_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead)   
{ 
 	u16 i;    												    
	W25X_FLASH_CS_LOW();                            //使能器件   
    SPI1_ReadWriteByte(W25X_ReadData);         //发送读取命令   
    SPI1_ReadWriteByte((u8)((ReadAddr)>>16));  //发送24bit地址    
    SPI1_ReadWriteByte((u8)((ReadAddr)>>8));   
    SPI1_ReadWriteByte((u8)ReadAddr);   
    for(i=0;i<NumByteToRead;i++)
	{ 
        pBuffer[i]=SPI1_ReadWriteByte(0XFF);   //循环读数  
    }
	W25X_FLASH_CS_HIGH();                            //取消片选     	      
}  
//SPI在一页(0~65535)内写入少于256个字节的数据
//在指定地址开始写入最大256字节的数据
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大256),该数不应该超过该页的剩余字节数!!!	 
void SPI_Flash_Write_Page(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)
{
 	u16 i;  
	rt_kprintf("SPI WRITE PAGE %X %d\r\n",WriteAddr,NumByteToWrite);
    SPI_FLASH_Write_Enable();                  //SET WEL 
	W25X_FLASH_CS_LOW();                           //使能器件   
    SPI1_ReadWriteByte(W25X_PageProgram);      //发送写页命令   
    SPI1_ReadWriteByte((u8)((WriteAddr)>>16)); //发送24bit地址    
    SPI1_ReadWriteByte((u8)((WriteAddr)>>8));   
    SPI1_ReadWriteByte((u8)WriteAddr);   
    for(i=0;i<NumByteToWrite;i++)SPI1_ReadWriteByte(pBuffer[i]);//循环写数  
	W25X_FLASH_CS_HIGH();                            //取消片选 
	SPI_Flash_Wait_Busy();					   //等待写入结束
} 
//无检验写SPI FLASH 
//必须确保所写的地址范围内的数据全部为0XFF,否则在非0XFF处写入的数据将失败!
//具有自动换页功能 
//在指定地址开始写入指定长度的数据,但是要确保地址不越界!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大65535)
//CHECK OK
void SPI_Flash_Write_NoCheck(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 			 		 
	u16 pageremain;	   
	pageremain=256-WriteAddr%256; //单页剩余的字节数		 	    
	if(NumByteToWrite<=pageremain)pageremain=NumByteToWrite;//不大于256个字节
	while(1)
	{	   
		SPI_Flash_Write_Page(pBuffer,WriteAddr,pageremain);
		if(NumByteToWrite==pageremain)break;//写入结束了
	 	else //NumByteToWrite>pageremain
		{
			pBuffer+=pageremain;
			WriteAddr+=pageremain;	

			NumByteToWrite-=pageremain;			  //减去已经写入了的字节数
			if(NumByteToWrite>256)pageremain=256; //一次可以写入256个字节
			else pageremain=NumByteToWrite; 	  //不够256个字节了
		}
	};	    
} 
//写SPI FLASH  
//在指定地址开始写入指定长度的数据
//该函数带擦除操作!
//pBuffer:数据存储区
//WriteAddr:开始写入的地址(24bit)
//NumByteToWrite:要写入的字节数(最大65535)  		   
u8 SPI_FLASH_BUF[4096];
//u8 SPI_FLASH_BUF2[4096];
#define W25X_SECTION_SIZE 4096
#define USE_STATIC_SPIFLASH_BUFFER
#ifdef USE_STATIC_SPIFLASH_BUFFER
#define spiflash_page_buffer SPI_FLASH_BUF
#else
static unsigned char *spiflash_page_buffer;
#endif
static unsigned short spiflash_page_buffer_index = 0;
static unsigned short spiflash_page_sector_index = 0;
static uint32_t history_baseaddr = 0;

static unsigned char readbuffer[256];

void debugbufferx(char *name , unsigned char *buf , int len)
{
	int i=0;
	rt_kprintf("Buffer name : %s \r\n",name);
	for(i=0;i<len;i++)
	{
		rt_kprintf("[%02X]",buf[i]);
	}
	rt_kprintf("\r\n");
}

void SPI_Flash_Write2(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite);

void init_write_flash(unsigned int addr , unsigned char *spiflash_buffer)
{
#ifdef USE_STATIC_SPIFLASH_BUFFER
#else
	spiflash_page_buffer = spiflash_buffer;
#endif
	history_baseaddr = addr;
	spiflash_page_buffer_index = 0;
	spiflash_page_sector_index = 0;
	//
}
void append_w25x_byte(uint32_t BaseAddr , unsigned char data)
{
	int i,j,k;
	spiflash_page_buffer[spiflash_page_buffer_index++] = data;
	if (spiflash_page_buffer_index >= W25X_SECTION_SIZE)
	{
		//debugbufferx("WRITE",spiflash_page_buffer,1000);
		SPI_Flash_Write2(spiflash_page_buffer,spiflash_page_sector_index*4096 + history_baseaddr,4096);
		memset(spiflash_page_buffer,0x0,4096);
		SPI_Flash_Read(spiflash_page_buffer,spiflash_page_sector_index*4096 + history_baseaddr,4096);
		//debugbufferx("READ",spiflash_page_buffer,1000);
		
		//rt_thread_sleep(500);
		
		//rt_thread_sleep(10);
		//清空缓冲
		spiflash_page_buffer_index = 0;
		//增加扇区计数器
		spiflash_page_sector_index ++;
	}
	
}

void append_w25x_append_buffer(unsigned char *buffer , int length)
{
	int i=0;
	for(i=0;i<length;i++)
	{
		append_w25x_byte(history_baseaddr,buffer[i]);
	}
}

void w25x_append_end(void)
{
	int i=0;
	unsigned int count = W25X_SECTION_SIZE - spiflash_page_buffer_index;
	rt_kprintf("Write buffer %X %d \r\n",spiflash_page_sector_index*W25X_SECTION_SIZE + spiflash_page_buffer_index);
	for(i=0;i<count;i++)
	{
		append_w25x_byte(0,0xFF);
	}
}


void SPI_Flash_Write2(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 
	u32 secpos;
	u16 secoff;
	u16 secremain;	   
 	u16 i;    

	secpos=WriteAddr/4096;//扇区地址 0~511 for w25x16
	secoff=WriteAddr%4096;//在扇区内的偏移
	secremain=4096-secoff;//扇区剩余空间大小   

	if(NumByteToWrite<=secremain)secremain=NumByteToWrite;//不大于4096个字节
	while(1) 
	{	
		//SPI_Flash_Read(SPI_FLASH_BUF,secpos*4096,4096);//读出整个扇区的内容
		//for(i=0;i<secremain;i++)//校验数据
		//{
		//	if(SPI_FLASH_BUF[secoff+i]!=0XFF)break;//需要擦除  	  
		//}
		i=0;//必须擦除
		if(i<secremain)//需要擦除
		{
			SPI_Flash_Erase_Sector(secpos);//擦除这个扇区
			for(i=0;i<secremain;i++)	   //复制
			{
				SPI_FLASH_BUF[i+secoff]=pBuffer[i];	  
			}
			SPI_Flash_Write_NoCheck(SPI_FLASH_BUF,secpos*4096,4096);//写入整个扇区  

		}else SPI_Flash_Write_NoCheck(pBuffer,WriteAddr,secremain);//写已经擦除了的,直接写入扇区剩余区间. 				   
		if(NumByteToWrite==secremain)break;//写入结束了
		else//写入未结束
		{
			secpos++;//扇区地址增1
			secoff=0;//偏移位置为0 	 

		  pBuffer+=secremain;  //指针偏移
			WriteAddr+=secremain;//写地址偏移	   
		  NumByteToWrite-=secremain;				//字节数递减
			if(NumByteToWrite>4096)secremain=4096;	//下一个扇区还是写不完
			else secremain=NumByteToWrite;			//下一个扇区可以写完了
		}	 
	};	 	 
}




void SPI_Flash_Write(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 
	u32 secpos;
	u16 secoff;
	u16 secremain;	   
 	u16 i;    

	secpos=WriteAddr/4096;//扇区地址 0~511 for w25x16
	secoff=WriteAddr%4096;//在扇区内的偏移
	secremain=4096-secoff;//扇区剩余空间大小   

	if(NumByteToWrite<=secremain)secremain=NumByteToWrite;//不大于4096个字节
	while(1) 
	{	
		SPI_Flash_Read(SPI_FLASH_BUF,secpos*4096,4096);//读出整个扇区的内容
		for(i=0;i<secremain;i++)//校验数据
		{
			if(SPI_FLASH_BUF[secoff+i]!=0XFF)break;//需要擦除  	  
		}
		if(i<secremain)//需要擦除
		{
			SPI_Flash_Erase_Sector(secpos);//擦除这个扇区
			for(i=0;i<secremain;i++)	   //复制
			{
				SPI_FLASH_BUF[i+secoff]=pBuffer[i];	  
			}
			SPI_Flash_Write_NoCheck(SPI_FLASH_BUF,secpos*4096,4096);//写入整个扇区  

		}else SPI_Flash_Write_NoCheck(pBuffer,WriteAddr,secremain);//写已经擦除了的,直接写入扇区剩余区间. 				   
		if(NumByteToWrite==secremain)break;//写入结束了
		else//写入未结束
		{
			secpos++;//扇区地址增1
			secoff=0;//偏移位置为0 	 

		   	pBuffer+=secremain;  //指针偏移
			WriteAddr+=secremain;//写地址偏移	   
		   	NumByteToWrite-=secremain;				//字节数递减
			if(NumByteToWrite>4096)secremain=4096;	//下一个扇区还是写不完
			else secremain=NumByteToWrite;			//下一个扇区可以写完了
		}	 
	};	 	 
}
//擦除整个芯片
//整片擦除时间:
//W25X16:25s 
//W25X32:40s 
//W25X64:40s 
//等待时间超长...
void SPI_Flash_Erase_Chip(void)   
{                                             
    SPI_FLASH_Write_Enable();                  //SET WEL 
    SPI_Flash_Wait_Busy();   
  	W25X_FLASH_CS_LOW();                            //使能器件   
    SPI1_ReadWriteByte(W25X_ChipErase);        //发送片擦除命令  
	W25X_FLASH_CS_HIGH();                             //取消片选     	      
	SPI_Flash_Wait_Busy();   				   //等待芯片擦除结束
}   
//擦除一个扇区
//Dst_Addr:扇区地址 0~511 for w25x16
//擦除一个山区的最少时间:150ms
void SPI_Flash_Erase_Sector(u32 Dst_Addr)   
{   
	Dst_Addr*=4096;
    SPI_FLASH_Write_Enable();                  //SET WEL 	 
    SPI_Flash_Wait_Busy();   
  	W25X_FLASH_CS_LOW();                            //使能器件   
    SPI1_ReadWriteByte(W25X_SectorErase);      //发送扇区擦除指令 
    SPI1_ReadWriteByte((u8)((Dst_Addr)>>16));  //发送24bit地址    
    SPI1_ReadWriteByte((u8)((Dst_Addr)>>8));   
    SPI1_ReadWriteByte((u8)Dst_Addr);  
	W25X_FLASH_CS_HIGH();                             //取消片选     	      
    SPI_Flash_Wait_Busy();   				   //等待擦除完成
}  
//等待空闲
void SPI_Flash_Wait_Busy(void)   
{   
	while ((SPI_Flash_ReadSR()&0x01)==0x01);   // 等待BUSY位清空
}  
//进入掉电模式
void SPI_Flash_PowerDown(void)   
{ 
  	W25X_FLASH_CS_LOW();                            //使能器件   
    SPI1_ReadWriteByte(W25X_PowerDown);        //发送掉电命令  
	W25X_FLASH_CS_HIGH();                            //取消片选     	      
    delay_us(3);                               //等待TPD  
}   
//唤醒
void SPI_Flash_WAKEUP(void)   
{  
  	W25X_FLASH_CS_LOW();                           //使能器件   
    SPI1_ReadWriteByte(W25X_ReleasePowerDown);   //  send W25X_PowerDown command 0xAB    
	  W25X_FLASH_CS_HIGH();                            //取消片选     	      
    delay_us(3);                               //等待TRES1
}   





#if 0
//发送字节
uint8_t SPI_Flash_SendByte(uint8_t byte)
{
	/*! Loop while DR register in not emplty */
	while (SPI_I2S_GetFlagStatus(W25X_FLASH_SPI, SPI_I2S_FLAG_TXE) == RESET);

	/*!Send byte through the SPI1 peripheral */
	SPI_I2S_SendData(W25X_FLASH_SPI, byte);

	/*! Wait to receive a byte */
	while (SPI_I2S_GetFlagStatus(W25X_FLASH_SPI, SPI_I2S_FLAG_RXNE) == RESET);

	/*! Return the byte read from the SPI bus */
	return SPI_I2S_ReceiveData(W25X_FLASH_SPI);
}

//读取字节
uint8_t SPI_Flash_ReadByte(void)
{
	return (SPI_Flash_SendByte(W25X_DUMMY_BYTE));
}

//SPI3初始化
void SPI_Flash_Init(void)
{
	uint16_t id;
	
	SPI_InitTypeDef SPI_InitStructure;
	
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource10,GPIO_AF_SPI3);
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource11,GPIO_AF_SPI3);
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource12,GPIO_AF_SPI3);

	SPI3_GPIO_Init();

	/*!&lt; Deselect the FLASH: Chip Select high */
	W25X_FLASH_CS_HIGH();

	/*!&lt; SPI configuration */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;

	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;

	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(W25X_FLASH_SPI, &SPI_InitStructure);

	/*!&lt; Enable the W25X_FLASH_SPI */
	SPI_Cmd(W25X_FLASH_SPI, ENABLE);
	SPI_Flash_SendByte(0xff);

	id = SPI_Flash_ReadID();
	id = 0xFF;
	
}



//读取设备ID

uint16_t SPI_Flash_ReadID(void)
{
	uint16_t Temp = 0, Temp0 = 0,Temp1 = 0;

	/*Select the FLASH: Chip Select low */
	W25X_FLASH_CS_LOW();

	/*! Send &quot;RDID &quot; instruction */
	SPI_Flash_SendByte(W25X_ManufactDeviveID);
	SPI_Flash_SendByte(0x00);
	SPI_Flash_SendByte(0x00);
	SPI_Flash_SendByte(0x00);
	/*! Read a byte from the FLASH */
	Temp0 = SPI_Flash_SendByte(W25X_DUMMY_BYTE);

	/*!&lt; Read a byte from the FLASH */
	Temp1 = SPI_Flash_SendByte(W25X_DUMMY_BYTE);
	/*!&lt; Deselect the FLASH: Chip Select high */
	W25X_FLASH_CS_HIGH();

	Temp = (Temp0 & 8) | Temp1;

	return Temp;
}


#define W25X_WriteEnable        0x06    //写使能  备注：xcc
#define W25X_WriteDisable        0x04     //写禁能
#define W25X_ReadStatusReg        0x05    //读状态寄存器
#define W25X_WriteStatusReg        0x01       //写状态寄存器
#define W25X_ReadData            0x03       //读数据
#define W25X_FastReadData        0x0B      //读数据
#define W25X_FastReadDual        0x3B      //快读双输出
#define W25X_PageProgram        0x02     //页编程
#define W25X_BlockErase            0xD8     //块擦除
#define W25X_SectorErase        0x20     //扇区擦除
#define W25X_ChipErase            0xC7     //芯片擦除
#define W25X_PowerDown            0xB9     //掉电
#define W25X_ReleasePowerDown    0xAB     //释放掉电
#define W25X_DeviceID            0xAB      //器件ID
#define W25X_ManufactDeviceID    0x90      //制造/器件ID
#define W25X_JedecDeviceID        0x9F      //JEDEC ID


static unsigned char CSREG;
#define uchar unsigned char
#define uint unsigned short
#define uint32 unsigned int
#define uint8 unsigned char
#define W25X_CS CSREG

#define SPI_Get_Byte SPI_Flash_ReadByte
#define SPI_Send_Byte SPI_Flash_SendByte

static void delay(unsigned int x)
{}
	
static uint8 upper_128[16];
static uint8 tx_buff[16];

uchar  SPI_Read_StatusReg()            //读状态寄存器  备注:Xcc
{    
		uchar byte = 0;
    W25X_FLASH_CS_LOW();                            //    enable device
    SPI_Send_Byte(W25X_ReadStatusReg);        //    send Read Status Register command
    byte = SPI_Get_Byte();                    //    receive byte
    W25X_FLASH_CS_HIGH();                         //    disable device    
    return byte;
}
void    SPI_Write_StatusReg(byte)         //写状态寄存器
{   
		W25X_FLASH_CS_LOW();//    enable device
    SPI_Send_Byte(W25X_WriteStatusReg);        //    select write to status register
    SPI_Send_Byte(byte);                    //    data that will change the status(only bits 2,3,7 can be written)
    W25X_FLASH_CS_HIGH();                           //    disable the device
}
void    SPI_Write_Enable()                   //写使能
{   
		W25X_FLASH_CS_LOW();                           //    enable device
    SPI_Send_Byte(W25X_WriteEnable);        //    send W25X_Write_Enable command
    W25X_FLASH_CS_HIGH();                            //    disable device
}
void    SPI_Write_Disable()                   //写禁能
{   W25X_FLASH_CS_LOW();                           //    enable device
    SPI_Send_Byte(W25X_WriteDisable);        //    send W25X_WriteW25X_DIsable command
    W25X_FLASH_CS_HIGH();                           //    disable device
}

void    SPI_Wait_Busy()                                 //等待忙结束
{
	while (1)
	{
		unsigned char reg = SPI_Read_StatusReg();
		if (reg != 0x03)
			break;
	}
  //SPI_Read_StatusReg();                //    waste time until not busy WEL & Busy bit all be 1 (0x03)
}
void    SPI_PowerDown()
{    W25X_CS = 0;                            //    enable device
    SPI_Send_Byte(W25X_PowerDown);            //    send W25X_PowerDown command 0xB9
    W25X_CS = 1;                            //    disable device
    delay(6);                                //    remain CS high for tPD = 3uS
}
void    SPI_ReleasePowerDown()
{    W25X_CS = 0;                            //    enable device
    SPI_Send_Byte(W25X_ReleasePowerDown);    //    send W25X_PowerDown command 0xAB
    W25X_CS = 1;                            //    disable device
    delay(6);                                //    remain CS high for tRES1 = 3uS
}

#define SPI_Flash_Write_Enable SPI_Write_Enable
#define SPI_Flash_Write_Disable SPI_Write_Disable

#define SPI_Flash_WaitForWriteEnd SPI_Wait_Busy



void SPI_Flash_EraseChip(void)
{
	SPI_Flash_WaitForWriteEnd();
	/*!&lt; Send write enable instruction */
	SPI_Flash_Write_Enable();
	SPI_Flash_WaitForWriteEnd();
	/*!&lt; Bulk Erase */
	/*!&lt; Select the FLASH: Chip Select low */
	W25X_FLASH_CS_LOW();
	/*!&lt; Send Bulk Erase instruction */
	SPI_Flash_SendByte(W25X_CMD_ChipErase);
	/*!&lt; Deselect the FLASH: Chip Select high */
	W25X_FLASH_CS_HIGH();
	 
	/*!&lt; Wait the end of Flash writing */
	SPI_Flash_WaitForWriteEnd();
}


//读
void SPI_Flash_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
	
	SPI_Flash_WaitForWriteEnd();
	/*Select the FLASH: Chip Select low */
	W25X_FLASH_CS_LOW();
	 
	/*Send &quot;Read from Memory &quot; instruction */
	SPI_Flash_SendByte(W25X_CMD_ReadData);
	 
	/*!&lt; Send ReadAddr high nibble address byte to read from */
	SPI_Flash_SendByte((ReadAddr & 0xFF0000));
	/*!&lt; Send ReadAddr medium nibble address byte to read from */
	SPI_Flash_SendByte((ReadAddr& 0xFF00));
	/*!&lt; Send ReadAddr low nibble address byte to read from */
	SPI_Flash_SendByte(ReadAddr & 0xFF);
	 
	while (NumByteToRead--) /* while there is data to be read */
	{
		/*!&lt; Read a byte from the FLASH */
		*pBuffer = SPI_Flash_SendByte(W25X_DUMMY_BYTE);
		/*!&lt; Point to the next location where the byte read will be saved */
		pBuffer++;
	}
	
	W25X_FLASH_CS_HIGH();
	
	SPI_Flash_WaitForWriteEnd();
}

//写一页256字节内。
void SPI_Flash_WritePage(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
	rt_kprintf("#################### WRITE ADDR 0x%X  SIZE %d\r\n",WriteAddr,NumByteToWrite);
	SPI_Flash_WaitForWriteEnd();
	/* Enable the write access to the FLASH */
	SPI_Flash_Write_Enable();
	 
	/* Select the FLASH: Chip Select low */
	W25X_FLASH_CS_LOW();
	/* Send &quot;Write to Memory &quot; instruction */
	SPI_Flash_SendByte(W25X_CMD_PageProgram);
	/*Send WriteAddr high nibble address byte to write to */
	SPI_Flash_SendByte((WriteAddr & 0xFF0000)) ;
	/* Send WriteAddr medium nibble address byte to write to */
	SPI_Flash_SendByte((WriteAddr & 0xFF00));
	/* Send WriteAddr low nibble address byte to write to */
	SPI_Flash_SendByte(WriteAddr & 0xFF);
	 
	/* while there is data to be written on the FLASH */
	while (NumByteToWrite--)
	{
		/* Send the current byte */
		SPI_Flash_SendByte(*pBuffer);
		/* Point on the next byte to be written */
		pBuffer++;
	}
	 
	/*Deselect the FLASH: Chip Select high */
	W25X_FLASH_CS_HIGH();
	 
	/* Wait the end of Flash writing */
	SPI_Flash_WaitForWriteEnd();
}

void SPI_Flash_Write(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
	
	unsigned short pageremain;	   
	pageremain=256-WriteAddr%256; //单页剩余的字节数		 	    
	if(NumByteToWrite<=pageremain)pageremain=NumByteToWrite;//不大于256个字节
	while(1)
	{	   
		SPI_Flash_WritePage(pBuffer,WriteAddr,pageremain);
		if(NumByteToWrite==pageremain)break;//写入结束了
	 	else //NumByteToWrite>pageremain
		{
			pBuffer+=pageremain;
			WriteAddr+=pageremain;	

			NumByteToWrite-=pageremain;			  //减去已经写入了的字节数
			if(NumByteToWrite>256)pageremain=256; //一次可以写入256个字节
			else pageremain=NumByteToWrite; 	  //不够256个字节了
		}
	};	    
}

void SPI_Erase_Sector(uint32 Dst_Addr)                //扇区擦除
{
		rt_kprintf("#################### ERAS ADDR 0x%X  SIZE %d\r\n",Dst_Addr,256);
		SPI_Flash_WaitForWriteEnd();
    SPI_Write_Enable();                                    //    set WEL
    W25X_FLASH_CS_LOW();			//    enable device
    SPI_Send_Byte(W25X_SectorErase);                    //    send Sector Erase command
    SPI_Send_Byte((uchar)((Dst_Addr & 0xFFFFFF) >> 16));//    send 3 address bytes
    SPI_Send_Byte((uchar)((Dst_Addr & 0xFFFF) >> 8));
    SPI_Send_Byte((uchar)Dst_Addr & 0xFF);
    W25X_FLASH_CS_HIGH();                                       //    disable device
		SPI_Flash_WaitForWriteEnd();
}
 


/////////////////////////////
//=================================================================================================
//SPI_Read_StatusReg        Reads the status register of the serial flash
//SPI_Write_StatusReg        Performs a write to the status register
//SPI_Write_Enable            Write enables the serial flash
//SPI_Write_Disable            Write disables the serial flash
//SPI_Read_ID1                Reads the device ID using the instruction 0xAB
//SPI_Read_ID2                Reads the manufacturer ID and device ID with 0x90
//SPI_Read_ID3()            Reads the JedecDevice ID
//SPI_Read_Byte                Reads one byte from the serial flash and returns byte(max of 20 MHz CLK frequency)
//SPI_Read_nBytes            Reads multiple bytes(max of 20 MHz CLK frequency)
//SPI_FastRead_Byte            Reads one byte from the serial flash and returns byte(max of 33 MHz CLK frequency)
//SPI_FastRead_nBytes        Reads multiple bytes(max of 33 MHz CLK frequency)
//SPI_Write_Byte            Program one byte to the serial flash
//SPI_Write_nBytes            Program n bytes to the serial flash, n<=256
//SPI_Erase_Chip            Erases entire serial flash
//SPI_Erase_Sector            Erases one sector (64 KB) of the serial flash
//SPI_Wait_Busy                Polls status register until busy bit is low
//=================================================================================================


uchar    SPI_Read_ID1()                        //释放掉电/器件ID
{    uchar byte;
    W25X_CS = 0;                            //    enable device
    SPI_Send_Byte(W25X_DeviceID);            //    send read device ID command (ABh)
    SPI_Send_Byte(0);                        //    send address
    SPI_Send_Byte(0);                        //    send address
    SPI_Send_Byte(0);                        //    send 3_Dummy address
    byte = SPI_Get_Byte();                    //    receive Device ID byte    
    W25X_CS  = 1;                            //    disable device
    delay(4);                                //    remain CS high for tRES2 = 1.8uS
    return byte;
}
uint    SPI_Read_ID2(uchar ID_Addr)             //制造/器件ID
{    uint IData16;
    W25X_CS = 0;                            //    enable device
    SPI_Send_Byte(W25X_ManufactDeviceID);    //    send read ID command (90h)
    SPI_Send_Byte(0x00);                    //    send address
    SPI_Send_Byte(0x00);                    //    send address
    SPI_Send_Byte(ID_Addr);                    //    send W25Pxx selectable ID address 00H or 01H
    IData16 = SPI_Get_Byte()<<8;            //    receive Manufature or Device ID byte
    IData16 |= SPI_Get_Byte();                //    receive Device or Manufacture ID byte
    W25X_CS = 1;                            //    disable device    
    return IData16;
}
uint    SPI_Read_ID3()                           //读JEDEC ID
{    uint IData16;
    W25X_CS = 0;                            //    enable device
    SPI_Send_Byte(W25X_JedecDeviceID);        //    send read ID command (9Fh)
    IData16 = SPI_Get_Byte()<<8;            //    receive Manufature or Device ID byte
    IData16 |= SPI_Get_Byte();                //    receive Device or Manufacture ID byte
    tx_buff[2] = SPI_Get_Byte();    
    W25X_CS = 1;                            //    disable device    
    return IData16;
}
uchar    SPI_Read_Byte(uint32 Dst_Addr)                //读某地址 数据
{    uchar byte = 0;    
    W25X_CS = 0;                                        //    enable device
    SPI_Send_Byte(W25X_ReadData);                        //    read command
    SPI_Send_Byte((uchar)((Dst_Addr & 0xFFFFFF) >> 16));//    send 3 address bytes
    SPI_Send_Byte((uchar)((Dst_Addr & 0xFFFF) >> 8));
    SPI_Send_Byte((uchar)(Dst_Addr & 0xFF));
    byte = SPI_Get_Byte();
    W25X_CS = 1;                                        //    disable device    
    return byte;                                        //    return one byte read
}
void    SPI_Read_nBytes(uint32 Dst_Addr, uchar nBytes_128)    //读某地址起nBytes_128字节以内内容
{    uint32 i = 0;    
    W25X_CS = 0;                                        //    enable device
    SPI_Send_Byte(W25X_ReadData);                        //    read command
    SPI_Send_Byte(((Dst_Addr & 0xFFFFFF) >> 16));        //    send 3 address bytes
    SPI_Send_Byte(((Dst_Addr & 0xFFFF) >> 8));
    SPI_Send_Byte(Dst_Addr & 0xFF);
    for (i = 0; i < nBytes_128; i++)                    //    read until no_bytes is reached
        upper_128[i] = SPI_Get_Byte();                    //    receive byte and store at address 80H - FFH
    W25X_CS = 1;                                        //    disable device
}
uchar    SPI_FastRead_Byte(uint32 Dst_Addr)                  //快读 某地址 数据
{    uchar byte = 0;
    W25X_CS = 0;                                        //    enable device
    SPI_Send_Byte(W25X_FastReadData);                    //    fast read command
    SPI_Send_Byte(((Dst_Addr & 0xFFFFFF) >> 16));        //    send 3 address bytes
    SPI_Send_Byte(((Dst_Addr & 0xFFFF) >> 8));
    SPI_Send_Byte(Dst_Addr & 0xFF);
    SPI_Send_Byte(0xFF);                                //    dummy byte
    byte = SPI_Get_Byte();
    W25X_CS = 1;                                        //    disable device    
    return byte;                                        //    return one byte read
}
void    SPI_FastRead_nBytes(uint32 Dst_Addr, uchar nBytes_128)      //快读 某地址 nBytes_128 个字节数据
{    uchar i = 0;    
    W25X_CS = 0;                                        //    enable device
    SPI_Send_Byte(W25X_FastReadData);                    //    read command
    SPI_Send_Byte(((Dst_Addr & 0xFFFFFF) >> 16));        //    send 3 address bytes
    SPI_Send_Byte(((Dst_Addr & 0xFFFF) >> 8));
    SPI_Send_Byte(Dst_Addr & 0xFF);
    SPI_Send_Byte(0xFF);                                //    dummy byte
    for (i = 0; i < nBytes_128; i++)                    //    read until no_bytes is reached
        upper_128[i] = SPI_Get_Byte();                    //    receive byte and store at address 80H - FFH
    W25X_CS = 1;                                        //    disable device
}
void    SPI_Write_Byte(uint32 Dst_Addr, uchar byte)          //页编程
{    W25X_CS = 0;                                    //    enable device
    SPI_Write_Enable();                                //    set WEL
    SPI_Wait_Busy();    
    W25X_CS = 0;    
    SPI_Send_Byte(W25X_PageProgram);                //    send Byte Program command
    SPI_Send_Byte(((Dst_Addr & 0xFFFFFF) >> 16));    //    send 3 address bytes
    SPI_Send_Byte(((Dst_Addr & 0xFFFF) >> 8));
    SPI_Send_Byte(Dst_Addr & 0xFF);
    SPI_Send_Byte(byte);                            //    send byte to be programmed
    W25X_CS = 1;                                    //    disable device
}
void    SPI_Write_nBytes(uint32 Dst_Addr, uchar nBytes_128)         //页编程 128个字节
{    
    uchar i, byte;    
    W25X_CS = 0;                    /* enable device */
    SPI_Write_Enable();                /* set WEL */
    W25X_CS = 0;
    SPI_Send_Byte(W25X_PageProgram);         /* send Byte Program command */
    SPI_Send_Byte(((Dst_Addr & 0xFFFFFF) >> 16));    /* send 3 address bytes */
    SPI_Send_Byte(((Dst_Addr & 0xFFFF) >> 8));
    SPI_Send_Byte(Dst_Addr & 0xFF);
    
    for (i = 0; i < nBytes_128; i++)
    {
        byte = upper_128[i];
        SPI_Send_Byte(byte);        /* send byte to be programmed */
    }    
    W25X_CS = 1;                /* disable device */
}
void    SPI_Erase_Chip()                     //擦除芯片
{
    W25X_CS = 0;                                        //    enable device
    SPI_Write_Enable();                                    //    set WEL
    W25X_CS = 0;
    SPI_Wait_Busy();
    W25X_CS = 0;
    SPI_Send_Byte(W25X_ChipErase);                        //    send Chip Erase command
    W25X_CS = 1;                                        //    disable device
}


#ifdef USE_STATIC_SPIFLASH_BUFFER
static unsigned char spiflash_page_buffer[4096];
#else
static unsigned char *spiflash_page_buffer;
#endif
static unsigned short spiflash_page_buffer_index = 0;
static unsigned short spiflash_page_sector_index = 0;
static uint32_t history_baseaddr = 0;

static unsigned char readbuffer[256];

void init_write_flash(unsigned int addr , unsigned char *spiflash_buffer)
{
#ifdef USE_STATIC_SPIFLASH_BUFFER
#else
	spiflash_page_buffer = spiflash_buffer;
#endif
	history_baseaddr = addr;
	spiflash_page_buffer_index = 0;
	spiflash_page_sector_index = 0;
	//
}
void append_w25x_byte(uint32_t BaseAddr , unsigned char data)
{
	int i,j,k;
	spiflash_page_buffer[spiflash_page_buffer_index++] = data;
	if (spiflash_page_buffer_index >= W25X_SECTION_SIZE)
	{
		//unsigned char buff[W25X_SECTION_SIZE];
		uint32_t tmpAddr;
		
		//擦除并写入
		tmpAddr = BaseAddr + (spiflash_page_sector_index*W25X_SECTION_SIZE);
		SPI_Erase_Sector(spiflash_page_sector_index*4096);
		
		for(i=0;i<16;i++)
		{
			SPI_Flash_WritePage(spiflash_page_buffer+(i*256),tmpAddr+(i*256),256);
			SPI_Flash_ReadBuffer(readbuffer,tmpAddr+(i*256),256);
			rt_kprintf("debug read:\r\n");
			for(j=0;j<256;j++)
			{
				rt_kprintf("%02X",readbuffer[j]);
			}
			rt_kprintf("\r\n");
		}
		//SPI_Flash_Write(spiflash_page_buffer,tmpAddr,W25X_SECTION_SIZE);
		
		
		SPI_Flash_ReadBuffer(readbuffer,tmpAddr,256);
		rt_kprintf("debug read:\r\n");
		for(i=0;i<256;i++)
		{
			rt_kprintf("%02X",readbuffer[i]);
		}
		rt_kprintf("\r\n");
		
		//清空缓冲
		spiflash_page_buffer_index = 0;
		//增加页计数器
		spiflash_page_sector_index ++;
	}
	
}

void append_w25x_append_buffer(unsigned char *buffer , int length)
{
	int i=0;
	for(i=0;i<length;i++)
	{
		append_w25x_byte(history_baseaddr,buffer[i]);
	}
}

void w25x_append_end(void)
{
	int i=0;
	unsigned int count = W25X_SECTION_SIZE - spiflash_page_buffer_index;
	rt_kprintf("Write buffer %X %d \r\n",spiflash_page_sector_index*W25X_SECTION_SIZE + spiflash_page_buffer_index);
	for(i=0;i<count;i++)
	{
		append_w25x_byte(0,0xFF);
	}
}


#include "MD5/md5sum.h"
#include "my_stdc_func/my_stdc_func.h"
#include "my_stdc_func/debugl.h"

void w25x_append_end_check(void)
{
	int i=0;
	int di =0;
	char str[32];
	md5_state_t state;
	md5_byte_t digest[16];
	char hex_output[16*2 + 1];
	unsigned char *buff;
	unsigned int total;
	int rcnt = 0;
	int __index = 0;
	
	//写入剩余扇区的字节
	unsigned int count = W25X_SECTION_SIZE - spiflash_page_sector_index;
	for(i=0;i<count;i++)
	{
		append_w25x_byte(0,0xFF);
	}
	
	
	//开始校验
	total = (spiflash_page_sector_index*W25X_SECTION_SIZE) + spiflash_page_sector_index;
	
	#define BUFFER_SIZE_RFLASH 256
	buff = (unsigned char*)rt_malloc(BUFFER_SIZE_RFLASH);
	if (buff <= 0)
	{
		DEBUGL->debug("MALLOC ERR\r\n");
		for(;;);
	}
	
	md5_init(&state);
	
	while(total >0)
	{
		
		if (total > BUFFER_SIZE_RFLASH)
		{
			SPI_Flash_ReadBuffer(buff,history_baseaddr + (rcnt*BUFFER_SIZE_RFLASH),BUFFER_SIZE_RFLASH);
			md5_append(&state,buff,BUFFER_SIZE_RFLASH);
			total -= BUFFER_SIZE_RFLASH;
//			rt_kprintf("\r\n\r\n");
//			for(i=0;i<BUFFER_SIZE_RFLASH;i++)
//			{
//				rt_kprintf("%d:[%d]",__index,buff[i]);
//				__index++;
//			}
//			rt_kprintf("\r\n\r\n");
			
		}else {
			SPI_Flash_ReadBuffer(buff,history_baseaddr + (rcnt*BUFFER_SIZE_RFLASH),total);
//			rt_kprintf("\r\n\r\n");
//			for(i=0;i<total;i++)
//			{
//				rt_kprintf("%d:[%d]",__index,buff[i]);
//				__index++;
//			}
//			rt_kprintf("\r\n\r\n");
			md5_append(&state,buff,total);
			total = 0;
			
		}
		
		rcnt ++;
	}
	
	md5_finish(&state, digest);
	snprintf(str,sizeof(str),"FLASH MD5:");
	for (di = 0; di < 16; ++di)
	{
		char md5str[4];
		snprintf(md5str,sizeof(md5str),"%02x",digest[di]);
		strcat(str,md5str);
	}
	DEBUGL->debug("%s\r\n",str);
	
}
#endif

void spiflash_powerdown(void)
{
//	//SPI1_ReadWriteByte(W25X_ReadData);         //发送读取命令   
//	//SPI_FLASH_WriteEnable();
//  W25X_FLASH_CS_LOW();
//  SPI1_ReadWriteByte(Power_Down);
//  W25X_FLASH_CS_HIGH();
//  //SPI_Flash_WaitForWriteEnd();
}

void spiflash_release_powerdown(void)
{
	
//	//SPI_FLASH_WriteEnable();
//  W25X_FLASH_CS_LOW();
//  SPI1_ReadWriteByte(ReleacePowerDown);
//  W25X_FLASH_CS_HIGH();
//  //SPI_Flash_WaitForWriteEnd();
//	//
}

