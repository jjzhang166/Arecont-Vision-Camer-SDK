//---------------------------------------------------------------------------
#pragma hdrstop
//---------------------------------------------------------------------------
#include "GIdUDPClient.h"
#include <string.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
GIdUDPClient::GIdUDPClient()
   : timeout(3000)
{
}
//---------------------------------------------------------------------------
void GIdUDPClient::SendBuffer(char* buff, unsigned long size)
{
   GIdUDPBase::SendBufferTo(*this, buff, size);
}
//---------------------------------------------------------------------------
void GIdUDPClient::Send(const GIdString& string)
{
   GIdUDPBase::SendTo(*this, string);
}
//---------------------------------------------------------------------------
unsigned long GIdUDPClient::ReceiveBuffer(char* buff, const unsigned long size, GIdAddress& addr)
{
   return GIdUDPBase::ReceiveBuffer(buff, size, addr, timeout);
}
//---------------------------------------------------------------------------
unsigned long GIdUDPClient::ReceiveBuffer(char* buff, const unsigned long size)
{
   return GIdUDPBase::ReceiveBuffer(buff, size, timeout);
}
//---------------------------------------------------------------------------
GIdString GIdUDPClient::ReceiveString(GIdAddress& addr)
{
   return GIdUDPBase::ReceiveString(addr, timeout);
}
//---------------------------------------------------------------------------
GIdString GIdUDPClient::ReceiveString()
{
   return GIdUDPBase::ReceiveString(timeout);
}
//---------------------------------------------------------------------------
void GIdUDPClient::Host(const char* host)
{
   if(strcmp(GIdAddress::Host(), host)){
      ReInitSock();
      GIdAddress::Host(host);
   }
}
//---------------------------------------------------------------------------
void GIdUDPClient::Host(const std::string& host)
{
   Host(host.c_str());
}
//---------------------------------------------------------------------------
char* GIdUDPClient::Host() const
{
   return GIdAddress::Host();
}
//---------------------------------------------------------------------------
void GIdUDPClient::Port(const unsigned port)
{
   if(port != GIdAddress::Port()){
      ReInitSock();
      GIdAddress::Port(port);
   }
}
//---------------------------------------------------------------------------
unsigned GIdUDPClient::Port() const
{
   return GIdAddress::Port();
}
//---------------------------------------------------------------------------
