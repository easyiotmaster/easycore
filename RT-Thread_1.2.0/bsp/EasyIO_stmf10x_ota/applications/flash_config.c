#include "flash_config.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct USER_CONFIG uconfig;

static char * makeJson(void);
static int parseJson(char * pMsg);
static void __set_default_config(void);

void __read_uconfig(void)
{

	if (parseJson((char*)USER_CONFIG_FLASH_ADDRESS) < 0) 
	{
		
		//第一个config区域如果失败了，则解析第二块，如果两个都失败了，才恢复默认
		if (parseJson((char*)(USER_CONFIG_FLASH_ADDRESS + (u32)USER_CONFIG_FLASH_SIZE/2)) < 0)
		{
			//设置为缺省值
			__set_default_config();
		}
		
		//如果第一个第二个其中有一个失败了，则将新的配置参数写入
		__write_uconfig();
	}else{
		//如果第一个配置区解析成功了，则检备份区是否相同
		if (strstr((char*)USER_CONFIG_FLASH_ADDRESS,(char*)(USER_CONFIG_FLASH_ADDRESS + (u32)USER_CONFIG_FLASH_SIZE/2)) > 0)
		{
			//第一个区域等于第二个区域，说明成功
		}else{
			//重新对第一个区域进行备份
			FLASH_ProgramStart((u32)USER_CONFIG_FLASH_ADDRESS+((u32)USER_CONFIG_FLASH_SIZE/2),(u32)USER_CONFIG_FLASH_SIZE/2);
			FLASH_AppendBuffer((unsigned char*)USER_CONFIG_FLASH_ADDRESS,USER_CONFIG_FLASH_SIZE/2);
			FLASH_AppendEnd();
			FLASH_ProgramDone();
			
		}
	}
	
	rt_kprintf("CONFIG JSON STR %s \r\n",(char*)USER_CONFIG_FLASH_ADDRESS);
}

#include "easy_core_config.h"
static void __set_default_config(void)
{
	memset(&uconfig,0x0,sizeof(uconfig));
	sprintf(uconfig.openfire_hostname,"%s","easy-iot.cc");
	uconfig.openfire_port = 5222;
	//
}


void __write_uconfig(void)
{
	char *json_str = makeJson();
	
	if (json_str > 0)
	{
		//写第一个区
		FLASH_ProgramStart((u32)USER_CONFIG_FLASH_ADDRESS,(u32)USER_CONFIG_FLASH_SIZE/2);
		FLASH_AppendBuffer((unsigned char*)json_str,strlen(json_str)+1);
		FLASH_AppendEnd();
		FLASH_ProgramDone();
		
		//写第二个区
		FLASH_ProgramStart((u32)USER_CONFIG_FLASH_ADDRESS+((u32)USER_CONFIG_FLASH_SIZE/2),(u32)USER_CONFIG_FLASH_SIZE/2);
		FLASH_AppendBuffer((unsigned char*)json_str,strlen(json_str)+1);
		FLASH_AppendEnd();
		FLASH_ProgramDone();
		
		rt_free(json_str);
	}
	
	
	//
}


#include "json/cJSON.h"
static char * makeJson(void)
{
		char * p;
    cJSON * pJsonRoot = NULL;
	
	//user_app_conf

    pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot)
    {
        //error happend here
        return NULL;
    }

		cJSON_AddNumberToObject(pJsonRoot, "DISABLE_WATCHDOG", uconfig.DISABLE_WATCHDOG);
		cJSON_AddStringToObject(pJsonRoot, "presence_str", uconfig.presence_str);
		cJSON_AddStringToObject(pJsonRoot, "openfire_hostname", uconfig.openfire_hostname);
		cJSON_AddNumberToObject(pJsonRoot, "openfire_port", uconfig.openfire_port);
		cJSON_AddStringToObject(pJsonRoot, "openfire_domain", uconfig.openfire_domain);
		cJSON_AddStringToObject(pJsonRoot, "openfire_username", uconfig.openfire_username);
		cJSON_AddStringToObject(pJsonRoot, "openfire_password", uconfig.openfire_password);
		
		cJSON_AddNumberToObject(pJsonRoot, "serialbaud", uconfig.serialbaud);
		
		cJSON_AddStringToObject(pJsonRoot, "tcp_host", uconfig.tcp_host);
		cJSON_AddNumberToObject(pJsonRoot, "tcp_port", uconfig.tcp_port);
		
		//apn info
		cJSON_AddStringToObject(pJsonRoot, "CCID", uconfig.APNINFO.CCID);
		cJSON_AddStringToObject(pJsonRoot, "APN", uconfig.APNINFO.APN);
		cJSON_AddStringToObject(pJsonRoot, "USERNAME", uconfig.APNINFO.USERNAME);
		cJSON_AddStringToObject(pJsonRoot, "PASSWORD", uconfig.APNINFO.PASSWORD);
		
		//OTA
		cJSON_AddNumberToObject(pJsonRoot, "ota_flag", uconfig.ota_flag);
		cJSON_AddNumberToObject(pJsonRoot, "ota_newfw_address", uconfig.ota_newfw_address);
		cJSON_AddNumberToObject(pJsonRoot, "ota_newfw_size", uconfig.ota_newfw_size);
		cJSON_AddStringToObject(pJsonRoot, "ota_target_fw_md5", uconfig.ota_target_fw_md5);
		//ota_target_fw_md5
		
		//bootloader_ver_a
		cJSON_AddNumberToObject(pJsonRoot, "bootloader_ver_a", uconfig.bootloader_ver_a);
		cJSON_AddNumberToObject(pJsonRoot, "bootloader_ver_b", uconfig.bootloader_ver_b);
		
		cJSON_AddStringToObject(pJsonRoot, "JSON_FLAG", "JSON");
		
    p = cJSON_Print(pJsonRoot);
  // else use : 
    // char * p = cJSON_PrintUnformatted(pJsonRoot);
    if(NULL == p)
    {
        //convert json list to string faild, exit
        //because sub json pSubJson han been add to pJsonRoot, so just delete pJsonRoot, if you also delete pSubJson, it will coredump, and error is : double free
        cJSON_Delete(pJsonRoot);
        return NULL;
    }
    //free(p);
    
    cJSON_Delete(pJsonRoot);

    return p;
}


static int parseJson(char * pMsg)
{
		cJSON * pSub;
		cJSON * pJson;
    if(NULL == pMsg)
    {
        return -1;
    }
    pJson = cJSON_Parse(pMsg);
    if(NULL == pJson)                                                                                         
    {
        // parse faild, return
      return -1;
    }

		//DISABLE_WATCHDOG
		pSub = cJSON_GetObjectItem(pJson, "DISABLE_WATCHDOG");
    if(NULL != pSub)
    {
			rt_kprintf("openfire_port : %d\n", pSub->valueint);
			uconfig.DISABLE_WATCHDOG = pSub->valueint;
      //get object named "hello" faild
    }
		
    pSub = cJSON_GetObjectItem(pJson, "presence_str");
    if(NULL != pSub)
    {
			rt_kprintf("presence_str : %d\n", pSub->valueint);
			sprintf(uconfig.presence_str,"%s",pSub->valuestring);
      //get object named "hello" faild
    }
		
		pSub = cJSON_GetObjectItem(pJson, "openfire_hostname");
    if(NULL != pSub)
    {
			rt_kprintf("openfire_hostname : %s\n", pSub->valuestring);
			sprintf(uconfig.openfire_hostname,"%s",pSub->valuestring);
      //get object named "hello" faild
    }
		
		pSub = cJSON_GetObjectItem(pJson, "openfire_port");
    if(NULL != pSub)
    {
			rt_kprintf("openfire_port : %d\n", pSub->valueint);
			uconfig.openfire_port = pSub->valueint;
      //get object named "hello" faild
    }
		
		pSub = cJSON_GetObjectItem(pJson, "openfire_domain");
    if(NULL != pSub)
    {
			rt_kprintf("openfire_domain : %s\n", pSub->valuestring);
			sprintf(uconfig.openfire_domain,"%s",pSub->valuestring);
      //get object named "hello" faild
    }
		
		pSub = cJSON_GetObjectItem(pJson, "openfire_domain");
    if(NULL != pSub)
    {
			rt_kprintf("openfire_username : %s\n", pSub->valuestring);
			sprintf(uconfig.openfire_username,"%s",pSub->valuestring);
      //get object named "hello" faild
    }
		
		pSub = cJSON_GetObjectItem(pJson, "openfire_domain");
    if(NULL != pSub)
    {
			rt_kprintf("openfire_password : %s\n", pSub->valuestring);
			sprintf(uconfig.openfire_password,"%s",pSub->valuestring);
      //get object named "hello" faild
    }
		
		pSub = cJSON_GetObjectItem(pJson, "tcp_host");
    if(NULL != pSub)
    {
			rt_kprintf("tcp_host : %d\n", pSub->valuestring);
			sprintf(uconfig.tcp_host,"%s",pSub->valuestring);
      //get object named "hello" faild
    }
		
		pSub = cJSON_GetObjectItem(pJson, "tcp_port");
    if(NULL != pSub)
    {
			rt_kprintf("tcp_port : %d\n", pSub->valueint);
			uconfig.tcp_port = pSub->valueint;
      //get object named "hello" faild
    }
		
		pSub = cJSON_GetObjectItem(pJson, "serialbaud");
    if(NULL != pSub)
    {
			rt_kprintf("serialbaud : %d\n", pSub->valueint);
			uconfig.serialbaud = pSub->valueint;
      //get object named "hello" faild
    }
		
		//APN
		
		pSub = cJSON_GetObjectItem(pJson, "CCID");
    if(NULL != pSub)
    {
			rt_kprintf("CCID : %s\n", pSub->valuestring);
			sprintf(uconfig.APNINFO.CCID,"%s",pSub->valuestring);
      //get object named "hello" faild
    }
		
		pSub = cJSON_GetObjectItem(pJson, "APN");
    if(NULL != pSub)
    {
			rt_kprintf("APN : %s\n", pSub->valuestring);
			snprintf(uconfig.APNINFO.APN,sizeof(uconfig.APNINFO.APN),"%s",pSub->valuestring);
      //get object named "hello" faild
    }
		
		pSub = cJSON_GetObjectItem(pJson, "USERNAME");
    if(NULL != pSub)
    {
			rt_kprintf("USERNAME : %s\n", pSub->valuestring);
			//sprintf(user_app_conf.APNINFO.USERNAME,"%s",pSub->valuestring);
			snprintf(uconfig.APNINFO.USERNAME,sizeof(uconfig.APNINFO.USERNAME),"%s",pSub->valuestring);
      //get object named "hello" faild
    }
		
		pSub = cJSON_GetObjectItem(pJson, "PASSWORD");
    if(NULL != pSub)
    {
			rt_kprintf("PASSWORD : %s\n", pSub->valuestring);
			snprintf(uconfig.APNINFO.PASSWORD,sizeof(uconfig.APNINFO.PASSWORD),"%s",pSub->valuestring);
      //get object named "hello" faild
    }


		//OTA
		pSub = cJSON_GetObjectItem(pJson, "ota_flag");
    if(NULL != pSub)
    {
			rt_kprintf("ota_flag : %d\n", pSub->valueint);
			uconfig.ota_flag = pSub->valueint;
    }
		pSub = cJSON_GetObjectItem(pJson, "ota_newfw_address");
    if(NULL != pSub)
    {
			rt_kprintf("ota_newfw_address : %d\n", pSub->valueint);
			uconfig.ota_newfw_address = pSub->valueint;
    }
		pSub = cJSON_GetObjectItem(pJson, "ota_newfw_size");
    if(NULL != pSub)
    {
			rt_kprintf("ota_newfw_size : %d\n", pSub->valueint);
			uconfig.ota_newfw_size = pSub->valueint;
    }
		pSub = cJSON_GetObjectItem(pJson, "ota_target_fw_md5");
    if(NULL != pSub)
    {
			rt_kprintf("ota_target_fw_md5 : %s\n", pSub->valuestring);
			snprintf(uconfig.ota_target_fw_md5,sizeof(uconfig.ota_target_fw_md5),"%s",pSub->valuestring);
    }
		//ota_target_fw_md5
		pSub = cJSON_GetObjectItem(pJson, "bootloader_ver_a");
    if(NULL != pSub)
    {
			rt_kprintf("bootloader_ver_a : %d\n", pSub->valueint);
			uconfig.bootloader_ver_a = pSub->valueint;
    }
		pSub = cJSON_GetObjectItem(pJson, "bootloader_ver_b");
    if(NULL != pSub)
    {
			rt_kprintf("bootloader_ver_b : %d\n", pSub->valueint);
			uconfig.bootloader_ver_b = pSub->valueint;
    }
		
    cJSON_Delete(pJson);
}

void debug_user_config(void)
{
}


