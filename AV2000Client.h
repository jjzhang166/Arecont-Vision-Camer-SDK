//---------------------------------------------------------------------------
#ifndef AV2000ClientH
#define AV2000ClientH
//---------------------------------------------------------------------------
#include <string>
#include <map>
#include <vector>

#include "AV2000Types.h"
#include "GIdTFTPClient.h"
#include "GUtil.h"


//#define _TEST_INTERNAL_REGISTER
#define EXIF_LENGTH  (510)

//---------------------------------------------------------------------------

typedef unsigned char GByte;
//---------------------------------------------------------------------------
class EAVException : public GException{
public:
   explicit EAVException(const char* msg)
      : GException(0, msg, 1)
   {
   }
};
//---------------------------------------------------------------------------
class EAVParameterOutOfRange : public EAVException{
public:
   explicit EAVParameterOutOfRange(const char* msg)
      : EAVException(msg)
   {
   }
};
//---------------------------------------------------------------------------
class EAVCameraUnknown : public EAVException{
public:
   explicit EAVCameraUnknown(const char* msg)
      : EAVException(msg)
   {
   }
};
//---------------------------------------------------------------------------
class EAVNotSupported : public EAVException{
public:
   explicit EAVNotSupported(const char* msg)
      : EAVException(msg)
   {
   }
};
//---------------------------------------------------------------------------
class EAVParameterUnknown : public EAVException{
public:
   EAVParameterUnknown()
      : EAVException("")
   {
   }
   explicit EAVParameterUnknown(const char* msg)
      : EAVException(msg)
   {
   }
};
//---------------------------------------------------------------------------

class AV2000ClientBase{
   class Client;
protected:
   static PTR_InfoSetRegister info_set_register;
   static PTR_InfoGetRegister info_get_register;
   static PTR_ReceivePacket handler_receive_packet;
   static PTR_SendAck handler_send_ack;
   const int number;
	Client* client;
   bool type_of_registers_proc_old;
   unsigned curr_page, requested_block_size;

#ifdef _TEST_INTERNAL_REGISTER
   std::map<unsigned, std::map<unsigned, unsigned> > registers;
#endif

protected:
   virtual void OnBeforeClientOperation(GIdTFTPClient::OPERATION){}
   virtual void OnAfterClientConnect(GIdTFTPClient::OPERATION){}
   virtual void OnClientWork(GIdTFTPClient::WORK_MODE, unsigned, unsigned long size){}
   virtual void OnEndClientOperation(GIdTFTPClient::OPERATION){}
   virtual void OnReceivePacket(unsigned packet, unsigned long size);
   virtual void OnSendAck(unsigned ack);
private:
   //
public:
   explicit AV2000ClientBase(int, unsigned n = 4);
   virtual ~AV2000ClientBase();
   void Host(const GString&);
   const char* Host() const;
   void Port(unsigned);
   unsigned Port();
   void Register(GByte, unsigned);
   unsigned Register(GByte);
   void Page(unsigned);

   void ReceiveTimeout(unsigned);
   unsigned ReceiveTimeout() const;
   void GetSockets(void*, unsigned char*);

   static void DefineInfoSetRegister(PTR_InfoSetRegister);
   static void DefineInfoGetRegister(PTR_InfoGetRegister);
   static void DefineHandlerOnReceivePacket(PTR_ReceivePacket);
   static void DefineHandlerOnSendAck(PTR_SendAck);
   void InitializeCopyRightStrings(EXIF_STRING_ID,unsigned char*);
};
//---------------------------------------------------------------------------
class AV2000ClientInherit1 : public AV2000ClientBase{
   struct LightingConformity;
   struct LightingCoefs;

   class Available;
   class SensorSizeMax;

public:
   struct Camera;
private:
   static LightingConformity lighting_conformity;
   static LightingCoefs lighting_coefs;
protected:
   static unsigned FRAME5TH_1300;
   Camera* pcamera;
   static Available available;
   static SensorSizeMax sensor_size_max;
   struct {
      int left, top, width, height,
          left_black_white, top_black_white, width_black_white, height_black_white;
   } sensor_geometry;
   int request_width, request_height, request_resolution, request_quality, short_exposures,
       number_of_sensor, multisensor_resolution, multisensor_is_zoomed; // only for multisensor camera
   long clighting, lightingfix;
   unsigned char md_matrix[8];
   enum LOW_LIGHT { SPEED, QUALITY, BALANCED, CUSTOM, HIGH_SPEED, MOON_LIGHT };
public:
   unsigned char pc_camera_make[16];
public:
   // functions for set parameter
   void Gamma(long);
   void Brightness(long);
   void AutoExposition(long);
   void Sharpness(long);
   void Saturation(long);
   void Blue(long);
   void Red(long);
   void Speed(long);
   void Illumination(long);
   void Lighting(long);
   void Lightingfix(long);
   void LowLightMode(long);
   void Roll(long);
   void IrisEnable(long);
   void IrisSpeed(long);
   void IrisGain(long);
   void IrisReposEnable(long);
   void IrisReposFStops(long);
   void IrisReposFStopsMin(long);
   void IrisReposPeriod(long);
   void IrisReposStabPeriod(long);
   void DayNightMode(long);
   void DayNightTriggerNight(long);
   void DayNightTriggerDay(long);
   void ExposureMode(long);
   void ExposureWindowLeft(long);
   void ExposureWindowTop(long);
   void ExposureWindowWidth(long);
   void ExposureWindowHeight(long);

   void SensorLeftChecked(long);
   void SensorTopChecked(long);
   void SensorWidthChecked(long);
   void SensorHeightChecked(long);
   void SensorBlackWhiteLeftChecked(long);
   void SensorBlackWhiteTopChecked(long);
   void SensorBlackWhiteWidthChecked(long);
   void SensorBlackWhiteHeightChecked(long);

   void SensorLeftRaw(long);
   void SensorTopRaw(long);
   void SensorWidthRaw(long);
   void SensorHeightRaw(long);
   void SensorBlackWhiteLeftRaw(long);
   void SensorBlackWhiteTopRaw(long);
   void SensorBlackWhiteWidthRaw(long);
   void SensorBlackWhiteHeightRaw(long);
   void RequestLeftChecked(long);
   void RequestLeftRaw(long);
   void RequestTopChecked(long);
   void RequestTopRaw(long);
   void RequestWidthChecked(long);
   void RequestWidthRaw(long);
   void RequestHeightChecked(long);
   void RequestHeightRaw(long);
   void RequestResolution(long);
   void RequestQuality(long);
   void RequestedBlockSize(long);
   void MdEnabled(long);
   void MdMode(long);
   void MdLevelThresh(long);
   void MdTotalZones(long);
   void MdZoneSize(long);
   void MdDetail(long);
   void MdExplosureSensitivity(long);
   void MdMatrix(long);
   void MsBitRate(long val);
   long MsBitRate();

   void MsNumberOfSensor(long);
   void MsChannelEnable(long);
   void MsFullResEnable(long);
   void MsZoomWinEnable(long);
   void MsOneShotEnable(long);
   void MsQuadMode(long);

   void ShortExposures(long);
   void TimeoutOnCamera(long);
   void RepeatFromCamera(long);

   void ScEnabled(long);
   void ScStrobe(long);
   void ScAlwaysSend(long);
   void ScOutputHigh(long);
   void ScOutputLow(long);

   // functions for get parameter
   long Gamma();
   long Brightness();
   long AutoExposition();
   long Sharpness();
   long Saturation();
   long Blue();
   long Red();
   long Speed();
   long Illumination();
   long Lighting();
   long Lightingfix();
   long LowLightMode();
   long Roll();
   long IrisEnable();
   long IrisSpeed();
   long IrisGain();
   long IrisReposEnable();
   long IrisReposFStops();
   long IrisReposFStopsMin();
   long IrisReposPeriod();
   long IrisReposStabPeriod();
   long DayNightMode();
   long DayNightTriggerNight();
   long DayNightTriggerDay();
   long ExposureMode();
   long ExposureWindowLeft();
   long ExposureWindowTop();
   long ExposureWindowWidth();
   long ExposureWindowHeight();
   long SensorLeft();
   long SensorTop();
   long SensorWidth();
   long SensorHeight();
   long SensorBlackWhiteLeft();
   long SensorBlackWhiteTop();
   long SensorBlackWhiteWidth();
   long SensorBlackWhiteHeight();
   long RequestLeft();
   long RequestTop();
   long RequestWidth();
   long RequestHeight();
   long RequestResolution();
   long RequestQuality();
   long RequestedBlockSize();
   long MdEnabled();
   long MdMode();
   long MdLevelThresh();
   long MdTotalZones();
   long MdZoneSize();
   long MdDetail();
   long MdExplosureSensitivity();
   long MdMatrix();

   long MsNumberOfSensor();
   long MsChannelEnable();
   long MsFullResEnable();
   long MsZoomWinEnable();
   long MsOneShotEnable();
   long MsIsZoomed();
   long MsQuadMode();
   long ShortExposures();
   long TimeoutOnCamera();
   long RepeatFromCamera();

   long ScEnabled();
   long ScStrobe();
   long ScAlwaysSend();
   long ScOutputHigh();
   long ScOutputLow();
   long ScOptoInput();

   virtual void DoChangeCamera();
   void PatchRollForVersionLess51719(long);
   long MaxOfDayNightTriggerNight(long low_light_mode);

   void CorrectLeft(const int max, const int divl, const int divw, long& left, long& width);
   void CorrectWidth(const int max, const int divl, const int divw, long& left, long& width);
   void Correct2100Sencor14RegisterBug();
   void UpdateSensorGeometry();

   unsigned MsSensorAsBits(unsigned); // переводит номер сенсора в биты
public:
   explicit AV2000ClientInherit1(int);
   virtual ~AV2000ClientInherit1();
   Camera UpdateCamera();
   Camera IdentOfCamera() const;
   unsigned long Version() const;
   unsigned long Model() const;
   unsigned long Revision() const;
   unsigned long Mini() const;
   unsigned long DayNight() const;
   void Permanently();
   void SetCustomMode(long& knee_point, long& max_analog_gain, long& max_knee_gain, long& max_exposure_time, long& max_digital_gain);
   void GetCustomMode(long& knee_point, long& max_analog_gain, long& max_knee_gain, long& max_exposure_time, long& max_digital_gain);

   friend struct LightingConformity;
   friend struct LightingCoefs;
};
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
struct ImageSize
{
    ImageSize():width(0), height(0){}
    int width, height;
};

class MpegRefSolver
{
private:
        typedef std::map<int, ImageSize> cont_type;
        cont_type images;

        ImageSize& get(int id)
        {
		cont_type::iterator it = images.find(id);
		if (it == images.end())
                {
                        images[id] = ImageSize();
                        return images[id];
                }
		return (it->second);
        }

public:
        bool haveToBeIframe(int w, int h, int stream_id)
        {
            ImageSize& size = get(stream_id);

            bool result = (w!=size.width || h!=size.height );

            size.width = w;
            size.height = h;

            return result;
        }

        void haveToBeIframe(int stream_id)
        {
            ImageSize& size = get(stream_id);
            size.width = 0;
            size.height = 0;
        }

        void haveToBeIframeAll()
        {
                cont_type::iterator it = images.begin();
                for (;it!=images.end();++it)
                {
                        (it->second).width = 0;
                        (it->second).height = 0;
                }
        }

};

//---------------------------------------------------------------------------


class AV2000Calculate : public AV2000ClientInherit1{
   class Quality;
protected:
   const int divisible_width, divisible_height;
   int max_request_width, max_request_height, max_header_height;
   Quality* quality;
   GString filename;
   IMAGE_RESOLUTION resolution;
   ZoomInfo zinfo;
   int doublescan, black_white;
   bool per_cent_size;
   unsigned short ssn;
   long dynamic_quality, use_dynamic_quality;
   MpegRefSolver mMpegHelper;
private:
   void CalculateZoomHeaderRect (float zoom, int dx, int dy, int coef, int& header_width, int& header_height, GRect& request_rect);
   //void AddParameterToSendString(GString& msg, const GString& key, const GString& val);
protected:
   virtual void DoChangeCamera();
public:
   explicit AV2000Calculate(int);
   virtual ~AV2000Calculate();
   void Resolution(IMAGE_RESOLUTION);
   IMAGE_RESOLUTION Resolution();
   void Zoom(const ZoomInfo&);
   ZoomInfo Zoom() const;
   void DoubleScan(long);
   long DoubleScan();
   long CorrectQuality(long);
   void SetDynamicQuality(long);
   void QualityFull(long);
   void QualityHalf(long);
   void QualityZoom(long);
   void Width(long);
   void Height(long);
   void PerCentImageRectangle(long);
   long QualityFull();
   long QualityHalf();
   long QualityZoom();
   long PerCentImageRectangle();
   long Width();
   long Height();

   void makeAskIFrameFirst();

   GString CalculateRequest(CodecID codec, int streamId, int* Ifarme);
   GString CalculateRequest(int, int, int, int, CodecID codec, int streamId, int* Ifarme, int kbps, int intraFramePeriod, int channel = 0);
   void AddRectToRequest3130(GString&, const GRect&);
   int BlackWhite() const;
};
//---------------------------------------------------------------------------
class AV2000Client : public AV2000Calculate{
   class Packet;
   class Default;
   struct Size{
      int x0, y0, width, height;
   };
private:
   typedef void (AV2000Client::*HANDLER_SET_PROPERTY)(long);
   typedef long (AV2000Client::*HANDLER_GET_PROPERTY)();
   typedef std::map<CAMERA_PARAMETER, HANDLER_SET_PROPERTY> HANDLERS_SET_PROPERIES;
   typedef std::map<CAMERA_PARAMETER, HANDLER_GET_PROPERTY> HANDLERS_GET_PROPERIES;
protected:
   static Default deflt;
   HANDLERS_SET_PROPERIES handlers_set_property;
   HANDLERS_GET_PROPERIES handlers_get_property;
   CodecID mprevCodec;
protected:
   GBaseMemoryStream* memory;
   //GString request;
   unsigned long begin_memory_position;
   unsigned packet, receive_begin_timeout;
   Packet* end_packet;
   IRIS_STATUS iris_status;
   unsigned char mac[6];
   enum { motion_size = 64 };
   unsigned char motion_array[motion_size];
   unsigned char *pc_time_stamp;
   int status_aux_in, status_aux_out;
   unsigned char last_packet[1024]; // I suppose that size of lasr packet <= 1024

   int m_channel;
private:
   void BeforeImage(CodecID codec, int streamId, int* Ifarme);
   void BeforeImage(int, int, int, int, CodecID codec, int streamId, int* Ifarme, int kbps, int intraFramePeriod);
   void BeforeImageDefault(CodecID codec, int streamId, int* Ifarme);
   void AfterImage(CodecID codec, int* Ifarme);
   void OverheadInformation();

   bool isH264Model() const;


   unsigned int createSPS_PPS(unsigned char* data); //sergey



   IRIS_STATUS UpdateIrisStatus(GStream& strm, unsigned long idx);
   void ResetMac();
   Size ExtractSize(unsigned char*);

private:
   void OnBeforeClientOperation(GIdTFTPClient::OPERATION);
   void OnAfterClientConnect(GIdTFTPClient::OPERATION);
   void OnClientWork(GIdTFTPClient::WORK_MODE, unsigned, unsigned long size);
   void OnEndClientOperation(GIdTFTPClient::OPERATION);
protected:
   virtual void OnCameraReinited();
   virtual void DoChangeCamera();
public:
   //unsigned char buff[256];
   //const unsigned char last_request;
   GString request;
   explicit AV2000Client(int n);
   virtual ~AV2000Client();
   void BufferImage(GBaseMemoryStream*);
   GBaseMemoryStream* BufferImage() const;
   void Image(CodecID codec, int streamId, int* Ifarme);
   void Image(int left, int top, int width, int height, CodecID codec, int streamId, int* Ifarme, int kbps, int intraFramePeriod);
   void SetChannel(int channel);
   void ImageDefault(CodecID codec, int streamId, int* Ifarme);
   const unsigned char* Mac() const;
   void Parameter(CAMERA_PARAMETER, long value);
   long Parameter(CAMERA_PARAMETER);
   void ReceiveBeginTimeout(unsigned);
   unsigned ReceiveBeginTimeout() const;
   void RepeatAknowlegements(unsigned);
   unsigned RepeatAknowlegements() const;
   void FactoryDefault();
   IRIS_STATUS IrisStatus() const;
   const unsigned char* MotionArray() const;
   const unsigned char* GetFirmwareTimestamp() const;
   const unsigned char* LastPacket() const;
   const unsigned char* GetCameraMakeBuffer() const;
   const int GetAuxInStatus() const;
   const int GetAuxOutStatus() const;
};

//---------------------------------------------------------------------------
template <class T> class AV2000ClientUse : public AV2000Client{
   typedef void (T::*Handler_OnCameraReinited)();
private:
   T* object;
   Handler_OnCameraReinited handler_on_camera_reinited;
private:
   virtual void OnCameraReinited();
public:
   explicit AV2000ClientUse(int, T*);
   AV2000ClientUse(int, T*, Handler_OnCameraReinited);
   virtual ~AV2000ClientUse(){}
   void DefineHandlerOnCameraReinited(Handler_OnCameraReinited);
};

//---------------------------------------------------------------------------

// несколько в одном для смены портов
class AV2000ClientBase::Client{
   AV2000ClientBase& loader;
   const unsigned requested_block_size;
   unsigned size, number, port, receive_timeout, repeat_aknowlegements;
   GString host;
   std::vector<GIdTFTPClientUse<AV2000ClientBase>*> clients;
private:
   void UpdateNumber();
protected:
   virtual void OnBeforeClientOperation(GIdTFTPClient::OPERATION){}
   virtual void OnAfterClientConnect(GIdTFTPClient::OPERATION){}
   virtual void OnClientWork(GIdTFTPClient::WORK_MODE, unsigned, unsigned long size){}
   virtual void OnEndClientOperation(GIdTFTPClient::OPERATION){}
public:
   Client(AV2000ClientBase&, unsigned size);
   ~Client();
   void Host(const GString&);
   const char* Host() const;
   void Port(unsigned);
   unsigned Port() const;
   void ReceiveTimeout(unsigned);
   unsigned ReceiveTimeout() const;
   void RepeatAknowlegements(unsigned);
   unsigned RepeatAknowlegements() const;
   void RequestedBlockSize(unsigned);
   unsigned RequestedBlockSize() const;
   void Get(const GIdString& name, GBaseMemoryStream& mem);
   void Put(const GBaseMemoryStream&, const GIdString& name);
   GIdTFTPClientUse<AV2000ClientBase>* operator[](unsigned);
   GIdTFTPClientUse<AV2000ClientBase>* Current();
   unsigned Size() const;
};
//---------------------------------------------------------------------------
struct AV2000ClientInherit1::Camera{
   unsigned long model, version, revision, proc_fpga, net_fpga, mini, ai, dn;
   unsigned long isregular, isH264, isdome, ispanoramic, isAV818X, isdualsensor; 
   //-------------------------
   Camera();
   Camera(unsigned long, unsigned long);
   bool operator==(const Camera&) const;
   bool operator!=(const Camera&) const;
};

//---------------------------------------------------------------------------
class AV2000ClientInherit1::Available{
   typedef std::map<unsigned long, std::map<CAMERA_PARAMETER, bool> > AVAILABLE_BY_MODEL;
   typedef std::map<unsigned long, std::map<CAMERA_PARAMETER, bool>, CompareInvert<unsigned long> > AVAILABLE_BY_VERSION;
   typedef std::map<unsigned long, std::map<CAMERA_PARAMETER, bool>, CompareInvert<unsigned long> > AVAILABLE_BY_REVISION;
private:
   AVAILABLE_BY_MODEL available_by_model;
   AVAILABLE_BY_VERSION available_by_version;
   AVAILABLE_BY_REVISION revision;
public:
   Available();
   bool operator()(CAMERA_PARAMETER, const Camera&);
   long operator()(CAMERA_PARAMETER, const Camera&, long);
};
//---------------------------------------------------------------------------

struct AV2000ClientInherit1::LightingConformity : public std::map<long, std::map<unsigned long/*camera_model*/, std::map<GByte, unsigned> > >{
   LightingConformity();
};
//---------------------------------------------------------------------------

struct AV2000ClientInherit1::LightingCoefs : public std::map<long /*lighting value*/, std::map<unsigned long/*camera_model*/, std::map<GByte, std::pair<long/*A*/, long/*B*/ > > > >{
   LightingCoefs();
	
   long operator()(const std::pair<long, long>& coef, long sensor_w, bool=false);
   bool IsExists(long lighting, unsigned long model);
};
//---------------------------------------------------------------------------

class AV2000ClientInherit1::SensorSizeMax{
   typedef std::map<unsigned long, std::pair<unsigned, unsigned> > CONFORMITY;
   CONFORMITY conformity;
public:
   SensorSizeMax();
   int Width(const Camera&) const;
   int Height(const Camera&) const;
   std::pair<unsigned, unsigned> Range(const Camera&) const;
};
//---------------------------------------------------------------------------


class AV2000Calculate::Quality{
   typedef std::map<unsigned long, std::map<int, float>,  CompareInvert<unsigned long> > CONFORMITY_QUANTINIZATION;
private:
   struct HeaderInfo;
   struct Conformity;
private:
   static Conformity conformity_quantization;
   const int restart_interval;
   std::map<IMAGE_RESOLUTION, int> value;
   std::map<HeaderInfo, std::vector<GByte> >* pheaders;
   std::vector<GByte>* header;
   std::map<int, float> quantization_table;
private:
   void CalculateHeader(std::vector<GByte>&, int, int, int, float, float);
public:
   Quality();
   ~Quality();
   int operator()(IMAGE_RESOLUTION);
   void Set(IMAGE_RESOLUTION, int);
   void LoadHeader(int, int, IMAGE_RESOLUTION);
   void LoadHeaderByQuality(int, int, int);
   void WriteHeader(GStream& dest);
   const std::vector<GByte>& Header() const;
   void DoChangeCamera(const Camera&);
};
//---------------------------------------------------------------------------
struct AV2000Calculate::Quality::Conformity : public CONFORMITY_QUANTINIZATION{
   //------------------------------------
   Conformity();
};
//---------------------------------------------------------------------------
struct AV2000Calculate::Quality::HeaderInfo{
   int columns, rows, coef;
   //---------------------------
   HeaderInfo();
   HeaderInfo(int img_col, int img_row, int coef);
   bool operator<(const HeaderInfo& right) const;
};
//---------------------------------------------------------------------------

class AV2000Client::Packet{
   unsigned number, size;
public:
   Packet();
   void operator()(unsigned nm, unsigned sz);
   unsigned Size() const;
   unsigned Number() const;
   void Reset();
};
//---------------------------------------------------------------------------
class AV2000Client::Default : public std::map<CAMERA_PARAMETER, long>{
public:
   Default();
};
//---------------------------------------------------------------------------
ZoomInfo MakeZoomInfo(float, int, int);
//---------------------------------------------------------------------------


template <class T> AV2000ClientUse<T>::AV2000ClientUse(int n, T* t)
   : AV2000Client(n), object(t), handler_on_camera_reinited(0)
{
}
//---------------------------------------------------------------------------
template <class T> AV2000ClientUse<T>::AV2000ClientUse(int n, T* t, Handler_OnCameraReinited handler)
   : AV2000Client(n), object(t), handler_on_camera_reinited(handler)
{
}
//---------------------------------------------------------------------------
template <class T> void AV2000ClientUse<T>::OnCameraReinited()
{
   AV2000Client::OnCameraReinited();
   if(object && handler_on_camera_reinited)
      (object->*handler_on_camera_reinited)();
}
//---------------------------------------------------------------------------
template <class T> void AV2000ClientUse<T>::DefineHandlerOnCameraReinited(Handler_OnCameraReinited handler)
{
   handler_on_camera_reinited = handler;
}
//---------------------------------------------------------------------------

#endif
