#ifndef __xmpp_helper__
#define __xmpp_helper__

int get_xmpp_id(void);
int get_auth_str(char * username , char * password , char * out);
int get_bind_str(char * username , char * domain , char *out);
char *get_xml_att(char *head , const char *str_in , char * str_out , char out_lim);
int get_header1_str(char *domain_ip,char *out);
int get_ping_rs_str(char *dst , char *jid , char *pingid ,  char *out , int len);
int get_ping_str(char *domain , char *out,int outlen);
#endif
