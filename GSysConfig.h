//---------------------------------------------------------------------------
#ifndef GSysConfigH
#define GSysConfigH
//---------------------------------------------------------------------------

#ifndef _linux_
#define _linux_
#endif

#define __MT

//---------------------------------------------------------------------------
#ifdef _windows_

#include <winsock2.h>

#define GSockAddrSize int

#endif
//---------------------------------------------------------------------------

#ifdef _linux_
    #ifdef __MT
      #include <pthread.h>
    #endif
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>
#include <time.h>

#define SOCKET int
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#define TIMEVAL timeval
#define SOCKADDR_IN sockaddr_in
#define SOCKADDR sockaddr
#define LPSOCKADDR sockaddr*
#define LPSOCKADDR_IN sockaddr_in*

#define SD_BOTH SHUT_RDWR
#define MSG_PARTIAL 0
#define GSockAddrSize socklen_t

void ZeroMemory(void* ptr, unsigned long size);
void Sleep(unsigned long);

#endif
//---------------------------------------------------------------------------
#endif
