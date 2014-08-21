//---------------------------------------------------------------------------
#ifndef GIdAddressH
#define GIdAddressH
//---------------------------------------------------------------------------
#include "GSysConfig.h"
#include "GIdTypes.h"
//---------------------------------------------------------------------------
class GIdAddress : public SOCKADDR_IN{
public:
	enum TYPE { NONE, ANY, LOOPBACK, BROADCAST };
public:
	static GIdAddress CreateBroadcast(unsigned port);
public:
	GIdAddress();
	GIdAddress(const char*, unsigned = 0);
	GIdAddress(const GIdString&, unsigned = 0);
	GIdAddress& operator()(const char* host, unsigned port);
	GIdAddress& operator()(const GIdString& host, unsigned port);
	GIdAddress& operator()(TYPE type);
	void Host(const char* host) { sin_addr.s_addr = inet_addr(host); }
	void Host(const std::string& host) { sin_addr.s_addr = inet_addr(host.c_str()); }
	char* Host() const;
	void Port(const unsigned port) { sin_port = htons(port); }
	GIdString AsString() const;
	unsigned Port() const { return ntohs(sin_port); }
	operator SOCKADDR() const { return *this; }
	operator LPSOCKADDR() const { return reinterpret_cast<LPSOCKADDR>(const_cast<GIdAddress*>(this)); }	
	operator LPSOCKADDR_IN() const { return reinterpret_cast<LPSOCKADDR_IN>(const_cast<GIdAddress*>(this)); }
};
//---------------------------------------------------------------------------
#endif
