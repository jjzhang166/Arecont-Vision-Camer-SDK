//---------------------------------------------------------------------------
#pragma hdrstop
//---------------------------------------------------------------------------
#include "Clients.h"
#include "AVErrorCodes.h"
#include <string.h>
#include <time.h>
#include <stdlib.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
using namespace EAV;
//---------------------------------------------------------------------------
ClientErrorInherit1::ClientErrorInherit1() 
{ 
	Reset();
}
//---------------------------------------------------------------------------
void ClientErrorInherit1::Reset()
{
   code = 0;
   description[0] = '\0';
}
//---------------------------------------------------------------------------
void ClientErrorInherit1::SetInfo(int c, const char* msg, unsigned char byte_info)
{
	code = c;
   strncpy(description, msg, 254);
   description[255] = byte_info;
}
//---------------------------------------------------------------------------
void(*AV2000CLIENT::handler_on_reinited)(int)  = 0;
//---------------------------------------------------------------------------

Clients::Clients(unsigned size)
   : arr(size, 0)
{
   srand( (unsigned)time( NULL ) );
}
//---------------------------------------------------------------------------
Clients::~Clients()
{
   for(unsigned i = 0; i < arr.size(); i++){
      delete arr[i];
      arr[i] = 0;
   }
}
//---------------------------------------------------------------------------
void Clients::Create(unsigned number)
{
   if(number >= arr.size())
      throw Exception(CLIENT_OUT_OF_RANGE);
   if(arr[number])
      throw Exception(CLIENT_ALREADY_EXISTS);
   arr[number] = new TYPE(number);
}
//---------------------------------------------------------------------------
void Clients::Destroy(unsigned number)
{
   if(number < arr.size()){
      delete arr[number];
      arr[number] = 0;
   }
}
//---------------------------------------------------------------------------
Clients::TYPE* Clients::operator[](unsigned number)
{
   if(number >= arr.size())
      throw Exception(CLIENT_OUT_OF_RANGE);
   if(!arr[number])
      throw Exception(CLIENT_NOT_EXISTS);
   return arr[number];
}
//---------------------------------------------------------------------------
unsigned Clients::Size() const
{
   return arr.size();
}
//---------------------------------------------------------------------------
