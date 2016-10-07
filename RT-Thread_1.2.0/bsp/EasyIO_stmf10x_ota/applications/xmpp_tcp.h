#ifndef __xmpp_tcp_h__
#define __xmpp_tcp_h__

extern struct TcpClientCon xmpp_tcp_client;
void init_xmpp_tcp(void);
void setting_presence_routring(void*p);
#endif