//---------------------------------------------------------------------------

#ifndef ClientsH
#define ClientsH
//---------------------------------------------------------------------------
#include "AV2000Types.h"
#include "AV2000Client.h"
#include "GIdTypes.h"
#include "SetCamera.h"
//---------------------------------------------------------------------------
struct ClientErrorInherit1 : public ClientError{
	ClientErrorInherit1();
	void Reset();
   void SetInfo(int code, const char*, unsigned char byte_info);
};
//---------------------------------------------------------------------------
struct AV2000CLIENT{
	static void(*handler_on_reinited)(int);

	const int number;
	AV2000ClientUse<AV2000CLIENT> client;
	ClientErrorInherit1 error;
	GExternalAllocateStream memory;
   //--------
	explicit AV2000CLIENT(int n)
	: number(n), client(n, this, &AV2000CLIENT::OnReinited)
	{
		client.BufferImage(&memory);
	}
   //-------------
	void OnReinited()
	{
		if(handler_on_reinited)
			handler_on_reinited(number);
	}
};
//---------------------------------------------------------------------------

class AV2000Set : public SetCamera{
public:
	ClientErrorInherit1 error;
};
//---------------------------------------------------------------------------
class Clients{
   typedef AV2000CLIENT TYPE;
   typedef std::vector<TYPE*> ARRAY;
public:
   class Exception;
private:
   ARRAY arr;
public:
   explicit Clients(unsigned);
   ~Clients();
   TYPE* operator[](unsigned);
   void Create(unsigned);
   void Destroy(unsigned);
   unsigned Size() const;
};
//---------------------------------------------------------------------------
class Clients::Exception : public GException{
public:
   explicit Exception(int code)
      : GException(code, "error: client", 1)
   {
   }
};
//---------------------------------------------------------------------------
#endif
