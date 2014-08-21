//---------------------------------------------------------------------------
#pragma hdrstop
//---------------------------------------------------------------------------
#include "GIdComponent.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
void GIdComponent::ReInitSock()
{
	DestroySock();
	CreateSock();
}
//---------------------------------------------------------------------------
GIdTimeout GIdComponent::Select(const GIdTimeout& tm)
{
	if(INVALID_SOCKET == sock)
		GenerateException((char*)"invalid socket", 0);

#ifdef _windows_
	FD_SET fd = {1, sock};
#endif

#ifdef _linux_
	fd_set fd;
	FD_ZERO(&fd);
	FD_SET(sock, &fd);
#endif
	GIdTimeout time = tm;
	int ret = select(sock+1, &fd, 0, 0, &time);
	if(SOCKET_ERROR == ret)
		GenerateException((char*)"select failed");	
	if(!ret)
		GenerateException(ETIMEOUT, (char*)"Timeout");
	return time;
}
//---------------------------------------------------------------------------
GIdComponent::GIdComponent()
: sock(INVALID_SOCKET)
{
}
//---------------------------------------------------------------------------
GIdComponent::GIdComponent(SOCKET s)
: sock(s)
{
	if(INVALID_SOCKET == s)
		GenerateException((char*)"external socket failed!");
}

//---------------------------------------------------------------------------
GIdComponent::~GIdComponent()
{
	DestroySock();
}
//---------------------------------------------------------------------------

void GIdComponent::GenerateException(char* msg)
{
	throw GIdException(msg, GetSysErr());
}
//---------------------------------------------------------------------------
void GIdComponent::GenerateException(char* msg, int code)
{
	throw GIdException(msg, code);
}
//---------------------------------------------------------------------------
void GIdComponent::GenerateException(SOCK_ERR err, char* msg)
{
	switch(err){
		case EBROADCAST :
			throw GIdEBroadcast("broadcast failed");
		case ETIMEOUT :
			throw GIdETimeout();
		case ECLOSEDGRACEFULLY :
			throw GIdEClosedGracefully();
	}
	GenerateException(msg);
}
//---------------------------------------------------------------------------
void GIdComponent::DestroySock()
{
	shutdown (sock, SD_BOTH);

#ifdef _windows_
	closesocket(sock);
#endif

#ifdef _linux_
	close(sock);
#endif

	sock = INVALID_SOCKET;
}
//---------------------------------------------------------------------------
SOCKET GIdComponent::Socket() const
{
	return sock;
}
//---------------------------------------------------------------------------
GIdAddress GIdComponent::LocalBinding()
{
	GIdAddress addr;
	GSockAddrSize size = sizeof(addr);

	if(getsockname(sock, addr, &size))
		GenerateException((char*)"LocalBinding failed");
	return addr;
}
//---------------------------------------------------------------------------
GIdAddress GIdComponent::PeerBinding()
{
	GIdAddress addr;
	GSockAddrSize size = sizeof(addr);
	if(getpeername(sock, addr, &size))
		GenerateException((char*)"PeerBinding failed");
	return addr;
}
//---------------------------------------------------------------------------
