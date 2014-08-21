//---------------------------------------------------------------------------
#ifndef GIdUDPBaseH
#define GIdUDPBaseH
//---------------------------------------------------------------------------
#include "GIdDatagram.h"
#include "GIdTypes.h"
#include "GIdBuffer.h"
#include "GMemoryStream.h"
//---------------------------------------------------------------------------
class GIdUDPBase : public GIdDatagram{
protected:
   GMemoryStream* buffer;
public:
   GIdUDPBase();
   virtual ~GIdUDPBase();
   void Broadcast(const char*, unsigned long size, int port);
   void Broadcast(const GIdString&, int port);
   void SendBufferTo(const GIdAddress&, char* buff, const unsigned long size);
   void SendBuffer(char*, const unsigned long size);
   void SendTo(const GIdAddress&, const GIdString& string);
   void Send(const GIdString& string);
   unsigned long ReceiveBuffer(char* buff, const unsigned long size, GIdAddress& addr, const GIdTimeout& = 0);
   unsigned long ReceiveBuffer(char* buff, const unsigned long size, const GIdTimeout& = 0);
   GIdString ReceiveString(GIdAddress& addr, const GIdTimeout& tmout = 0);
   GIdString ReceiveString(const GIdTimeout& tmout = 0);
   void BufferSize(unsigned long nsz) { buffer->Size(nsz); }
   unsigned long BufferSize() const { return buffer->Capacity(); }
   void Connect(const GIdAddress&);
};
//---------------------------------------------------------------------------
#endif
