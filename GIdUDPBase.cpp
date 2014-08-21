//---------------------------------------------------------------------------
#pragma hdrstop
//---------------------------------------------------------------------------
#include "GIdUDPBase.h"
#include "GMemoryStream.h"
#include <string.h>
#include <iostream>
#include <sys/socket.h>

//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
GIdUDPBase::GIdUDPBase()
: buffer(0)
{
	buffer = new GMemoryStream(8192);
}
//---------------------------------------------------------------------------
GIdUDPBase::~GIdUDPBase()
{
	delete buffer;
	buffer = 0;
}
//---------------------------------------------------------------------------
void GIdUDPBase::Broadcast(const char* buff, unsigned long size, int port)
{
	const size_t optlen = sizeof(SO_BROADCAST);
	char optval[optlen];
        char msg[256];  
	
	for(int i = 0; i < optlen; i++)
		optval[i] = 1;
	
	if(SOCKET_ERROR == setsockopt(sock, SOL_SOCKET, SO_BROADCAST, optval, optlen))
		GenerateException(EBROADCAST,(char*)"Set Broadcast Option");

        
	struct sockaddr_in ip4addr;

	ip4addr.sin_family = AF_INET;
	ip4addr.sin_port = htons(port);
	ip4addr.sin_addr.s_addr = INADDR_BROADCAST;


	//caused socket error 
	//if(SOCKET_ERROR == sendto(sock, buff, size, 0, GIdAddress::CreateBroadcast(port), sizeof(GIdAddress)))

	if(SOCKET_ERROR == sendto(sock, buff, size, 0, (struct sockaddr*)&ip4addr, sizeof(ip4addr)))
		GenerateException(EBROADCAST,(char*)"Error On Send Broadcast");

	for(int i = 0; i < optlen; i++)
		optval[i] = 0;
	if(SOCKET_ERROR == setsockopt(sock, SOL_SOCKET, SO_BROADCAST, optval, optlen))
		GenerateException(EBROADCAST,(char*)"Error On Send Broadcast");
}
//---------------------------------------------------------------------------
void GIdUDPBase::Broadcast(const GIdString& string, int port)
{
	Broadcast(string.c_str(), strlen(string.c_str()), port);
}
//---------------------------------------------------------------------------
void GIdUDPBase::SendBufferTo(const GIdAddress& addr, char* buff, unsigned long size)
{
	int real = sendto(sock, buff, size, 0, addr, sizeof(addr));
	if(SOCKET_ERROR == real)
		GenerateException((char*)"SendBufferTo");
	if(real != size)
		GenerateException((char*)"Send Size Failed");
}
//---------------------------------------------------------------------------
void GIdUDPBase::SendBuffer(char* buff, unsigned long size)
{
	int real = send(sock, buff, size, 0);
	if(SOCKET_ERROR == real)
		GenerateException((char*)"SendBufferTo");
	if(real != size)
		GenerateException((char*)"Send Size Failed");
}
//---------------------------------------------------------------------------
void GIdUDPBase::SendTo(const GIdAddress& addr, const GIdString& string)
{
	SendBufferTo(addr, const_cast<char*>(string.c_str()), strlen(string.c_str()));
}
//---------------------------------------------------------------------------
void GIdUDPBase::Send(const GIdString& string)
{
	SendBuffer(const_cast<char*>(string.c_str()), strlen(string.c_str()));
}
//---------------------------------------------------------------------------
unsigned long GIdUDPBase::ReceiveBuffer(char* buff, const unsigned long size, GIdAddress& addr, const GIdTimeout& tmout)
{
	Select(tmout);
	GSockAddrSize fromlen = sizeof(GIdAddress),
	flags = MSG_PARTIAL, received = recvfrom(sock, buff, size, flags, addr, &fromlen);
	if(SOCKET_ERROR == received)
		GenerateException((char*)"ReceiveBuffer failed!");
	if(!received)
		GenerateException((char*)ECLOSEDGRACEFULLY);
	return received;
}
//---------------------------------------------------------------------------
unsigned long GIdUDPBase::ReceiveBuffer(char* buff, const unsigned long size, const GIdTimeout& tmout)
{
	Select(tmout);
	int flags = MSG_PARTIAL, received = recv(sock, buff, size, flags);
	if(SOCKET_ERROR == received)
		GenerateException((char*)"ReceiveBuffer failed!");
	if(!received)
		GenerateException((char*)ECLOSEDGRACEFULLY);
	return received;
}
//---------------------------------------------------------------------------
GIdString GIdUDPBase::ReceiveString(GIdAddress& addr, const GIdTimeout& tmout)
{
	const char zero = '\0';
	buffer->Clear();
	buffer->Size(ReceiveBuffer(buffer->Buffer(), buffer->Capacity(), addr, tmout));
	buffer->Position(buffer->Size());
	buffer->Write(&zero, 1);
	return buffer->Buffer();
}
//---------------------------------------------------------------------------
GIdString GIdUDPBase::ReceiveString(const GIdTimeout& tmout)
{
	const char zero = '\0';
	buffer->Clear();
	buffer->Size(ReceiveBuffer(buffer->Buffer(), buffer->Capacity(), tmout));
	buffer->Position(buffer->Size());
	buffer->Write(&zero, 1);
	return buffer->Buffer();
}
//---------------------------------------------------------------------------
void GIdUDPBase::Connect(const GIdAddress& addr)
{
	if(SOCKET_ERROR == connect(sock, addr, sizeof(GIdAddress)))
		GenerateException((char*)"connect failed!");
}
//---------------------------------------------------------------------------
