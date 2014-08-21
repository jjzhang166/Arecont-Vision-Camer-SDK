//---------------------------------------------------------------------------
#pragma hdrstop
//---------------------------------------------------------------------------
#include "AVLoadSo.h"
#include <iostream>
#include <string.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
EDll::EDll(const char* in_msg, int in_code)
   : code(in_code)
{
   int n = strlen(in_msg);
   if(n > 255)
      n = 255;
   strncpy(msg, in_msg, n);
   msg[n] = '\0';
}
//---------------------------------------------------------------------------
const char* EDll::what() const throw()
{
   return msg;
}
//---------------------------------------------------------------------------
int EDll::get_code() const
{
   return code;
}
//---------------------------------------------------------------------------
AV2000F::AV2000F()
   : hClientDll(0)
{
	pCreateClient = 0;
	pDestroyClient = 0;
   pMaxClients = 0;
	pGetLastClientError = 0;
	pSetClientIp = 0;
	pGetClientIp = 0;
	pGetClientPort = 0;
   pSetClientTimeout = 0;
   pGetClientTimeout = 0;
   pSetRegister = 0;
   pGetRegister = 0;
	pGetImage = 0;
   pGetImage2 = 0;
   pGetWindowImage = 0;
   pGetWindowImageQ = 0;
   pGetDefaultImage = 0;
	pGetImageEx = 0;
   pGetImage2Ex = 0;
   pGetWindowImageEx = 0;
   pGetWindowImageQEx = 0;
   pGetDefaultImageEx = 0;
	pSetAV2000Parameter = 0;
	pGetAV2000Parameter = 0;
   pFactoryDefault = 0;
   pPermanently = 0;
   pUpdateVersion = 0;
   pModel = 0;
   pVersion = 0;
   pRevision = 0;
   pMini = 0;
   pSetCustomMode = 0;
   pGetCustomMode = 0;

	pSetClientBuffer = 0;
	pSetAllocateFunction = 0;
	pSetDeallocateFunction = 0;
   pReinitedFunction = 0;

   pFindCameras = 0;
   pGetCameras = 0;
   pSetCameraIp = 0;
   pGetLastSetError = 0;

   pDefineInfoSetRegister = 0;
   pDefineInfoGetRegister = 0;

   pIsBlackWhite = 0;
}
//---------------------------------------------------------------------------
AV2000F::~AV2000F()
{
	dlclose(hClientDll);
}
//---------------------------------------------------------------------------
void AV2000F::Initialization()
{
   hClientDll = dlopen("./AV2000SDK.so",RTLD_LAZY);
	if(!hClientDll)
		throw EDll("\"AV2000SDK.so\" library not found or corrupted", -1);

	// load functions from dll
	pCreateClient = (PTR_CreateClient)(dlsym(hClientDll, "CreateClient"));
	if(!pCreateClient)
      throw EDll("function \"CreateClient\" not found");

	pDestroyClient = (PTR_DestroyClient)(dlsym(hClientDll, "DestroyClient"));
	if(!pDestroyClient)
      throw EDll("function \"DestroyClient\" not found");

	pMaxClients = (PTR_MaxClients)(dlsym(hClientDll, "MaxClients"));
	if(!pMaxClients)
      throw EDll("function \"_MaxClients\" not found");

	pGetLastClientError = (PTR_GetLastClientError)(dlsym(hClientDll, "GetLastClientError"));
	if(pGetLastClientError == NULL)
      throw EDll("function \"GetLastClientError\" not found");

	pSetClientIp = (PTR_SetClientIp)(dlsym(hClientDll, "SetClientIp"));
	if(pSetClientIp == NULL)
      throw EDll("function \"SetClientIp\" not found");

	pGetClientIp = (PTR_GetClientIp)(dlsym(hClientDll, "GetClientIp"));
	if(pGetClientIp == NULL)
      throw EDll("function \"GetClientIp\" not found");

	pGetClientPort = (PTR_GetClientPort)(dlsym(hClientDll, "GetClientPort"));
	if(pGetClientPort == NULL)
      throw EDll("function \"GetClientPort\" not found");

	pSetClientTimeout = (PTR_SetClientTimeout)(dlsym(hClientDll, "SetClientTimeout"));
	if(pSetClientTimeout == NULL)
      throw EDll("function \"SetClientTimeout\" not found");

	pGetClientTimeout = (PTR_GetClientTimeout)(dlsym(hClientDll, "GetClientTimeout"));
	if(pGetClientTimeout == NULL)
      throw EDll("function \"GetClientTimeout\" not found");

	pSetRegister = (PTR_SetAV2000Register)(dlsym(hClientDll, "SetAV2000Register"));
	if(!pSetRegister)
      throw EDll("function \"_SetAV2000Register\" not found");

	pGetRegister = (PTR_GetAV2000Register)(dlsym(hClientDll, "GetAV2000Register"));
	if(!pGetRegister)
      throw EDll("function \"pGetAV2000Register\" not found");

	pGetTimeStampBuffer = (PTR_GetTimeStampBuffer)(dlsym(hClientDll, "GetTimeStampBuffer"));
	if(!pGetTimeStampBuffer)
	  throw EDll("function \"pGetTimeStampBuffer\" not found");

	pGetImage = (PTR_GetImage)(dlsym(hClientDll, "GetImage"));
	if(pGetImage == NULL)
      throw EDll("function \"GetImage\" not found");

	if( !(pGetImage2 = (PTR_GetImage2)(dlsym(hClientDll, "GetImage2"))) )
      throw EDll("function \"GetImage2\" not found");

	pGetImageEx = (PTR_GetImageEx)(dlsym(hClientDll, "GetImageEx"));
	if(pGetImageEx == NULL)
      throw EDll("function \"GetImageEx\" not found");

	if( !(pGetImage2Ex = (PTR_GetImage2Ex)(dlsym(hClientDll, "GetImage2Ex"))) )
      throw EDll("function \"GetImage2Ex\" not found");

	pSetAV2000Parameter = (PTR_SetAV2000Parameter)(dlsym(hClientDll, "SetAV2000Parameter"));
	if(pSetAV2000Parameter == NULL)
      throw EDll("function \"SetAV2000Parameter\" not found");

	pGetAV2000Parameter = (PTR_GetAV2000Parameter)(dlsym(hClientDll, "GetAV2000Parameter"));
	if(pGetAV2000Parameter == NULL)
      throw EDll("function \"GetAV2000Parameter\" not found");

	pFactoryDefault = (PTR_FactoryDefault)(dlsym(hClientDll, "FactoryDefault"));
	if(pFactoryDefault == NULL)
      throw EDll("function \"FactoryDefault\" not found");

	pPermanently = (PTR_Permanently)(dlsym(hClientDll, "Permanently"));
	if(!pPermanently)
      throw EDll("function \"Permanently\" not found");

	pUpdateVersion = (PTR_UpdateVersion)(dlsym(hClientDll, "UpdateVersion"));
	if(pUpdateVersion == NULL)
      throw EDll("function \"UpdateVersion\" not found");

	pModel = (PTR_Model)(dlsym(hClientDll, "Model"));
	if(pModel == NULL)
      throw EDll("function \"Model\" not found");

	pVersion = (PTR_Version)(dlsym(hClientDll, "Version"));
	if(pVersion == NULL)
      throw EDll("function \"Version\" not found");

	pMini = (PTR_Mini)(dlsym(hClientDll, "Mini"));
	if(pMini == NULL)
      throw EDll("function \"Mini\" not found");

   if( !(pSetCustomMode = (PTR_SetCustomMode)(dlsym(hClientDll, "SetCustomMode"))) )
      throw EDll("function \"SetCustomMode\" not found");

   if( !(pGetCustomMode = (PTR_GetCustomMode)(dlsym(hClientDll, "GetCustomMode"))) )
      throw EDll("function \"GetCustomMode\" not found");

	pSetClientBuffer = (PTR_SetClientBuffer)(dlsym(hClientDll, "SetClientBuffer"));
	if(!pSetClientBuffer)
      throw EDll("function \"SetClientBuffer\" not found");

	pSetAllocateFunction = (PTR_SetAllocateFunction)(dlsym(hClientDll, "SetAllocateFunction"));
	if(!pSetAllocateFunction)
      throw EDll("function \"SetAllocateFunction\" not found");

	pSetDeallocateFunction = (PTR_SetDeallocateFunction)(dlsym(hClientDll, "SetDeallocateFunction"));
	if(!pSetDeallocateFunction)
      throw EDll("function \"SetDeallocateFunction\" not found");

	pReinitedFunction = (PTR_SetReinitedFunction)(dlsym(hClientDll, "SetReinitedFunction"));
	if(!pReinitedFunction)
      throw EDll("function \"SetReinitedFunction\" not found");

	pFindCameras = (PTR_FindCameras)(dlsym(hClientDll, "FindCameras"));
	if(!pFindCameras)
      throw EDll("function \"FindCameras\" not found");

	pGetCameras = (PTR_GetCameras)(dlsym(hClientDll, "GetCameras"));
	if(!pGetCameras)
      throw EDll("function \"GetCameras\" not found");

	pSetCameraIp = (PTR_SetCameraIp)(dlsym(hClientDll, "SetCameraIp"));
	if(!pSetCameraIp)
      throw EDll("function \"SetCameraIp\" not found");

	pGetLastSetError = (PTR_GetLastSetError)(dlsym(hClientDll, "GetLastSetError"));
	if(!pGetLastSetError)
      throw EDll("function \"GetLastSetError\" not found");

   pDefineInfoSetRegister = (PTR_DefineInfoSetRegister)(dlsym(hClientDll, "DefineInfoSetRegister"));
	if(!pDefineInfoSetRegister)
      throw EDll("function \"DefineInfoSetRegister\" not found");

   pDefineInfoGetRegister = (PTR_DefineInfoGetRegister)(dlsym(hClientDll, "DefineInfoGetRegister"));
	if(!pDefineInfoGetRegister)
      throw EDll("function \"DefineInfoGetRegister\" not found");

	if( !(pIsBlackWhite = (PTR_IsBlackWhite)(dlsym(hClientDll, "IsBlackWhite"))) )
      throw EDll("function \"IsBlackWhite\" not found");

	if( !(pGetWindowImage = (PTR_GetWindowImage)(dlsym(hClientDll, "GetWindowImage"))) )
      throw EDll("function \"GetWindowImage\" not found");

	if( !(pGetWindowImageQ = (PTR_GetWindowImageQ)(dlsym(hClientDll, "GetWindowImageQ"))) )
      throw EDll("function \"GetWindowImageQ\" not found");

	if( !(pGetMac = (PTR_GetMac)(dlsym(hClientDll, "GetMac"))) )
      throw EDll("function \"GetMac\" not found");

	if( !(pGetDefaultImage = (PTR_GetDefaultImage)(dlsym(hClientDll, "GetDefaultImage"))) )
      throw EDll("function \"GetDefaultImage\" not found");

	if( !(pGetWindowImageEx = (PTR_GetWindowImageEx)(dlsym(hClientDll, "GetWindowImageEx"))) )
      throw EDll("function \"GetWindowImageEx\" not found");

	if( !(pGetWindowImageQEx = (PTR_GetWindowImageQEx)(dlsym(hClientDll, "GetWindowImageQEx"))) )
      throw EDll("function \"GetWindowImageQEx\" not found");

	if( !(pGetDefaultImageEx = (PTR_GetDefaultImageEx)(dlsym(hClientDll, "GetDefaultImageEx"))) )
      throw EDll("function \"GetDefaultImageEx\" not found");

	if( !(pRevision = (PTR_Revision)(dlsym(hClientDll, "Revision"))) )
      throw EDll("function \"Revision\" not found");

	if( !(pGetMotionArray = (PTR_GetMotionArray)(dlsym(hClientDll, "GetMotionArray"))) )
      throw EDll("function \"GetMotionArray\" not found");

}
//---------------------------------------------------------------------------
