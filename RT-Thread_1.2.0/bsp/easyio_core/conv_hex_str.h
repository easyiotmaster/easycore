#ifndef __c_h_s_h__
#define __c_h_s_h__


void BufferToHex(unsigned char *inbuf , short inbuf_len , char *out , short outlen);
int HexToBuffer(char *hexstr , unsigned char *out , short outlen);

#endif
