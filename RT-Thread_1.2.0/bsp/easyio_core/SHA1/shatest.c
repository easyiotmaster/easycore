/*
 *  shatest.c
 *
 *  Copyright (C) 1998, 2009
 *  Paul E. Jones <paulej@packetizer.com>
 *  All Rights Reserved
 *
 *****************************************************************************
 *  $Id: shatest.c 12 2009-06-22 19:34:25Z paulej $
 *****************************************************************************
 *
 *  Description:
 *      This file will exercise the SHA1 class and perform the three
 *      tests documented in FIPS PUB 180-1.
 *
 *  Portability Issues:
 *      None.
 *
 */

#include <stdio.h>
#include <string.h>
#include "sha1.h"
#include "BASE64/cbase64.h"
#include "my_stdc_func/debugl.h"
#define printf rt_kprintf

/*
 *  Define patterns for testing
 */
 //861311001202901
 //352253060037693
 //#define TESTA   "352253060037693"

//crtMV0xGJuoUAd3RC0RRtdPSXaw=

#include "sha1_password.h"

int make_sha1_pwd(char *imei ,char *pwd)
{
    SHA1Context sha;
    int i;
	
    DEBUGL->debug("\nImei : %s\n",imei);

    SHA1Reset(&sha);
    SHA1Input(&sha, (const unsigned char *) imei, strlen(imei));

    if (!SHA1Result(&sha))
    {
        DEBUGL->debug("ERROR-- could not compute message digest\n");
			return -1;
    }
    else
    {
				unsigned char * sha1_out_ptr;
				unsigned char sha1_out_buf[20];
				//char base64buf[64];
        DEBUGL->debug("\t");
        for(i = 0; i < 5 ; i++)
        {
						sha1_out_ptr = (unsigned char*)&sha.Message_Digest[i];
            DEBUGL->debug("%02X ", sha1_out_ptr[3]);
						DEBUGL->debug("%02X ", sha1_out_ptr[2]);
						DEBUGL->debug("%02X ", sha1_out_ptr[1]);
						DEBUGL->debug("%02X ", sha1_out_ptr[0]);
					
						sha1_out_buf[i*4 + 0] = sha1_out_ptr[3];
						sha1_out_buf[i*4 + 1] = sha1_out_ptr[2];
						sha1_out_buf[i*4 + 2] = sha1_out_ptr[1];
						sha1_out_buf[i*4 + 3] = sha1_out_ptr[0];
					
        }
        DEBUGL->debug("\n");
				
				//sha1_out_buf = (unsigned char *)&sha.Message_Digest[0];
				encode_base64(sha1_out_buf,20,pwd);
				DEBUGL->debug("RESULT : %s \r\n",pwd);
				
				
		}

    return 0;
}
