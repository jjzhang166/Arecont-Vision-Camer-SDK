//---------------------------------------------------------------------------
#ifndef AVLoadSoH
#define AVLoadSoH
//---------------------------------------------------------------------------
#include "AV2000Types.h"
#include <dlfcn.h>
#include <exception>
#include <string>
//---------------------------------------------------------------------------
class EDll : public std::exception{
   int code;
   char msg[256];
public:
   EDll(const char* in_message, int code = 1);
   virtual const char* what() const throw();
   int get_code() const;
};
//---------------------------------------------------------------------------
// definition dll functions
typedef int (*PTR_CreateClient)(int);
typedef void (*PTR_DestroyClient)(int);
typedef unsigned (*PTR_MaxClients)();
typedef const ClientError* const (*PTR_GetLastClientError)(int);
typedef int (*PTR_SetClientIp)(int, const char*);
typedef const char* (*PTR_GetClientIp)(int);
typedef void (*PTR_SetClientPort)(int,unsigned);
typedef unsigned (*PTR_GetClientPort)(int);
typedef void (*PTR_SetClientTimeout)(int, unsigned);
typedef unsigned (*PTR_GetClientTimeout)(int);
typedef int (*PTR_SetAV2000Register)(int, unsigned char registr, unsigned char* value);
typedef int (*PTR_GetAV2000Register)(int, unsigned char registr, unsigned char* value);
typedef int (*PTR_GetImage)(int number, char** data, unsigned long* size, IMAGE_RESOLUTION, float zoom, int dx, int dy);
typedef int (*PTR_GetImage2)(int number, char** data, unsigned long* size, unsigned long* capacity, IMAGE_RESOLUTION, float zoom, int dx, int dy);
typedef int (*PTR_GetWindowImage)(int number, char** pdata, unsigned long* size, unsigned long* capacity,  IMAGE_RESOLUTION, int left, int top, int width, int height);
typedef int (*PTR_GetDefaultImage)(int number, char** pdata, unsigned long* size, unsigned long* capacity);
//---------------------------added Feb/28/2007 by Sining---------------------------
typedef int (*PTR_GetWindowImageQ)(int number, char** pdata, unsigned long* size, unsigned long* capacity,  IMAGE_RESOLUTION resolution, long IsPercent, long Aquality, int left, int top, int width, int height);


typedef int (*PTR_GetImageEx)(int number, char** data, unsigned long* size, IMAGE_RESOLUTION, float zoom, int dx, int dy, int codec, int streamId, int* Ifarme);
typedef int (*PTR_GetImage2Ex)(int number, char** data, unsigned long* size, unsigned long* capacity, IMAGE_RESOLUTION, float zoom, int dx, int dy, int codec, int streamId, int* Ifarme);
typedef int (*PTR_GetWindowImageEx)(int number, char** pdata, unsigned long* size, unsigned long* capacity,  IMAGE_RESOLUTION, int left, int top, int width, int height, int codec, int streamId, int* Ifarme);
typedef int (*PTR_GetDefaultImageEx)(int number, char** pdata, unsigned long* size, unsigned long* capacity, int codec, int streamId, int* Ifarme);
typedef int (*PTR_GetWindowImageQEx)(int number, char** pdata, unsigned long* size, unsigned long* capacity,  IMAGE_RESOLUTION resolution, long IsPercent, long Aquality, int left, int top, int width, int height, int codec, int streamId, int* Ifarme, int kbps, int intra_frame_period);

/*//---------------------------added Feb/11/2007 by Sining---------------------------
typedef int (*PTR_TriggerSingleCapture)(int number);
typedef int (*PTR_GetSingleCapture)(int number);
typedef int (*PTR_SetSingleCapture)(int number, int enable);
typedef int (*PTR_SetCalibrateFlash)(int number);
typedef int (*PTR_GetCalibrationNumber)(int number);
*///---------------------------------------------------------------------------------

typedef int (*PTR_SetAV2000Parameter)(int, CAMERA_PARAMETER, long*);
typedef int (*PTR_GetAV2000Parameter) (int, CAMERA_PARAMETER, long*);
typedef int (*PTR_FactoryDefault)(int);
typedef int (*PTR_Permanently)(int);
typedef int (*PTR_UpdateVersion) (int);
typedef unsigned long (*PTR_Model) (int);
typedef unsigned long (*PTR_Version) (int);
typedef unsigned long (*PTR_Revision) (int);
typedef unsigned long (*PTR_Mini) (int);
typedef unsigned long (*PTR_IrisPresent) (int);
typedef int (*PTR_SetCustomMode)(int number, long* pknee_point, long* pmax_analog_gain, long* pmax_knee_gain, long* pmax_exposure_time, long* max_digital_gain);
typedef int (*PTR_GetCustomMode)(int number, long* pknee_point, long* pmax_analog_gain, long* pmax_knee_gain, long* pmax_exposure_time, long* max_digital_gain);
typedef int (*PTR_GetIrisStatus)(int);
typedef int (*PTR_CheckCamera)(const char*);
typedef int (*PTR_IsBlackWhite)(int number, int* pblack_white);
typedef int (*PTR_GetMac)(AV2000Addr*);

// new
typedef void (OnAlloc)(char**, unsigned long*);
typedef void (OnDealloc)(char*);
typedef void (OnReinited)(int);

typedef void (*PTR_SetClientBuffer)(int, char*, unsigned long);
typedef void (*PTR_SetAllocateFunction)(OnAlloc);
typedef void (*PTR_SetDeallocateFunction)(OnDealloc);
typedef void (*PTR_SetReinitedFunction)(OnReinited);
typedef void (*PTR_DefineInfoSetRegister)(PTR_InfoSetRegister);
typedef void (*PTR_DefineInfoGetRegister)(PTR_InfoGetRegister);
// added 2004.12.04
typedef int (*PTR_FindCameras)(unsigned* pattempts, unsigned timeout);
typedef int (*PTR_GetCameras)(AV2000Addr*, unsigned* psize);
typedef int (*PTR_SetCameraIp)(AV2000Addr*);
typedef const ClientError* const (*PTR_GetLastSetError)();
typedef const unsigned char* (*PTR_GetMotionArray)(int number);
typedef const unsigned char* (*PTR_GetLastPacket)(int number);
// added 2007.04.09
typedef const unsigned char* (*PTR_SetAuxIO)(int, int);
typedef const unsigned char* (*PTR_GetAuxIO)(int);
// added 2008.06.06
typedef void (*PTR_InitializeCopyRightStrings)(int,EXIF_STRING_ID,unsigned char*);
// added 2009.06.16 by Dhanashree 
typedef const unsigned char* (*PTR_GetTimeStampBuffer)(int number);
//---------------------------------------------------------------------------
struct AV2000F{
   void* hClientDll;

	PTR_CreateClient pCreateClient;
	PTR_DestroyClient pDestroyClient;
   PTR_MaxClients pMaxClients;
	PTR_GetLastClientError pGetLastClientError;
	PTR_SetClientIp pSetClientIp;
	PTR_GetClientIp pGetClientIp;
	PTR_GetClientPort pGetClientPort;
	PTR_SetClientPort pSetClientPort;
   PTR_SetClientTimeout pSetClientTimeout;
   PTR_GetClientTimeout pGetClientTimeout;
   PTR_SetAV2000Register pSetRegister;
   PTR_GetAV2000Register pGetRegister;
   /*merge future
   PTR_TriggerSingleCapture pTriggerSingleCapture;
   PTR_GetSingleCapture pGetSingleCapture;
   PTR_SetSingleCapture pSetSingleCapture;
   PTR_SetCalibrateFlash pSetCalibrateFlash;
   PTR_GetCalibrationNumber pGetCalibrationNumber;
   */
	PTR_GetImage pGetImage;
   PTR_GetImage2 pGetImage2;
   PTR_GetWindowImage pGetWindowImage;
   PTR_GetWindowImageQ pGetWindowImageQ;
   PTR_GetDefaultImage pGetDefaultImage;

  PTR_GetImageEx pGetImageEx;
  PTR_GetImage2Ex pGetImage2Ex;
  PTR_GetWindowImageEx pGetWindowImageEx;
  PTR_GetDefaultImageEx pGetDefaultImageEx;
  PTR_GetWindowImageQEx pGetWindowImageQEx;


	PTR_SetAV2000Parameter pSetAV2000Parameter;
	PTR_GetAV2000Parameter pGetAV2000Parameter;
   PTR_FactoryDefault pFactoryDefault;
   PTR_Permanently pPermanently;
   PTR_UpdateVersion pUpdateVersion;
   PTR_Model pModel;
   PTR_Version pVersion;
   PTR_Revision pRevision;
   PTR_Mini pMini;
   PTR_IrisPresent pIrisPresent;
   PTR_SetCustomMode pSetCustomMode;
   PTR_GetCustomMode pGetCustomMode;

	PTR_SetClientBuffer pSetClientBuffer;
	PTR_SetAllocateFunction pSetAllocateFunction;
	PTR_SetDeallocateFunction pSetDeallocateFunction;
   PTR_SetReinitedFunction pReinitedFunction;

   PTR_FindCameras pFindCameras;
   PTR_GetCameras  pGetCameras;
   PTR_SetCameraIp pSetCameraIp;
   PTR_GetLastSetError pGetLastSetError;

   PTR_DefineInfoSetRegister pDefineInfoSetRegister;
   PTR_DefineInfoGetRegister pDefineInfoGetRegister;

   PTR_GetIrisStatus pGetIrisStatus;
   PTR_CheckCamera pCheckCamera;
   PTR_IsBlackWhite pIsBlackWhite;
   PTR_GetMac pGetMac;
   PTR_GetMotionArray pGetMotionArray;
   PTR_GetLastPacket pGetLastPacket;
   //PTR_GetSockets pGetSockets;

   PTR_SetAuxIO pSetAuxIO;
   PTR_GetAuxIO pGetAuxIO;
   PTR_GetTimeStampBuffer pGetTimeStampBuffer;

   PTR_InitializeCopyRightStrings pInitializeCopyRightStrings;
   //------------------------------
   AV2000F();
   ~AV2000F();
   void Initialization();
};
//---------------------------------------------------------------------------
#endif
