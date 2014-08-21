//---------------------------------------------------------------------------
#ifndef GIdBaseH
#define GIdBaseH
//---------------------------------------------------------------------------
#include "GSysConfig.h"
#include "GException.h"
//---------------------------------------------------------------------------
int GetSysErr();
//---------------------------------------------------------------------------
class GIdException : public GException{
public:
	GIdException(const char*, int code, unsigned char = 0);
};
//---------------------------------------------------------------------------
class GIdETimeout : public GIdException{
public:
	GIdETimeout();
};
//---------------------------------------------------------------------------
class GIdEBroadcast : public GIdException{
public:
	explicit GIdEBroadcast(const char*);
};
//---------------------------------------------------------------------------
class GIdEClosedGracefully : public GIdException{
public:
   GIdEClosedGracefully();
};
//---------------------------------------------------------------------------
class GIdEAddress : public GIdException{
public:
   GIdEAddress();
};
//---------------------------------------------------------------------------
class GIdEReadString : public GIdException{
public:
   GIdEReadString();
};
//---------------------------------------------------------------------------

class GIdBase{
#ifdef _windows_
	class WinDllInit;
private:
	static WinDllInit win_dll_init;
#endif
	const int version;
public:
	GIdBase();
   virtual ~GIdBase(){}
	int Version() const;
};
//---------------------------------------------------------------------------
#ifdef _windows_
class GIdBase::WinDllInit{
	const int vhigh, vlow;
public:
	WinDllInit();
	~WinDllInit();
};
#endif
//---------------------------------------------------------------------------
#endif


