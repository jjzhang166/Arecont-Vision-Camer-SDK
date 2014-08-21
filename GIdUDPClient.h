//---------------------------------------------------------------------------
#ifndef GIdUDPClientH
#define GIdUDPClientH
//---------------------------------------------------------------------------
#include "GIdUDPBase.h"
//---------------------------------------------------------------------------
class GIdUDPClient : public GIdUDPBase, public GIdAddress{
protected:
   GIdTimeout timeout;
public:
   GIdUDPClient();
public:
   void ReceiveTimeout(unsigned long mlsc) { timeout(mlsc); }
   unsigned long ReceiveTimeout() const { return timeout(); }
public:
   void SendBuffer(char*, unsigned long size);
   void Send(const GIdString& string);
   unsigned long ReceiveBuffer(char* buff, const unsigned long size, GIdAddress& addr);
   unsigned long ReceiveBuffer(char* buff, const unsigned long size);
   GIdString ReceiveString(GIdAddress& addr);
   GIdString ReceiveString();
	void Host(const char* host);
	void Host(const std::string& host);
	char* Host() const;
	void Port(const unsigned port);
   unsigned Port() const;
};
//---------------------------------------------------------------------------
#endif
