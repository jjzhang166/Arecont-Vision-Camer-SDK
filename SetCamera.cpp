//---------------------------------------------------------------------------
#pragma hdrstop
//---------------------------------------------------------------------------
#include "SetCamera.h"
#include <algorithm>
#include "GUtil.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
AV2000AddrInherit::AV2000AddrInherit()
{
   ZeroMemory(ip, sizeof(unsigned char) * 4);
   ZeroMemory(ip, sizeof(unsigned char) * 6);
}
//---------------------------------------------------------------------------
GIdString AV2000AddrInherit::IpAsString() const
{
   return ToString(ip[0]) + '.' + ToString(ip[1]) + '.' + ToString(ip[2]) + '.' + ToString(ip[3]);
}
//---------------------------------------------------------------------------
GIdString AV2000AddrInherit::MacAsString() const
{
   return ToString(mac[0]) + '-' + ToString(mac[1]) + '-' + ToString(mac[2]) + '-' + ToString(mac[3]) + '-' + ToString(mac[4]) + '-' + ToString(mac[5]);
}
//---------------------------------------------------------------------------
bool CompareAV2000Addr::operator()(const AV2000Addr& left, const AV2000Addr& right) const
{
   for(unsigned i = 0; i < 6; i ++)
      if(left.mac[i] != right.mac[i])
         return left.mac[i] < right.mac[i];
   return false;
}
//---------------------------------------------------------------------------
void SetCamera::OnDetectCamera(std::vector<char>& data)
{
   if(RemoveIdent(data)){
      char command = data.at(0);
      data.erase(data.begin(), data.begin() + 1);
      switch(command){
      case 1 :
         cameras.insert(ExtractAddr(data));
         break;
      default :
         break;
      }
   }
}
//---------------------------------------------------------------------------
AV2000AddrInherit SetCamera::ExtractAddr(std::vector<char>& data)
{
   AV2000AddrInherit result;
   unsigned i;
   for(i = 1; i <= 6; i++)
      result.mac[i-1] = data[i-1];
   for(; i <= 10; i++)
      result.ip[i-7] = data[i-1];
   return result;
}
//---------------------------------------------------------------------------
bool SetCamera::RemoveIdent(std::vector<char>& data)
{
   std::vector<char>::iterator iter = std::search(data.begin(), data.end(), ident.begin(), ident.end());
   if(iter != data.end()){
      data.erase(data.begin(), data.begin() + ident.size());
      return true;
   }
   return false;
}
//---------------------------------------------------------------------------
SetCamera::SetCamera()
   : ident("Arecont_Vision-AV2000"), port(69)
{
}
//---------------------------------------------------------------------------
unsigned SetCamera::Find(unsigned attempts, unsigned timeout)
{
   GIdUDPBase client;

   GIdString send(ident);
   send += static_cast<char>(1);
   cameras.clear();
   //client.Broadcast(&send[0], send.size(), port);
   std::vector<char> recvdata;
   for(unsigned i = 0; i < attempts; i++){
      try {
         client.Broadcast(&send[0], send.size(), port);
         recvdata.resize(256);
         recvdata.resize(client.ReceiveBuffer(&recvdata[0], recvdata.size(), timeout));
	//return 0;
         OnDetectCamera(recvdata);
      }
      catch(const GIdException& ex){}
   }
   return cameras.size();
}
//---------------------------------------------------------------------------
void SetCamera::Set(const AV2000Addr& addr)
{
   GIdUDPBase client;

   GIdString send(ident);
   send.push_back(static_cast<char>(2));
   for(unsigned i = 0; i < 6; i++)
      send.push_back(static_cast<char>(addr.mac[i]));
   for(unsigned i = 0; i < 4; i++)
      send.push_back(static_cast<char>(addr.ip[i]));
   client.Broadcast(&send[0], send.size(), port);
}
//---------------------------------------------------------------------------
const SetCamera::CAMERAS& SetCamera::Cameras() const
{
   return cameras;
}
//---------------------------------------------------------------------------
GIdString SetCamera::AV2000Ident() const
{
   return ident;
}
//---------------------------------------------------------------------------
