//---------------------------------------------------------------------------
#ifndef GIdDatagramH
#define GIdDatagramH
//---------------------------------------------------------------------------
#include "GIdComponent.h"
#include "GIdAddress.h"
//---------------------------------------------------------------------------
class GIdDatagram : public GIdComponent{
protected:
	virtual void CreateSock();
public:
   GIdDatagram();
   virtual ~GIdDatagram(){}
};
//---------------------------------------------------------------------------
#endif
