//---------------------------------------------------------------------------
#ifndef GIdComponentH
#define GIdComponentH
//---------------------------------------------------------------------------
#include "GIdBase.h"
#include "GIdAddress.h"
//---------------------------------------------------------------------------
class GIdComponent : public GIdBase{
protected:
   enum SOCK_ERR{ EBROADCAST, ETIMEOUT, ECLOSEDGRACEFULLY };
protected:
   SOCKET sock;
protected:
	void GenerateException(char*);
   void GenerateException(char*, int);
   void GenerateException(SOCK_ERR, char* = "");
   void DestroySock();
   virtual void ReInitSock();
   virtual void CreateSock() = 0;
   virtual GIdTimeout Select(const GIdTimeout&);
public:
   GIdComponent();
   explicit GIdComponent(SOCKET s);
   virtual ~GIdComponent();
   SOCKET Socket() const;
	GIdAddress LocalBinding();
	GIdAddress PeerBinding();
/*
   void BufferSize(unsigned long nsz) { Size(nsz); }
   unsigned long BufferSize() const { return Size(); }
*/
};
//---------------------------------------------------------------------------
#endif
