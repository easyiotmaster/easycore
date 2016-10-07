#ifndef __xmpp_str__
#define __xmpp_str__

const static char HEADER1[] = "<?xml version='1.0' ?><stream:stream to='liud-home.3322.org' xmlns='jabber:client' xmlns:stream='http://etherx.jabber.org/streams'  xml:lang='en' version='1.0'>";
const static char HEADER1_1[] = "<stream:stream xmlns='jabber:client' xmlns:stream='http://etherx.jabber.org/streams' version='0.0'>";

const static char HEADER2[] = "<auth xmlns='urn:ietf:params:xml:ns:xmpp-sasl' mechanism='PLAIN'/>";
const static char LOGIN_STR[] = "<auth xmlns='urn:ietf:params:xml:ns:xmpp-sasl' mechanism='PLAIN'>AHRlc3QxADExMTExMQ==</auth>";
const static char LOGIN_STR_P1[] = "<auth xmlns='urn:ietf:params:xml:ns:xmpp-sasl' mechanism='PLAIN'>";
const static char LOGIN_STR_P2[] = "</auth>";

const static char IQ_BIND[] = "<iq id='uid:5166b09f:6b8b4567' type='set' from='test1@liud-home.3322.org' xmlns='jabber:client'><bind xmlns='urn:ietf:params:xml:ns:xmpp-bind'><resource/></bind></iq>";
const static char IQ_BIND_P1[] = "<iq type='set' from='";
const static char IQ_BIND_P2[] ="' xmlns='jabber:client'><bind xmlns='urn:ietf:params:xml:ns:xmpp-bind'><resource>Tracker</resource></bind></iq>";

const static char PRESENCE_P1[] = "<presence from='";
const static char PRESENCE_P2[] = "' xmlns='jabber:client'><priority>0</priority><c xmlns='http://jabber.org/protocol/caps' hash='sha-1' node='http://camaya.net/gloox' ver='Xm1CxRvuvrN8H+6JfyaO0Idzg/c='/></presence>";

const static char IQ_SESSION_P1[] = "<iq id='uid:516f58d9:327b23c6' type='set' from='";
const static char IQ_SESSION_P2[] = "' xmlns='jabber:client'><session xmlns='urn:ietf:params:xml:ns:xmpp-session'/></iq>";

#endif
