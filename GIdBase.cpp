//---------------------------------------------------------------------------
#pragma hdrstop
//---------------------------------------------------------------------------
#include "GIdBase.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
int GetSysErr()
{
#ifdef _windows_
	return WSAGetLastError();
#endif

#ifdef _linux_
	return errno;
#endif
}

//---------------------------------------------------------------------------
GIdException::GIdException(const char* msg, int code, unsigned char byte_info)
   : GException(code, msg, byte_info)
{
}
//---------------------------------------------------------------------------
GIdETimeout::GIdETimeout()
   : GIdException("timeout", 0, 1)
{
}
//---------------------------------------------------------------------------
GIdEBroadcast::GIdEBroadcast(const char* msg)
   : GIdException(msg, GetSysErr())
{
}
//---------------------------------------------------------------------------
GIdEClosedGracefully::GIdEClosedGracefully()
   : GIdException("closed gracefully", GetSysErr())
{
}
//---------------------------------------------------------------------------
GIdEAddress::GIdEAddress()
   : GIdException("bad address", 0, 1)
{
}
//---------------------------------------------------------------------------
GIdEReadString::GIdEReadString()
   : GIdException("read string", 0, 1)
{
}
//---------------------------------------------------------------------------

#ifdef _windows_

GIdBase::WinDllInit::WinDllInit()
: vhigh(2), vlow(2)
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int ret;
	wVersionRequested = MAKEWORD(vhigh, vlow);
	ret = WSAStartup(wVersionRequested, &wsaData);
	if(ret)
		throw GIdException("WSAStartup failed!", WSAGetLastError ());
	if(LOBYTE( wsaData.wVersion ) != vhigh || HIBYTE( wsaData.wVersion ) != vlow){
		WSACleanup();
		throw GIdException("WSAStartup failed!", WSAGetLastError ());
	}	
}
//---------------------------------------------------------------------------
GIdBase::WinDllInit::~WinDllInit()
{
   WSACleanup();
}
//---------------------------------------------------------------------------
GIdBase::WinDllInit GIdBase::win_dll_init;

#endif

//---------------------------------------------------------------------------
GIdBase::GIdBase() : version(1)
{
}
//---------------------------------------------------------------------------
int GIdBase::Version() const
{
   return version;
}
//---------------------------------------------------------------------------
