//---------------------------------------------------------------------------
#pragma hdrstop
//---------------------------------------------------------------------------
#include "GIdAddress.h"
#include "GIdBase.h"
#include "GUtil.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
GIdAddress GIdAddress::CreateBroadcast(unsigned port)
{
   GIdAddress result;
   result.sin_family = AF_INET;
   result.sin_addr.s_addr = INADDR_BROADCAST;
   result.sin_port = htons(port);
   return result;
}
//---------------------------------------------------------------------------
GIdAddress::GIdAddress()
{
   sin_family = AF_INET;
   sin_addr.s_addr = INADDR_NONE;
   sin_port = htons(0);
}
//---------------------------------------------------------------------------
GIdAddress::GIdAddress(const char* host, unsigned port)
{
   sin_family = AF_INET;
   sin_addr.s_addr = inet_addr(host);
   sin_port = htons(port);
}
//---------------------------------------------------------------------------
GIdAddress::GIdAddress(const GIdString& host, unsigned port)
{
   sin_family = AF_INET;
   sin_addr.s_addr = inet_addr(host.c_str());
   sin_port = htons(port);
}
//---------------------------------------------------------------------------
GIdAddress& GIdAddress::operator()(const char* host, unsigned port)
{
   sin_addr.s_addr = inet_addr(host);
   sin_port = htons(port);
   return *this;
}
//---------------------------------------------------------------------------
GIdAddress& GIdAddress::operator()(const GIdString& host, unsigned port)
{
   sin_addr.s_addr = inet_addr(host.c_str());
   sin_port = htons(port);
   return *this;
}
//---------------------------------------------------------------------------
GIdAddress& GIdAddress::operator()(TYPE type)
{
   switch(type){
   case NONE : sin_addr.s_addr = INADDR_NONE; sin_port = htons(0); break;
   case ANY : sin_addr.s_addr = INADDR_ANY; break;
   case LOOPBACK : sin_addr.s_addr = INADDR_LOOPBACK; break;
   case BROADCAST : sin_addr.s_addr = INADDR_BROADCAST; break;
   default : throw GIdEAddress();
   }
   return *this;
}
//---------------------------------------------------------------------------
char* GIdAddress::Host() const { return inet_ntoa(sin_addr); }
//---------------------------------------------------------------------------
GIdString GIdAddress::AsString() const
{
   return static_cast<GIdString>(inet_ntoa(sin_addr)) + ':' + ToString(ntohs(sin_port));
}
//---------------------------------------------------------------------------

