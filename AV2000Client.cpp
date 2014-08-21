//---------------------------------------------------------------------------
/*camera version
   61315 - resolution, quality, x0, y0, x1, y1 - ???????šª???š€šº?š°? (in the last packet)

*/
#pragma hdrstop
//---------------------------------------------------------------------------
#include "AV2000Client.h"
#include <cmath>
#include <math.h>
#include "SdkUtils.h"
#include <sstream>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <iostream>

#define _linux_
#define _debug_

#ifdef _windows_
#include <Windows.h>
#endif

//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
PTR_InfoSetRegister AV2000ClientBase::info_set_register = 0;
//---------------------------------------------------------------------------
PTR_InfoGetRegister AV2000ClientBase::info_get_register = 0;
//---------------------------------------------------------------------------
PTR_ReceivePacket AV2000ClientBase::handler_receive_packet = 0;
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
PTR_SendAck AV2000ClientBase::handler_send_ack = 0;

const int max_sps_pps_len = 30; // maximum len of sps&pps header. now it's about 18
unsigned char unit_del[5] = {0x00, 0x00, 0x01, 0x09, 0x10};

extern int create_sps_pps(
				   int frameWidth,
				   int frameHeight,
				   int deblock_filter,
				   unsigned char* data, int max_datalen);

//---------------------------------------------------------------------------
unsigned char MakeString[64], ModelString[32], SoftwareString[64], CopyRightString[64];
unsigned char TimeOrignial[20], TimeDigitalized[20];

//---------------------------------------------------------------------------
void AV2000ClientBase::OnReceivePacket(unsigned packet, unsigned long size)
{
   if(handler_receive_packet)
      handler_receive_packet(number, packet, size);
}
//---------------------------------------------------------------------------
void AV2000ClientBase::OnSendAck(unsigned ack)
{
   if(handler_send_ack)
      handler_send_ack(number, ack);
}
//---------------------------------------------------------------------------
AV2000ClientBase::AV2000ClientBase(int n, unsigned clients)
   : number(n), client(0), type_of_registers_proc_old(true), curr_page(0), requested_block_size(1450)
{
        MakeString[0] = 0;
        ModelString[0] = 0;
        SoftwareString[0] = 0;
        CopyRightString[0] = 0;

	client = new Client(*this, clients);
	for(unsigned i = 0; i < client->Size(); i++){
		(*client)[i]->DefineHadlerOnBefore(&AV2000ClientBase::OnBeforeClientOperation);
		(*client)[i]->DefineHadlerOnAfterConnect(&AV2000ClientBase::OnAfterClientConnect);
		(*client)[i]->DefineHandlerOnWork(&AV2000ClientBase::OnClientWork);
		(*client)[i]->DefineHadlerOnEnd(&AV2000ClientBase::OnEndClientOperation);
                #ifdef _windows_ 
                (*client)[i]->DefineHandlerOnReceivePacket(&AV2000ClientBase::OnReceivePacket);
                #endif 
	}

#ifdef _TEST_INTERNAL_REGISTER // set registers as multisensor camera
   registers[3][122] = 8360;
   registers[3][123] = 61333;
   registers[3][0x76] = 5;
#endif

}
//---------------------------------------------------------------------------
AV2000ClientBase::~AV2000ClientBase()
{
	delete client;
}
//---------------------------------------------------------------------------
void AV2000ClientBase::Host(const GString& h)
{
   if(Host() != h){
      client->RequestedBlockSize(requested_block_size);
	   client->Host(h);
   }
}
//---------------------------------------------------------------------------
const char* AV2000ClientBase::Host() const
{
	return client->Host();
}
//---------------------------------------------------------------------------
void AV2000ClientBase::Port(unsigned p)
{
	client->Port(p);
}
//---------------------------------------------------------------------------
unsigned AV2000ClientBase::Port()
{
	return client->Port();
}
//---------------------------------------------------------------------------
void AV2000ClientBase::Register(GByte r, unsigned value)
{
	if(value > 65535)
		throw EAVParameterOutOfRange("register value must be less then 65536");
   if(127 == r)
      curr_page = value;

   if(curr_page == 3 && r == 0x4E && value == 0) value = 1; //added by sining

#ifdef _TEST_INTERNAL_REGISTER
   registers[curr_page][r] = value;

#else
   char registr = r;
   GExternalAllocateStream mem;
   mem.Resize(256);
   if(type_of_registers_proc_old){
	   mem.Write(&registr, 1);
	   GByte byte = value / 256;
	   mem.Write(reinterpret_cast<char*>(&byte), 1);
	   byte = value - static_cast<unsigned>(byte) * 256;
	   mem.Write(reinterpret_cast<char*>(&byte), 1);
	   mem.Position(0);
	   client->Put(mem, "reg_set");
   }
   else
#ifdef _windows_
      if(127 != r)
         client->Get(static_cast<GString>("setreg?page=") + ToString<GString>(curr_page) + ";reg=" + ToString<GString>(r) + ";val=" + ToString<GString>(value) + ';', mem);

#else
      if(127 != r)
         client->Get(("setreg?page=") + ToString(curr_page) + ";reg=" + ToString(r) + ";val=" + ToString(value) + ';', mem);
#endif

#endif

   if(info_set_register)
      info_set_register(number, r, value);
}
//---------------------------------------------------------------------------
unsigned AV2000ClientBase::Register(GByte r)
{
   unsigned value;

#ifdef _TEST_INTERNAL_REGISTER
   value = registers[curr_page][r];

#else
   char registr = r;
   GExternalAllocateStream mem;
   GByte byte;
   mem.Resize(256);
   if(type_of_registers_proc_old){
	   mem.Write(&registr, 1);
	   mem.Position(0);
	   client->Put(mem, "reg_sset");
	   mem.Size(0);
	   client->Get("reg_get", mem);
   }
   else
#ifdef _windows_
      client->Get(static_cast<GString>("getreg?page=") + ToString<GString>(curr_page) + ";reg=" + ToString<GString>(r) + ';', mem);
#endif

#ifdef _linux_
      client->Get(("getreg?page=") + ToString(curr_page) + ";reg=" + ToString(r) + ';', mem);
#endif

   mem.Position(0);
   mem.Read(reinterpret_cast<char*>(&byte), 1);
   value = byte * 256;
   mem.Read(reinterpret_cast<char*>(&byte), 1);
   value += byte;

#endif

   if(info_get_register)
      info_get_register(number, r, value);

	return value;
}
//---------------------------------------------------------------------------
void AV2000ClientBase::InitializeCopyRightStrings(EXIF_STRING_ID id,unsigned char*pbuf)
{
   int i=0;
   switch(id){
   case CAMERA_MAKE_STRING :
        for(i=0; i<64; i++)
           if(pbuf[i])
              MakeString[i] = pbuf[i];
           else{
              MakeString[i] = 0;
              return;
           }
        break;
   case CAMERA_MODEL_STRING:
        for(i=0; i<32; i++)
           if(pbuf[i])
              ModelString[i] = pbuf[i];
           else{
              ModelString[i] = 0;
              return;
           }
        break;
   case SOFTWARE_STRING:
        for(i=0; i<64; i++)
           if(pbuf[i])
              SoftwareString[i] = pbuf[i];
           else{
              SoftwareString[i] = 0;
              return;
           }
        break;
   case COPYRIGHT_STRING:
        for(i=0; i<64; i++)
           if(pbuf[i])
              CopyRightString[i] = pbuf[i];
           else{
              CopyRightString[i] = 0;
              return;
           }
        break;
   }
}
//---------------------------------------------------------------------------
void AV2000ClientBase::Page(unsigned page)
{
   Register(127, curr_page = page);
}
//---------------------------------------------------------------------------
void AV2000ClientBase::ReceiveTimeout(unsigned v)
{
	client->ReceiveTimeout(v);
}
//---------------------------------------------------------------------------
unsigned AV2000ClientBase::ReceiveTimeout() const
{
	return client->ReceiveTimeout();
}
//---------------------------------------------------------------------------
void AV2000ClientBase::GetSockets(void* psocks, unsigned char* psize)
{
   if(*psize < client->Size() || !psocks){
      *psize = client->Size();
      throw EStreamNoMemory();
   }

   *psize = client->Size();

   for(unsigned i = 0; i < client->Size(); i++)
      *(static_cast<SOCKET*>(psocks) + i) = (*client)[i]->Socket();
}
//---------------------------------------------------------------------------
void AV2000ClientBase::DefineInfoSetRegister(PTR_InfoSetRegister ptr)
{
   info_set_register = ptr;
}
//---------------------------------------------------------------------------
void AV2000ClientBase::DefineInfoGetRegister(PTR_InfoGetRegister ptr)
{
   info_get_register = ptr;
}
//---------------------------------------------------------------------------
void AV2000ClientBase::DefineHandlerOnReceivePacket(PTR_ReceivePacket ptr)
{
   handler_receive_packet = ptr;
}
//---------------------------------------------------------------------------
void AV2000ClientBase::DefineHandlerOnSendAck(PTR_SendAck ptr)
{
   handler_send_ack = ptr;
}
//---------------------------------------------------------------------------


unsigned AV2000ClientInherit1::FRAME5TH_1300 = 0x58;
//---------------------------------------------------------------------------
AV2000ClientInherit1::LightingConformity AV2000ClientInherit1::lighting_conformity;
//---------------------------------------------------------------------------
AV2000ClientInherit1::LightingCoefs AV2000ClientInherit1::lighting_coefs;
//---------------------------------------------------------------------------
AV2000ClientInherit1::Available AV2000ClientInherit1::available;
//---------------------------------------------------------------------------
AV2000ClientInherit1::SensorSizeMax AV2000ClientInherit1::sensor_size_max;
//---------------------------------------------------------------------------
AV2000ClientInherit1::AV2000ClientInherit1(int n)
   : AV2000ClientBase(n), pcamera(0), request_width(0), request_height(0), request_resolution(0), request_quality(0),
      number_of_sensor(0), multisensor_resolution(imFULL), multisensor_is_zoomed(0), short_exposures(5), lightingfix(0)
{
   ZeroMemory(&sensor_geometry, sizeof(sensor_geometry));
   pcamera = new Camera;
}
//---------------------------------------------------------------------------
AV2000ClientInherit1::~AV2000ClientInherit1()
{
   delete pcamera;
}
//---------------------------------------------------------------------------
AV2000Client::Default AV2000Client::deflt;
//---------------------------------------------------------------------------
AV2000Client::Camera AV2000ClientInherit1::UpdateCamera()
{
   pcamera->version = 63333;
   pcamera->model = 2000;
   pcamera->mini = 0;
   pcamera->dn = 0;
   pcamera->ai = 0;
   int iAttempModel = 8360;

   type_of_registers_proc_old = false;
	
   client->RequestedBlockSize(requested_block_size = 1450);
   Camera prev = *pcamera;

   try
   {
	
      Page(3);
	pcamera->model = Register(122);
	
      if(pcamera->model == 8360 || pcamera->model == AV8365)
      {
         iAttempModel = Register(98);
         if(iAttempModel == 8180 || iAttempModel == AV8185)
            pcamera->model = iAttempModel;
      }
   }
   catch(GIdException&){
      //type_of_registers_proc_old = true;
   }
/*

      catch(std::exception& ex){
     FILE *fp = fopen("report.txt","a+");
     fprintf(fp,"%s\n",ex.what());
     fclose(fp);
   }

   int test_network = Register(pcamera->model);
*/   

   if(type_of_registers_proc_old){
	   Page(3);
      pcamera->model = Register(122);
   }

   Page(0);
   switch(Register(6)){
   case 725 :
      client->RequestedBlockSize(requested_block_size = 2904);
      break;
   default :
      client->RequestedBlockSize(requested_block_size = 1450);
   }

   pcamera->ispanoramic = (8360 == pcamera->model || 8180 == pcamera->model || AV8365 == pcamera->model || AV8185 == pcamera->model);
   pcamera->isAV818X = (8180 == pcamera->model || AV8185 == pcamera->model);
   pcamera->isregular = (pcamera->model == 1300 || pcamera->model == 2100 || pcamera->model == 3100 || pcamera->model == 5100);
   pcamera->isH264 = (pcamera->model % 10) == 5;
   pcamera->isdome = (pcamera->model % 100) == 55;
   pcamera->isdualsensor = (pcamera->model == AV3130 || pcamera->model == AV3135);

   if(pcamera->model != 2100 &&
        pcamera->model != 3100 &&
        pcamera->model != 3130 &&
        pcamera->model != AV3135 &&
        pcamera->model != 1300 &&
        pcamera->model != 5100 &&
        pcamera->model != 8360 &&
        pcamera->model != 8180 &&
        pcamera->model != AV8365 &&
        pcamera->model != AV8185 &&
        pcamera->model != AV1305 &&
        pcamera->model != AV2105 &&
        pcamera->model != AV3105 &&
        pcamera->model != AV5105 )
   {
      pcamera->model = 2000;
      //MessageBox(0, "Invalid model number", "Error", 0);
   }

   Page(3);
   pcamera->version = Register(123);
   if(!pcamera->ispanoramic)
   {
       pcamera->mini = Register(0x1A) == 0xAAAA;
       pcamera->dn = (Register(0x42) == 222) & 0x01;
       if(pcamera->dn == 0)
       {
          pcamera->ai = Register(0x50) & 2;
       }
   }

   if(!pcamera->version) // correct bug if camera answer that his version is 0
      switch(pcamera->model){
      case 2000 :
      case 2100 :
      case AV2105:
      case 3100 :
      case AV3105:
      case 1300 :
      case AV1305:
         pcamera->version = 50412;
         break;
      case 3130 :
      case AV3135 :
         pcamera->version = 52012;
         break;
      case 8180 :
      case 8360 :
      case AV8185 :
      case AV8365 :

         pcamera->version = 62410;
         break;
   }

   if(pcamera->version >= 52108){
      pcamera->revision = Register(0x76);
      //pcamera->revision = 5;
   }

   if( (pcamera->isdualsensor || pcamera->ispanoramic)
        || (pcamera->version >= 51617 &&
                        (
                        pcamera->isregular  ||
                        AV1305 == pcamera->model ||
                        AV2105 == pcamera->model ||
                        AV3105 == pcamera->model ||
                        AV5105 == pcamera->model
                        )) ){
      pcamera->proc_fpga = Register(0x78);
      pcamera->net_fpga = Register(0x79);
   }
   else
      pcamera->proc_fpga = pcamera->net_fpga = 0;


   if(prev != *pcamera){
      DoChangeCamera();
   }

   if(!pcamera->ispanoramic)
   {
      UpdateSensorGeometry();
      if(available(cpREQUEST_WIDTH, *pcamera)){
         request_width = RequestWidth();
         request_height = RequestHeight();
      }
      if(available(cpQUALITY, *pcamera))
         request_quality = RequestQuality();
      if(available(cpRESOLUTION, *pcamera))
         request_resolution = RequestResolution();
   }


   try{
      clighting = Lighting();
   }
   catch(EAVParameterUnknown&){
      clighting = 0;
   }
   /*
   if(!clighting)
      Lighting(clighting = 60);
   */

   if(pcamera->version >= 61705){
      TimeoutOnCamera(200);
      RepeatFromCamera(3);
   }

   pc_camera_make[0] = 0;
   if(pcamera->version >= 64512)
      for(int i=0; i<5; i++){
         pc_camera_make[i] = Register(0xAB+i);
         pc_camera_make[i+1] = 0;
      }

   return *pcamera;
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::UpdateSensorGeometry()
{
   try{
      sensor_geometry.left = SensorLeft();
   }
   catch(EAVNotSupported&){
      sensor_geometry.left = 0;
   }

   try{
      sensor_geometry.top = SensorTop();
   }
   catch(EAVNotSupported&){
      sensor_geometry.top = 0;
   }

   try{
      sensor_geometry.width = SensorWidth();
   }
   catch(EAVNotSupported&){
      sensor_geometry.width = sensor_size_max.Width(*pcamera);
   }

   try{
      sensor_geometry.height = SensorHeight();
   }
   catch(EAVNotSupported&){
      sensor_geometry.height = sensor_size_max.Height(*pcamera);
   }

   try{
      sensor_geometry.left_black_white = SensorBlackWhiteLeft();
      sensor_geometry.top_black_white = SensorBlackWhiteTop();
      sensor_geometry.width_black_white = SensorBlackWhiteWidth();
      sensor_geometry.height_black_white = SensorBlackWhiteHeight();
   }
   catch(EAVNotSupported&){
      sensor_geometry.left_black_white = sensor_geometry.top_black_white = sensor_geometry.width_black_white = sensor_geometry.height_black_white = 0;
   }
}
//---------------------------------------------------------------------------
unsigned AV2000ClientInherit1::MsSensorAsBits(unsigned sensor)
{
   switch(sensor){
   case 0 :
      return 1;
   case 1 :
      return 2;
   case 2 :
      return 4;
   case 3 :
      return 8;
   }
   throw EAVParameterUnknown("number of sensor unknown");
}
//---------------------------------------------------------------------------



AV2000Client::Camera AV2000ClientInherit1::IdentOfCamera() const
{
   return *pcamera;
}
//---------------------------------------------------------------------------
unsigned long AV2000ClientInherit1::Version() const
{
	if(!pcamera->version)
		throw EAVCameraUnknown("Version of camera is unknown");
	return pcamera->version;
}
//---------------------------------------------------------------------------
unsigned long AV2000ClientInherit1::Model() const
{
   if(!pcamera->version)
      throw EAVCameraUnknown("type of camera is unknown");
   return pcamera->model;
}
//---------------------------------------------------------------------------
unsigned long AV2000ClientInherit1::Revision() const
{
	return pcamera->revision;
}
//---------------------------------------------------------------------------
unsigned long AV2000ClientInherit1::Mini() const
{
	return pcamera->mini;
}
//---------------------------------------------------------------------------
unsigned long AV2000ClientInherit1::DayNight() const
{
	return pcamera->dn;
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::Permanently()
{
   Page(3);
   Register(112, 123);
   Register(113, 0);
}
//---------------------------------------------------------------------------
//input: 100/gamma (gamma in [1.0, 2.5])
//---------------------------------------------------------------------------
void AV2000ClientInherit1::Gamma(long val)
{
   double gamma[11];
   int x[11] = {0, 4, 8, 16, 32, 64, 96, 128, 160, 192, 224};

   if(val < 1 || val > 100)
      throw EAVParameterOutOfRange("gamma value is out of range [1,100]");

   for(int i=10; i>=4; i--)
      gamma[i] = 255*pow((x[i]-8)/216.0, val/100.0) + 0.5;
   for(int i=3; i>=0; i--)
      gamma[i] = x[i] * gamma[4] /32.0 + 0.5;

   if(pcamera->ispanoramic)
   {
     Page(6);
     for(int i=10; i>=0; i--)
        Register(0x69+i, (int)gamma[i]);
     Register(0x68, 1);

   }
   else
   {
     Page(3);
     for(int i=10; i>=0; i--)
        Register(0xA0+i, (int)gamma[i]);
     Register(0x9F, 1);
   }
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::Brightness(long val)
{
	if(val >= -99 && val <= 99){
		Page(3);
      if((pcamera->ispanoramic))
         Register(0x84 + number_of_sensor, val + 100);
      else
		   Register(2, val + 100);
	}
	else
		throw EAVParameterOutOfRange("value is incorrect");
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::AutoExposition(long val)
{
   Page(3);
   if((pcamera->ispanoramic))
      Register(number_of_sensor + 1, val ? 0xF : 0x6);
   else{
      unsigned r_h1 = Register(1);
      if(val)
         r_h1 |= 9U;
      else
         r_h1 &= ~9U;
      Register(1, r_h1);
   }
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::Sharpness(long val)
{
	if(val >= 0 && val <= 4){
      if((pcamera->ispanoramic))
      {
         Page(3);
         Register(0x2D + number_of_sensor, val);
      }
      else
         if(Version() < 50412){
		      Page(1);
		      Register(2, val + 112);
         }
         else{
            Page(3);
            Register(0x10, val);
         }
	}
	else
		throw EAVParameterOutOfRange("value is incorrect");
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::Saturation(long val)
{
      if(!pcamera->ispanoramic && Version() < 50412)
      {
	      switch(val){
         case 0 : val = 117; break;
         case 25 : val = 101; break;
         case 37 : val = 93; break;
         case 50 : val = 85; break;
         case 75 : val = 77; break;
         case 100 : val = 69; break;
         case 125 : val = 108; break;
         default : throw EAVParameterOutOfRange("value is incorrect");
	      }
	      Page(2);
	      Register(15, val);
      }
      else{
	      switch(val){
         case 0 : val = 0; break;
         case 25 : val = 1; break;
         case 37 : val = 2; break;
         case 50 : val = 3; break;
         case 75 : val = 4; break;
         case 100 : val = 5; break;
         case 125 : val = 6; break;
         default : throw EAVParameterOutOfRange("value is incorrect");
	      }
         Page(3);
         Register( (pcamera->ispanoramic ? 0x29 + number_of_sensor : 0x0F), val);
      }
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::Blue(long val)
{
  if(val >= -10 && val <= 10)
  {
        val += 128;
        Page(3);
        pcamera->ispanoramic ? Register(0x51 + number_of_sensor, val) : Register(69, val);
  }
  else
        throw EAVParameterOutOfRange("value is incorrect");
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::Red(long val)
{
  if(val >= -10 && val <= 10)
  {
        val += 128;
        Page(3);
        pcamera->ispanoramic ? Register(0x4D + number_of_sensor, val) : Register(68, val);
  }
  else
        throw EAVParameterOutOfRange("value is incorrect");
}
//---------------------------------------------------------------------------
//Comment:
// auto, indoor, outddor, mixed
//---------------------------------------------------------------------------
void AV2000ClientInherit1::Illumination(long val)
{
  if(val >= 0 && val <= 3)
  {
        Page(3);
        pcamera->ispanoramic? Register(0x35 + number_of_sensor, val) : Register(38, val);
  }
  else
        throw EAVParameterOutOfRange("value is incorrect");
}

//---------------------------------------------------------------------------
void AV2000ClientInherit1::Lightingfix(long val)
{
   if(5100 == pcamera->model || AV5105 == pcamera->model)
   {
      lightingfix = val;
   }
}

//---------------------------------------------------------------------------
//Caution:
// old firmware has a disgusting bug which alters short_exposure value
// when frame5th value is changed. the purpose of this behavior is to
// accommodate 50/60 Hz change but it really shoudn't be done this way
// To handle this bug, LGTNG_SHEX register must be reset after FRAME5TH
// is changed, otherwise you will see LGTNG_SHEX changed rediculously
//---------------------------------------------------------------------------
//Firmware definitions
//---------------------------------------------------------------------------
//Regular model:
//#define  LGTNG_SHEX	  0x25
// high byte: lighting frequency (50/60 Hz); low byte: short exposures time (1ms..10ms)
//---------------------------------------------------------------------------
// Panoramic model:
// 4 values high byte: lighting frequency (50/60 Hz); low byte: short exposures time (1ms..10ms)
//#define  LGTNG_SHEX	  0xF1
//---------------------------------------------------------------------------
//(*this)[1][model][4] = std::make_pair(458333, 332);
//---------------------------------------------------------------------------
void AV2000ClientInherit1::Lighting(long val)
{
   if(val != 50 && val != 60)
      throw EAVParameterOutOfRange("lighting must be 50 or 60");

   long lighting = 50 != val,
        low_light_mode = LowLightMode(),
        reg = 4;
   Page(3);
   if(lighting_coefs.IsExists(lighting, pcamera->model)){
      long sensor_width;
      for(std::map<GByte, std::pair<long, long> >::iterator iter = lighting_coefs[lighting][pcamera->model].begin(); iter != lighting_coefs[lighting][pcamera->model].end(); iter++)
      {
         (3130 == pcamera->model || AV3135 == pcamera->model) && FRAME5TH_1300 == iter->first ? sensor_width = sensor_geometry.width_black_white : sensor_width = sensor_geometry.width;

         if(HIGH_SPEED == low_light_mode)
         {
            double low_light_val = 0;
            int expsoure_time = 0;
            //comment: iter->first is register number
            //iter->second is constant pair
            if((pcamera->ispanoramic))
            {
               if(1)
               {
                  unsigned int lght_shex = Register(0xF1 + number_of_sensor);
                  expsoure_time = lght_shex & 0xFF;
                  long curr =  (val << 8) + (expsoure_time);
                  low_light_val = lighting_coefs(iter->second, sensor_width, lightingfix) * (expsoure_time) / 10.0 + 0.5;
                  Register( iter->first+number_of_sensor, static_cast<int>(low_light_val) );
                  Register( 0xF1+number_of_sensor, curr);
               }
            }
            else
            {
                  unsigned int lght_shex = Register(0x25);
                  expsoure_time = lght_shex & 0xFF;
                  long curr =  (val << 8) + (expsoure_time);
                  low_light_val = (double)lighting_coefs(iter->second, sensor_width, lightingfix) * (expsoure_time) / 10.0 + 0.5;
                  Register( iter->first, static_cast<int>(low_light_val) );
                  Register( 0x25, curr);
            }
         }
         else
         {
            double low_light_val = lighting_coefs(iter->second, sensor_width, lightingfix);
            if((pcamera->ispanoramic))
            {
               if(1)
               {
                  long curr = Register( 0xF1+number_of_sensor) & 0xFF;
                  curr |= val << 8;
                  Register( iter->first+number_of_sensor, static_cast<int>(low_light_val) );
                  Register( 0xF1+number_of_sensor, curr);
               }
            }
            else
            {
               long curr = Register( 0x25) & 0xFF;
               curr |= val << 8;
               Register( iter->first, static_cast<int>(low_light_val) );
               Register( 0x25, curr);
            }
         }
      }
   }
   /*
   else{
      for(std::map<GByte, unsigned>::iterator iter = lighting_conformity[lighting][pcamera->model].begin(); iter != lighting_conformity[lighting][pcamera->model].end(); iter++)
      {
         if((pcamera->ispanoramic))
            iter->first + number_of_sensor;

         if(HIGH_SPEED == low_light_mode)
         {
            double low_light_val = iter->second / 10.0 * short_exposures + 0.5;
            Register(iter->first, static_cast<int>(low_light_val) );
         }
         else
         { 
            Register(iter->first, static_cast<int>(iter->second) );
         }
      }
   }
   */
   clighting = (lighting ? 60 : 50);

   if(2100 == pcamera->model)
      Correct2100Sencor14RegisterBug();
}

//---------------------------------------------------------------------------
//Comment:
// enum LOW_LIGHT { SPEED, QUALITY, BALANCED, CUSTOM, HIGH_SPEED, MOON_LIGHT };
// rn7 = 7,    //MAX_ZONE     0x07
// rn11 = 11,  //FRAMEZONES   0x0B
// rn13 = 13,  //MAXGAIN      0x0D
//---------------------------------------------------------------------------
void AV2000ClientInherit1::LowLightMode(long val)
{
   if(SPEED != val && QUALITY != val && BALANCED != val && HIGH_SPEED != val && MOON_LIGHT != val)
         throw EAVParameterOutOfRange("LowLightMode input value is not valid");

   if(3130 == Model() || AV3135 == Model())
   {
      const unsigned FRAMEZONES_1300 = 0x59, FRAMEZONES = 0x0B, MAX_ZONE = 0x07, MAX_ZONE_1300 = 0x5A;
      Page(3);
      switch(val){
      case QUALITY :
         Register(MAX_ZONE, 20);
         Register(MAX_ZONE_1300, 20);
         Register(FRAMEZONES, 4);
         Register(FRAMEZONES_1300, 2);
         break;
      case SPEED :
         Register(MAX_ZONE, 8);
         Register(MAX_ZONE_1300, 8);
         Register(FRAMEZONES, 1);
         Register(FRAMEZONES_1300, 1);
         break;
      case BALANCED :
         Register(MAX_ZONE, 8);
         Register(MAX_ZONE_1300, 8);
         Register(FRAMEZONES, 4);
         Register(FRAMEZONES_1300, 2);
         break;
      case MOON_LIGHT :
         Register(MAX_ZONE, 50);
         Register(MAX_ZONE_1300, 50);
         Register(FRAMEZONES, 4);
         Register(FRAMEZONES_1300, 4);
         break;
      case HIGH_SPEED :
         Register(MAX_ZONE, 1);
         Register(MAX_ZONE_1300, 1);
         Register(FRAMEZONES, 1);
         Register(FRAMEZONES_1300, 1);
         break;
      }
      long max_of_dntn = MaxOfDayNightTriggerNight(val);
      if(DayNightTriggerNight() > max_of_dntn)
         DayNightTriggerNight(max_of_dntn);

   }
   else{
      unsigned rn7 = 7,    //MAX_ZONE	  0x07
               rn11 = 11,  //FRAMEZONES   0x0B
               rn13 = 13,  //MAXGAIN	  0x0D
               rv7, rv11, rv13;
      if((pcamera->ispanoramic))
      {
         rn7 = 0x11 + number_of_sensor;
         rn11 = 0x1D + number_of_sensor;
         rn13 = 0x25 + number_of_sensor;
      }
      switch(val){
      case SPEED : // speed
         rv7 = 8;
         rv11 = 1;
         rv13 = 20;
         break;
      case QUALITY : // quality
         rv7 = 20;
         rv11 = 4;
         rv13 = 5;
         break;
      case BALANCED : // balanced
         rv7 = 8;
         rv11 = 2;
         rv13 = 20;
         if(AV5100 == pcamera->model || AV5105 == pcamera->model)
         {
         	rv13 = 12;
         }
         break;
      case HIGH_SPEED :
         rv7 = 1;
         rv11 = 1;
         rv13 = 20;
         break;
      case MOON_LIGHT :
         rv7 = 50;
         rv11 = 4;
         rv13 = 5;
         break;
      }
      Page(3);
      Register(rn7, rv7);
      Register(rn11, rv11);
      Register(rn13, rv13);
   }
   Lighting(clighting);
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::Roll(long val)
{
   if((pcamera->ispanoramic))
   {
      Page(3);
      unsigned char roll = Register(0xEC);

      if(val)
         roll |= MsSensorAsBits(number_of_sensor);
      else
         roll &= ~MsSensorAsBits(number_of_sensor);

      Register(0xEC, roll);
      Register(0x56, 1);
   }
   else
      if(Version() < 50412){
         unsigned r32, r1;
         switch(val){
         case 0 :
            2000 == Model() || 2000 == Model() ? r32 = 4357 : r32 = 1;
            r1 = 6;
            break;
         case 180 :
            2000 == Model() || 2000 == Model() ? r32 = 53509 : r32 = 49152;
            r1 = 0;
            break;
         default :
            throw EAVParameterOutOfRange("roll must be 0 or 180");
         }
         Page(4);
         Register(32, r32);
         Page(1);
         Register(1, r1);
      }
      else{
         Page(3);
         unsigned r_h11 = Register(0x11);
         switch(val){
         case 0 :
            r_h11 &= ~1U;
            break;
         case 180 :
            r_h11 |= 1U;
            break;
         default :
            throw EAVParameterOutOfRange("roll must be 0 or 180");
         }
         Register(0x11, r_h11);
         if(Version() < 51719)
            PatchRollForVersionLess51719(val);
      }
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::IrisEnable(long val)
{
   if((pcamera->ispanoramic))
      throw EAVNotSupported("parameter is not supported");
   unsigned w;
   //val ? w = IrisReposEnable() | 3 : w = IrisReposEnable() & ~3;
   val ? w = IrisReposEnable() | 7U : w = IrisReposEnable() & ~7U;
   Page(3);
   Register(0x51, w);
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::IrisSpeed(long val) // 1 <= val <= 255
{
   if((pcamera->ispanoramic))
      throw EAVNotSupported("parameter is not supported");
   if(val < 1 || val > 255)
      throw EAVParameterOutOfRange("must be: 1 <= x <= 255");
   Page(3);
   Register(0x54, val);
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::IrisGain(long val)
{
   if((pcamera->ispanoramic))
      throw EAVNotSupported("parameter is not supported");
   if(val < 8 || val > 255)
      throw EAVParameterOutOfRange("must be: 8 <= x <= 255");
   Page(3);
   Register(0x53, val);
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::IrisReposEnable(long val)
{
   if((pcamera->ispanoramic))
      throw EAVNotSupported("parameter is not supported");
   unsigned w;
   val ? w = IrisEnable() | 4U : w = IrisEnable() & ~4U;
   Page(3);
   Register(0x51, w);
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::IrisReposFStops(long val)
{
   if((pcamera->ispanoramic))
      throw EAVNotSupported("parameter is not supported");
   if(val < 1 || val > 15)
      throw EAVParameterOutOfRange("must be: 1 <= x <= 15");

   val = static_cast<long>(std::pow(2.0, (val + 1) / 2.0) + 0.5);
   if( val >= 256)
      val = 255;
   Page(3);
   Register(0x60, val);
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::IrisReposFStopsMin(long val)
{
   if((pcamera->ispanoramic))
      throw EAVNotSupported("parameter is not supported");
   if(val < 1 || val > 19)
      throw EAVParameterOutOfRange("must be: 1 <= x <= 15");
   val = static_cast<long>(std::pow(2.0, (val + 1) / 2.0) + 0.5);
   if( val >= 256)
      val = 255;
   Page(3);
   Register(0x61, val);
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::IrisReposPeriod(long val)
{
   if((pcamera->ispanoramic))
      throw EAVNotSupported("parameter is not supported");
   if(val < 0 || val > 900)
      throw EAVParameterOutOfRange("must be: 0 <= x <= 900");
   switch(Model()){
   case 2100 :
   case 1300 :
   case AV1305:
   case AV2105:
      val *= 24 * 60;
      break;
   case 3100 :
   case AV3105 :   
      val *= 20 * 60;
      break;
   default:
      val *= 20 * 60;
   }
   Page(3);
   Register(0x63, val / 20);
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::IrisReposStabPeriod(long val)
{
   if((pcamera->ispanoramic))
      throw EAVNotSupported("parameter is not supported");
   if(val < 0 || val > 900)
      throw EAVParameterOutOfRange("must be: 0 <= x <= 900");
   switch(Model()){
   case 2100 :
   case AV2105 :
   case 1300 :
   case AV1305 :
      val *= 24 * 60;
      break;
   case 3100 :
   case AV3105 :
      val *= 20 * 60;
      break;
   default :
      val *= 20 * 60;
   }
   Page(3);
   Register(0x65, val / 20);
}
//---------------------------------------------------------------------------
//Comment:
//#define  REMOVE_FILTER_TH    0x3E //threshold for removing IR filter; values are in terms of gain*8 (default 64*3, range from 8 to about 512)
//#define  FILTER_DN_MARGIN    0x3F //margin for inserting IR filter when switching to day mode, values are powers of 2 used as "gain - (gain >> remove_filter_th)" (default 2)
//#define  IR_FILTER_EN	       0x40 //ir filter enable; 0 - auto; 1 - day; 2 - night
//#define  DN_SWITCH	       0x42 //value 222 if equipped with switch
//---------------------------------------------------------------------------
void AV2000ClientInherit1::DayNightMode(long val)
{
   if(pcamera->ispanoramic)
      throw EAVNotSupported("parameter is not supported");

   if(val < 0 || val > 2)
      throw EAVParameterOutOfRange("must be: 0 <= x <= 2");

   long map[3] = {0x0000, 0x0200, 0x0300};
   switch(Model()){
   case 3130 :
   case AV3135 :
      Page(3);
      Register(80, map[val]);
      break;
   default :
      Page(3);
      Register(0x40, val);
   }
}
//---------------------------------------------------------------------------
//Comment:
//#define  REMOVE_FILTER_TH    0x3E //threshold for removing IR filter; values are in terms of gain*8 (default 64*3, range from 8 to about 512)
//#define  FILTER_DN_MARGIN    0x3F //margin for inserting IR filter when switching to day mode, values are powers of 2 used as "gain - (gain >> remove_filter_th)" (default 2)
//#define  IR_FILTER_EN	       0x40 //ir filter enable; 0 - auto; 1 - day; 2 - night
//#define  DN_SWITCH	       0x42 //value 222 if equipped with switch
//---------------------------------------------------------------------------
void AV2000ClientInherit1::DayNightTriggerNight(long val)
{
   if(pcamera->ispanoramic)
      throw EAVNotSupported("parameter is not supported");

   switch(Model())
   {
   case 3130 :
   case AV3135 :
       if(val < 0 || val > 18)
          throw EAVParameterOutOfRange("must be: 0 <= x <= 18");
       long max;
       try{
          max = MaxOfDayNightTriggerNight(LowLightMode());
       }
       catch(EAVParameterUnknown&){
          max = MaxOfDayNightTriggerNight(3); // CUSTOM
       }
       if(val > max)
          val = max;
       Page(3);
       val = static_cast<long>(std::pow(2.0, val / 2.0) * 8.0 + 0.5);
       Register(85, val);
       break;
   default :
       if(val < 64 || val > 512)
          throw EAVParameterOutOfRange("must be: 64 <= x <= 512");
       Page(3);
       Register(0x3E, val);
   }
}
//---------------------------------------------------------------------------
//Comment:
//#define  REMOVE_FILTER_TH    0x3E //threshold for removing IR filter; values are in terms of gain*8 (default 64*3, range from 8 to about 512)
//#define  FILTER_DN_MARGIN    0x3F //margin for inserting IR filter when switching to day mode, values are powers of 2 used as "gain - (gain >> remove_filter_th)" (default 2)
//#define  IR_FILTER_EN	       0x40 //ir filter enable; 0 - auto; 1 - day; 2 - night
//#define  DN_SWITCH	       0x42 //value 222 if equipped with switch
//---------------------------------------------------------------------------
void AV2000ClientInherit1::DayNightTriggerDay(long val)
{
   if(pcamera->ispanoramic)
      throw EAVNotSupported("parameter is not supported");

   Page(3);
   switch(Model())
   {
   case 3130 :
   case AV3135 :
      if(val < 0 || val > 6)
         throw EAVParameterOutOfRange("must be: 0 <= x <= 6");
      val = static_cast<long>(std::pow(2.0, val / 2.0) * 8.0 + 0.5);
      Register(0x60, val);
      break;
   default :
      if(val < 1 || val > 4)
         throw EAVParameterOutOfRange("must be: 1 <= x <= 4");
      Register(0x3F, val);
   }
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::ExposureMode(long val)
{
   if(val < 0 || val > 2)
      throw EAVParameterOutOfRange("0 <= cpEXPOSURE_MODE <= 2");
   if(val==0) val = 1;
   Page(3);
   Register(0x4e, val);
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::ExposureWindowLeft(long val)
{
   if(val < 0 || val + ExposureWindowWidth() > sensor_size_max.Width(*pcamera))
      throw EAVParameterOutOfRange("0 <= cpEXPOSURE_WINDOW_LEFT <= max_width - cpEXPOSURE_WINDOW_WIDTH");
   Page(3);
   Register(0x4a, val);
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::ExposureWindowTop(long val)
{
   if(val < 0 || val + ExposureWindowHeight() > sensor_size_max.Height(*pcamera))
      throw EAVParameterOutOfRange("0 <= cpEXPOSURE_WINDOW_TOP <= max_height - cpEXPOSURE_WINDOW_HEIGHT");
   Page(3);
   Register(0x4b, val);
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::ExposureWindowWidth(long val)
{
   if(val < 0 || val + ExposureWindowLeft() > sensor_size_max.Width(*pcamera))
      throw EAVParameterOutOfRange("0 <= cpEXPOSURE_WINDOW_WIDTH <= max_width - cpEXPOSURE_WINDOW_LEFT");
   Page(3);
   Register(0x4c, val);
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::ExposureWindowHeight(long val)
{
   if(val < 0 || val + ExposureWindowTop() > sensor_size_max.Height(*pcamera))
      throw EAVParameterOutOfRange("0 <= cpEXPOSURE_WINDOW_HEIGHT <= max_height - cpEXPOSURE_WINDOW_TOP");
   Page(3);
   Register(0x4d, val);
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::SensorLeftChecked(long val)
{
   if(!CheckInto(0, sensor_size_max.Width(*pcamera) - 320, val))
      throw EAVParameterOutOfRange("left ordinate out of range");

   long width = sensor_geometry.width;;
   if(available(cpSENSOR_WIDTH, IdentOfCamera()))
      width = SensorWidth();

   CorrectWidth(sensor_size_max.Width(*pcamera), 32, 32, val, width);

   SensorLeftRaw(val);
   if(sensor_geometry.width != width && available(cpSENSOR_WIDTH, IdentOfCamera()))
      SensorWidthRaw(width);
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::SensorLeftRaw(long val)
{
   unsigned coef = 1,
            reg;
   if((pcamera->ispanoramic)){
      if(imHALF == multisensor_resolution){
         reg = 0xD8;
         coef = 2;
      }
      else
         reg = 0xD4;
   }
   else
      reg = 0x46;
   Page(3);
   Register(reg, val / coef);
   sensor_geometry.left = val;
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::SensorTopChecked(long val)
{
   if(!CheckInto(0, sensor_size_max.Height(*pcamera) - 240, val))
      throw EAVParameterOutOfRange("top ordinate out of range");

   long height = sensor_geometry.height;
   if(available(cpSENSOR_HEIGHT, IdentOfCamera()))
      height = SensorHeight();

   CorrectWidth(sensor_size_max.Height(*pcamera), 2, 16, val, height);

   SensorTopRaw(val);
   if(sensor_geometry.height != height && available(cpSENSOR_HEIGHT, IdentOfCamera()))
      SensorHeightRaw(height);
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::SensorTopRaw(long val)
{
   unsigned coef = 1,
            reg;
   if((pcamera->ispanoramic)){
      if(imHALF == multisensor_resolution){
         reg = 0xDA;
         coef = 2;
      }
      else
         reg = 0xD6;
   }
   else
      reg = 0x47;
   Page(3);
   Register(reg, val / coef);
   sensor_geometry.top = val;
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::SensorWidthChecked(long val)
{
   if(!CheckInto(320, sensor_size_max.Width(*pcamera), val))
      throw EAVParameterOutOfRange("width out of range");

   long left = sensor_geometry.left;
   if(available(cpSENSOR_LEFT, IdentOfCamera()))
      left = SensorLeft();

   CorrectLeft(sensor_size_max.Width(*pcamera), 32, 32, left, val);

   SensorWidthRaw(val);

   if(sensor_geometry.left != left && available(cpSENSOR_LEFT, IdentOfCamera()))
      SensorLeftRaw(val);
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::SensorWidthRaw(long val)
{
   if(!(pcamera->ispanoramic) && Version() < 51617){
      Page(4);
      val += 7;
      Register(4, val);
   }
   else{
      unsigned reg;
      Page(3);
      if((pcamera->ispanoramic)){
         if(imHALF == multisensor_resolution){
            val /= 2;
            reg = 0xD9;
         }
         else
            reg = 0xD5;
         val += SensorLeft();

      }
      else
         reg = 0x48;
      Register(reg, val);
   }
   sensor_geometry.width = val;
   Lighting(clighting);
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::SensorHeightChecked(long val)
{
   if(!CheckInto(240, sensor_size_max.Height(*pcamera), val))
      throw EAVParameterOutOfRange("height out of range");

   long top = sensor_geometry.top;
   if(available(cpSENSOR_TOP, IdentOfCamera()))
      top = SensorTop();

   CorrectLeft(sensor_size_max.Height(*pcamera), 2, 8, top, val);

   SensorHeightRaw(val);

   if(sensor_geometry.top != top && available(cpSENSOR_TOP, IdentOfCamera()))
      SensorTopRaw(top);
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::SensorHeightRaw(long val)
{
   if(!(pcamera->ispanoramic) && Version() < 51617){
      Page(4);
      val += 7;
      Register(3, val);
   }
   else{
      unsigned reg;
      Page(3);
      if((pcamera->ispanoramic)){
         if(imHALF == multisensor_resolution){
            val /= 2;
            reg = 0xDB;
         }
         else
            reg = 0xD7;
         val += SensorTop();
      }
      else
         reg = 0x49;
      Register(reg, val);
   }
   sensor_geometry.height = val;
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::SensorBlackWhiteLeftChecked(long val)
{
   if(!CheckInto(0, sensor_size_max.Width(Camera(1300, 0)) - 320, val))
      throw EAVParameterOutOfRange("left balck-white ordinate out of range");

   long width_black_white = sensor_geometry.width_black_white;
   if(available(cpSENSOR_BLACK_WHITE_WIDTH, IdentOfCamera()))
      width_black_white = SensorBlackWhiteWidth();

   CorrectWidth(sensor_size_max.Width(Camera(1300, 0)), 32, 32, val, width_black_white);

   SensorBlackWhiteLeftRaw(val);
   if(sensor_geometry.width_black_white != width_black_white && available(cpSENSOR_BLACK_WHITE_WIDTH, IdentOfCamera()))
      SensorBlackWhiteWidthRaw(width_black_white);
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::SensorBlackWhiteLeftRaw(long val)
{
   Page(3);
   Register(81, val);
   sensor_geometry.left_black_white = val;
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::SensorBlackWhiteTopChecked(long val)
{
   if(!CheckInto(0, sensor_size_max.Height(Camera(1300, 0)) - 240, val))
      throw EAVParameterOutOfRange("top ordinate out of range");

   long height_black_white = sensor_geometry.height_black_white;
   if(available(cpSENSOR_BLACK_WHITE_HEIGHT, IdentOfCamera()))
      height_black_white = SensorBlackWhiteHeight();

   CorrectWidth(sensor_size_max.Height(Camera(1300, 0)), 2, 16, val, height_black_white);

   SensorBlackWhiteTopRaw(val);
   if(sensor_geometry.height_black_white != height_black_white && available(cpSENSOR_BLACK_WHITE_HEIGHT, IdentOfCamera()))
      SensorBlackWhiteHeightRaw(height_black_white);
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::SensorBlackWhiteTopRaw(long val)
{
   Page(3);
   Register(82, val);
   sensor_geometry.top_black_white = val;
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::SensorBlackWhiteWidthChecked(long val)
{
   if(!CheckInto(320, sensor_size_max.Width(Camera(1300, 0)), val))
      throw EAVParameterOutOfRange("width out of range");

   long left_black_white = sensor_geometry.left_black_white;
   if(available(cpSENSOR_BLACK_WHITE_LEFT, IdentOfCamera()))
      left_black_white = SensorBlackWhiteLeft();

   CorrectLeft(sensor_size_max.Width(Camera(1300, 0)), 32, 32, left_black_white, val);

   SensorBlackWhiteWidthRaw(val);

   if(sensor_geometry.left_black_white != left_black_white && available(cpSENSOR_BLACK_WHITE_LEFT, IdentOfCamera()))
      SensorBlackWhiteLeftRaw(val);
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::SensorBlackWhiteWidthRaw(long val)
{
   Page(3);
   Register(83, val);
   sensor_geometry.width_black_white = val;
   Lighting(clighting);
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::SensorBlackWhiteHeightChecked(long val)
{
   if(!CheckInto(240, sensor_size_max.Height(Camera(1300, 0)), val))
      throw EAVParameterOutOfRange("height out of range");

   long top_black_white = sensor_geometry.top_black_white;
   if(available(cpSENSOR_BLACK_WHITE_TOP, IdentOfCamera()))
      top_black_white = SensorBlackWhiteTop();

   CorrectLeft(sensor_size_max.Height(Camera(1300, 0)), 2, 16, top_black_white, val);

   SensorBlackWhiteHeightRaw(val);

   if(sensor_geometry.top_black_white != top_black_white && available(cpSENSOR_BLACK_WHITE_TOP, IdentOfCamera()))
      SensorBlackWhiteTopRaw(top_black_white);
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::SensorBlackWhiteHeightRaw(long val)
{
   Page(3);
   Register(84, val);
   sensor_geometry.height_black_white = val;
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::RequestLeftChecked(long val)
{
   if((pcamera->ispanoramic)){
      unsigned registr = 0;
      switch(multisensor_resolution){
      case imFULL :
         registr = 0xD4;
         break;
      case imHALF :
         registr = 0xD8;
         break;
      case imZOOM :
         registr = 0xDC + number_of_sensor;
         break;
      }
      if(registr){
         Page(3);
         Register(registr, val);
      }
   }
   else{
      long sensor_width = SensorWidth(),
           request_width_orig = RequestWidth(),
           request_width_corrected = request_width_orig;

      if(!CheckInto(0, sensor_width - 320, val))
         throw EAVParameterOutOfRange("left ordinate out of range");

      CorrectWidth(sensor_width, 32, 32, val, request_width_corrected);

      RequestLeftRaw(val);
      RequestWidthRaw(request_width_corrected);
   }
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::RequestLeftRaw(long val)
{
   Page(3);
   Register(0x67, val);
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::RequestTopChecked(long val)
{
   if((pcamera->ispanoramic)){
      unsigned registr = 0;
      switch(multisensor_resolution){
      case imFULL :
         registr = 0xD6;
         break;
      case imHALF :
         registr = 0xDA;
         break;
      case imZOOM :
         registr = 0xE4 + number_of_sensor;
         break;
      }
      if(registr){
         Page(3);
         Register(registr, val);
      }
   }
   else{
      long sensor_height = SensorHeight(),
           request_height_orig = RequestHeight(),
           request_height_corrected = request_height_orig;

      if(!CheckInto(0, sensor_height - 240, val))
         throw EAVParameterOutOfRange("top ordinate out of range");

      CorrectWidth(sensor_height, 2, 16, val, request_height_corrected);

      RequestTopRaw(val);
      RequestHeightRaw(request_height_corrected);
   }
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::RequestTopRaw(long val)
{
   Page(3);
   Register(0x68, val);
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::RequestWidthChecked(long val)
{
   if((pcamera->ispanoramic)){
      unsigned registr = 0;
      switch(multisensor_resolution){
      case imFULL :
         registr = 0xD5;
         break;
      case imHALF :
         registr = 0xD9;
         break;
      case imZOOM :
         registr = 0xE0 + number_of_sensor;
         break;
      }
      if(registr){
         Page(3);
         Register(registr, RequestLeft() + val);
      }
   }
   else{
      long sensor_width = SensorWidth(),
           request_left_orig = RequestLeft(),
           request_left_corrected = request_left_orig;

      if(!CheckInto(320, sensor_width, val))
         throw EAVParameterOutOfRange("width out of range");

      CorrectLeft(sensor_width, 32, 32, request_left_corrected, val);

      RequestWidthRaw(val);
      RequestLeftRaw(request_left_corrected);
   }
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::RequestWidthRaw(long val)
{
   Page(3);
   Register(0x69, RequestLeft() + val);
   request_width = val;
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::RequestHeightChecked(long val)
{
   if((pcamera->ispanoramic)){
      unsigned registr = 0;
      switch(multisensor_resolution){
      case imFULL :
         registr = 0xD7;
         break;
      case imHALF :
         registr = 0xDB;
         break;
      case imZOOM :
         registr = 0xE8 + number_of_sensor;
         break;
      }
      if(registr){
         Page(3);
         Register(registr, RequestTop() + val);
      }
   }
   else{
      long sensor_height = SensorHeight(),
           request_top_orig = RequestTop(),
           request_top_corrected = request_top_orig;

      if(!CheckInto(240, sensor_height, val))
         throw EAVParameterOutOfRange("height out of range");

      CorrectLeft(sensor_height, 2, 16, request_top_corrected, val);

      RequestHeightRaw(val);
      RequestTopRaw(request_top_corrected);
   }
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::RequestHeightRaw(long val)
{
   Page(3);
   Register(0x6A, RequestLeft() + val);
   request_height = val;
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::RequestResolution(long val)
{
   Page(3);
   if(8360 == pcamera->model || 8180 == pcamera->model || AV8365 == pcamera->model || AV8185 == pcamera->model){
      if(imFULL != val && imHALF != val && imZOOM != val)
         throw EAVParameterOutOfRange("resolution must be imFULL, imHALF or imZOOM");
      multisensor_resolution = val;
   }
   else{
      Register(0x6C, val);
      request_resolution = val;
   }
}
//---------------------------------------------------------------------------
//Reg[3:0x6B] default quality
//---------------------------------------------------------------------------
void AV2000ClientInherit1::RequestQuality(long val)
{
   if(val < 1 || val > 21)
      throw EAVParameterOutOfRange("1 <= quality <= 21");
   Page(3);
   Register(0x6B, val);
   request_quality = val;
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::RequestedBlockSize(long val)
{
   if(1450 != val && 2904 != val)
      throw EAVParameterOutOfRange("requested_block_size must be 1450 or 2904");
   if((3130 == pcamera->model || AV3135 == pcamera->model) && pcamera->version < 61613 && 2904 == val)
      val = 1450;
   client->RequestedBlockSize(requested_block_size = val);
}
//---------------------------------------------------------------------------
//Panoramic camera MD register declarations
//---------------------------------------------------------------------------
// local registers for configuring all channels' motion detection parameters at once
//---------------------------------------------------------------------------
#define  PANORAMIC_ALL_MD_CTRL     0x31
// Motion detection control register
// bit 0: 1=motion detection enable; 0=disable
// bit 1: 1=sending image only if motion detection alarm; 0=sending image always
// bit 2: 1=motion detection alarm (read-only)
// bit 3: 1=prev motion detection status  (mask 0x08)
// bit 4: 1=motion notification mode (mask 0x10)
#define  PANORAMIC_ALL_MD_ZONES    0x32 // Motion detection zones
// low byte: total zones (1-64 default 64)
// low tetrad of high byte: zone size (1-15 default 8)
#define  PANORAMIC_ALL_MD_THRESHOLDS 0x33 // Motion detection sensitivity thresholds
// low byte: level threshold
// high byte: top sensitivity threshold (in percents of alarmed zones)
#define  PANORAMIC_ALL_MD_PRIVACY_ZONES 0x34 // include 0x134-0x137 (4 elements = 8 byte each bit is one zone (total 64))
//---------------------------------------------------------------------------
// local registers for Motion detection parameters, 4 channels each
//---------------------------------------------------------------------------
#define  PANORAMIC_MD_CTRL     0x38 // Motion detection control register
// bit 0: 1=motion detection enable; 0=disable
// bit 1: 1=sending image only if motion detection alarm; 0=sending image always
// bit 2: 1=motion detection alarm (read-only)
// bit 3: 1=prev motion detection status  (mask 0x08)
// bit 4: 1=motion notification mode (mask 0x10)
// high byte: "detail" minimal number of alarmed blocks for alarming zone (0-225 default 1)
#define  PANORAMIC_MD_ZONES    0x3C // Motion detection zones
// low byte: total zones (1-64 default 64)
// low tetrad of high byte: zone size (1-15 default 8)
#define  PANORAMIC_MD_THRESHOLDS 0x40 // Motion detection sensitivity thresholds
// low byte: level threshold
// high byte: top sensitivity threshold (in percents of alarmed zones)
#define  PANORAMIC_MD_PRIVACY_ZONES 0x44 // include 0x144-0x153 (4 channels: 4 elements = 8 byte each bit is one zone (total 64))
//---------------------------------------------------------------------------
void AV2000ClientInherit1::MdEnabled(long val)
{
   if(val != 0 && val != 1)
      throw EAVParameterOutOfRange("must be 0 or 1");
   if((pcamera->ispanoramic)){
      Page(6);
      Register(PANORAMIC_MD_CTRL+number_of_sensor, (Register(PANORAMIC_MD_CTRL+number_of_sensor) & ~1) | val);
      Page(3);
   }
   else{
      Page(3);
      Register(0x6D, (Register(0x6D) & ~1) | val);
   } 
}
//---------------------------------------------------------------------------
//Single sensor model motion registers convention:
// Reg(0x6D) bit0 Enable
//           bit 8~15 Detail, how many cells in a zone constitue a motion zone
// Reg(0x6E) bit0~7 total zones, always 64
//           bit8~11 zone size, how many cells (32x32 pixels) a zone has
// Reg(0x6F) Level: at least how much luma difference can be considered motion
//           luma is computed as average of 32x32 pixels
// Reg(0x21) 8 bytes, MD matrix, which zones are enabled
//---------------------------------------------------------------------------
void AV2000ClientInherit1::MdMode(long val)
{
   if(val != 0 && val != 1)
      throw EAVParameterOutOfRange("must be 0 or 1");
   val <<= 1;
   if((pcamera->ispanoramic)){
      Page(6);
      Register(PANORAMIC_MD_CTRL+number_of_sensor, (Register(PANORAMIC_MD_CTRL+number_of_sensor) & ~2) | val);
      Page(3);
   }
   else{
      Page(3);
      Register(0x6D, (Register(0x6D) & ~2) | val);
   }
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::MdLevelThresh(long val)
{
   if(val < 0 || val > 255)
      throw EAVParameterOutOfRange("1 <= x <= 255");
   if((pcamera->ispanoramic)){
       Page(6);
       Register(PANORAMIC_MD_THRESHOLDS+number_of_sensor, (Register(PANORAMIC_MD_THRESHOLDS+number_of_sensor) & ~0x00FF) | val);
       Page(3);
   }
   else{
       Page(3);
       Register(0x6F, (Register(0x6F) & 0xFF00) | val);
   }
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::MdTotalZones(long val)
{                             
   if(val < 1 || val > 64)
      throw EAVParameterOutOfRange("1 <= x <= 64");
   if((pcamera->ispanoramic)){
       Page(6);
       Register(PANORAMIC_MD_ZONES+number_of_sensor, (Register(PANORAMIC_MD_ZONES+number_of_sensor) & 0xFF00) | val);
       Page(3);
   }
   else{
       Page(3);
       Register(0x6E, (Register(0x6E) & 0xFF00) | val);
   }
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::MdZoneSize(long val)
{
   if(val < 1 || val > 15)
      throw EAVParameterOutOfRange("1 <= x <= 15");

   long detail = MdDetail();
   if(detail > val * val)
      MdDetail(val * val);

   val <<= 8;
   if((pcamera->ispanoramic)){
       Page(6);
       Register(PANORAMIC_MD_ZONES+number_of_sensor, (Register(PANORAMIC_MD_ZONES+number_of_sensor) & 0xF0FF) | val);
       Page(3);
   }
   else{
      Page(3);
      Register(0x6E, (Register(0x6E) & 0xF0FF) | val);
   }
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::MdDetail(long val)
{
   long zone_size = MdZoneSize();
   if(val > zone_size * zone_size)
      throw EAVParameterOutOfRange("0 <= cpMD_DETAIL <= cpMD_ZONE_SIZE * cpMD_ZONE_SIZE");
   val <<= 8;
   if((pcamera->ispanoramic)){
       Page(6);
       Register(PANORAMIC_MD_CTRL+number_of_sensor, (Register(PANORAMIC_MD_CTRL+number_of_sensor) & 0x00FF) | val);
       Page(3);
   }
   else{
      Page(3);
       Register(0x6D, (Register(0x6D) & 0x00FF) | val);
   }
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::MdExplosureSensitivity(long val)
{
   if(val < 0 || val > 64)
      throw EAVParameterOutOfRange("0 <= x <= 64");
   val <<= 8;
   if((pcamera->ispanoramic)){
       Page(6);
       Register(PANORAMIC_MD_THRESHOLDS+number_of_sensor, (Register(PANORAMIC_MD_THRESHOLDS+number_of_sensor) & 0x00FF) | val);
       Page(3);
   }
   else{
       Page(3);
       Register(0x6F, (Register(0x6F) & 0xFF) | val);
   }
}
//---------------------------------------------------------------------------
//Comment:
// firmware code use LSB for leftmost zone and MSB for rightmost zone, i.e.
// if(pregs[MD_PRIVACY_ZONES+bytecount]>>(i-mask) & 0x01)
// but this is not extremely intuative for users, thus in sdk we cycle shift
// the input bytes so that LSB is for rightmost zone and MSB for leftmost
//---------------------------------------------------------------------------
void AV2000ClientInherit1::MdMatrix(long val)
{
   //FILE *fp = fopen("sdk_report.txt","a+");
   unsigned char* arr = reinterpret_cast<unsigned char*>(val);
   for(int i = 0; i < 8; i += 2){
      //fprintf(fp,"%02X %02X ", arr[i], arr[i+1]);
      arr[i] = CycleShift(arr[i]);
      arr[i+1] = CycleShift(arr[i+1]);
      if((pcamera->ispanoramic)){
         Page(6);
         Register(PANORAMIC_MD_PRIVACY_ZONES+number_of_sensor*4 + i / 2, arr[i+1] * 256U + arr[i]);
         Page(3);
      }
      else{
         Page(3);
         Register(0x21 + i / 2, arr[i+1] * 256U + arr[i]);
      }
   }
   //fprintf(fp,"\n");
   //fclose(fp);
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::MsNumberOfSensor(long val)
{
   if(val < 1 || val > 4)
      throw EAVParameterOutOfRange("1 <= x <= 4");
   number_of_sensor = val - 1;
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::MsChannelEnable(long val)
{
   unsigned sensor_as_bits = MsSensorAsBits(number_of_sensor);
   Page(3);
   unsigned registr = 0xD0;
   if(val)
      Register(registr, Register(registr) | sensor_as_bits);
   else
      Register(registr, Register(registr) & ~sensor_as_bits);
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::MsFullResEnable(long val)
{
   unsigned sensor_as_bits = MsSensorAsBits(number_of_sensor);
   Page(3);
   unsigned registr = 0xD1;
   if(val)
      Register(registr, Register(registr) | sensor_as_bits);
   else
      Register(registr, Register(registr) & ~sensor_as_bits);
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::MsZoomWinEnable(long val)
{
   unsigned sensor_as_bits = MsSensorAsBits(number_of_sensor);
   Page(3);
   unsigned registr = 0xD2;
   if(val)
      Register(registr, Register(registr) | sensor_as_bits);
   else
      Register(registr, Register(registr) & ~sensor_as_bits);
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::MsOneShotEnable(long val)
{
   unsigned sensor_as_bits = MsSensorAsBits(number_of_sensor);
   Page(3);
   unsigned registr = 0xD3;
   if(val)
      Register(registr, Register(registr) | sensor_as_bits);
   else
      Register(registr, Register(registr) & ~sensor_as_bits);
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::MsBitRate(long val)
{

   if(val < 0 || val > 65000)
      throw EAVParameterOutOfRange("0 <= x <= 65000");

    //number_of_sensor


    unsigned registr;

    switch(multisensor_resolution)
    {
      case imFULL:
        registr = 0x60 + number_of_sensor;
        break;

      case imHALF:
        registr = 0x64 + number_of_sensor;

        break;
      default:
        throw EAVCameraUnknown("cpRESOLUTION is unknown");
    }

    Page(6);
    Register(registr, val);
    Page(3);

}

long AV2000ClientInherit1::MsBitRate()
{



    unsigned registr;

    switch(request_resolution)
    {
      case imFULL:
        registr = 0x60 + number_of_sensor;
        break;

      case imHALF:
        registr = 0x64 + number_of_sensor;

        break;
      default:
        throw EAVCameraUnknown("cpRESOLUTION is unknown");
    }

    Page(6);
    long ret = Register(registr);
    Page(3);


    return ret;

}

//---------------------------------------------------------------------------
void AV2000ClientInherit1::MsQuadMode(long val)
{
   Page(3);
   Register(0xF0, val ? 1 : 0);
}
//---------------------------------------------------------------------------
//lght_shex high byte lighting, low byte short exposure
//regular camera is 0x25, panoramic ix 0xF1
//---------------------------------------------------------------------------
void AV2000ClientInherit1::ShortExposures(long val)
{
   if(val < 1 || val > 80)
      throw EAVParameterOutOfRange("1 <= x <= 80");
   short_exposures = val;
   if((pcamera->ispanoramic))
   {
          unsigned int lght_shex = Register(0xF1 + number_of_sensor);
          long curr =  (val) + (lght_shex & 0xFF00);
          Register( 0xF1+number_of_sensor, curr);
   }
   else
   {
          unsigned int lght_shex = Register(0x25);
          long curr =  (val) + (lght_shex & 0xFF00);
          Register( 0x25, curr);
   }
   Lighting(clighting);
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::TimeoutOnCamera(long val)
{
   if(val < 0 || val > 3000)
      throw EAVParameterOutOfRange("0 <= x <= 3000");

   unsigned value = RepeatFromCamera();
   value <<= 12;
   value |= ((val / 10) & 0x7FF);
   Page(3);
   if(pcamera->ispanoramic)
      Register(0x60, value);
   else
      Register(0x4F, value);
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::RepeatFromCamera(long val)
{
   if(val < 0 || val > 5)
      throw EAVParameterOutOfRange("0 <= x <= 5");

   unsigned value = val;
   value <<= 12;
   value |= ((TimeoutOnCamera() / 10) & 0x7FF);
   Page(3);
   if(pcamera->ispanoramic)
      Register(0x60, value);
   else
      Register(0x4F, value);
}
//---------------------------------------------------------------------------


void AV2000ClientInherit1::ScEnabled(long val)
{
   if(!pcamera->mini)
      throw EAVNotSupported("parameter is not supported, only for mini-camera");
   Page(3);
   unsigned r = Register(0x19),
            bit = 1;
   Register(0x19, val ? r |= bit : r &= ~bit);
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::ScStrobe(long val)
{
   if(!pcamera->mini)
      throw EAVNotSupported("parameter is not supported, only for mini-camera");
   Page(3);
   unsigned r = Register(0x19),
            bit = 128;
   Register(0x19, val ? r |= bit : r &= ~bit);
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::ScAlwaysSend(long val)
{
   if(!pcamera->mini)
      throw EAVNotSupported("parameter is not supported, only for mini-camera");
   Page(3);
   unsigned r = Register(0x19),
            bit = 2;
   Register(0x19, val ? r |= bit : r &= ~bit);
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::ScOutputHigh(long val)
{
   if(!pcamera->mini)
      throw EAVNotSupported("parameter is not supported, only for mini-camera");
   Page(3);
   unsigned r = Register(0x19),
            bit = 0x10;
   Register(0x19, val ? r |= bit : r &= ~bit);
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::ScOutputLow(long val)
{
   if(!pcamera->mini)
      throw EAVNotSupported("parameter is not supported, only for mini-camera");
   Page(3);
   unsigned r = Register(0x19),
            bit = 0x20;
   Register(0x19, val ? r |= bit : r &= ~bit);
}
//---------------------------------------------------------------------------
//Comment:
// When ambient light is strong, analog_gain = digital_gain = 1, exposure is
// small value. As the ambient becomes darker, the following things happen
// 1) camera increase exposure while maintaining a_gain=d_gain=1, until
//    expousre reaches knee_point
// 2) camera maintain expousre = knee_point and increase a_gain until
//    a_gain reaches max_knee_gain
// 3) camera maintain a_gain = max_knee_gain and increase expousre until
//    exposure reaches max_exposure_time
// 4) camera maintain expousre = max_exposure_time and increase a_gain until
//    a_gain reaches max_analog_gain
// 5) camera maintain expousre = max_exposure_time and a_gain = max_analog_gain
//    and increase digital gain unitl d_gain = max_digital_gain
//---------------------------------------------------------------------------
void AV2000ClientInherit1::SetCustomMode(long& knee_point, long& max_analog_gain, long& max_knee_gain, long& max_exposure_time, long& max_digital_gain)
{
   if(knee_point < 1 || knee_point > 100)
      throw EAVParameterOutOfRange("1 <= knee_point <= 100");
   if(max_analog_gain < 1 || max_analog_gain > 10)
      throw EAVParameterOutOfRange("1 <= max_analog_gain <= 10");
   if(max_exposure_time > 100)
      throw EAVParameterOutOfRange("max_exposure_time <= 100");
   if(max_digital_gain < 32 || max_digital_gain > 127)
      throw EAVParameterOutOfRange("32 <= max_digital_gain <= 127");
   //if(max_knee_gain < max_analog_gain/* * 1.125*/)
      //max_knee_gain = max_analog_gain/* * 1.125 + 0.5*/;

   if(max_knee_gain < 2)
      max_knee_gain = 2;

   if(max_knee_gain > (max_analog_gain * max_digital_gain) / 32 )
      max_knee_gain = (max_analog_gain * max_digital_gain) / 32;

   if(max_exposure_time < knee_point)
      max_exposure_time = knee_point;

   Page(3);
   Register(11, knee_point);
   Register(5, (long)(max_analog_gain * 8 + 0.5));
   Register(13, max_knee_gain);
   Register(7, max_exposure_time);
   Register(8, max_digital_gain);

   if(2100 == pcamera->model)
      Correct2100Sencor14RegisterBug();
}
//---------------------------------------------------------------------------
//output: 100/gamma (gamma in [1.0, 2.5])
//version 64001 && proc_fpga 64017
//---------------------------------------------------------------------------
long AV2000ClientInherit1::Gamma()
{
    if(Version() < 64001 || pcamera->proc_fpga <64107){
            return 60;
    }

    int gamma4 = 0;
   if(pcamera->ispanoramic)
   {
     Page(6);
     gamma4 = Register(0x69+ 4);
   }
   else
   {
     Page(3);
     gamma4 = Register(0xA0+4);
   }

   double gamma = log10(24.0/216.0) / log10((double)gamma4 / 255.0);
   return (long) (100.0 / gamma +  0.5);
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::Brightness()
{
	Page(3);
   if((pcamera->ispanoramic))
      return Register(0x84 + number_of_sensor) - 100;
	return Register(2) - 100;
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::AutoExposition()
{
   Page(3);
   if((pcamera->ispanoramic))
      switch(Register(number_of_sensor + 1)){
      case 0xF :
         return 1;
      case 0x6 :
         return 0;
      default :
         throw EAVParameterUnknown("cpAUTO_EXPOSITION is unknown");
      }
   return Register(1) & 1;
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::Sharpness()
{
   if((pcamera->ispanoramic)){
      Page(3);
      return Register(0x2D + number_of_sensor);
   }

   if(Version() < 50412){
	   Page(1);
	   return Register(2) - 112;
   }
   else{
      Page(3);
      return Register(0x10);
   }
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::Saturation()
{
	long val;
   if(8360 != pcamera->model && 8180 != pcamera->model && AV8365 != pcamera->model && AV8185 != pcamera->model && Version() < 50412){
      Page(2);
	   switch(Register(15)){
      case 117 : val = 0; break;
      case 101 : val = 25; break;
      case 93 : val = 37; break;
      case 85 : val = 50; break;
      case 77 : val = 75; break;
      case 69 : val = 100; break;
      case 108 : val = 125; break;
      default : throw EAVParameterUnknown("unknown");
	   }
   }
   else{
      Page(3);
      if(8360 == pcamera->model || 8180 == pcamera->model || AV8365 == pcamera->model || AV8185 == pcamera->model)
         val = Register(0x29 + number_of_sensor);
      else
         val = Register(0x0F);
	   switch(val){
      case 0 : val = 0; break;
      case 1 : val = 25; break;
      case 2 : val = 37; break;
      case 3 : val = 50; break;
      case 4 : val = 75; break;
      case 5 : val = 100; break;
      case 6 : val = 125; break;
      default : throw EAVParameterUnknown("unknown");
	   }
   }
	return val;
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::Blue()
{
	Page(3);
	return ((pcamera->ispanoramic) ? Register(0x51 + number_of_sensor) : Register(69)) - 128;
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::Red()
{
	Page(3);
	return ((pcamera->ispanoramic) ? Register(0x4D + number_of_sensor) : Register(68) ) - 128;
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::Illumination()
{
   Page(3);
   return (pcamera->ispanoramic) ? Register(0x35 + number_of_sensor) : Register(38);
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::Lightingfix()
{
   return lightingfix;
}
//---------------------------------------------------------------------------
//Firmware definitions
//---------------------------------------------------------------------------
//Regular model:
//#define  LGTNG_SHEX	  0x25
// high byte: lighting frequency (50/60 Hz); low byte: short exposures time (1ms..10ms)
//---------------------------------------------------------------------------
// Panoramic model:
// 4 values high byte: lighting frequency (50/60 Hz); low byte: short exposures time (1ms..10ms)
//#define  LGTNG_SHEX	  0xF1
//---------------------------------------------------------------------------
long AV2000ClientInherit1::Lighting()
{
   Page(3);
   unsigned int val = (pcamera->ispanoramic) ? Register(0xF1 + number_of_sensor) : Register(0x25);
   short_exposures = val & 0xFF;
   return val >> 8;

   //---------------code below no longer in use-----------------
   long lighting, errs,
        low_light_mode = LowLightMode();
   if(HIGH_SPEED == low_light_mode)
      return 0;

   Page(3);

   for(lighting = 0; lighting <= 1; lighting++){
      errs = 0;
      if(lighting_coefs.IsExists(lighting, pcamera->model)){
         long sensor_width;
         //iterr->first: register number, 4
         //iter->second: value pair, (coef,238), (coef,251), ...
         std::map<GByte, std::pair<long, long> >::iterator iter = lighting_coefs[lighting][pcamera->model].begin();
         for(; iter != lighting_coefs[lighting][pcamera->model].end(); iter++){
            if(pcamera->ispanoramic)
               iter->first + number_of_sensor;
            (3130 == pcamera->model || AV3135 == pcamera->model) && FRAME5TH_1300 == iter->first ? sensor_width = sensor_geometry.width_black_white : sensor_width = sensor_geometry.width;
            /*
            if(HIGH_SPEED == low_light_mode){
               long temp_short_exp = Register( iter->first) * 10.0 / lighting_coefs(iter->second, sensor_width) + 0.5;
               if(1 <= temp_short_exp && temp_short_exp <= 10)
                  short_exposures = temp_short_exp;
               else{
                  errs++;
                  break;
               }
            }
            else
            */
              long regval = Register( iter->first),
                   expected = lighting_coefs(iter->second, sensor_width);
               if( regval !=  expected ){
                  /*
                  FILE *fp = fopen("report.txt","a+");
                  fprintf(fp,"cam %d, reg(%d)==%d, expecting %d/(%d,%d)==%d\r\n", pcamera->model, iter->first, regval, iter->second.first, iter->second.second, sensor_width, expected);
                  fclose(fp);
                  */
                  errs++;
                  break;
               }
         }
      }
      else{
         for(std::map<GByte, unsigned>::iterator iter = lighting_conformity[lighting][pcamera->model].begin(); iter != lighting_conformity[lighting][pcamera->model].end(); iter++){
            if(pcamera->ispanoramic)
               iter->first + number_of_sensor;
            /*
            if(HIGH_SPEED == low_light_mode){
               long temp_short_exp = Register( iter->first) * 10.0 / iter->second + 0.5;
               if(1 <= temp_short_exp && temp_short_exp <= 10)
                  short_exposures = temp_short_exp;
               else{
                  errs++;
                  break;
               }
            }
            */
            if(Register(iter->first) != iter->second ){
               errs++;
               break;
            }
         }
      }
      if(!errs)
         break;
   }

   if(errs)
      return 0;
      //throw EAVParameterUnknown("cpLIGHTING is unknown");

   return lighting ? 60 : 50;
}
//---------------------------------------------------------------------------
//Caution:
// for AV5100 and AV5105, max_knee_gain in BALANCE mode is restricted to 12
//---------------------------------------------------------------------------
//Comment:
// Register(5, (long)(max_analog_gain * 8 + 0.5));
// Register(7, max_exposure_time);
// Register(8, max_digital_gain);
// Register(11, knee_point);
// Register(13, max_knee_gain);
//---------------------------------------------------------------------------
//Output:
// enum LOW_LIGHT { SPEED, QUALITY, BALANCED, CUSTOM, HIGH_SPEED, MOON_LIGHT };
// 0 �C maintain higher frame rate
// 1 �C higher image quality
// 2 �C balanced
// 3 �C custom mode (read only value, to set us SetCustomMode)
// 4 �C HIGH_SPEED mode  fixed shutter widths (set cpSHORT_EXPOSURES)
// 5 �C MoonLight. mode  long exposures with proprieryta noise cancellation
//---------------------------------------------------------------------------
long AV2000ClientInherit1::LowLightMode()
{
   if(3130 == Model() || AV3135 == Model())
   {

      const unsigned FRAMEZONES_1300 = 0x59, FRAMEZONES = 0x0B, MAX_ZONE = 0x07, MAX_ZONE_1300 = 0x5A;
      Page(3);
      unsigned max_zone = Register(MAX_ZONE),
               max_zone_1300 = Register(MAX_ZONE_1300),
               framezones = Register(FRAMEZONES),
               framezones_1300 = Register(FRAMEZONES_1300);

      if(20 == max_zone && 20 == max_zone_1300 && 4 == framezones && 2 == framezones_1300)
         return QUALITY;

      if(8 == max_zone && 8 == max_zone_1300){
         if( 1 == framezones && 1 == framezones_1300)
            return SPEED;
         if(4 == framezones && 2 == framezones_1300)
            return BALANCED;
      }

      if(50 == max_zone && 50 == max_zone && 4 == framezones && 4 == framezones_1300)
         return MOON_LIGHT;

      if(1 == max_zone && 1 == max_zone && 1 == framezones && 1 == framezones_1300)
         return HIGH_SPEED;
   }
   else{
      unsigned rn7 = 7,    // max_exposure_time
               rn11 = 11,  // knee_point
               rn13 = 13,  // max_knee_gain
               rv7, rv11, rv13;
      if(pcamera->ispanoramic){
         rn7 = 0x11 + number_of_sensor;
         rn11 = 0x1D + number_of_sensor;
         rn13 = 0x25 + number_of_sensor;
      }
      Page(3);
      rv7 = Register(rn7);
      rv11 = Register(rn11);
      rv13 = Register(rn13);
      if(8 == rv7 && 1 == rv11 && 20 == rv13)
         return SPEED;
      if(20 == rv7 && 4 == rv11 && 5 == rv13)
         return QUALITY;
      if(pcamera->model == AV5100 || pcamera->model == AV5105)
      {
         if(8 == rv7 && 2 == rv11 && 12 == rv13)
             return BALANCED;
      }
      else if(8 == rv7 && 2 == rv11 && 20 == rv13)
         return BALANCED;
      if(1 == rv7 && 1 == rv11 && 20 == rv13)
         return HIGH_SPEED;
      if(50 == rv7 && 4 == rv11 && 5 == rv13)
         return MOON_LIGHT;
   }
   return CUSTOM;
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::Roll()
{
   long ret;
   if(pcamera->ispanoramic){
      Page(3);
      ret = ( (Register(0xEC) & MsSensorAsBits(number_of_sensor)) ? 180 : 0);
   }
   else{
      if(Version() < 50412){
         unsigned r32, r1;
         Page(4);
         r32 = Register(32);
         Page(1);
         r1 = Register(1);
         switch(Model()){
         case 2000 :
         case 2100 :
         case AV2105 :
         case 1300 :
         case AV1305 :
            if(4357 == r32 && 6 == r1)
               return 0;
            if(r32 == 53509 && 0 == r1)
               return 180;
         case 3100 :
         case AV3105 :
         case 5100 :
         case AV5105 :
         case 3130 :
         case AV3135 :
            if(1 == r32 && 6 == r1)
               return 0;
            if(49152 == r32 && 0 == r1)
               return 180;
         }
         throw EAVParameterUnknown("roll unknown");
      }
      else{
         Page(3);
         ret = (Register(0x11) & 1 ? 180 : 0);
      }
   }
   return ret;
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::IrisEnable()
{
   if(pcamera->ispanoramic)
      throw EAVNotSupported("parameter is not supported");
   Page(3);
   return static_cast<unsigned>(Register(0x51) & 3);
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::IrisSpeed()
{
   if(pcamera->ispanoramic)
      throw EAVNotSupported("parameter is not supported");
   Page(3);
   return Register(0x54);
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::IrisGain()
{
   if(pcamera->ispanoramic)
      throw EAVNotSupported("parameter is not supported");
   Page(3);
   return Register(0x53);
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::IrisReposEnable()
{
   if(pcamera->ispanoramic)
      throw EAVNotSupported("parameter is not supported");
   Page(3);
   return static_cast<unsigned>(Register(0x51) & 4);
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::IrisReposFStops()
{
   if(pcamera->ispanoramic)
      throw EAVNotSupported("parameter is not supported");
   Page(3);
   unsigned r = Register(0x60);
   if(!r)
      throw EAVParameterUnknown();
   if(1 == r)
      return 0;
   return (long)(std::log(static_cast<double>(r)) / std::log(2.0) * 2 - 1 + 0.5);
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::IrisReposFStopsMin()
{
   if(pcamera->ispanoramic)
      throw EAVNotSupported("parameter is not supported");
   Page(3);
   unsigned r = Register(0x61);
   if(!r)
      throw EAVParameterUnknown();
   if(1 == r)
      return 0;
   return (long)(std::log(static_cast<double>(r)) / std::log(2.0) * 2 - 1 + 0.5);
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::IrisReposPeriod()
{
   if(pcamera->ispanoramic)
      throw EAVNotSupported("parameter is not supported");
   Page(3);
   unsigned long r = static_cast<unsigned long>(Register(0x63)) * 20;
   switch(Model()){
   case 2100 :
   case AV2105 :
   case 1300 :
   case AV1305 :   
      r /= 24 * 60;
      break;
   case 3100 :
   case AV3105 :   
      r /= 20 * 60;
      break;
   default:
      r /= 20 * 60;
   }
   return r;
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::IrisReposStabPeriod()
{
   if(pcamera->ispanoramic)
      throw EAVNotSupported("parameter is not supported");
   Page(3);
   unsigned long r = static_cast<unsigned long>(Register(0x65)) * 20;
   switch(Model()){
   case 2100 :
   case AV2105 :
   case 1300 :
   case AV1305 :
      r /= 24 * 60;
      break;
   case 3100 :
   case AV3105 :   
      r /= 20 * 60;
      break;
   default:
      r /= 20 * 60;
   }
   return r;
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::DayNightMode()
{
   if(pcamera->ispanoramic)
      throw EAVNotSupported("parameter is not supported");
   Page(3);
   if(3130 !=Model() && AV3135 !=Model())
      return Register(0x40);

   switch(Register(80)){
   case 0x0000 :
   case 3 :
      return 0;
   case 0x0200 :
   case 1536 :
      return 1;
   case 0x0300 :
   case 771 :
      return 2;
   }
   throw EAVParameterUnknown("unknown");
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::DayNightTriggerNight()
{
   if(pcamera->ispanoramic)
      throw EAVNotSupported("parameter is not supported");

   long ret = 0;
   Page(3);
   switch(Model())
   {
   case 3130 :
   case AV3135 :
       ret = (long)((std::log(Register(85) / 8.0) / std::log(2.0)) * 2 + 0.5);
       break;
   default :
       ret = Register(0x3E);
   }
   return ret;
}
//---------------------------------------------------------------------------
//Comment:
//#define  REMOVE_FILTER_TH    0x3E //threshold for removing IR filter; values are in terms of gain*8 (default 64*3, range from 8 to about 512)
//#define  FILTER_DN_MARGIN    0x3F //margin for inserting IR filter when switching to day mode, values are powers of 2 used as "gain - (gain >> remove_filter_th)" (default 2)
//#define  IR_FILTER_EN	       0x40 //ir filter enable; 0 - auto; 1 - day; 2 - night
//#define  DN_SWITCH	       0x42 //value 222 if equipped with switch
//---------------------------------------------------------------------------
long AV2000ClientInherit1::DayNightTriggerDay()
{
   if(pcamera->ispanoramic)
      throw EAVNotSupported("parameter is not supported");
   long ret = 0;
   Page(3);
   switch(Model())
   {
   case 3130 :
   case AV3135 :
       ret = (long)((std::log(Register(0x60) / 8.0) / std::log(2.0)) * 2 + 0.5);
       break;
   default :
       ret = Register(0x3F);
   }
   return ret;
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::ExposureMode()
{
   Page(3);
   unsigned ret = Register(0x4e) & 3;
   if(3 == ret)
      throw EAVParameterUnknown("unknown");
   return ret;
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::ExposureWindowLeft()
{
   Page(3);
   return Register(0x4a);
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::ExposureWindowTop()
{
   Page(3);
   return Register(0x4b);
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::ExposureWindowWidth()
{
   Page(3);
   return Register(0x4c);
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::ExposureWindowHeight()
{
   Page(3);
   return Register(0x4d);
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::SensorLeft()
{
   unsigned coef = 1,
            reg = 0x46;
   if(pcamera->ispanoramic){
      if(imHALF == multisensor_resolution){
         coef = 2;
         reg = 0xD8;
      }
      else
         reg = 0xD4;
   }

   Page(3);
   return Register(reg) * coef;
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::SensorTop()
{
   unsigned coef = 1,
            reg = 0x47;
   if(pcamera->ispanoramic){
      if(imHALF == multisensor_resolution){
         coef = 2;
         reg = 0xDA;
      }
      else
         reg = 0xD6;
   }

   Page(3);
   return Register(reg) * coef;
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::SensorWidth()
{
   if(!pcamera->ispanoramic && Version() < 51617){
      Page(4);
      return Register(4) - 7;
   }
   else{
      unsigned reg = 0x48,
               sub = 0,
               coef = 1;
      if(pcamera->ispanoramic){
         if(imHALF == multisensor_resolution){
            reg = 0xD9;
            coef = 2;
         }
         else
            reg = 0xD5;
         sub = SensorLeft();
      }
      Page(3);
      return Register(reg) * coef - sub;
   }
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::SensorHeight()
{
   if(!pcamera->ispanoramic && Version() < 51617){
      Page(4);
      return Register(3) - 7;
   }
   else{
      unsigned reg = 0x49,
               sub = 0,
               coef = 1;
      if(pcamera->ispanoramic){
         if(imHALF == multisensor_resolution){
            reg = 0xDB;
            coef = 2;
         }
         else
            reg = 0xD7;
         sub = SensorTop();
      }
      Page(3);
      return Register(reg) * coef - sub;
   }
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::SensorBlackWhiteLeft()
{
   Page(3);
   return Register(81);
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::SensorBlackWhiteTop()
{
   Page(3);
   return Register(82);
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::SensorBlackWhiteWidth()
{
   Page(3);
   return Register(83);
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::SensorBlackWhiteHeight()
{
   Page(3);
   return Register(84);
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::RequestLeft()
{
   Page(3);
   if(pcamera->ispanoramic){
      unsigned registr = 0;
      switch(multisensor_resolution){
      case imFULL :
         registr = 0xD4;
         break;
      case imHALF :
         registr = 0xD8;
         break;
      case imZOOM :
         registr = 0xDC + number_of_sensor;
         break;
      default :
         throw EAVParameterUnknown("bad multisensor_resolution");
      }
      return Register(registr);
   }
   return Register(0x67);
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::RequestTop()
{
   Page(3);
   if(pcamera->ispanoramic){
      unsigned registr = 0;
      switch(multisensor_resolution){
      case imFULL :
         registr = 0xD6;
         break;
      case imHALF :
         registr = 0xDA;
         break;
      case imZOOM :
         registr = 0xE4 + number_of_sensor;
         break;
      default :
         throw EAVParameterUnknown("bad multisensor_resolution");
      }
      return Register(registr);
   }
   return Register(0x68);
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::RequestWidth()
{
   Page(3);
   if(pcamera->ispanoramic){
      unsigned registr = 0;
      switch(multisensor_resolution){
      case imFULL :
         registr = 0xD5;
         break;
      case imHALF :
         registr = 0xD9;
         break;
      case imZOOM :
         registr = 0xE0 + number_of_sensor;
         break;
      default :
         throw EAVParameterUnknown("bad multisensor_resolution");
      }
      return Register(registr) - RequestLeft();
   }
   return Register(0x69) - RequestLeft();
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::RequestHeight()
{
   Page(3);
   if(pcamera->ispanoramic){
      unsigned registr = 0;
      switch(multisensor_resolution){
      case imFULL :
         registr = 0xD7;
         break;
      case imHALF :
         registr = 0xDB;
         break;
      case imZOOM :
         registr = 0xE8 + number_of_sensor;
         break;
      default :
         throw EAVParameterUnknown("bad multisensor_resolution");
      }
      return Register(registr) - RequestTop();
   }
   return Register(0x6A) - RequestTop();
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::RequestResolution()
{
   if(pcamera->ispanoramic)
      return multisensor_resolution;
   Page(3);
   return Register(0x6C);
}
//---------------------------------------------------------------------------
//Read default quality from firmware
//---------------------------------------------------------------------------
long AV2000ClientInherit1::RequestQuality()
{
   Page(3);
   return Register(0x6B);
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::RequestedBlockSize()
{
   return requested_block_size;
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::MdEnabled()
{
   long ret = 0;
   if(pcamera->ispanoramic){
      Page(6);
      ret = Register(PANORAMIC_MD_CTRL+number_of_sensor);
      Page(3);
   }
   else{
      Page(3);
      ret = Register(0x6D);
   }
   return ret & 1;
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::MdMode()
{
   long ret = 0;
   if(pcamera->ispanoramic){
      Page(6);
      ret = Register(PANORAMIC_MD_CTRL+number_of_sensor);
      Page(3);
   }
   else{
      Page(3);
      ret = Register(0x6D);
   }
   return  ret & 2;
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::MdLevelThresh()
{
   long ret = 0;
   if(pcamera->ispanoramic){
       Page(6);
       ret = Register(PANORAMIC_MD_THRESHOLDS+number_of_sensor);
       Page(3);
   }
   else{
       Page(3);
       ret = Register(0x6F);
   }
   return  ret & 0xFF;
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::MdTotalZones()
{
   long ret = 0;
   if(pcamera->ispanoramic){
       Page(6);
       ret = Register(PANORAMIC_MD_ZONES+number_of_sensor);
       Page(3);
   }
   else{
       Page(3);
       ret = Register(0x6E);
   }
   return  ret & 0xFF;
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::MdZoneSize()
{
   long ret = 0;
   if(pcamera->ispanoramic){
       Page(6);
       ret = Register(PANORAMIC_MD_ZONES + number_of_sensor);
       Page(3);
   }
   else{
       Page(3);
       ret = Register(0x6E);
   }
   ret >>= 8;
   return ret & 0xF;
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::MdDetail()
{
   long ret = 0;
   if(pcamera->ispanoramic){
       Page(6);
       ret = Register(PANORAMIC_MD_ZONES+number_of_sensor);
       Page(3);
   }
   else{
       Page(3);
       ret = Register(0x6D);
   }
   ret >>= 8;
   return ret;
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::MdExplosureSensitivity()
{
   long ret = 0;
   if(pcamera->ispanoramic){
       Page(6);
       ret = Register(PANORAMIC_MD_THRESHOLDS+number_of_sensor);
       Page(3);
   }
   else{
       Page(3);
       ret = Register(0x6F);
   }
   ret >>= 8;
   return ret;
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::MdMatrix()
{
   unsigned reg;
   if(pcamera->ispanoramic){
       Page(6);
       reg = Register(PANORAMIC_MD_PRIVACY_ZONES+number_of_sensor*4 + 0);
       Copy(reinterpret_cast<unsigned char*>(&reg), &md_matrix[0], 2);
       reg = Register(PANORAMIC_MD_PRIVACY_ZONES+number_of_sensor*4 + 1);
       Copy(reinterpret_cast<unsigned char*>(&reg), &md_matrix[2], 2);
       reg = Register(PANORAMIC_MD_PRIVACY_ZONES+number_of_sensor*4 + 2);
       Copy(reinterpret_cast<unsigned char*>(&reg), &md_matrix[4], 2);
       reg = Register(PANORAMIC_MD_PRIVACY_ZONES+number_of_sensor*4 + 3);
       Copy(reinterpret_cast<unsigned char*>(&reg), &md_matrix[6], 2);
       Page(3);
   }
   else {
       Page(3);
       reg = Register(0x21);
       Copy(reinterpret_cast<unsigned char*>(&reg), &md_matrix[0], 2);
       reg = Register(0x22);
       Copy(reinterpret_cast<unsigned char*>(&reg), &md_matrix[2], 2);
       reg = Register(0x23);
       Copy(reinterpret_cast<unsigned char*>(&reg), &md_matrix[4], 2);
       reg = Register(0x24);
       Copy(reinterpret_cast<unsigned char*>(&reg), &md_matrix[6], 2);
   }
   return reinterpret_cast<long>(md_matrix);
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::MsNumberOfSensor()
{
   return number_of_sensor + 1;
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::MsChannelEnable()
{
   Page(3);
   return Register(0xD0) & MsSensorAsBits(number_of_sensor);
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::MsFullResEnable()
{
   Page(3);
   return Register(0xD1) & MsSensorAsBits(number_of_sensor);
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::MsZoomWinEnable()
{
   Page(3);
   return Register(0xD2) & MsSensorAsBits(number_of_sensor);
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::MsOneShotEnable()
{
   Page(3);
   return Register(0xD3) & MsSensorAsBits(number_of_sensor);
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::MsIsZoomed()
{
   return multisensor_is_zoomed;
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::MsQuadMode()
{
   Page(3);
   return Register(0xF0) & 1 ? 1 : 0;
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::ShortExposures()
{
   unsigned int lght_shex;
   Page(3);
   if((pcamera->ispanoramic))
   {
          lght_shex = Register(0xF1 + number_of_sensor);
   }
   else
   {
          lght_shex = Register(0x25);
   }
   short_exposures = (lght_shex & 0xFF);
   return short_exposures;
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::TimeoutOnCamera()
{
   long ret;

   Page(3);
   if(pcamera->ispanoramic)
      ret = Register(0x60);
   else
      ret = Register(0x4F);

   ret = (ret & 0x7FF) * 10;

   return ret;
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::RepeatFromCamera()
{
   long ret;

   Page(3);
   if(pcamera->ispanoramic)
      ret = Register(0x60);
   else
      ret = Register(0x4F);

   ret >>= 12;

   return ret;
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::ScEnabled()
{
   if(!pcamera->mini)
      throw EAVNotSupported("parameter is not supported, only for mini-camera");
   Page(3);
   return Register(0x19) & 1;
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::ScStrobe()
{
   if(!pcamera->mini)
      throw EAVNotSupported("parameter is not supported, only for mini-camera");
   Page(3);
   return Register(0x19) & 128;
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::ScAlwaysSend()
{
   if(!pcamera->mini)
      throw EAVNotSupported("parameter is not supported, only for mini-camera");
   Page(3);
   return Register(0x19) & 2;
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::ScOutputHigh()
{
   if(!pcamera->mini)
      throw EAVNotSupported("parameter is not supported, only for mini-camera");
   Page(3);
   return Register(0x19) & 0x10;
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::ScOutputLow()
{
   if(!pcamera->mini)
      throw EAVNotSupported("parameter is not supported, only for mini-camera");
   Page(3);
   return Register(0x19) & 0x20;
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::ScOptoInput()
{
   if(!pcamera->mini)
      throw EAVNotSupported("parameter is not supported, only for mini-camera");
   Page(3);
   return Register(0x19) & 0x30;
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::GetCustomMode(long& knee_point, long& max_analog_gain, long& max_knee_gain, long& max_exposure_time, long& max_digital_gain)
{
   /*
   if(knee_point < 1 || knee_point > 100)
      throw EAVParameterOutOfRange("1 <= knee_point <= 100");
   if(max_analog_gain < 1 || max_analog_gain > 10)
      throw EAVParameterOutOfRange("1 <= max_analog_gain <= 10");
   if(max_knee_gain < max_analog_gain * 1.125 + 0.5 || max_knee_gain > max_analog_gain * 3.0 + 0.5)
      throw EAVParameterOutOfRange("max_analog_gain * 1.125 <= max_analog_gain <= max_analog_gain * 3.0");
   if(max_exposure_time < knee_point || max_exposure_time > 100)
      throw EAVParameterOutOfRange("knee_point <= max_exposure_time <= 100");
   */
   Page(3);
   knee_point = Register(11);
   max_analog_gain = Register(5) / 8;
   max_knee_gain = Register(13);
   max_exposure_time = Register(7);
   max_digital_gain = Register(8);
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::DoChangeCamera()
{
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::PatchRollForVersionLess51719(long roll)
{
      Page(1);
      Register(1, roll ? 0 : 6);
      if (2100 == Model() || AV2105 == Model())
      {
         Page(3);
         if( Register(0x7B) >= 51617 && Register(0x78) >= 50614 ){
            // hide dark cols
            Page(4);
            Register(30, roll ? 0x8040 : 0x80C0);
            Register(0x05, roll ? 35 : 16);
            // disable correction
            Page(2);
            Register(24, roll ? 16 : 176);
         }
         Page(4);
         Register(0x20, roll ? 0xD105 : 0x1105);
      }
      else{
         Page(4);
         Register(0x20, roll ? 0xC000 : 1);
      }
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::MaxOfDayNightTriggerNight(long low_light_mode)
{
   const unsigned SPEED = 0, QUALITY = 1, BALANCED = 2, CUSTOM = 3;
   switch(low_light_mode){
   case QUALITY :
      return 17;
   case SPEED :
      return 18;
   case BALANCED :
      return 14;
   }
   return 14;
}
//---------------------------------------------------------------------------

void AV2000ClientInherit1::CorrectLeft(const int max, const int divl, const int divw, long& left, long& width)
{
   width = RoundAuto(width, divw);
   if(left + width > max)
      left = max - width;
   left = RoundAuto(left, divl);
   if(left + width > max)
      left -= divl;
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::CorrectWidth(const int max, const int divl, const int divw, long& left, long& width)
{
   left = RoundAuto(left, divl);
   if(left + width > max)
      width = max - left;
   width = RoundAuto(width, divw);
   if(left + width > max)
      width -= divw;
}
//---------------------------------------------------------------------------
void AV2000ClientInherit1::Correct2100Sencor14RegisterBug()
{
   Page(3);
   unsigned r4 = Register(4);
   if(r4 * Register(7) > 16383)
      Register(7, 16383 / r4);
}
//---------------------------------------------------------------------------


void AV2000Calculate::DoChangeCamera()
{
   AV2000ClientInherit1::DoChangeCamera();
   quality->DoChangeCamera(*pcamera);
	if(Version() <= 192){
		quality->Set(imFULL, 3);
		quality->Set(imHALF, 5);
		quality->Set(imZOOM, 5);
	}

   if(!max_request_width || !max_request_height || !max_header_height)
      switch(pcamera->model){
      case 2000 :
         max_request_width = 1520;
         max_request_height = 1144;
         max_header_height = 1140;
         break;
      case 1300 :
      case AV1305 :
         max_request_width = 1280;
         max_request_height = 1024;
         max_header_height = 1024;
         break;
      case 2100 :
      case AV2105 :
         max_request_width = 1600;
         max_request_height = 1200;
         max_header_height = 1200;
         break;
      case 3100 :
      case AV3105 :      
         max_request_width = 1920;
         max_request_height = 1200;
         max_header_height = 1200;
         break;
      case 5100 :
      case AV5105 :      
         max_request_width = 2560;
         max_request_height = 1600;
         max_header_height = 1600;
         break;
      }

   int width = SensorWidth(),
       height = SensorHeight();

   if(max_request_width > width)
      max_request_width = width;
   if(max_request_height > height)
      max_request_height = max_header_height = height;



}
//---------------------------------------------------------------------------
void AddParameterToSendString(GString& msg, const GString& key, const GString& val)
{
	int length = msg.size();
	if(length >= 1 && msg[length-1] != '?') msg += '&';
	msg = msg + key + '=' + val;
}
//---------------------------------------------------------------------------
AV2000Calculate::AV2000Calculate(int n)
   :  AV2000ClientInherit1(n),
      divisible_width(16), divisible_height(8), resolution(imFULL),
      max_request_width(0), max_request_height(0), max_header_height(0),
      doublescan(0), filename("image"), quality(0), per_cent_size(false)
{
	quality = new Quality();
   /*
	quality->Set(imFULL, 3);
	quality->Set(imHALF, 5);
	quality->Set(imZOOM, 5);
   */
	zinfo.zoom = 1.0;
	zinfo.dx = zinfo.dy = 0;

   ssn = rand() % 65535 + 1;
}
//---------------------------------------------------------------------------
AV2000Calculate::~AV2000Calculate()
{
	delete quality;
}
//---------------------------------------------------------------------------
void AV2000Calculate::Resolution(IMAGE_RESOLUTION r)
{
	resolution = r;
}
//---------------------------------------------------------------------------
IMAGE_RESOLUTION AV2000Calculate::Resolution()
{
	return resolution;
}
//---------------------------------------------------------------------------
void AV2000Calculate::Zoom(const ZoomInfo& inf)
{
	if(inf.zoom > 1.0 || inf.zoom < 0.01 || inf.dx < -50 || inf.dx > 50 || inf.dy < -50 || inf.dy > 50)
		throw EAVParameterOutOfRange("Zoom parameters are incorrect");
	zinfo = inf;
}
//---------------------------------------------------------------------------
ZoomInfo AV2000Calculate::Zoom() const
{
	return zinfo;
}
//---------------------------------------------------------------------------
void AV2000Calculate::DoubleScan(long v)
{
	doublescan = v;
}
//---------------------------------------------------------------------------
long AV2000Calculate::DoubleScan()
{
	return doublescan;
}
//---------------------------------------------------------------------------
long AV2000Calculate::CorrectQuality(long value)
{
   if(value < 1)
      return 1;
   if(value > 21)
      return 21;
   return value;
}
//---------------------------------------------------------------------------
void AV2000Calculate::SetDynamicQuality(long value)
{
   use_dynamic_quality = 1;
   dynamic_quality = CorrectQuality(value);
}
//---------------------------------------------------------------------------
void AV2000Calculate::QualityFull(long value)
{
   value = CorrectQuality(value);

   if(pcamera->ispanoramic){
      Page(3);
      Register(0xED, value);
   }
   else
      if(Version() > 192)
          quality->Set(imFULL, value);
      else
	  throw EAVNotSupported("QualityFull() is not supported by this firmware version");
}
//---------------------------------------------------------------------------
void AV2000Calculate::QualityHalf(long value)
{
   value = CorrectQuality(value);

   if(pcamera->ispanoramic){
      Page(3);
      Register(0xEE, value);
   }
   else
      if(Version() > 192)
          quality->Set(imHALF, value);
      else
	  throw EAVNotSupported("QualityHalf() is not supported by this firmware version");
}
//---------------------------------------------------------------------------
void AV2000Calculate::QualityZoom(long value)
{
   value = CorrectQuality(value);

   if(pcamera->ispanoramic){
      Page(3);
      Register(0xEF, value);
   }
   else
      if(Version() > 192)
          quality->Set(imZOOM, value);
      else
	  throw EAVNotSupported("QualityZoom() is not supported by this firmware version");
}
//---------------------------------------------------------------------------
void AV2000Calculate::PerCentImageRectangle(long val)
{
   per_cent_size = val;
}
//---------------------------------------------------------------------------
void AV2000Calculate::Width(long val)
{
   if(val < 320 || val > sensor_size_max.Width(*pcamera))
      throw EAVParameterOutOfRange("cpWIDTH out of range");
   max_request_width = val;
}
//---------------------------------------------------------------------------
void AV2000Calculate::Height(long val)
{
   if(val < 240 || val > sensor_size_max.Height(*pcamera))
      throw EAVParameterOutOfRange("cpHEIGHT out of range");
   max_request_height = max_header_height = val;
}
//---------------------------------------------------------------------------
long AV2000Calculate::QualityFull()
{
   if(pcamera->ispanoramic){
      Page(3);
      return Register(0xED);
   }
   return (*quality)(imFULL);
}
//---------------------------------------------------------------------------
long AV2000Calculate::QualityHalf()
{
   if(pcamera->ispanoramic){
      Page(3);
      return Register(0xEE);
   }
   return (*quality)(imHALF);
}
//---------------------------------------------------------------------------
long AV2000Calculate::QualityZoom()
{
   if(pcamera->ispanoramic){
      Page(3);
      return Register(0xEF);
   }
   return (*quality)(imZOOM);
}
//---------------------------------------------------------------------------
long AV2000Calculate::PerCentImageRectangle()
{
   return per_cent_size;
}
//---------------------------------------------------------------------------
long AV2000Calculate::Width()
{
   return max_request_width;
}
//---------------------------------------------------------------------------
long AV2000Calculate::Height()
{
   return max_request_height;
}
//---------------------------------------------------------------------------
int AV2000Calculate::BlackWhite() const
{
   return black_white;
}
//---------------------------------------------------------------------------
void AV2000Calculate::CalculateZoomHeaderRect (float zoom, int dx, int dy, int coef, int& header_width, int& header_height, GRect& request_rect)
{
#ifdef _DEBUG
	if(zoom > 1.0) throw EAVParameterOutOfRange("::CalculateZoomRect -> zoom not valid");
#endif
	if(dx < -50) dx = -50;
	else if(dx > 50) dx = 50;
	if(dy < -50) dy = -50;
	else if(dy > 50) dy = 50;

	const unsigned ax = 32;
	unsigned kx = divisible_width * coef,
            ky = divisible_height * coef;

	int request_width = (int)( max_request_width * zoom + 0.5 ),
	    request_height = (int)( max_request_height * zoom + 0.5 );

	request_width = RoundUp(request_width, kx);
	if(request_width > max_request_width)
		request_width -= kx;
	request_height = RoundUp(request_height, ky);
	if(request_height > max_request_height)
		request_height -= ky;

	request_rect.left = (int)((max_request_width - request_width) * ((50 - dx) / 100.0));
	request_rect.left -= request_rect.left % ax;
	request_rect.right = request_rect.left + request_width;

	request_rect.top = (int)((max_request_height - request_height) * ((50 - dy) / 100.0));
	request_rect.top += request_rect.top % 2;
	request_rect.bottom = request_rect.top + request_height;

	header_width = request_rect.Width() / coef;
	header_height = request_rect.Height() / coef - (request_rect.bottom - max_header_height) * (request_rect.bottom > max_header_height);
}
//---------------------------------------------------------------------------






//---------------------------------------------------------------------------
void AV2000Client::DoChangeCamera()
{
   AV2000Calculate::DoChangeCamera();
}
//---------------------------------------------------------------------------
AV2000Client::AV2000Client(int n)
   : AV2000Calculate(n), memory(0), end_packet(0), iris_status(irUNKNOWN)
{
   //request = static_cast<GString>("image?res=full;x0=0;y0=0;x1=1920;y1=1200;quality=15;doublescan=0;ssn="  + ToString(ssn) + ';');

   //last_request = malloc(512);
   receive_begin_timeout = client->ReceiveTimeout();

	end_packet = new Packet;

   ResetMac();

   for(unsigned i = 0; i < motion_size; i++)
      motion_array[i] = i;

	handlers_set_property[cpGAMMA] = &AV2000ClientInherit1::Gamma;
	handlers_set_property[cpBRIGHTNESS] = &AV2000ClientInherit1::Brightness;
   handlers_set_property[cpAUTO_EXPOSITION] = &AV2000ClientInherit1::AutoExposition;
	handlers_set_property[cpSHARPNESS] = &AV2000ClientInherit1::Sharpness;
	handlers_set_property[cpSATURATION] = &AV2000ClientInherit1::Saturation;
	handlers_set_property[cpBLUE] = &AV2000ClientInherit1::Blue;
	handlers_set_property[cpRED] = &AV2000ClientInherit1::Red;
	handlers_set_property[cpILLUMINATION] = &AV2000ClientInherit1::Illumination;
	handlers_set_property[cpLIGHTING] = &AV2000ClientInherit1::Lighting;
	handlers_set_property[cpLIGHTINGFIX] = &AV2000ClientInherit1::Lightingfix;
	handlers_set_property[cpCAMERA_MODE] = &AV2000ClientInherit1::LowLightMode;
	handlers_set_property[cpQUALITY_FULL] = &AV2000Calculate::QualityFull;
	handlers_set_property[cpQUALITY_HALF] = &AV2000Calculate::QualityHalf;
	handlers_set_property[cpQUALITY_ZOOM] = &AV2000Calculate::QualityZoom;
	handlers_set_property[cpDOUBLESCAN] = &AV2000Calculate::DoubleScan;
   handlers_set_property[cpROLL] = &AV2000Calculate::Roll;

   handlers_set_property[cpIRIS_ENABLED] = &AV2000ClientInherit1::IrisEnable;
   handlers_set_property[cpIRIS_SPEED] = &AV2000ClientInherit1::IrisSpeed;
   handlers_set_property[cpIRIS_GAIN] = &AV2000ClientInherit1::IrisGain;
   //handlers_set_property[cpIRIS_PEROSITION_ENABLED] = &AV2000ClientInherit1::IrisReposEnable;
   handlers_set_property[cpIRIS_REPOSITION_F_STOPS] = &AV2000ClientInherit1::IrisReposFStops;
   handlers_set_property[cpIRIS_REPOSITION_F_STOPS_MIN] = &AV2000ClientInherit1::IrisReposFStopsMin;
   handlers_set_property[cpIRIS_REPOSITION_PERIOD] = &AV2000ClientInherit1::IrisReposPeriod;
   handlers_set_property[cpIRIS_REPOSITION_STABLE_PERIOD] = &AV2000ClientInherit1::IrisReposStabPeriod;

   handlers_set_property[cpDAY_NIGHT_MODE] = &AV2000ClientInherit1::DayNightMode;
   handlers_set_property[cpDAY_NIGHT_TRIGGER_NIGHT] = &AV2000ClientInherit1::DayNightTriggerNight;
   handlers_set_property[cpDAY_NIGHT_TRIGGER_DAY] = &AV2000ClientInherit1::DayNightTriggerDay;

   handlers_set_property[cpEXPOSURE_MODE] = &AV2000ClientInherit1::ExposureMode;
   handlers_set_property[cpEXPOSURE_WINDOW_LEFT] = &AV2000ClientInherit1::ExposureWindowLeft;
   handlers_set_property[cpEXPOSURE_WINDOW_TOP] = &AV2000ClientInherit1::ExposureWindowTop;
   handlers_set_property[cpEXPOSURE_WINDOW_WIDTH] = &AV2000ClientInherit1::ExposureWindowWidth;
   handlers_set_property[cpEXPOSURE_WINDOW_HEIGHT] = &AV2000ClientInherit1::ExposureWindowHeight;
   handlers_set_property[cpSENSOR_LEFT] = &AV2000ClientInherit1::SensorLeftChecked;
   handlers_set_property[cpSENSOR_TOP] = &AV2000ClientInherit1::SensorTopChecked;
   handlers_set_property[cpSENSOR_WIDTH] = &AV2000ClientInherit1::SensorWidthChecked;
   handlers_set_property[cpSENSOR_HEIGHT] = &AV2000ClientInherit1::SensorHeightChecked;
   handlers_set_property[cpSENSOR_BLACK_WHITE_LEFT] = &AV2000ClientInherit1::SensorBlackWhiteLeftChecked;
   handlers_set_property[cpSENSOR_BLACK_WHITE_TOP] = &AV2000ClientInherit1::SensorBlackWhiteTopChecked;
   handlers_set_property[cpSENSOR_BLACK_WHITE_WIDTH] = &AV2000ClientInherit1::SensorBlackWhiteWidthChecked;
   handlers_set_property[cpSENSOR_BLACK_WHITE_HEIGHT] = &AV2000ClientInherit1::SensorBlackWhiteHeightChecked;
   handlers_set_property[cpPER_CENT_IMAGE_RECTANGLE] = &AV2000Calculate::PerCentImageRectangle;
   handlers_set_property[cpREQUEST_LEFT] = &AV2000ClientInherit1::RequestLeftChecked;
   handlers_set_property[cpREQUEST_TOP] = &AV2000ClientInherit1::RequestTopChecked;
   handlers_set_property[cpREQUEST_WIDTH] = &AV2000ClientInherit1::RequestWidthChecked;
   handlers_set_property[cpREQUEST_HEIGHT] = &AV2000ClientInherit1::RequestHeightChecked;
   handlers_set_property[cpQUALITY] = &AV2000ClientInherit1::RequestQuality;
   handlers_set_property[cpRESOLUTION] = &AV2000ClientInherit1::RequestResolution;
   handlers_set_property[cpREQUESTED_BLOCK_SIZE] = &AV2000ClientInherit1::RequestedBlockSize;

   handlers_set_property[cpMD_ENABLED] = &AV2000ClientInherit1::MdEnabled;
   handlers_set_property[cpMD_MODE] = &AV2000ClientInherit1::MdMode;
   handlers_set_property[cpMD_LEVEL_THRESH] = &AV2000ClientInherit1::MdLevelThresh;
   handlers_set_property[cpMD_TOTAL_ZONES] = &AV2000ClientInherit1::MdTotalZones;
   handlers_set_property[cpMD_ZONE_SIZE] = &AV2000ClientInherit1::MdZoneSize;
   handlers_set_property[cpMD_DETAIL] = &AV2000ClientInherit1::MdDetail;
   handlers_set_property[cpMD_EXPLOSURE_SENSITIVITY] = &AV2000ClientInherit1::MdExplosureSensitivity;
   handlers_set_property[cpMD_MATRIX] = &AV2000ClientInherit1::MdMatrix;
   handlers_set_property[cpMS_NUMBER_OF_SENSOR] = &AV2000ClientInherit1::MsNumberOfSensor;
   handlers_set_property[cpMS_CHANNEL_ENABLE] = &AV2000ClientInherit1::MsChannelEnable;
   handlers_set_property[cpMS_FULL_RES_ENABLE] = &AV2000ClientInherit1::MsFullResEnable;
   handlers_set_property[cpMS_ZOOM_WIN_ENABLE] = &AV2000ClientInherit1::MsZoomWinEnable;
   handlers_set_property[cpMS_ONE_SHOT_ENABLE] = &AV2000ClientInherit1::MsOneShotEnable;
   handlers_set_property[cpMS_QUAD_MODE] = &AV2000ClientInherit1::MsQuadMode;
   handlers_set_property[cpWIDTH] = &AV2000Calculate::Width;
   handlers_set_property[cpHEIGHT] = &AV2000Calculate::Height;
   handlers_set_property[cpSHORT_EXPOSURES] = &AV2000Calculate::ShortExposures;
   handlers_set_property[cpTIMEOUT_ON_CAMERA] = &AV2000ClientInherit1::TimeoutOnCamera;
   handlers_set_property[cpREPEAT_FROM_CAMERA] = &AV2000ClientInherit1::RepeatFromCamera;
   handlers_set_property[cpSC_ENABLED] = &AV2000ClientInherit1::ScEnabled;
   handlers_set_property[cpSC_STROBE] = &AV2000ClientInherit1::ScStrobe;
   handlers_set_property[cpSC_ALWAYS_SEND] = &AV2000ClientInherit1::ScAlwaysSend;
   handlers_set_property[cpSC_OUTPUT_HIGH] = &AV2000ClientInherit1::ScOutputHigh;
   handlers_set_property[cpSC_OUTPUT_LOW] = &AV2000ClientInherit1::ScOutputLow;
   handlers_set_property[cpBIT_RATE] = &AV2000ClientInherit1::MsBitRate;


	handlers_get_property[cpGAMMA] = &AV2000ClientInherit1::Gamma;
	handlers_get_property[cpBRIGHTNESS] = &AV2000ClientInherit1::Brightness;
   handlers_get_property[cpAUTO_EXPOSITION] = &AV2000ClientInherit1::AutoExposition;
	handlers_get_property[cpSHARPNESS] = &AV2000ClientInherit1::Sharpness;
	handlers_get_property[cpSATURATION] = &AV2000ClientInherit1::Saturation;
	handlers_get_property[cpBLUE] = &AV2000ClientInherit1::Blue;
	handlers_get_property[cpRED] = &AV2000ClientInherit1::Red;
	handlers_get_property[cpILLUMINATION] = &AV2000ClientInherit1::Illumination;
	handlers_get_property[cpLIGHTING] = &AV2000ClientInherit1::Lighting;
	handlers_get_property[cpLIGHTINGFIX] = &AV2000ClientInherit1::Lightingfix;
	handlers_get_property[cpCAMERA_MODE] = &AV2000ClientInherit1::LowLightMode;
	handlers_get_property[cpQUALITY_FULL] = &AV2000Calculate::QualityFull;
	handlers_get_property[cpQUALITY_HALF] = &AV2000Calculate::QualityHalf;
	handlers_get_property[cpQUALITY_ZOOM] = &AV2000Calculate::QualityZoom;
	handlers_get_property[cpDOUBLESCAN] = &AV2000Calculate::DoubleScan;
   handlers_get_property[cpROLL] = &AV2000Calculate::Roll;

   handlers_get_property[cpIRIS_ENABLED] = &AV2000ClientInherit1::IrisEnable;
   handlers_get_property[cpIRIS_SPEED] = &AV2000ClientInherit1::IrisSpeed;
   handlers_get_property[cpIRIS_GAIN] = &AV2000ClientInherit1::IrisGain;
   //handlers_get_property[cpIRIS_PEROSITION_ENABLED] = &AV2000ClientInherit1::IrisReposEnable;
   handlers_get_property[cpIRIS_REPOSITION_F_STOPS] = &AV2000ClientInherit1::IrisReposFStops;
   handlers_get_property[cpIRIS_REPOSITION_F_STOPS_MIN] = &AV2000ClientInherit1::IrisReposFStopsMin;
   handlers_get_property[cpIRIS_REPOSITION_PERIOD] = &AV2000ClientInherit1::IrisReposPeriod;
   handlers_get_property[cpIRIS_REPOSITION_STABLE_PERIOD] = &AV2000ClientInherit1::IrisReposStabPeriod;

   handlers_get_property[cpDAY_NIGHT_MODE] = &AV2000ClientInherit1::DayNightMode;
   handlers_get_property[cpDAY_NIGHT_TRIGGER_NIGHT] = &AV2000ClientInherit1::DayNightTriggerNight;
   handlers_get_property[cpDAY_NIGHT_TRIGGER_DAY] = &AV2000ClientInherit1::DayNightTriggerDay;

   handlers_get_property[cpEXPOSURE_MODE] = &AV2000ClientInherit1::ExposureMode;
   handlers_get_property[cpEXPOSURE_WINDOW_LEFT] = &AV2000ClientInherit1::ExposureWindowLeft;
   handlers_get_property[cpEXPOSURE_WINDOW_TOP] = &AV2000ClientInherit1::ExposureWindowTop;
   handlers_get_property[cpEXPOSURE_WINDOW_WIDTH] = &AV2000ClientInherit1::ExposureWindowWidth;
   handlers_get_property[cpEXPOSURE_WINDOW_HEIGHT] = &AV2000ClientInherit1::ExposureWindowHeight;
   handlers_get_property[cpSENSOR_LEFT] = &AV2000ClientInherit1::SensorLeft;
   handlers_get_property[cpSENSOR_TOP] = &AV2000ClientInherit1::SensorTop;
   handlers_get_property[cpSENSOR_WIDTH] = &AV2000ClientInherit1::SensorWidth;
   handlers_get_property[cpSENSOR_HEIGHT] = &AV2000ClientInherit1::SensorHeight;
   handlers_get_property[cpSENSOR_BLACK_WHITE_LEFT] = &AV2000ClientInherit1::SensorBlackWhiteLeft;
   handlers_get_property[cpSENSOR_BLACK_WHITE_TOP] = &AV2000ClientInherit1::SensorBlackWhiteTop;
   handlers_get_property[cpSENSOR_BLACK_WHITE_WIDTH] = &AV2000ClientInherit1::SensorBlackWhiteWidth;
   handlers_get_property[cpSENSOR_BLACK_WHITE_HEIGHT] = &AV2000ClientInherit1::SensorBlackWhiteHeight;
   handlers_get_property[cpPER_CENT_IMAGE_RECTANGLE] = &AV2000Calculate::PerCentImageRectangle;
   handlers_get_property[cpREQUEST_LEFT] = &AV2000ClientInherit1::RequestLeft;
   handlers_get_property[cpREQUEST_TOP] = &AV2000ClientInherit1::RequestTop;
   handlers_get_property[cpREQUEST_WIDTH] = &AV2000ClientInherit1::RequestWidth;
   handlers_get_property[cpREQUEST_HEIGHT] = &AV2000ClientInherit1::RequestHeight;
   handlers_get_property[cpQUALITY] = &AV2000ClientInherit1::RequestQuality;
   handlers_get_property[cpRESOLUTION] = &AV2000ClientInherit1::RequestResolution;
   handlers_get_property[cpREQUESTED_BLOCK_SIZE] = &AV2000ClientInherit1::RequestedBlockSize;

   handlers_get_property[cpMD_ENABLED] = &AV2000ClientInherit1::MdEnabled;
   handlers_get_property[cpMD_MODE] = &AV2000ClientInherit1::MdMode;
   handlers_get_property[cpMD_LEVEL_THRESH] = &AV2000ClientInherit1::MdLevelThresh;
   handlers_get_property[cpMD_TOTAL_ZONES] = &AV2000ClientInherit1::MdTotalZones;
   handlers_get_property[cpMD_ZONE_SIZE] = &AV2000ClientInherit1::MdZoneSize;
   handlers_get_property[cpMD_DETAIL] = &AV2000ClientInherit1::MdDetail;
   handlers_get_property[cpMD_EXPLOSURE_SENSITIVITY] = &AV2000ClientInherit1::MdExplosureSensitivity;
   handlers_get_property[cpMD_MATRIX] = &AV2000ClientInherit1::MdMatrix;

   handlers_get_property[cpMS_NUMBER_OF_SENSOR] = &AV2000ClientInherit1::MsNumberOfSensor;
   handlers_get_property[cpMS_CHANNEL_ENABLE] = &AV2000ClientInherit1::MsChannelEnable;
   handlers_get_property[cpMS_FULL_RES_ENABLE] = &AV2000ClientInherit1::MsFullResEnable;
   handlers_get_property[cpMS_ZOOM_WIN_ENABLE] = &AV2000ClientInherit1::MsZoomWinEnable;
   handlers_get_property[cpMS_ONE_SHOT_ENABLE] = &AV2000ClientInherit1::MsOneShotEnable;
   handlers_get_property[cpMS_IS_ZOOMED] = &AV2000ClientInherit1::MsIsZoomed;
   handlers_get_property[cpMS_QUAD_MODE] = &AV2000ClientInherit1::MsQuadMode;

   handlers_get_property[cpWIDTH] = &AV2000Calculate::Width;
   handlers_get_property[cpHEIGHT] = &AV2000Calculate::Height;

   handlers_get_property[cpSHORT_EXPOSURES] = &AV2000Calculate::ShortExposures;
   handlers_get_property[cpTIMEOUT_ON_CAMERA] = &AV2000ClientInherit1::TimeoutOnCamera;
   handlers_get_property[cpREPEAT_FROM_CAMERA] = &AV2000ClientInherit1::RepeatFromCamera;

   handlers_get_property[cpSC_ENABLED] = &AV2000ClientInherit1::ScEnabled;
   handlers_get_property[cpSC_STROBE] = &AV2000ClientInherit1::ScStrobe;
   handlers_get_property[cpSC_ALWAYS_SEND] = &AV2000ClientInherit1::ScAlwaysSend;
   handlers_get_property[cpSC_OUTPUT_HIGH] = &AV2000ClientInherit1::ScOutputHigh;
   handlers_get_property[cpSC_OUTPUT_LOW] = &AV2000ClientInherit1::ScOutputLow;
   handlers_get_property[cpSC_OPTO_INPUT] = &AV2000ClientInherit1::ScOptoInput;

   handlers_get_property[cpBIT_RATE] = &AV2000ClientInherit1::MsBitRate;

}
//---------------------------------------------------------------------------
AV2000Client::~AV2000Client()
{
	delete end_packet;
}
//---------------------------------------------------------------------------
void AV2000Client::BufferImage(GBaseMemoryStream* ptr)
{
	memory = ptr;
}
//---------------------------------------------------------------------------
GBaseMemoryStream* AV2000Client::BufferImage() const
{
	return memory;
}
//---------------------------------------------------------------------------
void AV2000Client::OnCameraReinited()
{
   UpdateCamera();
   Page(3);
   if(pcamera->ispanoramic)
      Register(0x55, 0);
   else{
      Register(16, Register(16));  // correct bug for black images
      Register(14, 0);
   }
}
//---------------------------------------------------------------------------
const unsigned char* AV2000Client::MotionArray() const
{
   return motion_array;
}
//---------------------------------------------------------------------------
const unsigned char* AV2000Client::GetFirmwareTimestamp() const
{
   return pc_time_stamp;
}

//---------------------------------------------------------------------------
const unsigned char* AV2000Client::GetCameraMakeBuffer() const
{
   return pc_camera_make;
}
//---------------------------------------------------------------------------
const int AV2000Client::GetAuxInStatus() const
{
   return status_aux_in;
}

//---------------------------------------------------------------------------
const int AV2000Client::GetAuxOutStatus() const
{
   return status_aux_out;
}

//---------------------------------------------------------------------------
const unsigned char* AV2000Client::LastPacket() const
{
   return last_packet;
}
//---------------------------------------------------------------------------
const unsigned char* AV2000Client::Mac() const
{
   return mac;
}
//---------------------------------------------------------------------------
IRIS_STATUS AV2000Client::UpdateIrisStatus(GStream& in_strm, unsigned long idx)
{
   char* strm = in_strm.Buffer();
   GByte is = strm[idx],
        ir = strm[idx+1];

   if( !(is & 128) )
      return irDISABLED;
   if( !(is & 64) )
      return irMANUAL;

   if(0 == ir)
      return irIDLE;
   if(1 == ir)
      if(is & 32)
         return irEVALUATING_TOO_DARK;
      else
         return irEVALUATING;
   if(ir >= 2 && ir <= 3)
      return irCLOSING;
   if(ir >= 4 && ir <= 5)
      return irCLOSED;
   if(ir >= 6 && ir <= 8)
      return irOPENING;

   return irUNKNOWN;
}
//---------------------------------------------------------------------------
void AV2000Client::ResetMac()
{
   mac[0] = mac[1] = mac[2] = mac[3] = mac[4] = mac[5] = 0;
   for(int i=0; i<120; i++)
      last_packet[i] = 0;
}
//---------------------------------------------------------------------------
bool AV2000Client::isH264Model() const
{
        return AV1305 == pcamera->model ||  AV2105 == pcamera->model || AV3105 == pcamera->model || AV5105 == pcamera->model || AV8185 == pcamera->model || AV8365 == pcamera->model || AV3135 == pcamera->model;
}
//---------------------------------------------------------------------------
void AV2000Client::Image(CodecID codec, int streamId, int* Ifarme)
{
    if (mprevCodec!=codec) // we have to ask I frame in case of change codec; first time it will work automaticaly
    {
            makeAskIFrameFirst();
            mprevCodec = codec;

    }
    
   if(pcamera->ispanoramic){
      ImageDefault(codec, streamId, Ifarme);
      return;
      //throw EAVException("AV8360/8180/8365/8185 supported only ImageDefault");
   }

   if (codec == H264_CODEC && !isH264Model() )
      throw EAVException("this camera model do not support H264");

    BeforeImage(codec, streamId, Ifarme);
    client->Get(request, *memory);
    AfterImage(codec, Ifarme);
}
//---------------------------------------------------------------------------
void AV2000Client::Image(int left, int top, int width, int height, CodecID codec, int streamId, int* Ifarme, int kbps, int intraFramePeriod)
{
    if (mprevCodec!=codec) // we have to ask I frame in case of change codec; first time it will work automaticaly
    {
            makeAskIFrameFirst();
            mprevCodec = codec;

    }

   if(pcamera->ispanoramic){
      ImageDefault(codec, streamId, Ifarme);
      return;
      //throw EAVException("AV8360/8180/8365/8185 supported only ImageDefault");
   }

   if (codec == H264_CODEC && !isH264Model())
      throw EAVException("this camera model do not support H264");

	BeforeImage(left, top, width, height, codec, streamId, Ifarme, kbps, intraFramePeriod);  //prepare memory position
	client->Get(request, *memory);
	AfterImage(codec, Ifarme);
}
//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
void AV2000Client::SetChannel(int channel)
{
    m_channel = channel;
}
//---------------------------------------------------------------------------
void AV2000Client::ImageDefault(CodecID codec, int streamId, int* Ifarme)
{
      if (mprevCodec!=codec) // we have to ask I frame in case of change codec; first time it will work automaticaly
      {
              makeAskIFrameFirst();
              mprevCodec = codec;
      }

     if(3130 == pcamera->model || AV3135 == pcamera->model)
        throw EAVNotSupported("AV3130 do not support ImageDefault");

     if (codec == H264_CODEC && !isH264Model())
        throw EAVException("this camera model do not support H.264");

      BeforeImageDefault((CodecID)codec, streamId, Ifarme);
      client->Get(request, *memory);
      AfterImage(codec, Ifarme);
}
//---------------------------------------------------------------------------
void AV2000Client::BeforeImage(CodecID codec, int streamId, int* Ifarme)
{
        ResetMac();
	request = CalculateRequest(codec,streamId, Ifarme );
	memory->Size(595+EXIF_LENGTH);

        if (codec==JPEG_CODEC)
	        memory->Position(begin_memory_position = memory->Size() - 2);
        else
                memory->Position(begin_memory_position = max_sps_pps_len);

}
//---------------------------------------------------------------------------
//Check input range and divisibility
//---------------------------------------------------------------------------
void AV2000Client::BeforeImage(int left, int top, int width, int height, CodecID codec, int streamId, int* Ifarme, int kbps, int intraFramePeriod)
{
   if(left < 0) left = 0;
   if(top < 0) top = 0;

   if(per_cent_size && (3130 == Model() || AV3135 == Model() ) )
   {
      if(left + width > 100)
         throw EAVParameterOutOfRange("left or/and width out of range");
      if(top + height > 100)
         throw EAVParameterOutOfRange("top or/and height out of range");
   }
   else{
      int div = (imFULL == resolution) ? 16 : 32,
          half_div = div / 2,
          vertical_div = 8;
      unsigned long mask = (imFULL == resolution) ? ~0x0f : ~0x1f,
          half_mask = mask >> 1,
          mask16 = ~0x0f,
          mask8 = ~0x07;

      left &= mask16;
      if(width > sensor_geometry.width){
         left=0; width = sensor_geometry.width;
      }
      width &= mask;
      if(left + width > sensor_geometry.width){
         left=0; width = sensor_geometry.width;
      }
      top &= ~1;
      if(height > sensor_geometry.height){
         top=0; height = sensor_geometry.height;
      }
      height &= mask8;
      if(top + height > sensor_geometry.height){
         top=0;  height = sensor_geometry.height;
      }
   }

   ResetMac();
	request = CalculateRequest(left, top, width, height, codec,streamId, Ifarme, kbps, intraFramePeriod);
	memory->Size(595+EXIF_LENGTH);

        if (codec==JPEG_CODEC)
	        memory->Position(begin_memory_position = memory->Size() - 2);
        else
                memory->Position(begin_memory_position = max_sps_pps_len);
}
//---------------------------------------------------------------------------
void AV2000Client::BeforeImageDefault(CodecID codec, int streamId, int* Ifarme)
{
   ResetMac();
   if(!pcamera->ispanoramic)
   {
      int div_header;
      switch(request_resolution){
      case imFULL :
         div_header = 1;
         break;
      case imHALF :
         div_header = 2;
         break;
      default:
         throw EAVCameraUnknown("cpRESOLUTION is unknown");
      }
      const int as_resolution = imZOOM + 1;
      quality->Set(static_cast<IMAGE_RESOLUTION>(as_resolution), request_quality);
      if(3130 != Model() && AV3135 != Model())
	      quality->LoadHeader(request_width / div_header, request_height / div_header, static_cast<IMAGE_RESOLUTION>(as_resolution));
   }

    memory->Size(595+EXIF_LENGTH);
    if (codec==JPEG_CODEC)
            memory->Position(begin_memory_position = memory->Size() - 2);
    else
            memory->Position(begin_memory_position = max_sps_pps_len);



   filename = (codec == H264_CODEC) ? "h264" : "image";

   request = filename + '?';

   if (codec == H264_CODEC)
   {
        //ssn for h264 started from 1
        char idbuff[32];
        sprintf(idbuff, "%d", streamId+1);  
        AddParameterToSendString(request, "ssn", idbuff); 

        //if (mMpegHelper.haveToBeIframe(width, height, streamId) || (*Ifarme) > 0  )
        sprintf(idbuff, "%d", (*Ifarme) > 0 ? 1:0);  
        AddParameterToSendString(request, "iframe", idbuff);

        request = ReplaceSymbol(request, '&', ';') + ';';
   }

}
//---------------------------------------------------------------------------
GString AV2000Calculate::CalculateRequest(CodecID codec, int streamId, int* Ifarme)
{
	GRect zoom_rect, request_rect;
        bool res_half = false;

	int header_width, header_height;
	switch(resolution)
        {
		case imZOOM :
			if(zinfo.zoom  < 0.5)
                        {
                                if(3130 == Model() || AV3135 == Model())
                                   request_rect = CalculateZoomHeaderRect3130(1920, 1200, 1.0 / zinfo.zoom, zinfo.dx, zinfo.dy, 1);
                                else
				   CalculateZoomHeaderRect(zinfo.zoom, zinfo.dx, zinfo.dy, 1, header_width, header_height, request_rect);
				//AddParameterToSendString(URL, "res", "full");
			}
			else
                        {
                                if(3130 == Model() || AV3135 == Model())
                                   request_rect = CalculateZoomHeaderRect3130(1920, 1200, 1.0 / zinfo.zoom, zinfo.dx, zinfo.dy, 2);
                                else
				   CalculateZoomHeaderRect(zinfo.zoom, zinfo.dx, zinfo.dy, 2, header_width, header_height, request_rect);
				//AddParameterToSendString(URL, "res", "half");
                                res_half = true;
			}
			break;

		case imHALF :
                        if(3130 != Model() && AV3135 != Model())
			   CalculateZoomHeaderRect(1.0, 0, 0, 2, header_width, header_height, request_rect);
			//AddParameterToSendString(URL, "res", "half");
                        res_half = true;
			break;

		case imFULL :
                        if(3130 != Model() && AV3135 != Model())
			   CalculateZoomHeaderRect(1.0, 0, 0, 1, header_width, header_height, request_rect);
			//AddParameterToSendString(URL, "res", "full");
			break;

		default:
			throw EAVParameterOutOfRange("Unknown regime");
	}

        //======================================
        filename = (codec == H264_CODEC) ? "h264" : "image";

        GString URL = filename + '?';
        if (res_half)
                AddParameterToSendString(URL, "res", "half");
        else
                AddParameterToSendString(URL, "res", "full");

        //=======================================

         if(3130 == Model() || AV3135 == Model())
         {
            if(imZOOM == resolution)
               AddRectToRequest3130(URL, request_rect);
         }
         else
         {
#ifdef _windows_
	   AddParameterToSendString(URL, "x0", ToString<GString>(request_rect.left));
	   AddParameterToSendString(URL, "y0", ToString<GString>(request_rect.top));
	   AddParameterToSendString(URL, "x1", ToString<GString>(request_rect.right));
	   AddParameterToSendString(URL, "y1", ToString<GString>(request_rect.bottom));
#else
	   AddParameterToSendString(URL, "x0", ToString(request_rect.left));
	   AddParameterToSendString(URL, "y0", ToString(request_rect.top));
	   AddParameterToSendString(URL, "x1", ToString(request_rect.right));
	   AddParameterToSendString(URL, "y1", ToString(request_rect.bottom));
#endif

         }

	if(Version() > 192)
        {
                GString qual = "quality";
                int q = (*quality)(resolution);
                if (codec == H264_CODEC){   q = 37 - q;  qual = "qp";};
                if (codec == H264_CODEC){ qual = "qp"; ;};
#ifdef _windows_
		AddParameterToSendString(URL, qual, ToString<GString>( q ));
#else
		AddParameterToSendString(URL, qual, ToString( q ));
#endif
        }

	if(Version() >= 40820)
        {
#ifdef _windows_
	AddParameterToSendString(URL, "doublescan", ToString<GString>(doublescan));
#else
	AddParameterToSendString(URL, "doublescan", ToString(doublescan));
#endif
        }

        if (codec == H264_CODEC)
        {
               int width  = request_rect.right - request_rect.left;
               int height = request_rect.bottom - request_rect.top  ;

               if (res_half)
               {
                  width=width/2;
                  height=height/2;
               }

               if (mMpegHelper.haveToBeIframe(width, height, streamId) || (*Ifarme) > 0  )
#ifdef _windows_
                      AddParameterToSendString(URL, "iframe", ToString<GString>(1));
#else
                      AddParameterToSendString(URL, "iframe", ToString(1));
#endif
               else
#ifdef _windows_
                      AddParameterToSendString(URL, "iframe", ToString<GString>(0));
#else
                      AddParameterToSendString(URL, "iframe", ToString(0));
#endif
        }


        if(Version() >= 62019)
        {
                if (codec == H264_CODEC)
#ifdef _windows_
                        AddParameterToSendString(URL, "ssn", ToString<GString>(streamId+1)); 
#else
                        AddParameterToSendString(URL, "ssn", ToString(streamId+1)); 
#endif

                else
#ifdef _windows_
                        AddParameterToSendString(URL, "ssn", ToString<GString>(ssn));
#else
                        AddParameterToSendString(URL, "ssn", ToString(ssn));
#endif
        }

        URL = ReplaceSymbol(URL, '&', ';') + ';';

        if(3130 != Model() && AV3135 != Model())
                 quality->LoadHeader(header_width, header_height, resolution);

#ifdef _DEBUG
   if(3130 != Model() && AV3135 != Model())
   {
	   if(resolution != imZOOM && resolution != imHALF && resolution != imFULL)
		   throw EAVParameterOutOfRange("regime unknown");
	   if(GString::npos != URL.find("full") && (request_rect.Width() % 16 || request_rect.Height() % 8))
		   throw EAVParameterOutOfRange(": request_rect, regime: imFULL -> not divisible");
	   if(GString::npos != URL.find("half") && (request_rect.Width() % 32 || request_rect.Height() % 16))
		   throw EAVParameterOutOfRange(": request_rect, regime: imHALF -> not divisible");
	   if(request_rect.right > max_request_width)
		   throw EAVParameterOutOfRange(": request_rect, right too big");
   }
#endif

	return URL;

}
//---------------------------------------------------------------------------
//URL=image?res=full;x0=0;y0=0;x1=320;y1=240;quailty=15;ssn=241;
//---------------------------------------------------------------------------
void AV2000Calculate::makeAskIFrameFirst()
{
        mMpegHelper.haveToBeIframeAll();
}
//---------------------------------------------------------------------------
GString AV2000Calculate::CalculateRequest(int left, int top, int width, int height, CodecID codec, int streamId, int* Ifarme, int kbps, int intraFramePeriod, int channel)
{
    GRect zoom_rect, request_rect;
    //======================================
    filename = (codec == H264_CODEC) ? "h264" : "image";

    //=======================================

    GString URL = filename + '?';
   int div_header;

   switch(resolution)
   {
   case imHALF :
      div_header = 2;
      AddParameterToSendString(URL, "res", "half");
      break;
   case imFULL :
      div_header = 1;
      AddParameterToSendString(URL, "res", "full");
      break;
   default :
      throw EAVParameterOutOfRange("IMAGE_RESOLUTION must be imHALF or imFULL");
   }

   if(3130 == Model() || AV3135 == Model())
   {
           AddRectToRequest3130(URL, GRect(left, top, left + width, top + height));
   }
   else
   {
#ifdef _windows_
	   AddParameterToSendString(URL, "x0", ToString<GString>(left));
	   AddParameterToSendString(URL, "y0", ToString<GString>(top));
	   AddParameterToSendString(URL, "x1", ToString<GString>(left + width));
	   AddParameterToSendString(URL, "y1", ToString<GString>(top + height));
#else
	   AddParameterToSendString(URL, "x0", ToString(left));
	   AddParameterToSendString(URL, "y0", ToString(top));
	   AddParameterToSendString(URL, "x1", ToString(left + width));
	   AddParameterToSendString(URL, "y1", ToString(top + height));
#endif 
   }

    if(Version() > 192)
    {
        int q ;
        GString qual = "quality";
        if(use_dynamic_quality)
           q = dynamic_quality;
        else
           q = (*quality)(resolution);;
        if (codec == H264_CODEC){   q = 37 - q;  qual = "qp";};
        if (codec == H264_CODEC){ qual = "qp"; ;};
        #ifdef _windows_
          AddParameterToSendString(URL, qual, ToString<GString>( q ));
        #else
          AddParameterToSendString(URL, qual, ToString( q ));
        #endif 

    }


    if(Version() >= 40820)
        #ifdef _windows_
            AddParameterToSendString(URL, "doublescan", ToString<GString>(doublescan));
        #else
            AddParameterToSendString(URL, "doublescan", ToString(doublescan));
        #endif

    if (codec == H264_CODEC )
    {
           if (resolution == imHALF)
           {
              width=width/2;
              height=height/2;
           }

           if (mMpegHelper.haveToBeIframe(width, height, streamId) || (*Ifarme) > 0  )
           #ifdef _windows_  
                  AddParameterToSendString(URL, "iframe", ToString<GString>(1));
           #else
                  AddParameterToSendString(URL, "iframe", ToString(1));
           #endif 
           else
           #ifdef _windows_  
                  AddParameterToSendString(URL, "iframe", ToString<GString>(0));
           #else
                  AddParameterToSendString(URL, "iframe", ToString(0));
           #endif 

           if (kbps)
           {
           #ifdef _windows_  
                AddParameterToSendString(URL, "bitrate", ToString<GString>(kbps));
                AddParameterToSendString(URL, "intra_period", ToString<GString>(intraFramePeriod));
           #else
                AddParameterToSendString(URL, "bitrate", ToString(kbps));
                AddParameterToSendString(URL, "intra_period", ToString(intraFramePeriod));
           #endif 
           }
    }


   if(Version() >= 62019)
   {
          if (codec == H264_CODEC)
           #ifdef _windows_  
                  AddParameterToSendString(URL, "ssn", ToString<GString>(streamId +1 ));//ssn for h264 started from 1
           #else
                  AddParameterToSendString(URL, "ssn", ToString(streamId +1 ));//ssn for h264 started from 1
           #endif 

          else
           #ifdef _windows_  
                  AddParameterToSendString(URL, "ssn", ToString<GString>(ssn));
           #else
                  AddParameterToSendString(URL, "ssn", ToString(ssn));
           #endif 

   }

   URL = ReplaceSymbol(URL, '&', ';') + ';';

   if(3130 != Model() && AV3135 != Model())
   {
      if(use_dynamic_quality)
      {
	   quality->LoadHeaderByQuality(width / div_header, height / div_header,dynamic_quality);
      }
      else
      {
    	   quality->LoadHeader(width / div_header, height / div_header, resolution);
      }
      use_dynamic_quality = 0;
   }
   return URL;
}
//---------------------------------------------------------------------------
void AV2000Calculate::AddRectToRequest3130(GString& URL, const GRect& request_rect)
{
   int startx=0, starty=0, endx=320, endy=240, limitx=320, limity=240;
   int div = (imFULL == resolution) ? 16 : 32, half_div = div / 2;

   if(per_cent_size){
           #ifdef _windows_  
	   AddParameterToSendString(URL, "x0", ToString<GString>(request_rect.left) + '%');
	   AddParameterToSendString(URL, "y0", ToString<GString>(request_rect.top) + '%');
	   AddParameterToSendString(URL, "x1", ToString<GString>(request_rect.right) + '%');
	   AddParameterToSendString(URL, "y1", ToString<GString>(request_rect.bottom) + '%');
           #else
	   AddParameterToSendString(URL, "x0", ToString(request_rect.left) + '%');
	   AddParameterToSendString(URL, "y0", ToString(request_rect.top) + '%');
	   AddParameterToSendString(URL, "x1", ToString(request_rect.right) + '%');
	   AddParameterToSendString(URL, "y1", ToString(request_rect.bottom) + '%');
           #endif 
   }
   else{
      limitx = (request_rect.right - request_rect.left);  //width limit, 1920
      limity = (request_rect.bottom - request_rect.top);  //height limit, 1536
      if(BlackWhite()){
         div = 32, half_div = div / 2;
         if(limitx >= 1920){
            startx = 0;
            limitx = 1280;
         }
         else{
            int trimed = std::max(request_rect.left-64,0);
            startx = (int)(trimed/1.5+0.5) & ~1;
                startx -= startx % div;
            limitx = (int)(limitx/1.5+0.5);             //scale width
               limitx = std::min(limitx, 1280);
         }
         starty = (int)(request_rect.top/1.5+0.5) & ~1;
             starty -= starty % half_div;
         limity = (int)(limity/1.5+0.5);                 //scale height
            limity = std::min(limity, 1024);
         limitx -=  std::max(startx + limitx - 1280, 0);
             limitx -= limitx % div;
         limity -=  std::max(starty + limity - 1024, 0);
             limity -= limity % half_div;
         endx = (startx+limitx);
         endy = (starty+limity);
      }
      else{  //color mode
         limitx = std::min(limitx, 2048);
         limity = std::min(limity, 1536);
         startx = (request_rect.left); //2048x1536
             startx -= startx % div;
         starty = (request_rect.top);
             starty -= starty % half_div;
         limitx -=  std::max(startx + limitx - 2048, 0) ;
             limitx -= limitx % div;
         limity -=  std::max(starty + limity - 1536, 0) ;
             limity -= limity % half_div;
         //if(limity == 1088 || limity == 1072)
         //    limity = 1080;
         endx = (startx+limitx);
         endy = (starty+limity);
      }
      #ifdef _windows_  
      AddParameterToSendString(URL, "x0", ToString<GString>(startx));
      AddParameterToSendString(URL, "y0", ToString<GString>(starty));
      AddParameterToSendString(URL, "x1", ToString<GString>(endx));
      AddParameterToSendString(URL, "y1", ToString<GString>(endy));
      #else
      AddParameterToSendString(URL, "x0", ToString(startx));
      AddParameterToSendString(URL, "y0", ToString(starty));
      AddParameterToSendString(URL, "x1", ToString(endx));
      AddParameterToSendString(URL, "y1", ToString(endy));
      #endif 
   }
}
//---------------------------------------------------------------------------
void AV2000Client::AfterImage(CodecID codec, int* Ifarme)
{

    int iframe_index = (AV8365 == Model() || AV8185 == Model()) ? 89 : 93;

    switch(Model())
    {
    case AV8365:
    case AV8185:
        iframe_index = 89;
        break;
    case AV3135:
        iframe_index = 88;
        break;
    default:
        iframe_index = 93;

    }

    if (codec==H264_CODEC && (end_packet->Size() < iframe_index))
        throw EAVException("last packet is too short!");
    
    if(end_packet->Number())
    {
        unsigned long size_of_data_buffer = begin_memory_position + (end_packet->Number() - 1) * client->Current()->RequestedBlockSize();
	memory->Size(size_of_data_buffer + end_packet->Size());

        if (codec==H264_CODEC)
                *Ifarme = (*memory)[  size_of_data_buffer + iframe_index - 1 ];


       if(Version() >= 40820)
          OverheadInformation();

       unsigned char sps_pps[max_sps_pps_len];
       int sps_pps_len = 0;
       if (codec==H264_CODEC && (*Ifarme)) // sps&pps only needed in the case of I frame.
        sps_pps_len = createSPS_PPS(sps_pps);

       if (codec==H264_CODEC)
       {
          unsigned char* data = (unsigned char*)memory->Buffer()  + begin_memory_position;

          if (!(data[0] == 0 && data[1] == 0)) // if header is not present
          {
              // we have to put start code and first byte of slice header of the frame anyway for I and P frames
              unsigned char* begining_of_theslice = sps_pps + sps_pps_len;
              begining_of_theslice[0] = begining_of_theslice[1] = begining_of_theslice[2] = 0; begining_of_theslice[3] = 1;
              begining_of_theslice[4] = (*Ifarme) ? 0x65 : 0x41;
              sps_pps_len+=5;
          }
       }


       if(end_packet->Number() > 1)
       {
         memory->Size(memory->Size() - end_packet->Size());


         if (codec==JPEG_CODEC)
         {
                memory->Position(0);
                quality->WriteHeader(*memory);
                memory->Position(0);
                
                unsigned int *pmem = reinterpret_cast<unsigned int*>(memory->Buffer());
                unsigned int chksum = 0, offset = 0, chksum_pos = 0x1A0 / sizeof(unsigned int);
                for(int i=0; i<chksum_pos; i++)
                   chksum += pmem[i];
                offset = 512 / sizeof(unsigned int);
                for(int i=offset; i<offset+offset; i++)
                   chksum += pmem[i];
                offset = 1024 / sizeof(unsigned int);
                for(int i=offset; i<size_of_data_buffer/sizeof(unsigned int); i+=offset)
                   chksum += pmem[i];
                pmem[chksum_pos] = chksum;
         }
         else //H264 we have to remove zerows at the and of last packet
         {
                unsigned char* data = (unsigned char*)memory->Buffer();
                int len = memory->Size();

                int zeros = 0;
                for (;;)
                {
                        if (data[len-1-zeros] == 0)
                                ++zeros;
                        else
                                break;

                }
                len-=zeros;

                memory->Size(len + sizeof(unit_del)); // we have to put unit_delimetr
                memory->Position(len);
                memory->Write((char*)unit_del,sizeof(unit_del));


                //we have to put sps_pps; note if it is not I frame then sps_pps_len = 0;
                memory->Position(begin_memory_position - sps_pps_len);
                memory->Write((char*)sps_pps,sps_pps_len);
                memory->Position(begin_memory_position - sps_pps_len);
         }
       }
       else
         memory->Size(0);
    }
}
//---------------------------------------------------------------------------
unsigned int AV2000Client::createSPS_PPS(unsigned char* data) //sergey
{
        unsigned long idx = (end_packet->Number() - 1) * client->Current()->RequestedBlockSize() + begin_memory_position;

    Size size;
    IMAGE_RESOLUTION res;

    if(AV8365 == pcamera->model || AV8185 == pcamera->model)
    {
        GByte* arr = reinterpret_cast<unsigned char*>(memory->Buffer() + idx + 0x0C);
        arr[0] & 4 ? res = imFULL : res = imHALF;
        size = ExtractSize(&arr[2]);
    }
    else if(AV3135 == pcamera->model )
    {
        size = ExtractSize(reinterpret_cast<unsigned char*>(memory->Buffer() + idx + 12));
    }
    else
    {
        GByte* arr = reinterpret_cast<GByte*>(memory->Buffer() + idx + 0x0C + 4 + 64);
        IMAGE_RESOLUTION res;
        arr[0] & 4 ? res = imFULL : res = imHALF;
        size = ExtractSize(&arr[2]);

        if(!size.width && 3100 == pcamera->model)
        size.width = 2048;

        if(imHALF == res)
        {
          size.width /= 2;
          size.height /= 2;
        }
    }


         return  create_sps_pps( size.width,size.height, 0,  data, max_sps_pps_len);

}
//---------------------------------------------------------------------------
void AV2000Client::OverheadInformation()
{
    unsigned long idx = begin_memory_position + (end_packet->Number() - 1) * client->Current()->RequestedBlockSize();

    // in case of h264 we have to add some data(UD) to the end of frame data; it makes
    //possible situaiton we can erase last_packet
    // to provide getLastPacket works correct we have to copy last packet to the other place
    int last_packet_size = end_packet->Size();
    unsigned char* plast_packet = reinterpret_cast<unsigned char*>(memory->Buffer() + idx);

    memcpy(last_packet, plast_packet, last_packet_size);

    if (last_packet[0]&1) // if camera restart is happend we should ask I frame;  i don't know Volodya MUST do it in camera
        makeAskIFrameFirst();

    if(Version() >= 40820){
	   if(memory->Size() - idx >= 2 && ((*memory)[idx] & 1))
		   OnCameraReinited();

      mac[0] = (*memory)[idx+6];
      mac[1] = (*memory)[idx+7];
      mac[2] = (*memory)[idx+8];
      mac[3] = (*memory)[idx+9];
      mac[4] = (*memory)[idx+10];
      mac[5] = (*memory)[idx+11];

      if(3130 == Model() || AV3135 == Model())
      {
         Size size = ExtractSize(reinterpret_cast<unsigned char*>(memory->Buffer() + idx + 12));
         if(use_dynamic_quality)
            quality->LoadHeaderByQuality(size.width, size.height, dynamic_quality);
         else
            quality->LoadHeader(size.width, size.height, resolution);
         use_dynamic_quality = 0;
         switch(static_cast<GByte>((*memory)[idx+20])){
         case 0 : // color (3130) sensor
            black_white = 0;
            break;
         case 1 : // black-white (1300) sensor
            black_white = 1;
            break;
         case 2 :
            // panoramic camera - in the future
            break;
         }
      }
      else
         black_white = 0;

      if(8360 == pcamera->model || 8180 == pcamera->model || AV8365 == pcamera->model || AV8185 == pcamera->model){                     // íå èìåå?iris
         GByte* arr = reinterpret_cast<unsigned char*>(memory->Buffer() + idx + 0x0C) /* + (4 + 64)  a1 ! */;
         number_of_sensor = arr[0] & 3;
         multisensor_is_zoomed = arr[0] & 8;
         IMAGE_RESOLUTION res;
         arr[0] & 4 ? res = imFULL : res = imHALF;
         Size size = ExtractSize(&arr[2]);


         //if(imHALF == res) { size.width /= 2; size.height /= 2; }

         quality->LoadHeaderByQuality(size.width, size.height, arr[1]);
      }


      if( !pcamera->ispanoramic && 3130 != pcamera->model && AV3135 != pcamera->model && pcamera->version >= 61315){
         GByte* arr = reinterpret_cast<GByte*>(memory->Buffer() + idx + 0x0C + 4 + 64);
         IMAGE_RESOLUTION res;
         arr[0] & 4 ? res = imFULL : res = imHALF;
         Size size = ExtractSize(&arr[2]);

         if(!size.width && 3100 == pcamera->model)
            size.width = 2048;

         if(imHALF == res){
            size.width /= 2;
            size.height /= 2;
         }
         quality->LoadHeaderByQuality(size.width, size.height, arr[1]);
      }


      if( (1300 == Model() ||
           2100 == Model() ||
           3100 == Model() ||
           5100 == Model() ||
           AV1305 == Model() ||
           AV2105 == Model() ||
           AV3105 == Model() ||
           AV5105 == Model() 
           ) && Version() >= 51531)
         iris_status = UpdateIrisStatus(*memory, idx + 12);

      int mm = 0;
      if( (1300 == Model() ||
           2100 == Model() ||
           3100 == Model() ||
           5100 == Model() ||
           AV1305 == Model() ||
           AV2105 == Model() ||
           AV3105 == Model() ||
           AV5105 == Model()
           ) && Version() >= 52220)
         mm = idx+0x0C+4;
      else if( pcamera->ispanoramic)
         mm = idx+24;
      else if( (3130 == pcamera->model || AV3135 == pcamera->model) && pcamera->version >= 61503)
            mm = idx + 21;

      if(mm)
         if(memory->Size() >= mm + motion_size){
            for(unsigned i = 0; i < motion_size; i++){
               motion_array[i] = (*memory)[mm+i];
            }
            /*
            FILE *fp=fopen("motion.txt", "a+");
            for(unsigned i = 0; i < 64; i++){
               fprintf(fp,"%02X ", motion_array[i]);
            }
            fprintf(fp,"\n");
            fclose(fp);
            */
         }
         else
            for(unsigned i = 0; i < motion_size; i++)
               motion_array[i] = 0x00;
   }

   if(Version() >= 64512){
      switch(Model()){
      case 1300:
      case 2100:
      case 3100:
      case 5100:
          if( last_packet_size >= 102)
          {
              pc_time_stamp = last_packet + 92; //last_packet[92:99]
              status_aux_in = last_packet[100];
              status_aux_out = last_packet[101];
          }
          break;
      case AV1305:
      case AV2105:
      case AV3105:
      case AV5105:
          if( last_packet_size >= 104)
          {
              pc_time_stamp = last_packet + 94; //last_packet[94:101]
              status_aux_in = last_packet[102];
              status_aux_out = last_packet[103];
          }
          break;
      case 3130:
      case 3135:
          if( last_packet_size >= 98)
          {
              pc_time_stamp = last_packet + 87; //last_packet[87:94]
              status_aux_in = last_packet[95];
              status_aux_out = last_packet[96];
          }
          break;
      }
   }
   else{
       pc_time_stamp = 0;
       status_aux_in = -1;
       status_aux_out = -1;
   }

}
//---------------------------------------------------------------------------
AV2000Client::Size AV2000Client::ExtractSize(unsigned char* arr)
{
   const int fc = 256;
   Size size;
   size.x0 = static_cast<GByte>(arr[0]) * fc + static_cast<GByte>(arr[1]),
   size.y0 = static_cast<GByte>(arr[2]) * fc + static_cast<GByte>(arr[3]);
   size.width = static_cast<GByte>(arr[4]) * fc + static_cast<GByte>(arr[5]) - size.x0,
   size.height = static_cast<GByte>(arr[6]) * fc + static_cast<GByte>(arr[7]) - size.y0;
   return size;
}
//---------------------------------------------------------------------------
void AV2000Client::Parameter(CAMERA_PARAMETER property, long value)
{
   if(available(property, IdentOfCamera()) && handlers_set_property[property])
	   (this->*handlers_set_property[property])(value);
   else
   {
      /*
      FILE *fp = fopen("sdkexception.txt", "a+");
      fprintf(fp, "property %d is not supported, value %d, error %d. (camera %d/%d/%d, dn(%d)) \n",
                   property, value, available((CAMERA_PARAMETER)property, IdentOfCamera(), 0),
                   IdentOfCamera().model, IdentOfCamera().version, IdentOfCamera().revision, IdentOfCamera().dn);
      fclose(fp);
      */
      throw EAVNotSupported("property not supported");
   }
}
//---------------------------------------------------------------------------
long AV2000Client::Parameter(CAMERA_PARAMETER property)
{
   if(available(property, IdentOfCamera()) && handlers_get_property[property])
	   return (this->*handlers_get_property[property])();
   else
      throw EAVNotSupported("property is not supported");
}
//---------------------------------------------------------------------------
void AV2000Client::ReceiveBeginTimeout(unsigned value)
{
   receive_begin_timeout = value;
}
//---------------------------------------------------------------------------
unsigned AV2000Client::ReceiveBeginTimeout() const
{
   return receive_begin_timeout;
}
//---------------------------------------------------------------------------
void AV2000Client::RepeatAknowlegements(unsigned value)
{
   client->RepeatAknowlegements(value);
}
//---------------------------------------------------------------------------
unsigned AV2000Client::RepeatAknowlegements() const
{
   return client->RepeatAknowlegements();
}
//---------------------------------------------------------------------------
void AV2000Client::OnBeforeClientOperation(GIdTFTPClient::OPERATION opr)
{
	end_packet->Reset();
   if(client->Current()->RequestedBlockSize() != requested_block_size)
      client->Current()->RequestedBlockSize(requested_block_size);

   client->Current()->ReceiveTimeout(0);
   try{
      for(int i = 1; i <= 64; i++)
         //client->Current()->GIdUDPBase::ReceiveBuffer(buff_client, 8192, GIdTimeout(0));
         client->Current()->GIdUDPBase::ReceiveString();
   }
   catch(GIdETimeout&)
   {
   int n = 0;
   }

	//try { static_cast<GIdUDPBase*>(client->Current())->ReceiveString(1); } catch(GIdException) { }
	client->Current()->ReceiveTimeout(receive_begin_timeout);
}
//---------------------------------------------------------------------------
inline void AV2000Client::OnAfterClientConnect(GIdTFTPClient::OPERATION opr)
{
   /*
   if(GIdTFTPClient::GET == opr)
      client->Current()->ReceiveTimeout(receive_middle_timeout);
      */
}
//---------------------------------------------------------------------------
inline void AV2000Client::OnClientWork(GIdTFTPClient::WORK_MODE mode, unsigned packet, unsigned long size)
{
	if(3 == packet && client->RepeatAknowlegements())
		client->Current()->ReceiveTimeout(receive_begin_timeout / client->RepeatAknowlegements());
	if(size < client->Current()->RequestedBlockSize())
		(*end_packet)(packet, size);
}
//---------------------------------------------------------------------------
void AV2000Client::OnEndClientOperation(GIdTFTPClient::OPERATION opr)
{
	client->Current()->ReceiveTimeout(receive_begin_timeout);
}
//---------------------------------------------------------------------------
void AV2000Client::FactoryDefault()
{
   for(HANDLERS_SET_PROPERIES::iterator iter = handlers_set_property.begin(); iter != handlers_set_property.end(); iter++)
      if(available(iter->first, IdentOfCamera()))
         try {
            Default::const_iterator diter = deflt.find(iter->first);
            if(diter != deflt.end())
               (this->*iter->second)(diter->second);
         }
         catch(EAVNotSupported&){
         }
   if(Version() >= 50412){
      Page(3);
      Register(0x70, 123);
      Register(0x7E , 0x80);
   }
   LowLightMode();
}
//---------------------------------------------------------------------------
IRIS_STATUS AV2000Client::IrisStatus() const
{
   return iris_status;
}


//---------------------------------------------------------------------------
AV2000ClientBase::Client::Client(AV2000ClientBase& in_loader, unsigned in_size)
   : loader(in_loader), size(in_size), receive_timeout(500), number(0), host("127.0.0.1"), port(69), requested_block_size(1450), repeat_aknowlegements(3)
{
	if(!size)
		size = 1;
	for(unsigned i = 0; i < size; i++){
		clients.push_back(new GIdTFTPClientUse<AV2000ClientBase>(loader));
		clients[i]->RequestedBlockSize(requested_block_size);
      clients[i]->ReceiveTimeout(receive_timeout);
      clients[i]->RepeatAknowlegements(repeat_aknowlegements);
      clients[i]->Host(host);
      clients[i]->Port(port);
   }
}
//---------------------------------------------------------------------------
AV2000ClientBase::Client::~Client()
{
	for(unsigned i = 0; i < clients.size(); i++)
           if(clients[i]){
                  delete clients[i];
                  clients[i] = 0;
           }
}
//---------------------------------------------------------------------------

void AV2000ClientBase::Client::Host(const GString& h)
{
   if(host != h){
	   for(unsigned i = 0; i < clients.size(); i++)
		   clients[i]->Host(h);
	   host = h;
   }
}
//---------------------------------------------------------------------------
const char* AV2000ClientBase::Client::Host() const
{
	return host.c_str();
}
//---------------------------------------------------------------------------
void AV2000ClientBase::Client::Port(unsigned p)
{
   if(port != p){
	   for(unsigned i = 0; i < clients.size(); i++)
		   clients[i]->Port(p);
      port = p;
   }
}
//---------------------------------------------------------------------------
unsigned AV2000ClientBase::Client::Port() const
{
	return port;
}
//---------------------------------------------------------------------------
void AV2000ClientBase::Client::ReceiveTimeout(unsigned rt)
{
	for(unsigned i = 0; i < clients.size(); i++)
		clients[i]->ReceiveTimeout(rt);
	receive_timeout = rt;
}
//---------------------------------------------------------------------------
unsigned AV2000ClientBase::Client::ReceiveTimeout() const
{
	return receive_timeout;
}
//---------------------------------------------------------------------------
void AV2000ClientBase::Client::RepeatAknowlegements(unsigned value)
{
   repeat_aknowlegements = value;
	for(unsigned i = 0; i < clients.size(); i++)
		clients[i]->RepeatAknowlegements(repeat_aknowlegements);
}
//---------------------------------------------------------------------------
unsigned AV2000ClientBase::Client::RepeatAknowlegements() const
{
   return repeat_aknowlegements;
}
//---------------------------------------------------------------------------
void AV2000ClientBase::Client::RequestedBlockSize(unsigned val)
{
   const_cast<unsigned&>(requested_block_size) = val;
	for(unsigned i = 0; i < clients.size(); i++)
		clients[i]->RequestedBlockSize(requested_block_size);
}
//---------------------------------------------------------------------------
inline unsigned AV2000ClientBase::Client::RequestedBlockSize() const
{
	return requested_block_size;
}
//---------------------------------------------------------------------------
inline void AV2000ClientBase::Client::UpdateNumber()
{
	if(++number == clients.size())
		number = 0;
}
//---------------------------------------------------------------------------
inline void AV2000ClientBase::Client::Get(const GIdString& name, GBaseMemoryStream& mem)
{
	UpdateNumber();
	clients[number]->Get(name, mem);
}
//---------------------------------------------------------------------------
inline void AV2000ClientBase::Client::Put(const GBaseMemoryStream& mem, const GIdString& name)
{
	UpdateNumber();
	clients[number]->Put(mem, name);
}
//---------------------------------------------------------------------------
inline GIdTFTPClientUse<AV2000ClientBase>* AV2000ClientBase::Client::operator[](unsigned idx)
{
	return clients[idx];
}
//---------------------------------------------------------------------------
inline GIdTFTPClientUse<AV2000ClientBase>* AV2000ClientBase::Client::Current()
{
	return clients[number];
}
//---------------------------------------------------------------------------
inline unsigned AV2000ClientBase::Client::Size() const
{
	return clients.size();
}
//---------------------------------------------------------------------------
AV2000ClientInherit1::Camera::Camera()
   : model(0), version(0), revision(0), proc_fpga(0), net_fpga(0), mini(0)
{
}
//---------------------------------------------------------------------------
AV2000ClientInherit1::Camera::Camera(unsigned long m, unsigned long v)
   : model(m), version(v), revision(0), proc_fpga(0), net_fpga(0), mini(0)
{
}
//---------------------------------------------------------------------------
bool AV2000ClientInherit1::Camera::operator==(const Camera& right) const
{
   return model == right.model && version == right.version && proc_fpga == right.proc_fpga && net_fpga == right.net_fpga;
}
//---------------------------------------------------------------------------
bool AV2000ClientInherit1::Camera::operator!=(const Camera& right) const
{
   return model != right.model || version != right.version || proc_fpga == right.proc_fpga || net_fpga != right.net_fpga;
}
//---------------------------------------------------------------------------

AV2000ClientInherit1::Available::Available()
{
   unsigned long ident = 2000;

   available_by_model[ident][cpGAMMA] = true;
   available_by_model[ident][cpBRIGHTNESS] = true;
   available_by_model[ident][cpAUTO_EXPOSITION] = true;
   available_by_model[ident][cpSHARPNESS] = true;
   available_by_model[ident][cpSATURATION] = true;
   available_by_model[ident][cpBLUE] = true;
   available_by_model[ident][cpRED] = true;
   available_by_model[ident][cpILLUMINATION] = true;
   available_by_model[ident][cpLIGHTING] = true;
   available_by_model[ident][cpCAMERA_MODE] = true;
   available_by_model[ident][cpROLL] = true;
   available_by_model[ident][cpQUALITY_FULL] = true;
   available_by_model[ident][cpQUALITY_HALF] = true;
   available_by_model[ident][cpQUALITY_ZOOM] = true;
   available_by_model[ident][cpDOUBLESCAN] = true;
   available_by_model[ident][cpSENSOR_WIDTH] = true;
   available_by_model[ident][cpSENSOR_HEIGHT] = true;
   available_by_model[ident][cpMS_NUMBER_OF_SENSOR] = true;
   available_by_model[ident][cpWIDTH] = true;
   available_by_model[ident][cpHEIGHT] = true;
   available_by_model[ident][cpSC_ENABLED] = true;
   available_by_model[ident][cpSC_STROBE] = true;
   available_by_model[ident][cpSC_ALWAYS_SEND] = true;
   available_by_model[ident][cpSC_OUTPUT_HIGH] = true;
   available_by_model[ident][cpSC_OUTPUT_LOW] = true;
   available_by_model[ident][cpSC_OPTO_INPUT] = true;

   available_by_model[2100] = available_by_model[ident];
   ident = 2100;
   available_by_model[ident][cpSENSOR_LEFT] = true;
   available_by_model[ident][cpSENSOR_TOP] = true;
   available_by_model[ident][cpIRIS_ENABLED] = true;
   available_by_model[ident][cpIRIS_SPEED] = true;
   available_by_model[ident][cpIRIS_GAIN] = true;
   //available_by_model[ident][cpIRIS_PEROSITION_ENABLED] = true;
   available_by_model[ident][cpIRIS_REPOSITION_F_STOPS] = true;
   available_by_model[ident][cpIRIS_REPOSITION_F_STOPS_MIN] = true;
   available_by_model[ident][cpIRIS_REPOSITION_PERIOD] = true;
   available_by_model[ident][cpIRIS_REPOSITION_STABLE_PERIOD] = true;
   available_by_model[ident][cpEXPOSURE_MODE] = true;
   available_by_model[ident][cpEXPOSURE_WINDOW_LEFT] = true;
   available_by_model[ident][cpEXPOSURE_WINDOW_TOP] = true;
   available_by_model[ident][cpEXPOSURE_WINDOW_WIDTH] = true;
   available_by_model[ident][cpEXPOSURE_WINDOW_HEIGHT] = true;
   available_by_model[ident][cpREQUEST_LEFT] = true;
   available_by_model[ident][cpREQUEST_TOP] = true;
   available_by_model[ident][cpREQUEST_WIDTH] = true;
   available_by_model[ident][cpREQUEST_HEIGHT] = true;
   available_by_model[ident][cpQUALITY] = true;
   available_by_model[ident][cpRESOLUTION] = true;
   available_by_model[ident][cpREQUESTED_BLOCK_SIZE] = true;
   available_by_model[ident][cpMD_ENABLED] = true;
   available_by_model[ident][cpMD_MODE] = true;
   available_by_model[ident][cpMD_LEVEL_THRESH] = true;
   available_by_model[ident][cpMD_TOTAL_ZONES] = true;
   available_by_model[ident][cpMD_ZONE_SIZE] = true;
   available_by_model[ident][cpMD_DETAIL] = true;
   available_by_model[ident][cpMD_EXPLOSURE_SENSITIVITY] = true;
   available_by_model[ident][cpMD_MATRIX] = true;
   available_by_model[ident][cpSHORT_EXPOSURES] = true;
   available_by_model[ident][cpTIMEOUT_ON_CAMERA] = true;
   available_by_model[ident][cpREPEAT_FROM_CAMERA] = true;
   available_by_model[ident][cpDAY_NIGHT_MODE] = true;
   available_by_model[ident][cpDAY_NIGHT_TRIGGER_NIGHT] = true;
   available_by_model[ident][cpDAY_NIGHT_TRIGGER_DAY] = true;

   available_by_model[AV2105] = available_by_model[2100];
   available_by_model[AV2105][cpH264] = true;



   available_by_model[5100] = available_by_model[3100] = available_by_model[1300] = available_by_model[ident];
   available_by_model[5100][cpLIGHTINGFIX] = true;

   available_by_model[3130] = available_by_model[2000];

   available_by_model[AV1305] = available_by_model[1300];
   available_by_model[AV1305][cpH264] = true;

   available_by_model[AV3105] = available_by_model[3100];
   available_by_model[AV3105][cpH264] = true;

   available_by_model[AV5105] = available_by_model[5100];
   available_by_model[AV5105][cpH264] = true;

   ident = 3130;
   available_by_model[ident][cpSENSOR_LEFT] = true;
   available_by_model[ident][cpSENSOR_TOP] = true;
   available_by_model[ident][cpDAY_NIGHT_MODE] = true;
   available_by_model[ident][cpDAY_NIGHT_TRIGGER_NIGHT] = true;
   available_by_model[ident][cpDAY_NIGHT_TRIGGER_DAY] = true;
   available_by_model[ident][cpSENSOR_BLACK_WHITE_LEFT] = true;
   available_by_model[ident][cpSENSOR_BLACK_WHITE_TOP] = true;
   available_by_model[ident][cpSENSOR_BLACK_WHITE_WIDTH] = true;
   available_by_model[ident][cpSENSOR_BLACK_WHITE_HEIGHT] = true;
   available_by_model[ident][cpPER_CENT_IMAGE_RECTANGLE] = true;
   available_by_model[ident][cpREQUESTED_BLOCK_SIZE] = true;
   available_by_model[ident][cpSHORT_EXPOSURES] = true;

   available_by_model[ident][cpMD_ENABLED] = true;
   available_by_model[ident][cpMD_MODE] = true;
   available_by_model[ident][cpMD_LEVEL_THRESH] = true;
   available_by_model[ident][cpMD_TOTAL_ZONES] = true;
   available_by_model[ident][cpMD_ZONE_SIZE] = true;
   available_by_model[ident][cpMD_DETAIL] = true;
   available_by_model[ident][cpMD_EXPLOSURE_SENSITIVITY] = true;
   available_by_model[ident][cpMD_MATRIX] = true;

   available_by_model[AV3135] = available_by_model[3130];
   available_by_model[AV3135][cpH264] = true;
   available_by_model[AV3135][cpBIT_RATE] = true;


   available_by_model[8360] = available_by_model[2100];
   ident = 8360;
   /*
   available_by_model[ident].erase(cpMD_ENABLED);
   available_by_model[ident].erase(cpMD_MODE);
   available_by_model[ident].erase(cpMD_LEVEL_THRESH);
   available_by_model[ident].erase(cpMD_TOTAL_ZONES);
   available_by_model[ident].erase(cpMD_ZONE_SIZE);
   available_by_model[ident].erase(cpMD_DETAIL);
   available_by_model[ident].erase(cpMD_EXPLOSURE_SENSITIVITY);
   available_by_model[ident].erase(cpMD_MATRIX);
   */
   available_by_model[ident].erase(cpWIDTH);
   available_by_model[ident].erase(cpHEIGHT);
   available_by_model[ident][cpMS_NUMBER_OF_SENSOR] = true;
   available_by_model[ident][cpMS_CHANNEL_ENABLE] = true;
   available_by_model[ident][cpMS_FULL_RES_ENABLE] = true;
   available_by_model[ident][cpMS_ZOOM_WIN_ENABLE] = true;
   available_by_model[ident][cpMS_ONE_SHOT_ENABLE] = true;
   available_by_model[ident][cpMS_IS_ZOOMED] = true;
   available_by_model[ident][cpMS_QUAD_MODE] = true;

   available_by_model[8180] = available_by_model[8360];

   available_by_model[AV8185] = available_by_model[8180];
   available_by_model[AV8185][cpH264] = true;
   available_by_model[AV8185][cpBIT_RATE] = true;

   available_by_model[AV8365] = available_by_model[8360];
   available_by_model[AV8365][cpH264] = true;
   available_by_model[AV8365][cpBIT_RATE] = true;



   ident = 0;
   available_by_version[ident][cpBRIGHTNESS] = true;
   available_by_version[ident][cpAUTO_EXPOSITION] = true;
   available_by_version[ident][cpSHARPNESS] = true;
   available_by_version[ident][cpSATURATION] = true;
   available_by_version[ident][cpBLUE] = true;
   available_by_version[ident][cpRED] = true;
   available_by_version[ident][cpILLUMINATION] = true;
   available_by_version[ident][cpLIGHTING] = true;
   available_by_version[ident][cpLIGHTINGFIX] = true;
   available_by_version[ident][cpCAMERA_MODE] = true;
   available_by_version[ident][cpROLL] = true;
   available_by_version[ident][cpDAY_NIGHT_MODE] = true;
   available_by_version[ident][cpDAY_NIGHT_TRIGGER_NIGHT] = true;
   available_by_version[ident][cpDAY_NIGHT_TRIGGER_DAY] = true;
   available_by_version[ident][cpH264] = true;
   available_by_version[ident][cpBIT_RATE] = true;


   available_by_version[ident][cpSENSOR_WIDTH] = true;
   available_by_version[ident][cpSENSOR_HEIGHT] = true;
   available_by_version[ident][cpSENSOR_LEFT] = true;
   available_by_version[ident][cpSENSOR_TOP] = true;
   available_by_version[ident][cpSENSOR_BLACK_WHITE_LEFT] = true;
   available_by_version[ident][cpSENSOR_BLACK_WHITE_TOP] = true;
   available_by_version[ident][cpSENSOR_BLACK_WHITE_WIDTH] = true;
   available_by_version[ident][cpSENSOR_BLACK_WHITE_HEIGHT] = true;
   available_by_version[ident][cpPER_CENT_IMAGE_RECTANGLE] = true;

   available_by_version[ident][cpMS_NUMBER_OF_SENSOR] = true;
   available_by_version[ident][cpMS_CHANNEL_ENABLE] = true;
   available_by_version[ident][cpMS_FULL_RES_ENABLE] = true;
   available_by_version[ident][cpMS_ZOOM_WIN_ENABLE] = true;
   available_by_version[ident][cpMS_ONE_SHOT_ENABLE] = true;
   available_by_version[ident][cpMS_IS_ZOOMED] = true;
   available_by_version[ident][cpMS_QUAD_MODE] = true;

   available_by_version[ident][cpWIDTH] = true;
   available_by_version[ident][cpHEIGHT] = true;
   available_by_version[ident][cpSHORT_EXPOSURES] = true;

   available_by_version[ident][cpSC_ENABLED] = true;
   available_by_version[ident][cpSC_STROBE] = true;
   available_by_version[ident][cpSC_ALWAYS_SEND] = true;
   available_by_version[ident][cpSC_OUTPUT_HIGH] = true;
   available_by_version[ident][cpSC_OUTPUT_LOW] = true;
   available_by_version[ident][cpSC_OPTO_INPUT] = true;

   available_by_version[40820] = available_by_version[ident];
   ident = 40820;
   available_by_version[ident][cpDOUBLESCAN] = true;
   available_by_version[ident][cpQUALITY_FULL] = true;
   available_by_version[ident][cpQUALITY_HALF] = true;
   available_by_version[ident][cpQUALITY_ZOOM] = true;

   available_by_version[50217] = available_by_version[ident];
   ident = 50217;
   available_by_version[ident][cpIRIS_ENABLED] = true;
   available_by_version[ident][cpIRIS_SPEED] = true;
   available_by_version[ident][cpIRIS_GAIN] = true;
   //available_by_version[ident][cpIRIS_PEROSITION_ENABLED] = true;
   available_by_version[ident][cpIRIS_REPOSITION_F_STOPS] = true;
   available_by_version[ident][cpIRIS_REPOSITION_F_STOPS_MIN] = true;
   available_by_version[ident][cpIRIS_REPOSITION_PERIOD] = true;
   available_by_version[ident][cpIRIS_REPOSITION_STABLE_PERIOD] = true;

   available_by_version[52024] = available_by_version[ident];
   ident = 52024;
   available_by_version[ident][cpEXPOSURE_MODE] = true;
   available_by_version[ident][cpEXPOSURE_WINDOW_LEFT] = true;
   available_by_version[ident][cpEXPOSURE_WINDOW_TOP] = true;
   available_by_version[ident][cpEXPOSURE_WINDOW_WIDTH] = true;
   available_by_version[ident][cpEXPOSURE_WINDOW_HEIGHT] = true;

   available_by_version[52108] = available_by_version[ident];
   ident = 52108;
   available_by_version[ident][cpREQUEST_LEFT] = true;
   available_by_version[ident][cpREQUEST_TOP] = true;
   available_by_version[ident][cpREQUEST_WIDTH] = true;
   available_by_version[ident][cpREQUEST_HEIGHT] = true;
   available_by_version[ident][cpQUALITY] = true;
   available_by_version[ident][cpRESOLUTION] = true;

   available_by_version[52109] = available_by_version[ident];
   ident = 52109;
   available_by_version[ident][cpREQUESTED_BLOCK_SIZE] = true;

   available_by_version[61215] = available_by_version[ident];
   ident = 61215;
   available_by_version[ident][cpMD_ENABLED] = true;
   available_by_version[ident][cpMD_MODE] = true;
   available_by_version[ident][cpMD_LEVEL_THRESH] = true;
   available_by_version[ident][cpMD_TOTAL_ZONES] = true;
   available_by_version[ident][cpMD_ZONE_SIZE] = true;
   available_by_version[ident][cpMD_DETAIL] = true;
   available_by_version[ident][cpMD_EXPLOSURE_SENSITIVITY] = true;
   available_by_version[ident][cpMD_MATRIX] = true;

   available_by_version[61705] = available_by_version[ident];
   ident = 61705;
   available_by_version[ident][cpTIMEOUT_ON_CAMERA] = true;
   available_by_version[ident][cpREPEAT_FROM_CAMERA] = true;

   available_by_version[64001] = available_by_version[ident];
   ident = 64001;
   available_by_version[ident][cpGAMMA] = true;

   //available_by_version[ident][] = true;

   ident = 0;
   revision[ident][cpGAMMA] = true;
   revision[ident][cpBRIGHTNESS] = true;
   revision[ident][cpAUTO_EXPOSITION] = true;
   revision[ident][cpSATURATION] = true;
   revision[ident][cpSHARPNESS] = true;
   revision[ident][cpBLUE] = true;
   revision[ident][cpRED] = true;
   revision[ident][cpILLUMINATION] = true;
   revision[ident][cpLIGHTING] = true;
   revision[ident][cpLIGHTINGFIX] = true;
   revision[ident][cpCAMERA_MODE] = true;
   revision[ident][cpQUALITY_FULL] = true;
   revision[ident][cpQUALITY_HALF] = true;
   revision[ident][cpQUALITY_ZOOM] = true;
   revision[ident][cpDOUBLESCAN] = true;
   revision[ident][cpROLL] = true;
   revision[ident][cpIRIS_ENABLED] = true;
   revision[ident][cpIRIS_SPEED] = true;
   revision[ident][cpIRIS_GAIN] = true;
   revision[ident][cpIRIS_REPOSITION_F_STOPS] = true;
   revision[ident][cpIRIS_REPOSITION_F_STOPS_MIN] = true;
   revision[ident][cpIRIS_REPOSITION_PERIOD] = true;
   revision[ident][cpIRIS_REPOSITION_STABLE_PERIOD] = true;
   revision[ident][cpDAY_NIGHT_MODE] = true;
   revision[ident][cpDAY_NIGHT_TRIGGER_NIGHT] = true;
   revision[ident][cpDAY_NIGHT_TRIGGER_DAY] = true;
   revision[ident][cpEXPOSURE_MODE] = true;
   revision[ident][cpEXPOSURE_WINDOW_LEFT] = true;
   revision[ident][cpEXPOSURE_WINDOW_TOP] = true;
   revision[ident][cpEXPOSURE_WINDOW_WIDTH] = true;
   revision[ident][cpEXPOSURE_WINDOW_HEIGHT] = true;
   revision[ident][cpSENSOR_LEFT] = true;
   revision[ident][cpSENSOR_TOP] = true;
   revision[ident][cpSENSOR_WIDTH] = true;
   revision[ident][cpSENSOR_HEIGHT] = true;
   revision[ident][cpSENSOR_BLACK_WHITE_LEFT] = true;
   revision[ident][cpSENSOR_BLACK_WHITE_TOP] = true;
   revision[ident][cpSENSOR_BLACK_WHITE_WIDTH] = true;
   revision[ident][cpSENSOR_BLACK_WHITE_HEIGHT] = true;
   revision[ident][cpPER_CENT_IMAGE_RECTANGLE] = true;
   revision[ident][cpREQUESTED_BLOCK_SIZE] = true;
   revision[ident][cpMD_ENABLED] = true;
   revision[ident][cpMD_EXPLOSURE_SENSITIVITY] = true;
   revision[ident][cpMD_MODE] = true;
   revision[ident][cpMD_LEVEL_THRESH] = true;
   revision[ident][cpMD_TOTAL_ZONES] = true;
   revision[ident][cpMD_ZONE_SIZE] = true;
   revision[ident][cpMD_DETAIL] = true;
   revision[ident][cpMD_MATRIX] = true;

   revision[ident][cpMS_NUMBER_OF_SENSOR] = true;
   revision[ident][cpMS_CHANNEL_ENABLE] = true;
   revision[ident][cpMS_FULL_RES_ENABLE] = true;
   revision[ident][cpMS_ZOOM_WIN_ENABLE] = true;
   revision[ident][cpMS_ONE_SHOT_ENABLE] = true;
   revision[ident][cpMS_IS_ZOOMED] = true;
   revision[ident][cpMS_QUAD_MODE] = true;

   revision[ident][cpWIDTH] = true;
   revision[ident][cpHEIGHT] = true;
   revision[ident][cpSHORT_EXPOSURES] = true;
   revision[ident][cpTIMEOUT_ON_CAMERA] = true;
   revision[ident][cpREPEAT_FROM_CAMERA] = true;

   revision[ident][cpSC_ENABLED] = true;
   revision[ident][cpSC_STROBE] = true;
   revision[ident][cpSC_ALWAYS_SEND] = true;
   revision[ident][cpSC_OUTPUT_HIGH] = true;
   revision[ident][cpSC_OUTPUT_LOW] = true;
   revision[ident][cpSC_OPTO_INPUT] = true;
   revision[ident][cpH264] = true;
   revision[ident][cpBIT_RATE] = true;

   revision[5] = revision[ident];
   ident = 5;
   revision[ident][cpREQUEST_LEFT] = true;
   revision[ident][cpREQUEST_TOP] = true;
   revision[ident][cpREQUEST_WIDTH] = true;
   revision[ident][cpREQUEST_HEIGHT] = true;
   revision[ident][cpQUALITY] = true;
   revision[ident][cpRESOLUTION] = true;

   //revision[ident][] = true;
}
//---------------------------------------------------------------------------
bool AV2000ClientInherit1::Available::operator()(CAMERA_PARAMETER param, const Camera& camera)
{
   return available_by_model[camera.model][param]
          && available_by_version.lower_bound(camera.version)->second[param]
          && revision.lower_bound(camera.revision)->second[param];
}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::Available::operator()(CAMERA_PARAMETER param, const Camera& camera, long bits)
{
   long bymodel = available_by_model[camera.model][param];
   long byversion = available_by_version.lower_bound(camera.version)->second[param];
   long byrevision = revision.lower_bound(camera.revision)->second[param];
   return bymodel | (byversion << 1) | (byrevision << 2);
}
//---------------------------------------------------------------------------

AV2000ClientInherit1::LightingConformity::LightingConformity()
{
   // 0 = 50Hg, 1- 60 Hg

   unsigned long model;

   (*this)[0][model = 1300][4] = 341;
   (*this)[1][model][4] = 284;
   (*this)[0][model = AV1305][4] = 341;
   (*this)[1][model][4] = 284;


   (*this)[0][model = 2000][4] = 270;
   (*this)[1][model][4] = 225;

   (*this)[0][model = 2100][4] = 285;
   (*this)[1][model][4] = 237;
   (*this)[0][model = AV2105][4] = 285;
   (*this)[1][model][4] = 237;


   (*this)[0][model = 3100][4] = 238;
   (*this)[1][model][4] = 199;
   (*this)[0][model = AV3105][4] = 238;
   (*this)[1][model][4] = 199;


   (*this)[0][model = 5100][4] = 251;
   (*this)[1][model][4] = 209;
   (*this)[0][model = AV5105][4] = 251;
   (*this)[1][model][4] = 209;


   (*this)[0][model = 8180][0x05] = 272;
   (*this)[1][model][0x05] = 227;
   (*this)[0][model = AV8185][0x05] = 272;
   (*this)[1][model][0x05] = 227;


   (*this)[0][model = 8360][0x05] = 272;
   (*this)[1][model][0x05] = 227;
   (*this)[0][model = AV8365][0x05] = 272;
   (*this)[1][model][0x05] = 227;


   const unsigned FRAME5TH = 0x04;
   // 50 Hz
  (*this)[0][model = 3130][FRAME5TH_1300] = 355;
  (*this)[0][model][FRAME5TH] = 235;

   // 60Hz
  (*this)[1][model][FRAME5TH_1300] = 296;
  (*this)[1][model][FRAME5TH] = 196;

  // 50 Hz
  (*this)[0][model = 3135][FRAME5TH_1300] = 355;
  (*this)[0][model][FRAME5TH] = 235;

   // 60Hz
  (*this)[1][model][FRAME5TH_1300] = 296;
  (*this)[1][model][FRAME5TH] = 196;

}
//---------------------------------------------------------------------------

AV2000ClientInherit1::LightingCoefs::LightingCoefs()
{
   // 0 = 50Hg, 1- 60 Hg

   unsigned long model;

   (*this)[0][model = 1300][4] = std::make_pair(550000, 332);
   (*this)[1][model][4] = std::make_pair(458333, 332);
   (*this)[0][model = AV1305][4] = std::make_pair(550000, 332);
   (*this)[1][model][4] = std::make_pair(458333, 332);


   (*this)[0][model = 2100][4] = std::make_pair(550000, 332);
   (*this)[1][model][4] = std::make_pair(458333, 332);
   (*this)[0][model = AV2105][4] = std::make_pair(550000, 332);
   (*this)[1][model][4] = std::make_pair(458333, 332);


   (*this)[0][model = 3100][4] = std::make_pair(550000, 398);
   (*this)[1][model][4] = std::make_pair(458333, 398);
   (*this)[0][model = AV3105][4] = std::make_pair(550000, 398);
   (*this)[1][model][4] = std::make_pair(458333, 398);


   //this is parameter set is for long body cameras (discontinued)
   (*this)[0][model = 5100][4] = std::make_pair(810000, 674);
   (*this)[1][model][4] = std::make_pair(675000, 674);
   (*this)[0][model = AV5105][4] = std::make_pair(810000, 674);
   (*this)[1][model][4] = std::make_pair(675000, 674);

   //this is parameter set is for mini body cameras
   (*this)[0][model = 5100][4] = std::make_pair(685385, 674);
   (*this)[1][model][4] = std::make_pair(571154, 674);
   (*this)[0][model = AV5105][4] = std::make_pair(685385, 674);
   (*this)[1][model][4] = std::make_pair(571154, 674);

   //this is parameter set for panoramic cameras
   (*this)[0][model = 8360][0x05] = std::make_pair(550000, 332);
   (*this)[1][model][0x05] = std::make_pair(458333, 332);
   (*this)[0][model = AV8365][0x05] = std::make_pair(550000, 332);
   (*this)[1][model][0x05] = std::make_pair(458333, 332);

   (*this)[0][model = 8180][0x05] = std::make_pair(550000, 332);
   (*this)[1][model][0x05] = std::make_pair(458333, 332);
   (*this)[0][model = AV8185][0x05] = std::make_pair(550000, 332);
   (*this)[1][model][0x05] = std::make_pair(458333, 332);

   const unsigned FRAME5TH = 0x04;

  (*this)[0][model = 3130][FRAME5TH] = std::make_pair(550000, 414);
  (*this)[1][model][FRAME5TH] = std::make_pair(458333, 414);

  (*this)[0][model][FRAME5TH_1300] = std::make_pair(550000, 267);
  (*this)[1][model][FRAME5TH_1300] = std::make_pair(458333, 267);

  (*this)[0][model = AV3135][FRAME5TH] = std::make_pair(550000, 414);
  (*this)[1][model][FRAME5TH] = std::make_pair(458333, 414);

  (*this)[0][model][FRAME5TH_1300] = std::make_pair(550000, 267);
  (*this)[1][model][FRAME5TH_1300] = std::make_pair(458333, 267);

}
//---------------------------------------------------------------------------
long AV2000ClientInherit1::LightingCoefs::operator()(const std::pair<long, long>& coef, long sensor_w, bool fix5meg)
{
   long iBValue = fix5meg ? 900 : coef.second;
   long res = (long)(static_cast<float>(coef.first) / (iBValue + sensor_w) + 0.5);
   return res;
}
//---------------------------------------------------------------------------
bool AV2000ClientInherit1::LightingCoefs::IsExists(long lighting, unsigned long model)
{
   return (*this)[lighting].find(model) != (*this)[lighting].end();
}
//---------------------------------------------------------------------------

AV2000ClientInherit1::SensorSizeMax::SensorSizeMax()
{
   conformity[1300] = std::make_pair(1280, 1024);
   conformity[AV1305] = std::make_pair(1280, 1024);
   conformity[2000] = std::make_pair(1536, 1140);
   conformity[2100] = std::make_pair(1600, 1200);
   conformity[AV2105] = std::make_pair(1600, 1200);
   conformity[3100] = std::make_pair(2048, 1536);
   conformity[AV3105] = std::make_pair(2048, 1536);
   conformity[3130] = std::make_pair(2048, 1536);
   conformity[AV3135] = std::make_pair(2048, 1536);
   conformity[5100] = std::make_pair(2592, 1944);
   conformity[AV5105] = std::make_pair(2592, 1944);   
   conformity[8180] = std::make_pair(1600, 1200);
   conformity[AV8185] = std::make_pair(1600, 1200);
   conformity[8360] = std::make_pair(1600, 1200);
   conformity[AV8365] = std::make_pair(1600, 1200);
}
//---------------------------------------------------------------------------
int AV2000ClientInherit1::SensorSizeMax::Width(const Camera& camera) const
{
   CONFORMITY::const_iterator iter = conformity.find(camera.model);
   if(iter == conformity.end())
      throw EAVCameraUnknown("");
   return iter->second.first;
}
//---------------------------------------------------------------------------
int AV2000ClientInherit1::SensorSizeMax::Height(const Camera& camera) const
{
   CONFORMITY::const_iterator iter = conformity.find(camera.model);
   if(iter == conformity.end())
      throw EAVCameraUnknown("");
   return iter->second.second;
}
//---------------------------------------------------------------------------
std::pair<unsigned, unsigned> AV2000ClientInherit1::SensorSizeMax::Range(const Camera& camera) const
{
   CONFORMITY::const_iterator iter = conformity.find(camera.model);
   if(iter == conformity.end())
      throw EAVCameraUnknown("");
   return iter->second;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

void AV2000Calculate::Quality::Set(IMAGE_RESOLUTION regime, int val)
{
   if(val < 1 || val > quantization_table.size())
      throw EAVParameterOutOfRange("Value of quality is not valid");
   value[regime] = val;
}
//---------------------------------------------------------------------------
AV2000Calculate::Quality::Conformity AV2000Calculate::Quality::conformity_quantization;
//---------------------------------------------------------------------------
inline int AV2000Calculate::Quality::operator()(IMAGE_RESOLUTION regime)
{
	return value[regime];
}
//---------------------------------------------------------------------------
AV2000Calculate::Quality::Conformity::Conformity()
{
   unsigned long version = 0;
	(*this)[version][1] = 1.5;
	(*this)[version][2] = 1.25;
	(*this)[version][3] = 1.0;
	(*this)[version][4] = 0.75;
	(*this)[version][5] = 0.5;

   version = 41229;
   (*this)[version][1] =  3.250;
   (*this)[version][2] =  3.125;
   (*this)[version][3] =  3.000;
   (*this)[version][4] =  2.875;
   (*this)[version][5] =  2.750;
   (*this)[version][6] =  2.625;
   (*this)[version][7] =  2.500;
   (*this)[version][8] =  2.375;
   (*this)[version][9] =  2.250;
   (*this)[version][10] = 2.125;
   (*this)[version][11] = 2.000;
   (*this)[version][12] = 1.875;
   (*this)[version][13] = 1.750;
   (*this)[version][14] = 1.625;
   (*this)[version][15] = 1.500;
   (*this)[version][16] = 1.375;
   (*this)[version][17] = 1.250;
   (*this)[version][18] = 1.125;
   (*this)[version][19] = 1.000;
   (*this)[version][20] = 0.825;
   (*this)[version][21] = 0.750;
   (*this)[version][22] = 0.625;
   (*this)[version][23] = 0.500;
   (*this)[version][24] = 0.375;
   (*this)[version][25] = 0.250;
}
//---------------------------------------------------------------------------
AV2000Calculate::Quality::Quality()
: pheaders(0), restart_interval(16)
{
	pheaders = new std::map<HeaderInfo, std::vector<GByte> >;
   value[imFULL] = 1;
   value[imHALF] = 1;
   value[imZOOM] = 1;
}
//---------------------------------------------------------------------------
AV2000Calculate::Quality::~Quality()
{
   delete pheaders;
}
//---------------------------------------------------------------------------
void AV2000Calculate::Quality::LoadHeader(int ImageColumns, int ImageRows, IMAGE_RESOLUTION regime)
{
	LoadHeaderByQuality(ImageColumns, ImageRows, value[regime]);
}
//---------------------------------------------------------------------------
void AV2000Calculate::Quality::LoadHeaderByQuality(int ImageColumns, int ImageRows, int coef /*quality*/)
{
   HeaderInfo hinfo(ImageColumns, ImageRows, coef);
	float quantization = quantization_table[coef];
	std::map<HeaderInfo, std::vector<GByte> >::iterator iter = pheaders->find(hinfo);
	if(iter == pheaders->end())
		CalculateHeader((*pheaders)[hinfo], hinfo.columns, hinfo.rows, restart_interval, quantization, quantization);
	header = &(*pheaders)[hinfo];
#ifdef _windows_
         //add timestamp
        SYSTEMTIME st;
        GetLocalTime(&st);
        sprintf((header->begin()+0x170), "%4d:%02d:%02d %02d:%02d:%02d" ,st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
        sprintf((header->begin()+0x184), "%4d:%02d:%02d %02d:%02d:%02d" ,st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
#endif
}
//---------------------------------------------------------------------------
void AV2000Calculate::Quality::WriteHeader(GStream& dest)
{
	dest.Write(reinterpret_cast<char*>(&(*header)[0]), header->size());
}
//---------------------------------------------------------------------------
const std::vector<GByte>& AV2000Calculate::Quality::Header() const
{
	return *header;
}
//---------------------------------------------------------------------------
void AV2000Calculate::Quality::DoChangeCamera(const Camera& camera)
{
   pheaders->clear();
   CONFORMITY_QUANTINIZATION::iterator iter = conformity_quantization.lower_bound(camera.version);
   if(iter == conformity_quantization.end())
      throw EAVCameraUnknown("unknown version");
   quantization_table = iter->second;
}
//---------------------------------------------------------------------------

inline AV2000Calculate::Quality::HeaderInfo::HeaderInfo()
: columns(0), rows(0), coef(0)
{
}
//---------------------------------------------------------------------------
inline AV2000Calculate::Quality::HeaderInfo::HeaderInfo(int img_col, int img_row, int cf)
: columns(img_col), rows(img_row), coef(cf)
{
}
//---------------------------------------------------------------------------
inline bool AV2000Calculate::Quality::HeaderInfo::operator<(const HeaderInfo& right) const
{
	if(columns != right.columns)
		return columns < right.columns;
	else
		if(rows != right.rows)
			return rows < right.rows;
		else
			if(coef != right.coef)
				return coef < right.coef;
	return false;
}
//---------------------------------------------------------------------------
void AV2000Calculate::Quality::CalculateHeader(std::vector<GByte>& dest, int ImageColumns, int ImageRows, int RestartInterval, float QuantizationLuma, float QuantizationChroma)
{
	std::vector<GByte>& header_buffer = dest;

	int bcnt,i,tmp;
	float quan_tmp;

	const int jpeg_natural_order[64] = {
		0,  1,  8, 16,  9,  2,  3, 10,
		17, 24, 32, 25, 18, 11,  4,  5,
		12, 19, 26, 33, 40, 48, 41, 34,
		27, 20, 13,  6,  7, 14, 21, 28,
		35, 42, 49, 56, 57, 50, 43, 36,
		29, 22, 15, 23, 30, 37, 44, 51,
		58, 59, 52, 45, 38, 31, 39, 46,
		53, 60, 61, 54, 47, 55, 62, 63,
	};


	static int std_qtables[128] = {
   // chrominance quantization table
		8,8,12,22,48,48,48,48,
		8,10,12,32,48,48,48,48,
		12,12,28,48,48,48,48,48,
		22,32,48,48,48,48,48,48,
		48,48,48,48,48,48,48,48,
		48,48,48,48,48,48,48,48,
		48,48,48,48,48,48,48,48,
		48,48,48,48,48,48,48,52,
   // luminance quantization table
		8,4,4,8,12,20,24,30,
		6,6,6,8,12,28,30,26,
		6,6,8,12,20,28,34,28,
		6,8,10,14,24,42,40,30,
		8,10,18,28,34,54,50,38,
		12,16,26,32,40,52,56,46,
		24,32,38,42,50,60,60,50,
		36,46,46,48,56,50,50,52};

       //standard Huffman table JPEG section K.3
	static const unsigned char bits_dc_luminance[17] = { /* 0-base */ 0, 0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 };
	static const unsigned char val_dc_luminance[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
	static const unsigned char bits_dc_chrominance[17] = { /* 0-base */ 0, 0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0 };
	static const unsigned char val_dc_chrominance[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };

	static const unsigned char bits_ac_luminance[17] = { /* 0-base */ 0, 0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 0x7d };
	static const unsigned char val_ac_luminance[] =
	{ 0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
		0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
		0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
		0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
		0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
		0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
		0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
		0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
		0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
		0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
		0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
		0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
		0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
		0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
		0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
		0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
		0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
		0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
		0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
		0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
		0xf9, 0xfa };

       //number of huffman codes of length i, starting from 1
	static const unsigned char bits_ac_chrominance[17] = { /* 0-base */ 0, 0, 2, 1, 2, 4, 4, 3, 4, 7, 5, 4, 4, 0, 1, 2, 0x77 };
       //JPEG run-Amplitude code (how many zeros,length of coef)
	static const unsigned char val_ac_chrominance[] =
	{ 0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
		0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
		0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
		0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
		0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,
		0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
		0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38,
		0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
		0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
		0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
		0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
		0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
		0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
		0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
		0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
		0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
		0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2,
		0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
		0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
		0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
		0xf9, 0xfa };


 	static unsigned char EXIF_Header[2+EXIF_LENGTH] =
        { 0xFF, 0xD8, 0xFF, 0xE0, 0x00, 0x10, 0x4A, 0x46,   //1  APP0,C010H,
          0x49, 0x46, 0x00, 0x01, 0x02, 0x01, 0x00, 0xB4,
          0x00, 0xB4, 0x00, 0x00, 0xFF, 0xE1, 0x01, 0xEA,   //   APP1,C1EAH,
          0x45, 0x78, 0x69, 0x66, 0x00, 0x00, 0x49, 0x49,   //   Exif  II
          0x2A, 0x00, 0x08, 0x00, 0x00, 0x00, 0x11, 0x00,

          0x0F, 0x01, 0x02, 0x00, 0x40, 0x00, 0x00, 0x00,   //6  Make,10FH,ASCII,C64bytes,Pos072H,Actual_090H
          0x72, 0x00, 0x00, 0x00, 0x10, 0x01, 0x02, 0x00,   //   Model,110H,ASCII,C32bytes,Pos0B2H,Actual_0D0H
          0x20, 0x00, 0x00, 0x00, 0xB2, 0x00, 0x00, 0x00,

          0x31, 0x01, 0x02, 0x00, 0x40, 0x00, 0x00, 0x00,   //   Software,131H,ASCII,C64bytes,Pos0D2H,Actual_0F0H
          0xD2, 0x00, 0x00, 0x00, 0x98, 0x82, 0x02, 0x00,   //   CopyRight,8298H,ASCII,C64bytes,Pos112H,Actual_130H
          0x40, 0x00, 0x00, 0x00, 0x12, 0x01, 0x00, 0x00,   //11

          0x03, 0x90, 0x02, 0x00, 0x14, 0x00, 0x00, 0x00,   //   TimeOriginal,9003H,ASCII,C20bytes,Pos152H,Actual_170H
          0x52, 0x01, 0x00, 0x00, 0x04, 0x90, 0x02, 0x00,   //   TimeDigital,9004H,ASCII,C20bytes,Pos166H,Actual_184H
          0x14, 0x00, 0x00, 0x00, 0x66, 0x01, 0x00, 0x00,

          0x0E, 0x01, 0x02, 0x00, 0x40, 0x00, 0x00, 0x00,   //   Descriptoin,10EH,ASCII,C64bytes,Pos182H,Actual_1A0H
          0x82, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   //16
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //18  Make starts here, 90H
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   //21
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //25  Make ends here


          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   //26  Model starts here, D0H
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   //29  Model ends here

          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   //30  Software starts here, F0H
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   //31
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   //36
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   //37  Software ends here
          
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   //37  CopyRight starts here, 130H
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   //41
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   //45  CopyRight ends here


          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   //46  TimeOriginal starts here
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   //50  TimeOriginal ends here

          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   //51
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   //52  Descriptoin starts here, 1A0H
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   //56
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   //61
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00    //64   Descriptoin ends here
        };

        for(i=0; i<64; i++)
           if(MakeString[i])
              EXIF_Header[0x90+i] = MakeString[i];
           else{
              EXIF_Header[0x90+i] = 0;
              break;
           }

        for(i=0; i<32; i++)
           if(ModelString[i])
              EXIF_Header[0xD0+i] = ModelString[i];
           else{
              EXIF_Header[0xD0+i] = 0;
              break;
           }

        for(i=0; i<64; i++)
           if(SoftwareString[i])
              EXIF_Header[0xF0+i] = SoftwareString[i];
           else{
              EXIF_Header[0xF0+i] = 0;
              break;
           }

        for(i=0; i<64; i++)
           if(CopyRightString[i])
              EXIF_Header[0x130+i] = CopyRightString[i];
           else{
              EXIF_Header[0x130+i] = 0;
              break;
           }


 //output start of EXIF image header
        for(int ii=0; ii<512; ii++)
    	    header_buffer.push_back(EXIF_Header[ii]);

 //output start of image marker FFD8
	//header_buffer.push_back(0xff);
	//header_buffer.push_back(0xd8);

  //output quantization tables marker

	header_buffer.push_back(0xff);
	header_buffer.push_back(0xdb);

  //output DQT segment length (132 = 2*64 + 4 that is x84)
	header_buffer.push_back(0x00);
	header_buffer.push_back(0x84);

  //specify beginning of luminance table with 8-bit precision
	header_buffer.push_back(0x00);

  //output luminance table
	for(bcnt = 0; bcnt < 64; bcnt++)
	{
		quan_tmp = std_qtables[jpeg_natural_order[bcnt] + 64] * QuantizationLuma + 0.5;
		//header_buffer[bytes_in_header_buffer] = (GByte) std_qtables[jpeg_natural_order[bcnt] + 64];
		header_buffer.push_back(static_cast<GByte>(quan_tmp));
	}

  //specify beginning of chrominance table with 8-bit precision
	header_buffer.push_back(0x01);

  //output chrominance table
	for(bcnt = 0; bcnt < 64; bcnt++)
	{
		quan_tmp = std_qtables[jpeg_natural_order[bcnt]] * QuantizationChroma + 0.5;
		header_buffer.push_back(static_cast<GByte>(quan_tmp));
		//header_buffer[bytes_in_header_buffer] = (GByte) std_qtables[jpeg_natural_order[bcnt]];
	}

   //**Sometimes I comment out the restart interval, since we can save some reg and logic and file space.
   //output DRI marker
	header_buffer.push_back(0xff);
	header_buffer.push_back(0xdd);

   //output DRI segment length in bytes
	header_buffer.push_back(0x00);
	header_buffer.push_back(0x04);

   //restart interval, only one intra image encoded
	header_buffer.push_back(0x00);
	header_buffer.push_back(static_cast<GByte>(RestartInterval));

   //output frame header marker
	header_buffer.push_back(0xff);
	header_buffer.push_back(0xc0);

   //output frame header length
	header_buffer.push_back(0x00);
	header_buffer.push_back(0x11);

   //sample precision
	header_buffer.push_back(0x08);

   //number of lines (480 = 0x01e0) //try 472 = 0x01d8

   //  header_buffer[bytes_in_header_buffer] = 0x01; bytes_in_header_buffer++;
   //  header_buffer[bytes_in_header_buffer] = 0xd8; bytes_in_header_buffer++;

	header_buffer.push_back(static_cast<GByte>(ImageRows/256));
	tmp = ImageRows;
	while(tmp>256) tmp=tmp-256;

	header_buffer.push_back(static_cast<GByte>(tmp));

   //number of columns (640 = 0x0280) //try 632 = 0x278

   //header_buffer[bytes_in_header_buffer] = 0x02; bytes_in_header_buffer++;
   //header_buffer[bytes_in_header_buffer] = 0x80; bytes_in_header_buffer++;

	header_buffer.push_back(static_cast<GByte>(ImageColumns/256));
	tmp = ImageColumns;
	while(tmp>256) tmp=tmp-256;

	header_buffer.push_back(static_cast<GByte>(tmp));

   //number of components in frame
	header_buffer.push_back(0x03);

   //first component description - luminance
	header_buffer.push_back(0x01);
	header_buffer.push_back(0x21);
	header_buffer.push_back(0x00);

   //second component description
	header_buffer.push_back(0x02);
	header_buffer.push_back(0x11);
	header_buffer.push_back(0x01);

   //third component description
	header_buffer.push_back(0x03);
	header_buffer.push_back(0x11);
	header_buffer.push_back(0x01);

   //output huffman table marker
	header_buffer.push_back(0xff);
	header_buffer.push_back(0xc4);

   //length of DHT segment
	header_buffer.push_back(0x01);
	header_buffer.push_back(0xa2);

   //luminance dc table descriptor (class 0; identifier 0)
	header_buffer.push_back(0x00);

   //number of huffman codes of length i
	for(i = 1; i < 17; i++)
		header_buffer.push_back(bits_dc_luminance[i]);

   //associated values
	for(i = 0; i < 12; i++)
		header_buffer.push_back(val_dc_luminance[i]);

   //luminance ac table descriptor (class 1; identifier 0)
	header_buffer.push_back(0x10);

   //number of huffman codes of length i
	for(i = 1; i < 17; i++)
		header_buffer.push_back(bits_ac_luminance[i]);

   //associated values
	for(i = 0; i < 162; i++)
		header_buffer.push_back(val_ac_luminance[i]);

   //chrominance dc table descriptor (class 0; identifier 1)
	header_buffer.push_back(0x01);

   //number of huffman codes of length i
	for(i = 1; i < 17; i++)
		header_buffer.push_back(bits_dc_chrominance[i]);

   //associated values
	for(i = 0; i < 12; i++)
		header_buffer.push_back(val_dc_chrominance[i]);

   //chrominance ac table descriptor (class 1; identifier 1)
	header_buffer.push_back(0x11);

   //number of huffman codes of length i
	for(i = 1; i < 17; i++)
		header_buffer.push_back(bits_ac_chrominance[i]);

   //associated values
	for(i = 0; i < 162; i++)
		header_buffer.push_back(val_ac_chrominance[i]);

   //output SOS  marker
	header_buffer.push_back(0xff);
	header_buffer.push_back(0xda);

   //length of SOS header segment
	header_buffer.push_back(0x00);
	header_buffer.push_back(0x0c);

   //number of components in scan
	header_buffer.push_back(0x03);

   //descriptors of components in scan
	header_buffer.push_back(0x01);
	header_buffer.push_back(0x00);
	header_buffer.push_back(0x02);
	header_buffer.push_back(0x11);
	header_buffer.push_back(0x03);
	header_buffer.push_back(0x11);
	header_buffer.push_back(0x00);
	header_buffer.push_back(0x3f);
	header_buffer.push_back(0x00);

   //this completes frame header: output it in file
}
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
AV2000Client::Packet::Packet()
: number(0), size(0)
{
}
//---------------------------------------------------------------------------
inline void AV2000Client::Packet::operator()(unsigned nm, unsigned sz)
{
	number = nm;
	size = sz;
}
//---------------------------------------------------------------------------
inline unsigned AV2000Client::Packet::Size() const
{
	return size;
}
//---------------------------------------------------------------------------
inline unsigned AV2000Client::Packet::Number() const
{
	return number;
}
//---------------------------------------------------------------------------
inline void AV2000Client::Packet::Reset()
{
	number = size = 0;
}
//---------------------------------------------------------------------------
AV2000Client::Default::Default()
{
   (*this)[cpGAMMA] = 60;
   (*this)[cpBRIGHTNESS] = 5;
   (*this)[cpAUTO_EXPOSITION] = 1;
   (*this)[cpSATURATION] = 100;
   (*this)[cpSHARPNESS] = 2;
   (*this)[cpBLUE] = 0;
   (*this)[cpRED] = 0;
   (*this)[cpILLUMINATION] = 0;
   (*this)[cpLIGHTING] = 50;
   (*this)[cpLIGHTINGFIX] = 0;
   (*this)[cpCAMERA_MODE] = 2;
   (*this)[cpQUALITY_FULL] = 1;
   (*this)[cpQUALITY_HALF] = 1;
   (*this)[cpQUALITY_ZOOM] = 1;
   (*this)[cpDOUBLESCAN] = 0;
   (*this)[cpROLL] = 0;
   (*this)[cpIRIS_ENABLED] = 0;
   (*this)[cpIRIS_SPEED] = 64;
   (*this)[cpIRIS_GAIN]= 160;
   //(*this)[cpIRIS_PEROSITION_ENABLED] = 1;
   (*this)[cpIRIS_REPOSITION_F_STOPS] = 5;
   (*this)[cpIRIS_REPOSITION_F_STOPS_MIN] = 3;
   (*this)[cpIRIS_REPOSITION_PERIOD] = 600;
   (*this)[cpIRIS_REPOSITION_STABLE_PERIOD] = 1;
   (*this)[cpDAY_NIGHT_MODE] = 0;
   (*this)[cpDAY_NIGHT_TRIGGER_NIGHT] = 8;
   (*this)[cpDAY_NIGHT_TRIGGER_DAY] = 2;
   //(*this)[] = ;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
ZoomInfo MakeZoomInfo(float zoom, int dx, int dy)
{
	ZoomInfo result;
	result.zoom = zoom;
	result.dx = dx;
	result.dy = dy;
	return result;
}
//---------------------------------------------------------------------------

