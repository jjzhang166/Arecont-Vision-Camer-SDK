//---------------------------------------------------------------------------
#pragma hdrstop
//---------------------------------------------------------------------------
#include "ClientInterface.h"
#include "Clients.h"
#include "AVErrorCodes.h"
#include "AV2000Client.h"
#include <stdio.h>
#include <iostream>

//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
Clients clients(256);
AV2000Set set;

//----------------------------------------------------------------
void Allocate(char** pbuffer, unsigned long* psize);
void Deallocate(char* buffer);

//---------------------------------------------------------------------------
void STDCALL UseDefaultAllocator()
{
      SetAllocateFunction(Allocate);
      SetDeallocateFunction(Deallocate);
}

//---------------------------------------------------------------------------
int STDCALL CreateClient(int number)
{
   try{
      clients.Create(number);
      return 0;
   }
   catch(Clients::Exception& ex){
      return ex.get_code();
   }
   catch(...){
      return -1;
   }
}
//---------------------------------------------------------------------------
void STDCALL DestroyClient(int number) // no exceptions
{
   clients.Destroy(number);
}
//---------------------------------------------------------------------------
unsigned STDCALL MaxClients() // no exceptions
{
   return clients.Size();
}
//---------------------------------------------------------------------------
const ClientError* const STDCALL GetLastClientError(int number)
{
   try{
      return &(clients[number]->error);
   }
   catch(Clients::Exception& ex){
   }
   return 0;
}
//---------------------------------------------------------------------------
int STDCALL SetClientIp(int number, const char* ip)
{
   try {
      if(!ip)
         throw EAVException("zero pointer");
      clients[number]->client.Host(ip);
      return 1;
   }
   catch(EStreamNoMemory& ex){ clients[number]->error.SetInfo(EAV::NO_ALLOCATE_MEMORY, ex.what(), ex.get_info()); }
   catch(GIdEAddress& ex){ clients[number]->error.SetInfo(EAV::BAD_SOCK_ADDRESS, ex.what(), ex.get_info()); }
   catch(GIdETimeout& ex){ clients[number]->error.SetInfo(EAV::TIMEOUT_ON_SOCK, ex.what(), ex.get_info()); }
   catch(GIdEReadString& ex){ clients[number]->error.SetInfo(EAV::READ_STRING_FROM_SOCK, ex.what(), ex.get_info()); }
   catch(GIdTFTPException& ex){ clients[number]->error.SetInfo(EAV::TFTP_PROTOCOL, ex.what(), ex.get_info()); }
   catch(GIdException& ex){ clients[number]->error.SetInfo(ex.get_code(), ex.what(), ex.get_info()); }
   catch(EAVParameterOutOfRange& ex){ clients[number]->error.SetInfo(EAV::CAMERA_PARAMATER_OUT_OF_RANGE, ex.what(), ex.get_info()); }
   catch(EAVCameraUnknown& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info()); }
   catch(EAVNotSupported& ex){ clients[number]->error.SetInfo(EAV::PARAMETER_NOT_SUPPORTED, ex.what(), ex.get_info()); }
   catch(EAVParameterUnknown& ex){ clients[number]->error.SetInfo(EAV::VALUE_OF_PARAMETER_UNKNOWN, ex.what(), ex.get_info()); }
   catch(EAVException& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_ERROR, ex.what(), ex.get_info()); }
   catch(Clients::Exception& ex){ }
   catch(std::exception& ex) { clients[number]->error.SetInfo(-1, ex.what(), 0); }
   catch(...) { clients[number]->error.SetInfo(-1, "unknown", 0); }
   return 0;
}
//---------------------------------------------------------------------------
const char* STDCALL GetClientIp(int number)
{
   try{
      return const_cast<char*>(clients[number]->client.Host());
   }
   catch(Clients::Exception& ex){
   }
   return 0;
}
//---------------------------------------------------------------------------
void STDCALL SetClientPort(int number, unsigned port)
{
   try{
      clients[number]->client.Port(port);
   }
   catch(Clients::Exception& ex){
   }
}
//---------------------------------------------------------------------------
unsigned STDCALL GetClientPort(int number)
{
   try{
      return clients[number]->client.Port();
   }
   catch(Clients::Exception& ex){
   }
   return 0;
}
//---------------------------------------------------------------------------
void STDCALL SetClientTimeout(int number, unsigned timeout)
{
   try{
      clients[number]->client.ReceiveTimeout(timeout);
      clients[number]->client.ReceiveBeginTimeout(timeout);
   }
   catch(Clients::Exception& ex){
   }
}
//---------------------------------------------------------------------------
unsigned STDCALL GetClientTimeout(int number)
{
   try{
      return clients[number]->client.ReceiveBeginTimeout();
   }
   catch(Clients::Exception& ex){
   }
   return 0;
}
//---------------------------------------------------------------------------
int STDCALL TriggerSingleCapture(int number)
{
   int ret = 0;
   int registr = 0x15;
   try {
      clients[number]->client.Register(127, 3);  //page 3
      clients[number]->client.Register(registr, 1);
      ret = 1;
   }
   catch(EStreamNoMemory& ex){ clients[number]->error.SetInfo(EAV::NO_ALLOCATE_MEMORY, ex.what(), ex.get_info()); }
   catch(GIdEAddress& ex){ clients[number]->error.SetInfo(EAV::BAD_SOCK_ADDRESS, ex.what(), ex.get_info()); }
   catch(GIdETimeout& ex){ clients[number]->error.SetInfo(EAV::TIMEOUT_ON_SOCK, ex.what(), ex.get_info()); }
   catch(GIdEReadString& ex){ clients[number]->error.SetInfo(EAV::READ_STRING_FROM_SOCK, ex.what(), ex.get_info()); }
   catch(GIdTFTPNoAllPacketsException& ex){ clients[number]->error.SetInfo(EAV::TFTP_MISSING_PACKETS, ex.what(), ex.get_info()); }
   catch(GIdTFTPException& ex){ clients[number]->error.SetInfo(EAV::TFTP_PROTOCOL, ex.what(), ex.get_info()); }
   catch(GIdException& ex){ clients[number]->error.SetInfo(ex.get_code(), ex.what(), ex.get_info()); }
   catch(EAVParameterOutOfRange& ex){ clients[number]->error.SetInfo(EAV::CAMERA_PARAMATER_OUT_OF_RANGE, ex.what(), ex.get_info()); }
   catch(EAVCameraUnknown& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info()); }
   catch(EAVNotSupported& ex){ clients[number]->error.SetInfo(EAV::PARAMETER_NOT_SUPPORTED, ex.what(), ex.get_info()); }
   catch(EAVParameterUnknown& ex){ clients[number]->error.SetInfo(EAV::VALUE_OF_PARAMETER_UNKNOWN, ex.what(), ex.get_info()); }
   catch(EAVException& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_ERROR, ex.what(), ex.get_info()); }
   catch(Clients::Exception& ex){
   }
   catch(std::exception& ex) { clients[number]->error.SetInfo(-1, ex.what(), 0); }
   catch(...) { clients[number]->error.SetInfo(-1, "unknown", 0); }
   return ret;
}
//---------------------------------------------------------------------------
int STDCALL GetSingleCapture(int number)
{
   int ret = 0;
   int registr = 25;
   try {
      clients[number]->client.Register(127, 3);  //page 3
      ret = clients[number]->client.Register(registr) & 1;
   }
   catch(EStreamNoMemory& ex){ clients[number]->error.SetInfo(EAV::NO_ALLOCATE_MEMORY, ex.what(), ex.get_info()); }
   catch(GIdEAddress& ex){ clients[number]->error.SetInfo(EAV::BAD_SOCK_ADDRESS, ex.what(), ex.get_info()); }
   catch(GIdETimeout& ex){ clients[number]->error.SetInfo(EAV::TIMEOUT_ON_SOCK, ex.what(), ex.get_info()); }
   catch(GIdEReadString& ex){ clients[number]->error.SetInfo(EAV::READ_STRING_FROM_SOCK, ex.what(), ex.get_info()); }
   catch(GIdTFTPNoAllPacketsException& ex){ clients[number]->error.SetInfo(EAV::TFTP_MISSING_PACKETS, ex.what(), ex.get_info()); }
   catch(GIdTFTPException& ex){ clients[number]->error.SetInfo(EAV::TFTP_PROTOCOL, ex.what(), ex.get_info()); }
   catch(GIdException& ex){ clients[number]->error.SetInfo(ex.get_code(), ex.what(), ex.get_info()); }
   catch(EAVParameterOutOfRange& ex){ clients[number]->error.SetInfo(EAV::CAMERA_PARAMATER_OUT_OF_RANGE, ex.what(), ex.get_info()); }
   catch(EAVCameraUnknown& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info()); }
   catch(EAVNotSupported& ex){ clients[number]->error.SetInfo(EAV::PARAMETER_NOT_SUPPORTED, ex.what(), ex.get_info()); }
   catch(EAVParameterUnknown& ex){ clients[number]->error.SetInfo(EAV::VALUE_OF_PARAMETER_UNKNOWN, ex.what(), ex.get_info()); }
   catch(EAVException& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_ERROR, ex.what(), ex.get_info()); }
   catch(Clients::Exception& ex){
   }
   catch(std::exception& ex) { clients[number]->error.SetInfo(-1, ex.what(), 0); }
   catch(...) { clients[number]->error.SetInfo(-1, "unknown", 0); }
   return ret;
}
//---------------------------------------------------------------------------
int STDCALL SetSingleCapture(int number, int enable)
{
   int ret = 0;
   int registr = 25, mode = 0;
   if(enable) mode = 0x83;
   try {
      clients[number]->client.Register(127, 3);  //page 3
      clients[number]->client.Register(registr, mode);
      ret = 1;
   }
   catch(EStreamNoMemory& ex){ clients[number]->error.SetInfo(EAV::NO_ALLOCATE_MEMORY, ex.what(), ex.get_info()); }
   catch(GIdEAddress& ex){ clients[number]->error.SetInfo(EAV::BAD_SOCK_ADDRESS, ex.what(), ex.get_info()); }
   catch(GIdETimeout& ex){ clients[number]->error.SetInfo(EAV::TIMEOUT_ON_SOCK, ex.what(), ex.get_info()); }
   catch(GIdEReadString& ex){ clients[number]->error.SetInfo(EAV::READ_STRING_FROM_SOCK, ex.what(), ex.get_info()); }
   catch(GIdTFTPNoAllPacketsException& ex){ clients[number]->error.SetInfo(EAV::TFTP_MISSING_PACKETS, ex.what(), ex.get_info()); }
   catch(GIdTFTPException& ex){ clients[number]->error.SetInfo(EAV::TFTP_PROTOCOL, ex.what(), ex.get_info()); }
   catch(GIdException& ex){ clients[number]->error.SetInfo(ex.get_code(), ex.what(), ex.get_info()); }
   catch(EAVParameterOutOfRange& ex){ clients[number]->error.SetInfo(EAV::CAMERA_PARAMATER_OUT_OF_RANGE, ex.what(), ex.get_info()); }
   catch(EAVCameraUnknown& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info()); }
   catch(EAVNotSupported& ex){ clients[number]->error.SetInfo(EAV::PARAMETER_NOT_SUPPORTED, ex.what(), ex.get_info()); }
   catch(EAVParameterUnknown& ex){ clients[number]->error.SetInfo(EAV::VALUE_OF_PARAMETER_UNKNOWN, ex.what(), ex.get_info()); }
   catch(EAVException& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_ERROR, ex.what(), ex.get_info()); }
   catch(Clients::Exception& ex){
   }
   catch(std::exception& ex) { clients[number]->error.SetInfo(-1, ex.what(), 0); }
   catch(...) { clients[number]->error.SetInfo(-1, "unknown", 0); }
   return ret;
}
//---------------------------------------------------------------------------
int STDCALL GetCalibrationNumber(int number)
{
   int ret = 0;
   int registr = 22;
   try {
      clients[number]->client.Register(127, 3);  //page 3
      ret = clients[number]->client.Register(registr);
   }
   catch(EStreamNoMemory& ex){ clients[number]->error.SetInfo(EAV::NO_ALLOCATE_MEMORY, ex.what(), ex.get_info()); }
   catch(GIdEAddress& ex){ clients[number]->error.SetInfo(EAV::BAD_SOCK_ADDRESS, ex.what(), ex.get_info()); }
   catch(GIdETimeout& ex){ clients[number]->error.SetInfo(EAV::TIMEOUT_ON_SOCK, ex.what(), ex.get_info()); }
   catch(GIdEReadString& ex){ clients[number]->error.SetInfo(EAV::READ_STRING_FROM_SOCK, ex.what(), ex.get_info()); }
   catch(GIdTFTPNoAllPacketsException& ex){ clients[number]->error.SetInfo(EAV::TFTP_MISSING_PACKETS, ex.what(), ex.get_info()); }
   catch(GIdTFTPException& ex){ clients[number]->error.SetInfo(EAV::TFTP_PROTOCOL, ex.what(), ex.get_info()); }
   catch(GIdException& ex){ clients[number]->error.SetInfo(ex.get_code(), ex.what(), ex.get_info()); }
   catch(EAVParameterOutOfRange& ex){ clients[number]->error.SetInfo(EAV::CAMERA_PARAMATER_OUT_OF_RANGE, ex.what(), ex.get_info()); }
   catch(EAVCameraUnknown& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info()); }
   catch(EAVNotSupported& ex){ clients[number]->error.SetInfo(EAV::PARAMETER_NOT_SUPPORTED, ex.what(), ex.get_info()); }
   catch(EAVParameterUnknown& ex){ clients[number]->error.SetInfo(EAV::VALUE_OF_PARAMETER_UNKNOWN, ex.what(), ex.get_info()); }
   catch(EAVException& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_ERROR, ex.what(), ex.get_info()); }
   catch(Clients::Exception& ex){
   }
   catch(std::exception& ex) { clients[number]->error.SetInfo(-1, ex.what(), 0); }
   catch(...) { clients[number]->error.SetInfo(-1, "unknown", 0); }
   return ret;
}
//---------------------------------------------------------------------------
int STDCALL SetTriggerCapture(int number)
{
   int ret = 0;
   int registr = 0x15, mode = 0x01;
   bool done = false;
   try {
      clients[number]->client.Register(127, 3);  //page 3
      clients[number]->client.Register(registr, mode);
      ret = 1;
   }
   catch(EStreamNoMemory& ex){ clients[number]->error.SetInfo(EAV::NO_ALLOCATE_MEMORY, ex.what(), ex.get_info()); }
   catch(GIdEAddress& ex){ clients[number]->error.SetInfo(EAV::BAD_SOCK_ADDRESS, ex.what(), ex.get_info()); }
   catch(GIdETimeout& ex){ clients[number]->error.SetInfo(EAV::TIMEOUT_ON_SOCK, ex.what(), ex.get_info()); }
   catch(GIdEReadString& ex){ clients[number]->error.SetInfo(EAV::READ_STRING_FROM_SOCK, ex.what(), ex.get_info()); }
   catch(GIdTFTPNoAllPacketsException& ex){ clients[number]->error.SetInfo(EAV::TFTP_MISSING_PACKETS, ex.what(), ex.get_info()); }
   catch(GIdTFTPException& ex){ clients[number]->error.SetInfo(EAV::TFTP_PROTOCOL, ex.what(), ex.get_info()); }
   catch(GIdException& ex){ clients[number]->error.SetInfo(ex.get_code(), ex.what(), ex.get_info()); }
   catch(EAVParameterOutOfRange& ex){ clients[number]->error.SetInfo(EAV::CAMERA_PARAMATER_OUT_OF_RANGE, ex.what(), ex.get_info()); }
   catch(EAVCameraUnknown& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info()); }
   catch(EAVNotSupported& ex){ clients[number]->error.SetInfo(EAV::PARAMETER_NOT_SUPPORTED, ex.what(), ex.get_info()); }
   catch(EAVParameterUnknown& ex){ clients[number]->error.SetInfo(EAV::VALUE_OF_PARAMETER_UNKNOWN, ex.what(), ex.get_info()); }
   catch(EAVException& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_ERROR, ex.what(), ex.get_info()); }
   catch(Clients::Exception& ex){
   }
   catch(std::exception& ex) { clients[number]->error.SetInfo(-1, ex.what(), 0); }
   catch(...) { clients[number]->error.SetInfo(-1, "unknown", 0); }
   return ret;
}
//---------------------------------------------------------------------------
int STDCALL SetCalibrateFlash(int number)
{
   int ret = 0, retry = 0;
   int registr = 25, mode = 0xC3;
   bool done = false;
   try {
      clients[number]->client.Register(127, 3);  //page 3
      clients[number]->client.Register(registr, mode);
      ret = 1;
   }
   catch(EStreamNoMemory& ex){ clients[number]->error.SetInfo(EAV::NO_ALLOCATE_MEMORY, ex.what(), ex.get_info()); }
   catch(GIdEAddress& ex){ clients[number]->error.SetInfo(EAV::BAD_SOCK_ADDRESS, ex.what(), ex.get_info()); }
   catch(GIdETimeout& ex){ clients[number]->error.SetInfo(EAV::TIMEOUT_ON_SOCK, ex.what(), ex.get_info()); }
   catch(GIdEReadString& ex){ clients[number]->error.SetInfo(EAV::READ_STRING_FROM_SOCK, ex.what(), ex.get_info()); }
   catch(GIdTFTPNoAllPacketsException& ex){ clients[number]->error.SetInfo(EAV::TFTP_MISSING_PACKETS, ex.what(), ex.get_info()); }
   catch(GIdTFTPException& ex){ clients[number]->error.SetInfo(EAV::TFTP_PROTOCOL, ex.what(), ex.get_info()); }
   catch(GIdException& ex){ clients[number]->error.SetInfo(ex.get_code(), ex.what(), ex.get_info()); }
   catch(EAVParameterOutOfRange& ex){ clients[number]->error.SetInfo(EAV::CAMERA_PARAMATER_OUT_OF_RANGE, ex.what(), ex.get_info()); }
   catch(EAVCameraUnknown& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info()); }
   catch(EAVNotSupported& ex){ clients[number]->error.SetInfo(EAV::PARAMETER_NOT_SUPPORTED, ex.what(), ex.get_info()); }
   catch(EAVParameterUnknown& ex){ clients[number]->error.SetInfo(EAV::VALUE_OF_PARAMETER_UNKNOWN, ex.what(), ex.get_info()); }
   catch(EAVException& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_ERROR, ex.what(), ex.get_info()); }
   catch(Clients::Exception& ex){
   }
   catch(std::exception& ex) { clients[number]->error.SetInfo(-1, ex.what(), 0); }
   catch(...) { clients[number]->error.SetInfo(-1, "unknown", 0); }
   return ret;
}
//---------------------------------------------------------------------------
int STDCALL GetImage(int number, char** pdata, unsigned long* size, IMAGE_RESOLUTION resolution, float zoom, int dx, int dy)
{
        return GetImageEx(number, pdata, size, resolution, zoom, dx, dy, JPEG_CODEC, 0, NULL);
}
//---------------------------------------------------------------------------
int STDCALL GetImage2(int number, char** pdata, unsigned long* size, unsigned long* capacity,  IMAGE_RESOLUTION resolution, float zoom, int dx, int dy)
{
        return GetImage2Ex(number, pdata, size, capacity, resolution, zoom, dx, dy, JPEG_CODEC, 0, NULL);
}
//---------------------------------------------------------------------------
int STDCALL GetWindowImage(int number, char** pdata, unsigned long* size, unsigned long* capacity,  IMAGE_RESOLUTION resolution, int left, int top, int width, int height)
{
        return GetWindowImageEx(number, pdata, size, capacity,  resolution, left, top, width, height, JPEG_CODEC, 0, NULL);
}
//---------------------------------------------------------------------------
//Image(x0,y0,x1,y1);
//   BeforeImage(x0,y0,x1,y1);
//      CalculateRequest(x0,y0,x1,y1);
//         AddRectToRequest3130(URL, request_rect);
//---------------------------------------------------------------------------
int STDCALL GetWindowImageQ(int number, char** pdata, unsigned long* size, unsigned long* capacity,  IMAGE_RESOLUTION resolution, long IsDoubleScan, long Aquality, int left, int top, int width, int height)
{
        return GetWindowImageQEx(number, pdata, size, capacity,  resolution, IsDoubleScan, Aquality, left, top, width, height, JPEG_CODEC, 0, NULL, 0, 0);
}
//---------------------------------------------------------------------------
int STDCALL GetDefaultImage(int number, char** pdata, unsigned long* size, unsigned long* capacity)
{
        return GetDefaultImageEx(number, pdata, size, capacity, JPEG_CODEC, 0, NULL);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
int STDCALL GetImageEx(int number, char** pdata, unsigned long* size, IMAGE_RESOLUTION resolution, float zoom, int dx, int dy, int codec, int streamId, int* Ifarme)
{
        //GetImage
   int ret = 0;
   try
   {
      if(!pdata || !size)
         throw EAVException("zero pointer");
      clients[number]->client.Resolution(resolution);
      clients[number]->client.Zoom(MakeZoomInfo(zoom, dx, dy));
      clients[number]->client.Image((CodecID)codec, streamId, Ifarme);
      ret = 1;
   }
   catch(EStreamNoMemory& ex){ clients[number]->error.SetInfo(EAV::NO_ALLOCATE_MEMORY, ex.what(), ex.get_info()); }
   catch(GIdEAddress& ex){ clients[number]->error.SetInfo(EAV::BAD_SOCK_ADDRESS, ex.what(), ex.get_info()); }
   catch(GIdETimeout& ex){ clients[number]->error.SetInfo(EAV::TIMEOUT_ON_SOCK, ex.what(), ex.get_info()); }
   catch(GIdEReadString& ex){ clients[number]->error.SetInfo(EAV::READ_STRING_FROM_SOCK, ex.what(), ex.get_info()); }
   catch(GIdTFTPNoAllPacketsException& ex){ clients[number]->error.SetInfo(EAV::TFTP_MISSING_PACKETS, ex.what(), ex.get_info()); }
   catch(GIdTFTPException& ex){ clients[number]->error.SetInfo(EAV::TFTP_PROTOCOL, ex.what(), ex.get_info()); }
   catch(GIdException& ex){ clients[number]->error.SetInfo(ex.get_code(), ex.what(), ex.get_info()); }
   catch(EAVParameterOutOfRange& ex){ clients[number]->error.SetInfo(EAV::CAMERA_PARAMATER_OUT_OF_RANGE, ex.what(), ex.get_info()); }
   catch(EAVCameraUnknown& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info()); }
   catch(EAVNotSupported& ex){ clients[number]->error.SetInfo(EAV::PARAMETER_NOT_SUPPORTED, ex.what(), ex.get_info()); }
   catch(EAVParameterUnknown& ex){ clients[number]->error.SetInfo(EAV::VALUE_OF_PARAMETER_UNKNOWN, ex.what(), ex.get_info()); }
   catch(EAVException& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_ERROR, ex.what(), ex.get_info()); }
   catch(Clients::Exception& ex){
   }
   catch(std::exception& ex) { clients[number]->error.SetInfo(-1, ex.what(), 0); }
   catch(...) { clients[number]->error.SetInfo(-1, "unknown", 0); }
   int position = clients[number]->memory.Position();
   *pdata = clients[number]->memory.Buffer() + position;
   *size = clients[number]->memory.Size() - position;

   if (ret!=1) // if some error have happened we should ask I frame first
        clients[number]->client.makeAskIFrameFirst();

   return ret;

}

//---------------------------------------------------------------------------
int STDCALL GetImage2Ex(int number, char** pdata, unsigned long* size, unsigned long* capacity,  IMAGE_RESOLUTION resolution, float zoom, int dx, int dy, int codec, int streamId, int* Ifarme)
{
        //GetImage2
   int ret = 0;
   try {
      if(!pdata || !size || !capacity)
         throw EAVException("zero pointer");
      clients[number]->client.Resolution(resolution);
      clients[number]->client.Zoom(MakeZoomInfo(zoom, dx, dy));
      clients[number]->client.Image((CodecID)codec, streamId, Ifarme);
      ret = 1;
   }
   catch(EStreamNoMemory& ex){ clients[number]->error.SetInfo(EAV::NO_ALLOCATE_MEMORY, ex.what(), ex.get_info()); }
   catch(GIdEAddress& ex){ clients[number]->error.SetInfo(EAV::BAD_SOCK_ADDRESS, ex.what(), ex.get_info()); }
   catch(GIdETimeout& ex){ clients[number]->error.SetInfo(EAV::TIMEOUT_ON_SOCK, ex.what(), ex.get_info()); }
   catch(GIdEReadString& ex){ clients[number]->error.SetInfo(EAV::READ_STRING_FROM_SOCK, ex.what(), ex.get_info()); }
   catch(GIdTFTPNoAllPacketsException& ex){ clients[number]->error.SetInfo(EAV::TFTP_MISSING_PACKETS, ex.what(), ex.get_info()); }
   catch(GIdTFTPException& ex){ clients[number]->error.SetInfo(EAV::TFTP_PROTOCOL, ex.what(), ex.get_info()); }
   catch(GIdException& ex){ clients[number]->error.SetInfo(ex.get_code(), ex.what(), ex.get_info()); }
   catch(EAVParameterOutOfRange& ex){ clients[number]->error.SetInfo(EAV::CAMERA_PARAMATER_OUT_OF_RANGE, ex.what(), ex.get_info()); }
   catch(EAVCameraUnknown& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info()); }
   catch(EAVNotSupported& ex){ clients[number]->error.SetInfo(EAV::PARAMETER_NOT_SUPPORTED, ex.what(), ex.get_info()); }
   catch(EAVParameterUnknown& ex){ clients[number]->error.SetInfo(EAV::VALUE_OF_PARAMETER_UNKNOWN, ex.what(), ex.get_info()); }
   catch(EAVException& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_ERROR, ex.what(), ex.get_info()); }
   catch(Clients::Exception& ex){
   }
   catch(std::exception& ex) { clients[number]->error.SetInfo(-1, ex.what(), 0); }
   catch(...) { clients[number]->error.SetInfo(-1, "unknown", 0); }
   int position = clients[number]->memory.Position();
   *pdata = clients[number]->memory.Buffer() + position;
   *size = clients[number]->memory.Size() - position;
   *capacity = clients[number]->memory.Capacity() - position;

   if (ret!=1) // if some error have happened we should ask I frame first
        clients[number]->client.makeAskIFrameFirst();


   return ret;

}

//---------------------------------------------------------------------------
int STDCALL GetWindowImageEx(int number, char** pdata, unsigned long* size, unsigned long* capacity,  IMAGE_RESOLUTION resolution, int left, int top, int width, int height, int codec, int streamId, int* Ifarme)
{
        //GetWindowImage
   int ret = 0;
   try {
      if(!pdata || !size || !capacity)
         throw EAVException("zero pointer");
      if(imHALF != resolution && imFULL != resolution)
         throw EAVParameterOutOfRange("resolution must be imFULL or imHALF");
      clients[number]->client.Resolution(resolution);
      clients[number]->client.Image(left, top, width, height, (CodecID)codec, streamId, Ifarme, 0 ,0);
      ret = 1;
   }
   catch(EStreamNoMemory& ex){ clients[number]->error.SetInfo(EAV::NO_ALLOCATE_MEMORY, ex.what(), ex.get_info()); }
   catch(GIdEAddress& ex){ clients[number]->error.SetInfo(EAV::BAD_SOCK_ADDRESS, ex.what(), ex.get_info()); }
   catch(GIdETimeout& ex){ clients[number]->error.SetInfo(EAV::TIMEOUT_ON_SOCK, ex.what(), ex.get_info()); }
   catch(GIdEReadString& ex){ clients[number]->error.SetInfo(EAV::READ_STRING_FROM_SOCK, ex.what(), ex.get_info()); }
   catch(GIdTFTPNoAllPacketsException& ex){ clients[number]->error.SetInfo(EAV::TFTP_MISSING_PACKETS, ex.what(), ex.get_info()); }
   catch(GIdTFTPException& ex){ clients[number]->error.SetInfo(EAV::TFTP_PROTOCOL, ex.what(), ex.get_info()); }
   catch(GIdException& ex){ clients[number]->error.SetInfo(ex.get_code(), ex.what(), ex.get_info()); }
   catch(EAVParameterOutOfRange& ex){ clients[number]->error.SetInfo(EAV::CAMERA_PARAMATER_OUT_OF_RANGE, ex.what(), ex.get_info()); }
   catch(EAVCameraUnknown& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info()); }
   catch(EAVNotSupported& ex){ clients[number]->error.SetInfo(EAV::PARAMETER_NOT_SUPPORTED, ex.what(), ex.get_info()); }
   catch(EAVParameterUnknown& ex){ clients[number]->error.SetInfo(EAV::VALUE_OF_PARAMETER_UNKNOWN, ex.what(), ex.get_info()); }
   catch(EAVException& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_ERROR, ex.what(), ex.get_info()); }
   catch(Clients::Exception& ex){
   }
   catch(std::exception& ex) { clients[number]->error.SetInfo(-1, ex.what(), 0); }
   catch(...) { clients[number]->error.SetInfo(-1, "unknown", 0); }
   int position = clients[number]->memory.Position();
   *pdata = clients[number]->memory.Buffer() + position;
   *size = clients[number]->memory.Size() - position;
   *capacity = clients[number]->memory.Capacity() - position;

   if (ret!=1) // if some error have happened we should ask I frame first
        clients[number]->client.makeAskIFrameFirst();

   return ret;

}

int STDCALL SetChannelNumber(int number, int channel)
{
    int ret = 0;
    try
    {
            clients[number]->client.SetChannel(channel);
            return 1;
    }
    catch(EStreamNoMemory& ex){ clients[number]->error.SetInfo(EAV::NO_ALLOCATE_MEMORY, ex.what(), ex.get_info()); }
    catch(GIdEAddress& ex){ clients[number]->error.SetInfo(EAV::BAD_SOCK_ADDRESS, ex.what(), ex.get_info()); }
    catch(GIdETimeout& ex){ clients[number]->error.SetInfo(EAV::TIMEOUT_ON_SOCK, ex.what(), ex.get_info()); }
    catch(GIdEReadString& ex){ clients[number]->error.SetInfo(EAV::READ_STRING_FROM_SOCK, ex.what(), ex.get_info()); }
    catch(GIdTFTPException& ex){ clients[number]->error.SetInfo(EAV::TFTP_PROTOCOL, ex.what(), ex.get_info()); }
    catch(GIdException& ex){ clients[number]->error.SetInfo(ex.get_code(), ex.what(), ex.get_info()); }
    catch(EAVParameterOutOfRange& ex){ clients[number]->error.SetInfo(EAV::CAMERA_PARAMATER_OUT_OF_RANGE, ex.what(), ex.get_info()); }
    catch(EAVCameraUnknown& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info()); }
    catch(EAVNotSupported& ex){ clients[number]->error.SetInfo(EAV::PARAMETER_NOT_SUPPORTED, ex.what(), ex.get_info()); }
    catch(EAVParameterUnknown& ex){ clients[number]->error.SetInfo(EAV::VALUE_OF_PARAMETER_UNKNOWN, ex.what(), ex.get_info()); }
    catch(EAVException& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_ERROR, ex.what(), ex.get_info()); }
    catch(Clients::Exception& ex){
    }
    catch(std::exception& ex) { clients[number]->error.SetInfo(-1, ex.what(), 0); }
    catch(...) { clients[number]->error.SetInfo(-1, "unknown", 0); }
    return 0;
}

//---------------------------------------------------------------------------
int STDCALL GetWindowImageQEx(int number, char** pdata, unsigned long* size, unsigned long* capacity,
                                IMAGE_RESOLUTION resolution, long IsDoubleScan, long Aquality,
                                int left, int top, int width, int height, int codec, int streamId, int* Ifarme, int kbps, int intraFramePeriod)
{
        // GetWindowImageQ
   int ret = 0;
   try {
      if(!pdata || !size || !capacity)
         throw EAVException("zero pointer");
      if(imHALF != resolution && imFULL != resolution)
         throw EAVParameterOutOfRange("resolution must be imFULL or imHALF");
      clients[number]->client.Resolution(resolution);
      //clients[number]->client.PerCentImageRectangle(IsPercent);
      clients[number]->client.DoubleScan(IsDoubleScan & 1);
      clients[number]->client.SetDynamicQuality(Aquality);
      clients[number]->client.Image(left, top, width, height, (CodecID)codec, streamId, Ifarme, kbps, intraFramePeriod);
      ret = 1;
   }
   catch(EStreamNoMemory& ex){ clients[number]->error.SetInfo(EAV::NO_ALLOCATE_MEMORY, ex.what(), ex.get_info()); }
   catch(GIdEAddress& ex){ clients[number]->error.SetInfo(EAV::BAD_SOCK_ADDRESS, ex.what(), ex.get_info()); }
   catch(GIdETimeout& ex){ clients[number]->error.SetInfo(EAV::TIMEOUT_ON_SOCK, ex.what(), ex.get_info()); }
   catch(GIdEReadString& ex){ clients[number]->error.SetInfo(EAV::READ_STRING_FROM_SOCK, ex.what(), ex.get_info()); }
   catch(GIdTFTPNoAllPacketsException& ex){ clients[number]->error.SetInfo(EAV::TFTP_MISSING_PACKETS, ex.what(), ex.get_info()); }
   catch(GIdTFTPException& ex){ clients[number]->error.SetInfo(EAV::TFTP_PROTOCOL, ex.what(), ex.get_info()); }
   catch(GIdException& ex){ clients[number]->error.SetInfo(ex.get_code(), ex.what(), ex.get_info()); }
   catch(EAVParameterOutOfRange& ex){ clients[number]->error.SetInfo(EAV::CAMERA_PARAMATER_OUT_OF_RANGE, ex.what(), ex.get_info()); }
   catch(EAVCameraUnknown& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info()); }
   catch(EAVNotSupported& ex){ clients[number]->error.SetInfo(EAV::PARAMETER_NOT_SUPPORTED, ex.what(), ex.get_info()); }
   catch(EAVParameterUnknown& ex){ clients[number]->error.SetInfo(EAV::VALUE_OF_PARAMETER_UNKNOWN, ex.what(), ex.get_info()); }
   catch(EAVException& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_ERROR, ex.what(), ex.get_info()); }
   catch(Clients::Exception& ex){
   }
   catch(std::exception& ex) { clients[number]->error.SetInfo(-1, ex.what(), 0); }
   catch(...) { clients[number]->error.SetInfo(-1, "unknown", 0); }
   if(ret){
      int position = clients[number]->memory.Position();
      *pdata = clients[number]->memory.Buffer() + position;
      *size = clients[number]->memory.Size() - position;
      *capacity = clients[number]->memory.Capacity() - position;
   }
   else{
      *pdata = NULL;
      *size = 0;
      *capacity = 0;
   }


   if (ret!=1) // if some error have happened we should ask I frame first
        clients[number]->client.makeAskIFrameFirst();
   

   return ret;

}

//---------------------------------------------------------------------------
int STDCALL GetDefaultImageEx(int number, char** pdata, unsigned long* size, unsigned long* capacity, int codec, int streamId, int* Ifarme)
{
        //GetDefaultImage
   int ret = 0;
   try {
      if(!pdata || !size || !capacity)
         throw EAVException("zero pointer");
      clients[number]->client.ImageDefault((CodecID)codec, streamId, Ifarme);
      ret = 1;
   }
   catch(EStreamNoMemory& ex){ clients[number]->error.SetInfo(EAV::NO_ALLOCATE_MEMORY, ex.what(), ex.get_info()); }
   catch(GIdEAddress& ex){ clients[number]->error.SetInfo(EAV::BAD_SOCK_ADDRESS, ex.what(), ex.get_info()); }
   catch(GIdETimeout& ex){ clients[number]->error.SetInfo(EAV::TIMEOUT_ON_SOCK, ex.what(), ex.get_info()); }
   catch(GIdEReadString& ex){ clients[number]->error.SetInfo(EAV::READ_STRING_FROM_SOCK, ex.what(), ex.get_info()); }
   catch(GIdTFTPNoAllPacketsException& ex){ clients[number]->error.SetInfo(EAV::TFTP_MISSING_PACKETS, ex.what(), ex.get_info()); }
   catch(GIdTFTPException& ex){ clients[number]->error.SetInfo(EAV::TFTP_PROTOCOL, ex.what(), ex.get_info()); }
   catch(GIdException& ex){ clients[number]->error.SetInfo(ex.get_code(), ex.what(), ex.get_info()); }
   catch(EAVParameterOutOfRange& ex){ clients[number]->error.SetInfo(EAV::CAMERA_PARAMATER_OUT_OF_RANGE, ex.what(), ex.get_info()); }
   catch(EAVCameraUnknown& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info()); }
   catch(EAVNotSupported& ex){ clients[number]->error.SetInfo(EAV::PARAMETER_NOT_SUPPORTED, ex.what(), ex.get_info()); }
   catch(EAVParameterUnknown& ex){ clients[number]->error.SetInfo(EAV::VALUE_OF_PARAMETER_UNKNOWN, ex.what(), ex.get_info()); }
   catch(EAVException& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_ERROR, ex.what(), ex.get_info()); }
   catch(Clients::Exception& ex){
   }
   catch(std::exception& ex) { clients[number]->error.SetInfo(-1, ex.what(), 0); }
   catch(...) { clients[number]->error.SetInfo(-1, "unknown", 0); }
   int position = clients[number]->memory.Position();
   *pdata = clients[number]->memory.Buffer() + position;
   *size = clients[number]->memory.Size() - position;
   *capacity = clients[number]->memory.Capacity() - position;

   if (ret!=1) // if some error have happened we should ask I frame first
        clients[number]->client.makeAskIFrameFirst();
   

   return ret;

}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------


int STDCALL SetAV2000Register(int number, unsigned char registr, unsigned char* value)
{
   try {
      if(!value)
         throw EAVException("zero pointer");
      clients[number]->client.Register(registr, value[0] * 256U + value[1]);
      return 1;
   }
   catch(EStreamNoMemory& ex){ clients[number]->error.SetInfo(EAV::NO_ALLOCATE_MEMORY, ex.what(), ex.get_info()); }
   catch(GIdEAddress& ex){ clients[number]->error.SetInfo(EAV::BAD_SOCK_ADDRESS, ex.what(), ex.get_info()); }
   catch(GIdETimeout& ex){ clients[number]->error.SetInfo(EAV::TIMEOUT_ON_SOCK, ex.what(), ex.get_info()); }
   catch(GIdEReadString& ex){ clients[number]->error.SetInfo(EAV::READ_STRING_FROM_SOCK, ex.what(), ex.get_info()); }
   catch(GIdTFTPException& ex){ clients[number]->error.SetInfo(EAV::TFTP_PROTOCOL, ex.what(), ex.get_info()); }
   catch(GIdException& ex){ clients[number]->error.SetInfo(ex.get_code(), ex.what(), ex.get_info()); }
   catch(EAVParameterOutOfRange& ex){ clients[number]->error.SetInfo(EAV::CAMERA_PARAMATER_OUT_OF_RANGE, ex.what(), ex.get_info()); }
   catch(EAVCameraUnknown& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info()); }
   catch(EAVNotSupported& ex){ clients[number]->error.SetInfo(EAV::PARAMETER_NOT_SUPPORTED, ex.what(), ex.get_info()); }
   catch(EAVParameterUnknown& ex){ clients[number]->error.SetInfo(EAV::VALUE_OF_PARAMETER_UNKNOWN, ex.what(), ex.get_info()); }
   catch(EAVException& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_ERROR, ex.what(), ex.get_info()); }
   catch(Clients::Exception& ex){
   }
   catch(std::exception& ex) { clients[number]->error.SetInfo(-1, ex.what(), 0); }
   catch(...) { clients[number]->error.SetInfo(-1, "unknown", 0); }
   return 0;
}
//---------------------------------------------------------------------------
int STDCALL GetAV2000Register(int number, unsigned char registr, unsigned char* pvalue)
{
   try {
      if(!pvalue)
         throw EAVException("zero pointer");
      unsigned res = clients[number]->client.Register(registr);
      pvalue[0] = res / 256;
      pvalue[1] = res % 256;
      return 1;
   }
   catch(EStreamNoMemory& ex){ clients[number]->error.SetInfo(EAV::NO_ALLOCATE_MEMORY, ex.what(), ex.get_info()); }
   catch(GIdEAddress& ex){ clients[number]->error.SetInfo(EAV::BAD_SOCK_ADDRESS, ex.what(), ex.get_info()); }
   catch(GIdETimeout& ex){ clients[number]->error.SetInfo(EAV::TIMEOUT_ON_SOCK, ex.what(), ex.get_info()); }
   catch(GIdEReadString& ex){ clients[number]->error.SetInfo(EAV::READ_STRING_FROM_SOCK, ex.what(), ex.get_info()); }
   catch(GIdTFTPException& ex){ clients[number]->error.SetInfo(EAV::TFTP_PROTOCOL, ex.what(), ex.get_info()); }
   catch(GIdException& ex){ clients[number]->error.SetInfo(ex.get_code(), ex.what(), ex.get_info()); }
   catch(EAVParameterOutOfRange& ex){ clients[number]->error.SetInfo(EAV::CAMERA_PARAMATER_OUT_OF_RANGE, ex.what(), ex.get_info()); }
   catch(EAVCameraUnknown& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info()); }
   catch(EAVNotSupported& ex){ clients[number]->error.SetInfo(EAV::PARAMETER_NOT_SUPPORTED, ex.what(), ex.get_info()); }
   catch(EAVParameterUnknown& ex){ clients[number]->error.SetInfo(EAV::VALUE_OF_PARAMETER_UNKNOWN, ex.what(), ex.get_info()); }
   catch(EAVException& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_ERROR, ex.what(), ex.get_info()); }
   catch(Clients::Exception& ex){ }
   catch(std::exception& ex) { clients[number]->error.SetInfo(-1, ex.what(), 0); }
   catch(...) { clients[number]->error.SetInfo(-1, "unknown", 0); }
   return 0;
}
//---------------------------------------------------------------------------
int STDCALL SetAV2000Parameter(int number, CAMERA_PARAMETER param, long* value)
{
   try {
      if(!value)
         throw EAVException("zero pointer");
      switch(param){
      case cpMD_MATRIX :
         clients[number]->client.Parameter(param, reinterpret_cast<long>(value));
         break;
      default :
         clients[number]->client.Parameter(param, *value);
      }
      return 1;
   }
   catch(EStreamNoMemory& ex){ clients[number]->error.SetInfo(EAV::NO_ALLOCATE_MEMORY, ex.what(), ex.get_info()); }
   catch(GIdEAddress& ex){ clients[number]->error.SetInfo(EAV::BAD_SOCK_ADDRESS, ex.what(), ex.get_info()); }
   catch(GIdETimeout& ex){ clients[number]->error.SetInfo(EAV::TIMEOUT_ON_SOCK, ex.what(), ex.get_info()); }
   catch(GIdEReadString& ex){ clients[number]->error.SetInfo(EAV::READ_STRING_FROM_SOCK, ex.what(), ex.get_info()); }
   catch(GIdTFTPException& ex){ clients[number]->error.SetInfo(EAV::TFTP_PROTOCOL, ex.what(), ex.get_info()); }
   catch(GIdException& ex){ clients[number]->error.SetInfo(ex.get_code(), ex.what(), ex.get_info()); }
   catch(EAVParameterOutOfRange& ex){ clients[number]->error.SetInfo(EAV::CAMERA_PARAMATER_OUT_OF_RANGE, ex.what(), ex.get_info()); }
   catch(EAVCameraUnknown& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info()); }
   catch(EAVNotSupported& ex){ clients[number]->error.SetInfo(EAV::PARAMETER_NOT_SUPPORTED, ex.what(), ex.get_info()); }
   catch(EAVParameterUnknown& ex){ clients[number]->error.SetInfo(EAV::VALUE_OF_PARAMETER_UNKNOWN, ex.what(), ex.get_info()); }
   catch(EAVException& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_ERROR, ex.what(), ex.get_info()); }
   catch(Clients::Exception& ex){
   }
   catch(std::exception& ex) { clients[number]->error.SetInfo(-1, ex.what(), 0); }
   catch(...) { clients[number]->error.SetInfo(-1, "unknown", 0); }
   return 0;
}
//---------------------------------------------------------------------------
int STDCALL GetAV2000Parameter(int number, CAMERA_PARAMETER param, long* result)
{
   try {
      if(!result)
         throw EAVException("zero pointer");
      switch(param){
      case cpMD_MATRIX :
         Copy(reinterpret_cast<unsigned char*>(clients[number]->client.Parameter(param)), reinterpret_cast<unsigned char*>(result), 8);
         break;
      default :
         *result = clients[number]->client.Parameter(param);
      }
      return 1;
   }
   catch(EStreamNoMemory& ex){ clients[number]->error.SetInfo(EAV::NO_ALLOCATE_MEMORY, ex.what(), ex.get_info()); }
   catch(GIdEAddress& ex){ clients[number]->error.SetInfo(EAV::BAD_SOCK_ADDRESS, ex.what(), ex.get_info()); }
   catch(GIdETimeout& ex){ clients[number]->error.SetInfo(EAV::TIMEOUT_ON_SOCK, ex.what(), ex.get_info()); }
   catch(GIdEReadString& ex){ clients[number]->error.SetInfo(EAV::READ_STRING_FROM_SOCK, ex.what(), ex.get_info()); }
   catch(GIdTFTPException& ex){ clients[number]->error.SetInfo(EAV::TFTP_PROTOCOL, ex.what(), ex.get_info()); }
   catch(GIdException& ex){ clients[number]->error.SetInfo(ex.get_code(), ex.what(), ex.get_info()); }
   catch(EAVParameterOutOfRange& ex){ clients[number]->error.SetInfo(EAV::CAMERA_PARAMATER_OUT_OF_RANGE, ex.what(), ex.get_info()); }
   catch(EAVCameraUnknown& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info()); }
   catch(EAVNotSupported& ex){ clients[number]->error.SetInfo(EAV::PARAMETER_NOT_SUPPORTED, ex.what(), ex.get_info()); }
   catch(EAVParameterUnknown& ex){ clients[number]->error.SetInfo(EAV::VALUE_OF_PARAMETER_UNKNOWN, ex.what(), ex.get_info()); }
   catch(EAVException& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_ERROR, ex.what(), ex.get_info()); }
   catch(Clients::Exception& ex){
   }
   catch(std::exception& ex) { clients[number]->error.SetInfo(-1, ex.what(), 0); }
   catch(...) { clients[number]->error.SetInfo(-1, "unknown", 0); }
   return 0;
}
//---------------------------------------------------------------------------
int STDCALL SetAuxIO(int number, int OnOff)
{
   int ret = 0;
   int registr = 25;
   int cmd = OnOff ? 0x20 : 0x10;
   try {
      clients[number]->client.Register(127, 3);  //page 3
      clients[number]->client.Register(registr,cmd);
      ret = 1;
   }
   catch(EStreamNoMemory& ex){ clients[number]->error.SetInfo(EAV::NO_ALLOCATE_MEMORY, ex.what(), ex.get_info()); }
   catch(GIdEAddress& ex){ clients[number]->error.SetInfo(EAV::BAD_SOCK_ADDRESS, ex.what(), ex.get_info()); }
   catch(GIdETimeout& ex){ clients[number]->error.SetInfo(EAV::TIMEOUT_ON_SOCK, ex.what(), ex.get_info()); }
   catch(GIdEReadString& ex){ clients[number]->error.SetInfo(EAV::READ_STRING_FROM_SOCK, ex.what(), ex.get_info()); }
   catch(GIdTFTPNoAllPacketsException& ex){ clients[number]->error.SetInfo(EAV::TFTP_MISSING_PACKETS, ex.what(), ex.get_info()); }
   catch(GIdTFTPException& ex){ clients[number]->error.SetInfo(EAV::TFTP_PROTOCOL, ex.what(), ex.get_info()); }
   catch(GIdException& ex){ clients[number]->error.SetInfo(ex.get_code(), ex.what(), ex.get_info()); }
   catch(EAVParameterOutOfRange& ex){ clients[number]->error.SetInfo(EAV::CAMERA_PARAMATER_OUT_OF_RANGE, ex.what(), ex.get_info()); }
   catch(EAVCameraUnknown& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info()); }
   catch(EAVNotSupported& ex){ clients[number]->error.SetInfo(EAV::PARAMETER_NOT_SUPPORTED, ex.what(), ex.get_info()); }
   catch(EAVParameterUnknown& ex){ clients[number]->error.SetInfo(EAV::VALUE_OF_PARAMETER_UNKNOWN, ex.what(), ex.get_info()); }
   catch(EAVException& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_ERROR, ex.what(), ex.get_info()); }
   catch(Clients::Exception& ex){
   }
   catch(std::exception& ex) { clients[number]->error.SetInfo(-1, ex.what(), 0); }
   catch(...) { clients[number]->error.SetInfo(-1, "unknown", 0); }
   return ret;
}
//---------------------------------------------------------------------------
int STDCALL GetAuxIO(int number)
{
   int ret = -1;
   int registr = 25;
   int cmd = 0x30;
   try {
      ret = clients[number]->client.GetAuxInStatus();
      if(ret == -1){
          clients[number]->client.Register(127, 3);  //page 3
          clients[number]->client.Register(registr,cmd);
          clients[number]->client.Register(127, 3);  //page 3
          ret = clients[number]->client.Register(registr) & 1;
      }
   }
   catch(EStreamNoMemory& ex){ clients[number]->error.SetInfo(EAV::NO_ALLOCATE_MEMORY, ex.what(), ex.get_info()); }
   catch(GIdEAddress& ex){ clients[number]->error.SetInfo(EAV::BAD_SOCK_ADDRESS, ex.what(), ex.get_info()); }
   catch(GIdETimeout& ex){ clients[number]->error.SetInfo(EAV::TIMEOUT_ON_SOCK, ex.what(), ex.get_info()); }
   catch(GIdEReadString& ex){ clients[number]->error.SetInfo(EAV::READ_STRING_FROM_SOCK, ex.what(), ex.get_info()); }
   catch(GIdTFTPNoAllPacketsException& ex){ clients[number]->error.SetInfo(EAV::TFTP_MISSING_PACKETS, ex.what(), ex.get_info()); }
   catch(GIdTFTPException& ex){ clients[number]->error.SetInfo(EAV::TFTP_PROTOCOL, ex.what(), ex.get_info()); }
   catch(GIdException& ex){ clients[number]->error.SetInfo(ex.get_code(), ex.what(), ex.get_info()); }
   catch(EAVParameterOutOfRange& ex){ clients[number]->error.SetInfo(EAV::CAMERA_PARAMATER_OUT_OF_RANGE, ex.what(), ex.get_info()); }
   catch(EAVCameraUnknown& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info()); }
   catch(EAVNotSupported& ex){ clients[number]->error.SetInfo(EAV::PARAMETER_NOT_SUPPORTED, ex.what(), ex.get_info()); }
   catch(EAVParameterUnknown& ex){ clients[number]->error.SetInfo(EAV::VALUE_OF_PARAMETER_UNKNOWN, ex.what(), ex.get_info()); }
   catch(EAVException& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_ERROR, ex.what(), ex.get_info()); }
   catch(Clients::Exception& ex){
   }
   catch(std::exception& ex) { clients[number]->error.SetInfo(-1, ex.what(), 0); }
   catch(...) { clients[number]->error.SetInfo(-1, "unknown", 0); }
   return ret;
}
//---------------------------------------------------------------------------
int STDCALL GetAuxOutStatus(int number)
{
   int ret = -1;
   try {
      ret = clients[number]->client.GetAuxOutStatus();
   }
   catch(EStreamNoMemory& ex){ clients[number]->error.SetInfo(EAV::NO_ALLOCATE_MEMORY, ex.what(), ex.get_info()); }
   catch(GIdEAddress& ex){ clients[number]->error.SetInfo(EAV::BAD_SOCK_ADDRESS, ex.what(), ex.get_info()); }
   catch(GIdETimeout& ex){ clients[number]->error.SetInfo(EAV::TIMEOUT_ON_SOCK, ex.what(), ex.get_info()); }
   catch(GIdEReadString& ex){ clients[number]->error.SetInfo(EAV::READ_STRING_FROM_SOCK, ex.what(), ex.get_info()); }
   catch(GIdTFTPNoAllPacketsException& ex){ clients[number]->error.SetInfo(EAV::TFTP_MISSING_PACKETS, ex.what(), ex.get_info()); }
   catch(GIdTFTPException& ex){ clients[number]->error.SetInfo(EAV::TFTP_PROTOCOL, ex.what(), ex.get_info()); }
   catch(GIdException& ex){ clients[number]->error.SetInfo(ex.get_code(), ex.what(), ex.get_info()); }
   catch(EAVParameterOutOfRange& ex){ clients[number]->error.SetInfo(EAV::CAMERA_PARAMATER_OUT_OF_RANGE, ex.what(), ex.get_info()); }
   catch(EAVCameraUnknown& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info()); }
   catch(EAVNotSupported& ex){ clients[number]->error.SetInfo(EAV::PARAMETER_NOT_SUPPORTED, ex.what(), ex.get_info()); }
   catch(EAVParameterUnknown& ex){ clients[number]->error.SetInfo(EAV::VALUE_OF_PARAMETER_UNKNOWN, ex.what(), ex.get_info()); }
   catch(EAVException& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_ERROR, ex.what(), ex.get_info()); }
   catch(Clients::Exception& ex){
   }
   catch(std::exception& ex) { clients[number]->error.SetInfo(-1, ex.what(), 0); }
   catch(...) { clients[number]->error.SetInfo(-1, "unknown", 0); }
   return ret;
}
//---------------------------------------------------------------------------
const unsigned char* STDCALL GetCameraMakeBuffer(int number)
{
   const unsigned char* ret = 0;
   try {
       ret = clients[number]->client.GetCameraMakeBuffer();
   }
   catch(EStreamNoMemory& ex){ clients[number]->error.SetInfo(EAV::NO_ALLOCATE_MEMORY, ex.what(), ex.get_info()); }
   catch(GIdEAddress& ex){ clients[number]->error.SetInfo(EAV::BAD_SOCK_ADDRESS, ex.what(), ex.get_info()); }
   catch(GIdETimeout& ex){ clients[number]->error.SetInfo(EAV::TIMEOUT_ON_SOCK, ex.what(), ex.get_info()); }
   catch(GIdEReadString& ex){ clients[number]->error.SetInfo(EAV::READ_STRING_FROM_SOCK, ex.what(), ex.get_info()); }
   catch(GIdTFTPNoAllPacketsException& ex){ clients[number]->error.SetInfo(EAV::TFTP_MISSING_PACKETS, ex.what(), ex.get_info()); }
   catch(GIdTFTPException& ex){ clients[number]->error.SetInfo(EAV::TFTP_PROTOCOL, ex.what(), ex.get_info()); }
   catch(GIdException& ex){ clients[number]->error.SetInfo(ex.get_code(), ex.what(), ex.get_info()); }
   catch(EAVParameterOutOfRange& ex){ clients[number]->error.SetInfo(EAV::CAMERA_PARAMATER_OUT_OF_RANGE, ex.what(), ex.get_info()); }
   catch(EAVCameraUnknown& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info()); }
   catch(EAVNotSupported& ex){ clients[number]->error.SetInfo(EAV::PARAMETER_NOT_SUPPORTED, ex.what(), ex.get_info()); }
   catch(EAVParameterUnknown& ex){ clients[number]->error.SetInfo(EAV::VALUE_OF_PARAMETER_UNKNOWN, ex.what(), ex.get_info()); }
   catch(EAVException& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_ERROR, ex.what(), ex.get_info()); }
   catch(Clients::Exception& ex){
   }
   catch(std::exception& ex) { clients[number]->error.SetInfo(-1, ex.what(), 0); }
   catch(...) { clients[number]->error.SetInfo(-1, "unknown", 0); }
   return ret;
}
//---------------------------------------------------------------------------
const unsigned char* STDCALL GetTimeStampBuffer(int number)
{
   const unsigned char* ret = 0;
   try {
      ret = clients[number]->client.GetFirmwareTimestamp();
   }
   catch(EStreamNoMemory& ex){ clients[number]->error.SetInfo(EAV::NO_ALLOCATE_MEMORY, ex.what(), ex.get_info()); }
   catch(GIdEAddress& ex){ clients[number]->error.SetInfo(EAV::BAD_SOCK_ADDRESS, ex.what(), ex.get_info()); }
   catch(GIdETimeout& ex){ clients[number]->error.SetInfo(EAV::TIMEOUT_ON_SOCK, ex.what(), ex.get_info()); }
   catch(GIdEReadString& ex){ clients[number]->error.SetInfo(EAV::READ_STRING_FROM_SOCK, ex.what(), ex.get_info()); }
   catch(GIdTFTPNoAllPacketsException& ex){ clients[number]->error.SetInfo(EAV::TFTP_MISSING_PACKETS, ex.what(), ex.get_info()); }
   catch(GIdTFTPException& ex){ clients[number]->error.SetInfo(EAV::TFTP_PROTOCOL, ex.what(), ex.get_info()); }
   catch(GIdException& ex){ clients[number]->error.SetInfo(ex.get_code(), ex.what(), ex.get_info()); }
   catch(EAVParameterOutOfRange& ex){ clients[number]->error.SetInfo(EAV::CAMERA_PARAMATER_OUT_OF_RANGE, ex.what(), ex.get_info()); }
   catch(EAVCameraUnknown& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info()); }
   catch(EAVNotSupported& ex){ clients[number]->error.SetInfo(EAV::PARAMETER_NOT_SUPPORTED, ex.what(), ex.get_info()); }
   catch(EAVParameterUnknown& ex){ clients[number]->error.SetInfo(EAV::VALUE_OF_PARAMETER_UNKNOWN, ex.what(), ex.get_info()); }
   catch(EAVException& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_ERROR, ex.what(), ex.get_info()); }
   catch(Clients::Exception& ex){
   }
   catch(std::exception& ex) { clients[number]->error.SetInfo(-1, ex.what(), 0); }
   catch(...) { clients[number]->error.SetInfo(-1, "unknown", 0); }
   return ret;
}
//---------------------------------------------------------------------------
int STDCALL SetIRFilter(int number, int OnOff)
{
   int ret = 0;
   int registr = 6;
   int cmd = OnOff ? 0x04 : 0x00;
   try {
      clients[number]->client.Register(127, 1);  //page 1
      clients[number]->client.Register(registr,cmd);
      clients[number]->client.Register(127, 3);  //page 1
      ret = 1;
   }
   catch(EStreamNoMemory& ex){ clients[number]->error.SetInfo(EAV::NO_ALLOCATE_MEMORY, ex.what(), ex.get_info()); }
   catch(GIdEAddress& ex){ clients[number]->error.SetInfo(EAV::BAD_SOCK_ADDRESS, ex.what(), ex.get_info()); }
   catch(GIdETimeout& ex){ clients[number]->error.SetInfo(EAV::TIMEOUT_ON_SOCK, ex.what(), ex.get_info()); }
   catch(GIdEReadString& ex){ clients[number]->error.SetInfo(EAV::READ_STRING_FROM_SOCK, ex.what(), ex.get_info()); }
   catch(GIdTFTPNoAllPacketsException& ex){ clients[number]->error.SetInfo(EAV::TFTP_MISSING_PACKETS, ex.what(), ex.get_info()); }
   catch(GIdTFTPException& ex){ clients[number]->error.SetInfo(EAV::TFTP_PROTOCOL, ex.what(), ex.get_info()); }
   catch(GIdException& ex){ clients[number]->error.SetInfo(ex.get_code(), ex.what(), ex.get_info()); }
   catch(EAVParameterOutOfRange& ex){ clients[number]->error.SetInfo(EAV::CAMERA_PARAMATER_OUT_OF_RANGE, ex.what(), ex.get_info()); }
   catch(EAVCameraUnknown& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info()); }
   catch(EAVNotSupported& ex){ clients[number]->error.SetInfo(EAV::PARAMETER_NOT_SUPPORTED, ex.what(), ex.get_info()); }
   catch(EAVParameterUnknown& ex){ clients[number]->error.SetInfo(EAV::VALUE_OF_PARAMETER_UNKNOWN, ex.what(), ex.get_info()); }
   catch(EAVException& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_ERROR, ex.what(), ex.get_info()); }
   catch(Clients::Exception& ex){
   }
   catch(std::exception& ex) { clients[number]->error.SetInfo(-1, ex.what(), 0); }
   catch(...) { clients[number]->error.SetInfo(-1, "unknown", 0); }
   return ret;
}
//---------------------------------------------------------------------------
int STDCALL GetIRFilter(int number)
{
   int ret = -1;
   int registr = 6;
   try {
      clients[number]->client.Register(127, 1);  //page 1
      ret = clients[number]->client.Register(registr);
      clients[number]->client.Register(127, 3);  //page 1
   }
   catch(EStreamNoMemory& ex){ clients[number]->error.SetInfo(EAV::NO_ALLOCATE_MEMORY, ex.what(), ex.get_info()); }
   catch(GIdEAddress& ex){ clients[number]->error.SetInfo(EAV::BAD_SOCK_ADDRESS, ex.what(), ex.get_info()); }
   catch(GIdETimeout& ex){ clients[number]->error.SetInfo(EAV::TIMEOUT_ON_SOCK, ex.what(), ex.get_info()); }
   catch(GIdEReadString& ex){ clients[number]->error.SetInfo(EAV::READ_STRING_FROM_SOCK, ex.what(), ex.get_info()); }
   catch(GIdTFTPNoAllPacketsException& ex){ clients[number]->error.SetInfo(EAV::TFTP_MISSING_PACKETS, ex.what(), ex.get_info()); }
   catch(GIdTFTPException& ex){ clients[number]->error.SetInfo(EAV::TFTP_PROTOCOL, ex.what(), ex.get_info()); }
   catch(GIdException& ex){ clients[number]->error.SetInfo(ex.get_code(), ex.what(), ex.get_info()); }
   catch(EAVParameterOutOfRange& ex){ clients[number]->error.SetInfo(EAV::CAMERA_PARAMATER_OUT_OF_RANGE, ex.what(), ex.get_info()); }
   catch(EAVCameraUnknown& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info()); }
   catch(EAVNotSupported& ex){ clients[number]->error.SetInfo(EAV::PARAMETER_NOT_SUPPORTED, ex.what(), ex.get_info()); }
   catch(EAVParameterUnknown& ex){ clients[number]->error.SetInfo(EAV::VALUE_OF_PARAMETER_UNKNOWN, ex.what(), ex.get_info()); }
   catch(EAVException& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_ERROR, ex.what(), ex.get_info()); }
   catch(Clients::Exception& ex){
   }
   catch(std::exception& ex) { clients[number]->error.SetInfo(-1, ex.what(), 0); }
   catch(...) { clients[number]->error.SetInfo(-1, "unknown", 0); }
   return ret;
}
//---------------------------------------------------------------------------
int STDCALL InitializeCopyRightStrings(int number, EXIF_STRING_ID id, unsigned char* pbuf)
{
   int ret = -1;
   int registr = 6;
   try {
      clients[number]->client.InitializeCopyRightStrings(id, pbuf);
      ret = 0;
   }
   catch(EStreamNoMemory& ex){ clients[number]->error.SetInfo(EAV::NO_ALLOCATE_MEMORY, ex.what(), ex.get_info()); }
   catch(GIdEAddress& ex){ clients[number]->error.SetInfo(EAV::BAD_SOCK_ADDRESS, ex.what(), ex.get_info()); }
   catch(GIdETimeout& ex){ clients[number]->error.SetInfo(EAV::TIMEOUT_ON_SOCK, ex.what(), ex.get_info()); }
   catch(GIdEReadString& ex){ clients[number]->error.SetInfo(EAV::READ_STRING_FROM_SOCK, ex.what(), ex.get_info()); }
   catch(GIdTFTPNoAllPacketsException& ex){ clients[number]->error.SetInfo(EAV::TFTP_MISSING_PACKETS, ex.what(), ex.get_info()); }
   catch(GIdTFTPException& ex){ clients[number]->error.SetInfo(EAV::TFTP_PROTOCOL, ex.what(), ex.get_info()); }
   catch(GIdException& ex){ clients[number]->error.SetInfo(ex.get_code(), ex.what(), ex.get_info()); }
   catch(EAVParameterOutOfRange& ex){ clients[number]->error.SetInfo(EAV::CAMERA_PARAMATER_OUT_OF_RANGE, ex.what(), ex.get_info()); }
   catch(EAVCameraUnknown& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info()); }
   catch(EAVNotSupported& ex){ clients[number]->error.SetInfo(EAV::PARAMETER_NOT_SUPPORTED, ex.what(), ex.get_info()); }
   catch(EAVParameterUnknown& ex){ clients[number]->error.SetInfo(EAV::VALUE_OF_PARAMETER_UNKNOWN, ex.what(), ex.get_info()); }
   catch(EAVException& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_ERROR, ex.what(), ex.get_info()); }
   catch(Clients::Exception& ex){
   }
   catch(std::exception& ex) { clients[number]->error.SetInfo(-1, ex.what(), 0); }
   catch(...) { clients[number]->error.SetInfo(-1, "unknown", 0); }
   return ret;
}

//---------------------------------------------------------------------------
int STDCALL FactoryDefault(int number)
{
   try {
      clients[number]->client.FactoryDefault();
      return 1;
   }
   catch(EStreamNoMemory& ex){ clients[number]->error.SetInfo(EAV::NO_ALLOCATE_MEMORY, ex.what(), ex.get_info()); }
   catch(GIdEAddress& ex){ clients[number]->error.SetInfo(EAV::BAD_SOCK_ADDRESS, ex.what(), ex.get_info()); }
   catch(GIdETimeout& ex){ clients[number]->error.SetInfo(EAV::TIMEOUT_ON_SOCK, ex.what(), ex.get_info()); }
   catch(GIdEReadString& ex){ clients[number]->error.SetInfo(EAV::READ_STRING_FROM_SOCK, ex.what(), ex.get_info()); }
   catch(GIdTFTPException& ex){ clients[number]->error.SetInfo(EAV::TFTP_PROTOCOL, ex.what(), ex.get_info()); }
   catch(GIdException& ex){ clients[number]->error.SetInfo(ex.get_code(), ex.what(), ex.get_info()); }
   catch(EAVParameterOutOfRange& ex){ clients[number]->error.SetInfo(EAV::CAMERA_PARAMATER_OUT_OF_RANGE, ex.what(), ex.get_info()); }
   catch(EAVCameraUnknown& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info()); }
   catch(EAVNotSupported& ex){ clients[number]->error.SetInfo(EAV::PARAMETER_NOT_SUPPORTED, ex.what(), ex.get_info()); }
   catch(EAVParameterUnknown& ex){ clients[number]->error.SetInfo(EAV::VALUE_OF_PARAMETER_UNKNOWN, ex.what(), ex.get_info()); }
   catch(EAVException& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_ERROR, ex.what(), ex.get_info()); }
   catch(Clients::Exception& ex){
   }
   catch(std::exception& ex) { clients[number]->error.SetInfo(-1, ex.what(), 0); }
   catch(...) { clients[number]->error.SetInfo(-1, "unknown", 0); }
   return 0;
}
//---------------------------------------------------------------------------
int STDCALL Permanently(int number)
{
   try {
      clients[number]->client.Permanently();
      return 1;
   }
   catch(EStreamNoMemory& ex){ clients[number]->error.SetInfo(EAV::NO_ALLOCATE_MEMORY, ex.what(), ex.get_info()); }
   catch(GIdEAddress& ex){ clients[number]->error.SetInfo(EAV::BAD_SOCK_ADDRESS, ex.what(), ex.get_info()); }
   catch(GIdETimeout& ex){ clients[number]->error.SetInfo(EAV::TIMEOUT_ON_SOCK, ex.what(), ex.get_info()); }
   catch(GIdEReadString& ex){ clients[number]->error.SetInfo(EAV::READ_STRING_FROM_SOCK, ex.what(), ex.get_info()); }
   catch(GIdTFTPException& ex){ clients[number]->error.SetInfo(EAV::TFTP_PROTOCOL, ex.what(), ex.get_info()); }
   catch(GIdException& ex){ clients[number]->error.SetInfo(ex.get_code(), ex.what(), ex.get_info()); }
   catch(EAVParameterOutOfRange& ex){ clients[number]->error.SetInfo(EAV::CAMERA_PARAMATER_OUT_OF_RANGE, ex.what(), ex.get_info()); }
   catch(EAVCameraUnknown& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info()); }
   catch(EAVNotSupported& ex){ clients[number]->error.SetInfo(EAV::PARAMETER_NOT_SUPPORTED, ex.what(), ex.get_info()); }
   catch(EAVParameterUnknown& ex){ clients[number]->error.SetInfo(EAV::VALUE_OF_PARAMETER_UNKNOWN, ex.what(), ex.get_info()); }
   catch(EAVException& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_ERROR, ex.what(), ex.get_info()); }
   catch(Clients::Exception& ex){
   }
   catch(std::exception& ex) { clients[number]->error.SetInfo(-1, ex.what(), 0); }
   catch(...) { clients[number]->error.SetInfo(-1, "unknown", 0); }
   return 0;
}
//---------------------------------------------------------------------------
int STDCALL UpdateVersion(int number)
{
   try{
	
	   clients[number]->client.UpdateCamera();
	
      return 1;
   }
   catch(EStreamNoMemory& ex){ clients[number]->error.SetInfo(EAV::NO_ALLOCATE_MEMORY, ex.what(), ex.get_info()); }
   catch(GIdEAddress& ex){ clients[number]->error.SetInfo(EAV::BAD_SOCK_ADDRESS, ex.what(), ex.get_info()); }
   catch(GIdETimeout& ex){ clients[number]->error.SetInfo(EAV::TIMEOUT_ON_SOCK, ex.what(), ex.get_info()); }
   catch(GIdEReadString& ex){ clients[number]->error.SetInfo(EAV::READ_STRING_FROM_SOCK, ex.what(), ex.get_info()); }
   catch(GIdTFTPException& ex){ clients[number]->error.SetInfo(EAV::TFTP_PROTOCOL, ex.what(), ex.get_info()); }
   catch(GIdException& ex){ clients[number]->error.SetInfo(ex.get_code(), ex.what(), ex.get_info()); }
   catch(EAVParameterOutOfRange& ex){ clients[number]->error.SetInfo(EAV::CAMERA_PARAMATER_OUT_OF_RANGE, ex.what(), ex.get_info()); }
   catch(EAVCameraUnknown& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info()); }
   catch(EAVNotSupported& ex){ clients[number]->error.SetInfo(EAV::PARAMETER_NOT_SUPPORTED, ex.what(), ex.get_info()); }
   catch(EAVParameterUnknown& ex){ clients[number]->error.SetInfo(EAV::VALUE_OF_PARAMETER_UNKNOWN, ex.what(), ex.get_info()); }
   catch(EAVException& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_ERROR, ex.what(), ex.get_info()); }
   catch(Clients::Exception& ex){
   }
   catch(std::exception& ex) { clients[number]->error.SetInfo(-1, ex.what(), 0); }
   catch(...) { clients[number]->error.SetInfo(-1, "unknown", 0); }
   return 0;
}
//---------------------------------------------------------------------------
unsigned long STDCALL Version(int number)
{
   try{
      return clients[number]->client.Version();
   }
   catch(EAVCameraUnknown& ex){
      clients[number]->error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info());
   }
   catch(Clients::Exception& ex){
   }
   return 0;
}
//---------------------------------------------------------------------------
unsigned long STDCALL Revision(int number)
{
   try{
      return clients[number]->client.Revision();
   }
   catch(EAVCameraUnknown& ex){
      clients[number]->error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info());
   }
   catch(Clients::Exception& ex){
   }
   return 0;
}
//---------------------------------------------------------------------------
unsigned long STDCALL Model(int number)
{
   try{
      return clients[number]->client.Model();
   }
   catch(EAVCameraUnknown& ex){
      clients[number]->error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info());
   }
   catch(Clients::Exception& ex){
   }
   return 0;
}
//---------------------------------------------------------------------------
unsigned long STDCALL Mini(int number)
{
   try{
      return clients[number]->client.Mini();
   }
   catch(EAVCameraUnknown& ex){
      clients[number]->error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info());
   }
   catch(Clients::Exception& ex){
   }
   return 0;
}
//---------------------------------------------------------------------------
void STDCALL SetClientBuffer(int number, char* buff, unsigned long capacity)
{
   try{
      clients[number]->memory.Buffer(reinterpret_cast<char*>(buff), capacity);
   }
   catch(Clients::Exception& ex){
   }
}
//---------------------------------------------------------------------------
int STDCALL IrisPresent(int number)
{
   int ret = 0;
   try{
      clients[number]->client.Register(127, 3);  //page 3
      ret = clients[number]->client.Register(0x50) & 2;
   }
   catch(EAVCameraUnknown& ex){
      clients[number]->error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info());
   }
   catch(Clients::Exception& ex){
   }
   return ret;
}
//---------------------------------------------------------------------------
int STDCALL GetIrisStatus(int number)
{
   try{
      return clients[number]->client.IrisStatus();
   }
   catch(EAVCameraUnknown& ex){
      clients[number]->error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info());
   }
   catch(Clients::Exception& ex){
   }
   return 0;
}
//---------------------------------------------------------------------------
unsigned long STDCALL DayNight(int number)
{
   unsigned long ret = 0;
   try{
      ret = clients[number]->client.DayNight();
   }
   catch(EAVCameraUnknown& ex){
      clients[number]->error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info());
   }
   catch(Clients::Exception& ex){
   }
   return ret;
}
//---------------------------------------------------------------------------
int STDCALL CheckCamera(const char* ip)
{
   try{
      AV2000Client client(-1);
      client.Host(ip);
      client.UpdateCamera();
      return 1;
   }
   catch(...) {
   }
   return 0;
}
//---------------------------------------------------------------------------
int STDCALL SetCustomMode(int number, long* pknee_point, long* pmax_analog_gain, long* pmax_knee_gain, long* pmax_exposure_time, long* pmax_digital_gain)
{
   try {
      if(!pknee_point || !pmax_analog_gain || !pmax_knee_gain || !pmax_exposure_time || !pmax_digital_gain)
         throw EAVException("zero pointer");
      clients[number]->client.SetCustomMode(*pknee_point, *pmax_analog_gain, *pmax_knee_gain, *pmax_exposure_time, *pmax_digital_gain);
      return 1;
   }
   catch(EStreamNoMemory& ex){ clients[number]->error.SetInfo(EAV::NO_ALLOCATE_MEMORY, ex.what(), ex.get_info()); }
   catch(GIdEAddress& ex){ clients[number]->error.SetInfo(EAV::BAD_SOCK_ADDRESS, ex.what(), ex.get_info()); }
   catch(GIdETimeout& ex){ clients[number]->error.SetInfo(EAV::TIMEOUT_ON_SOCK, ex.what(), ex.get_info()); }
   catch(GIdEReadString& ex){ clients[number]->error.SetInfo(EAV::READ_STRING_FROM_SOCK, ex.what(), ex.get_info()); }
   catch(GIdTFTPException& ex){ clients[number]->error.SetInfo(EAV::TFTP_PROTOCOL, ex.what(), ex.get_info()); }
   catch(GIdException& ex){ clients[number]->error.SetInfo(ex.get_code(), ex.what(), ex.get_info()); }
   catch(EAVParameterOutOfRange& ex){ clients[number]->error.SetInfo(EAV::CAMERA_PARAMATER_OUT_OF_RANGE, ex.what(), ex.get_info()); }
   catch(EAVCameraUnknown& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info()); }
   catch(EAVNotSupported& ex){ clients[number]->error.SetInfo(EAV::PARAMETER_NOT_SUPPORTED, ex.what(), ex.get_info()); }
   catch(EAVParameterUnknown& ex){ clients[number]->error.SetInfo(EAV::VALUE_OF_PARAMETER_UNKNOWN, ex.what(), ex.get_info()); }
   catch(EAVException& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_ERROR, ex.what(), ex.get_info()); }
   catch(Clients::Exception& ex){
   }
   catch(std::exception& ex) { clients[number]->error.SetInfo(-1, ex.what(), 0); }
   catch(...) { clients[number]->error.SetInfo(-1, "unknown", 0); }
   return 0;
}
//---------------------------------------------------------------------------
int STDCALL GetCustomMode(int number, long* pknee_point, long* pmax_analog_gain, long* pmax_knee_gain, long* pmax_exposure_time, long* pmax_digital_gain)
{
   try {
      if(!pknee_point || !pmax_analog_gain || !pmax_knee_gain || !pmax_exposure_time || !pmax_digital_gain)
         throw EAVException("zero pointer");
      clients[number]->client.GetCustomMode(*pknee_point, *pmax_analog_gain, *pmax_knee_gain, *pmax_exposure_time, *pmax_digital_gain);
      return 1;
   }
   catch(EStreamNoMemory& ex){ clients[number]->error.SetInfo(EAV::NO_ALLOCATE_MEMORY, ex.what(), ex.get_info()); }
   catch(GIdEAddress& ex){ clients[number]->error.SetInfo(EAV::BAD_SOCK_ADDRESS, ex.what(), ex.get_info()); }
   catch(GIdETimeout& ex){ clients[number]->error.SetInfo(EAV::TIMEOUT_ON_SOCK, ex.what(), ex.get_info()); }
   catch(GIdEReadString& ex){ clients[number]->error.SetInfo(EAV::READ_STRING_FROM_SOCK, ex.what(), ex.get_info()); }
   catch(GIdTFTPException& ex){ clients[number]->error.SetInfo(EAV::TFTP_PROTOCOL, ex.what(), ex.get_info()); }
   catch(GIdException& ex){ clients[number]->error.SetInfo(ex.get_code(), ex.what(), ex.get_info()); }
   catch(EAVParameterOutOfRange& ex){ clients[number]->error.SetInfo(EAV::CAMERA_PARAMATER_OUT_OF_RANGE, ex.what(), ex.get_info()); }
   catch(EAVCameraUnknown& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info()); }
   catch(EAVNotSupported& ex){ clients[number]->error.SetInfo(EAV::PARAMETER_NOT_SUPPORTED, ex.what(), ex.get_info()); }
   catch(EAVParameterUnknown& ex){ clients[number]->error.SetInfo(EAV::VALUE_OF_PARAMETER_UNKNOWN, ex.what(), ex.get_info()); }
   catch(EAVException& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_ERROR, ex.what(), ex.get_info()); }
   catch(Clients::Exception& ex){
   }
   catch(std::exception& ex) { clients[number]->error.SetInfo(-1, ex.what(), 0); }
   catch(...) { clients[number]->error.SetInfo(-1, "unknown", 0); }
   return 0;
}
//---------------------------------------------------------------------------
int STDCALL IsBlackWhite(int number, int* pblack_white)
{
   try {
      if(!pblack_white)
         throw EAVException("zero pointer");
      *pblack_white = clients[number]->client.BlackWhite();
      return 1;
   }
   catch(EAVCameraUnknown& ex){
      clients[number]->error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info());
   }
   catch(Clients::Exception& ex){
   }
   return 0;
}
 //---------------------------------------------------------------------------
int STDCALL GetMac(AV2000Addr* addr)
{
   if(addr)
      try{
         AV2000Client client(-1);
         GExternalAllocateStream memory;
         client.BufferImage(&memory);

#ifdef _linux_
         client.Host(ToString(addr->ip[0]) + '.' + ToString(addr->ip[1]) + '.' + ToString(addr->ip[2]) + '.' + ToString(addr->ip[3]));
#endif

#ifdef _windows_
         client.Host(ToString<GIdString>(addr->ip[0]) + '.' + ToString<GIdString>(addr->ip[1]) + '.' + ToString<GIdString>(addr->ip[2]) + '.' + ToString<GIdString>(addr->ip[3]));
#endif

         client.UpdateCamera();
         client.Resolution(imHALF);
         client.Image(0, 0, 64, 64, JPEG_CODEC, 0, NULL, 0, 0);
         Copy(client.Mac(), addr->mac, 6);
         return 1;
      }
      catch(...){
      }
   return 0;
}
//---------------------------------------------------------------------------
const unsigned char* STDCALL GetMotionArray(int number)
{
   try {
      return clients[number]->client.MotionArray();
   }
   catch(EStreamNoMemory& ex){ clients[number]->error.SetInfo(EAV::NO_ALLOCATE_MEMORY, ex.what(), ex.get_info()); }
   catch(GIdEAddress& ex){ clients[number]->error.SetInfo(EAV::BAD_SOCK_ADDRESS, ex.what(), ex.get_info()); }
   catch(GIdETimeout& ex){ clients[number]->error.SetInfo(EAV::TIMEOUT_ON_SOCK, ex.what(), ex.get_info()); }
   catch(GIdEReadString& ex){ clients[number]->error.SetInfo(EAV::READ_STRING_FROM_SOCK, ex.what(), ex.get_info()); }
   catch(GIdTFTPException& ex){ clients[number]->error.SetInfo(EAV::TFTP_PROTOCOL, ex.what(), ex.get_info()); }
   catch(GIdException& ex){ clients[number]->error.SetInfo(ex.get_code(), ex.what(), ex.get_info()); }
   catch(EAVParameterOutOfRange& ex){ clients[number]->error.SetInfo(EAV::CAMERA_PARAMATER_OUT_OF_RANGE, ex.what(), ex.get_info()); }
   catch(EAVCameraUnknown& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info()); }
   catch(EAVNotSupported& ex){ clients[number]->error.SetInfo(EAV::PARAMETER_NOT_SUPPORTED, ex.what(), ex.get_info()); }
   catch(EAVParameterUnknown& ex){ clients[number]->error.SetInfo(EAV::VALUE_OF_PARAMETER_UNKNOWN, ex.what(), ex.get_info()); }
   catch(EAVException& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_ERROR, ex.what(), ex.get_info()); }
   catch(Clients::Exception& ex){
   }
   catch(std::exception& ex) { clients[number]->error.SetInfo(-1, ex.what(), 0); }
   catch(...) { clients[number]->error.SetInfo(-1, "unknown", 0); }
   return 0;
}
//---------------------------------------------------------------------------
const unsigned char* STDCALL GetLastPacket(int number)
{
   try {
      return clients[number]->client.LastPacket();
   }
   catch(EStreamNoMemory& ex){ clients[number]->error.SetInfo(EAV::NO_ALLOCATE_MEMORY, ex.what(), ex.get_info()); }
   catch(GIdEAddress& ex){ clients[number]->error.SetInfo(EAV::BAD_SOCK_ADDRESS, ex.what(), ex.get_info()); }
   catch(GIdETimeout& ex){ clients[number]->error.SetInfo(EAV::TIMEOUT_ON_SOCK, ex.what(), ex.get_info()); }
   catch(GIdEReadString& ex){ clients[number]->error.SetInfo(EAV::READ_STRING_FROM_SOCK, ex.what(), ex.get_info()); }
   catch(GIdTFTPException& ex){ clients[number]->error.SetInfo(EAV::TFTP_PROTOCOL, ex.what(), ex.get_info()); }
   catch(GIdException& ex){ clients[number]->error.SetInfo(ex.get_code(), ex.what(), ex.get_info()); }
   catch(EAVParameterOutOfRange& ex){ clients[number]->error.SetInfo(EAV::CAMERA_PARAMATER_OUT_OF_RANGE, ex.what(), ex.get_info()); }
   catch(EAVCameraUnknown& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info()); }
   catch(EAVNotSupported& ex){ clients[number]->error.SetInfo(EAV::PARAMETER_NOT_SUPPORTED, ex.what(), ex.get_info()); }
   catch(EAVParameterUnknown& ex){ clients[number]->error.SetInfo(EAV::VALUE_OF_PARAMETER_UNKNOWN, ex.what(), ex.get_info()); }
   catch(EAVException& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_ERROR, ex.what(), ex.get_info()); }
   catch(Clients::Exception& ex){
   }
   catch(std::exception& ex) { clients[number]->error.SetInfo(-1, ex.what(), 0); }
   catch(...) { clients[number]->error.SetInfo(-1, "unknown", 0); }
   return 0;
}
//---------------------------------------------------------------------------
const unsigned char* STDCALL GetLastRequest(int number)
{
   return (unsigned char*)clients[number]->client.request.c_str();
}
//---------------------------------------------------------------------------
const unsigned char* STDCALL GetFirmwareTimeStamp(int number)
{
   return clients[number]->client.GetFirmwareTimestamp();
}
//---------------------------------------------------------------------------
int STDCALL GetSockets(int number, void* psocks, unsigned char* psize)
{
   try {
      if(!psize){
         clients[number]->error.SetInfo(EAV::ZERO_POINTER, "zero pointer", 1);
         return 0;
      }
      clients[number]->client.GetSockets(psocks, psize);
      return 1;
   }
   catch(EStreamNoMemory& ex){ clients[number]->error.SetInfo(EAV::NO_ALLOCATE_MEMORY, ex.what(), ex.get_info()); }
   catch(EAVException& ex){ clients[number]->error.SetInfo(EAV::UNKNOWN_ERROR, ex.what(), ex.get_info()); }
   catch(Clients::Exception& ex){
   }
   catch(std::exception& ex) { clients[number]->error.SetInfo(-1, ex.what(), 0); }
   catch(...) { clients[number]->error.SetInfo(-1, "unknown", 0); }
   return 0;
}

//----------------------------------------------------------------
void Allocate(char** pbuffer, unsigned long* psize)
{
	*psize = (*psize + 256) & (~0x03);
	*pbuffer = new char[*psize];
}
//----------------------------------------------------------------
void Deallocate(char* buffer)
{
	delete[] buffer;
}
//----------------------------------------------------------------


//---------------------------------------------------------------------------
void STDCALL SetCallbackFunction(PTR_TestCallback val)
{
}
//---------------------------------------------------------------------------
void STDCALL SetAllocateFunction(PTR_Allocate val)
{
   if (val==0) val = Allocate;
   GExternalAllocateStream::DefineHandlerAlloc(val);
}
//---------------------------------------------------------------------------
void STDCALL SetDeallocateFunction(PTR_Deallocate val)
{
   if (val==0) val = Deallocate;
   GExternalAllocateStream::DefineHandlerDealloc(val);
}
//---------------------------------------------------------------------------
void STDCALL SetReinitedFunction(PTR_Reinited handler)
{
   AV2000CLIENT::handler_on_reinited = handler;
}
//---------------------------------------------------------------------------
void STDCALL DefineOnReceivePacket(PTR_ReceivePacket handler)
{
   AV2000ClientBase::DefineHandlerOnReceivePacket(handler);
}
//---------------------------------------------------------------------------
void STDCALL DefineOnSendAck(PTR_SendAck handler)
{
   AV2000ClientBase::DefineHandlerOnSendAck(handler);
}
//---------------------------------------------------------------------------
void STDCALL DefineInfoSetRegister(PTR_InfoSetRegister ptr)
{
   AV2000ClientBase::DefineInfoSetRegister(ptr);
}
//---------------------------------------------------------------------------
void export_dll_function STDCALL DefineInfoGetRegister(PTR_InfoGetRegister ptr)
{
   AV2000ClientBase::DefineInfoGetRegister(ptr);
}
//---------------------------------------------------------------------------

int STDCALL FindCameras(unsigned* pattempts, unsigned timeout)
{
	try{
      if(!pattempts){
         set.error.SetInfo(EAV::ZERO_POINTER, "Zero pointer", 1);
         return 0;
      }
      unsigned int attempts = *pattempts, ret = 0;
      ret = set.Find(attempts, timeout);
      *pattempts = ret;
	//std::cout << "\ntestcheck";
      return 1;
   }
   catch(EStreamNoMemory& ex){ set.error.SetInfo(EAV::NO_ALLOCATE_MEMORY, ex.what(), ex.get_info()); }
   catch(GIdEAddress& ex){ set.error.SetInfo(EAV::BAD_SOCK_ADDRESS, ex.what(), ex.get_info()); }
   catch(GIdETimeout& ex){ set.error.SetInfo(EAV::TIMEOUT_ON_SOCK, ex.what(), ex.get_info()); }
   catch(GIdEReadString& ex){ set.error.SetInfo(EAV::READ_STRING_FROM_SOCK, ex.what(), ex.get_info()); }
   catch(GIdTFTPException& ex){ set.error.SetInfo(EAV::TFTP_PROTOCOL, ex.what(), ex.get_info()); }
   catch(GIdException& ex){ set.error.SetInfo(ex.get_code(), ex.what(), ex.get_info()); }
   catch(EAVParameterOutOfRange& ex){ set.error.SetInfo(EAV::CAMERA_PARAMATER_OUT_OF_RANGE, ex.what(), ex.get_info()); }
   catch(EAVCameraUnknown& ex){ set.error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info()); }
   catch(EAVNotSupported& ex){ set.error.SetInfo(EAV::PARAMETER_NOT_SUPPORTED, ex.what(), ex.get_info()); }
   catch(EAVParameterUnknown& ex){ set.error.SetInfo(EAV::VALUE_OF_PARAMETER_UNKNOWN, ex.what(), ex.get_info()); }
   catch(EAVException& ex){ set.error.SetInfo(EAV::UNKNOWN_ERROR, ex.what(), ex.get_info()); }
   catch(std::exception& ex) { set.error.SetInfo(-1, ex.what(), 0); }
   catch(...) { set.error.SetInfo(-1, "unknown", 0); }

   return 0;
}
//---------------------------------------------------------------------------
int STDCALL GetCameras(AV2000Addr* paddrs, unsigned *psize)
{
   try{
      if(!paddrs || !psize){
         set.error.SetInfo(EAV::ZERO_POINTER, "Zero pointer", 1);
         return 0;
      }
      SetCamera::CAMERAS cameras = set.Cameras();
      unsigned i = 0;
      for(SetCamera::CAMERAS::iterator iter = cameras.begin(); iter != cameras.end() && i < *psize; iter++, i++)
         paddrs[i] = *iter;
      *psize = i;
      return 1;
   }
   catch(EStreamNoMemory& ex){ set.error.SetInfo(EAV::NO_ALLOCATE_MEMORY, ex.what(), ex.get_info()); }
   catch(GIdEAddress& ex){ set.error.SetInfo(EAV::BAD_SOCK_ADDRESS, ex.what(), ex.get_info()); }
   catch(GIdETimeout& ex){ set.error.SetInfo(EAV::TIMEOUT_ON_SOCK, ex.what(), ex.get_info()); }
   catch(GIdEReadString& ex){ set.error.SetInfo(EAV::READ_STRING_FROM_SOCK, ex.what(), ex.get_info()); }
   catch(GIdTFTPException& ex){ set.error.SetInfo(EAV::TFTP_PROTOCOL, ex.what(), ex.get_info()); }
   catch(GIdException& ex){ set.error.SetInfo(ex.get_code(), ex.what(), ex.get_info()); }
   catch(EAVParameterOutOfRange& ex){ set.error.SetInfo(EAV::CAMERA_PARAMATER_OUT_OF_RANGE, ex.what(), ex.get_info()); }
   catch(EAVCameraUnknown& ex){ set.error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info()); }
   catch(EAVNotSupported& ex){ set.error.SetInfo(EAV::PARAMETER_NOT_SUPPORTED, ex.what(), ex.get_info()); }
   catch(EAVParameterUnknown& ex){ set.error.SetInfo(EAV::VALUE_OF_PARAMETER_UNKNOWN, ex.what(), ex.get_info()); }
   catch(EAVException& ex){ set.error.SetInfo(EAV::UNKNOWN_ERROR, ex.what(), ex.get_info()); }
   catch(std::exception& ex) { set.error.SetInfo(-1, ex.what(), 0); }
   catch(...) { set.error.SetInfo(-1, "unknown", 0); }

   return 0;
}
//---------------------------------------------------------------------------
int STDCALL SetCameraIp(AV2000Addr* paddr)
{
	try{
      if(!paddr){
         set.error.SetInfo(EAV::ZERO_POINTER, "Zero pointer", 1);
         return 0;
      }
      set.Set(*paddr);
      return 1;
   }
   catch(EStreamNoMemory& ex){ set.error.SetInfo(EAV::NO_ALLOCATE_MEMORY, ex.what(), ex.get_info()); }
   catch(GIdEAddress& ex){ set.error.SetInfo(EAV::BAD_SOCK_ADDRESS, ex.what(), ex.get_info()); }
   catch(GIdETimeout& ex){ set.error.SetInfo(EAV::TIMEOUT_ON_SOCK, ex.what(), ex.get_info()); }
   catch(GIdEReadString& ex){ set.error.SetInfo(EAV::READ_STRING_FROM_SOCK, ex.what(), ex.get_info()); }
   catch(GIdTFTPException& ex){ set.error.SetInfo(EAV::TFTP_PROTOCOL, ex.what(), ex.get_info()); }
   catch(GIdException& ex){ set.error.SetInfo(ex.get_code(), ex.what(), ex.get_info()); }
   catch(EAVParameterOutOfRange& ex){ set.error.SetInfo(EAV::CAMERA_PARAMATER_OUT_OF_RANGE, ex.what(), ex.get_info()); }
   catch(EAVCameraUnknown& ex){ set.error.SetInfo(EAV::UNKNOWN_CAMERA, ex.what(), ex.get_info()); }
   catch(EAVNotSupported& ex){ set.error.SetInfo(EAV::PARAMETER_NOT_SUPPORTED, ex.what(), ex.get_info()); }
   catch(EAVParameterUnknown& ex){ set.error.SetInfo(EAV::VALUE_OF_PARAMETER_UNKNOWN, ex.what(), ex.get_info()); }
   catch(EAVException& ex){ set.error.SetInfo(EAV::UNKNOWN_ERROR, ex.what(), ex.get_info()); }
   catch(std::exception& ex) { set.error.SetInfo(-1, ex.what(), 0); }
   catch(...) { set.error.SetInfo(-1, "unknown", 0); }

   return 0;
}
//---------------------------------------------------------------------------
const ClientError* const STDCALL GetLastSetError()
{
   return &set.error;
}
//---------------------------------------------------------------------------
