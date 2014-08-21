

#define _linux_

#include <fstream>
#include <iostream>
#include <string.h>
#include <map>
#include "AV2000Types.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef _windows_
   #include <windows.h>
   #include "AVLoadDll.h"
#endif

#ifdef _linux_
   #include "AVLoadSo.h"
#endif
//----------------------------------------------------------------
typedef std::map<std::string, std::string> PARAMS;
//----------------------------------------------------------------
void GetImage(AV2000F&, PARAMS&);
void GetImageH264(AV2000F&, PARAMS&);
void GetImageDef(AV2000F&, PARAMS&);
void CameraParameter(AV2000F&, PARAMS&);
void FactoryDefault(AV2000F&, const char* ip);
void Permanently(AV2000F&, const char* ip);
void FindCameras(AV2000F&);

void Allocate(char**, unsigned long*);
void Deallocate(char*);
void Reinited(int);
void Usage();
//----------------------------------------------------------------

int main(int argc, char* argv[])
{
   AV2000F dll;
   try{
      dll.Initialization();
   }
   catch(std::exception& ex){
      std::cout << "\nerror(s) on load library, description: " << ex.what() << std::endl;
      return 0;
   }
	dll.pSetAllocateFunction(Allocate);
	dll.pSetDeallocateFunction(Deallocate);
	
   dll.pReinitedFunction(Reinited);
	
	if(argc < 2){ std::cout << "no parameters into the command line" << std::endl; return 0; }

	// check for print help
	if(strchr(argv[1], '?')) {
      Usage();
      return 0;
   }


	int i;
	// read all parameters from command line
	PARAMS params;
	for(i = 2; i <= argc; i += 1)
		if(argv[i-1][0] == '-')
			if(i < argc)
            params[argv[i-1]] = argv[i];
			else
            params[argv[i-1]];

	/* read mode program
		img - get image;
		gcp - get camera parameter, if add "-all" - get all known parameters
		scp - set camera parameter
      fdf - factory defaults
      prm - permanently save
      find - find cameras

	*/

	PARAMS::iterator iter = params.find("-md");
	
	if(iter == params.end()){ std::cout << "\"-md\" - not found"; return 0; }

	
 if("find" == iter->second){
	FindCameras(dll);
	
      return 0;
   }

	// read IP address
	if(params.find("-ip") == params.end()) {
      std::cout << "IP address of camera not found\n";
      return 0;
   }

	if(iter->second == "img"){
      GetImage(dll, params);
      return 0;
   }

	if(iter->second == "h264"){
		GetImageH264(dll, params);
		return 0;
	}

	if(iter->second == "imgdef"){
      GetImageDef(dll, params);
      return 0;
   }


   if(iter->second == "gcp" || iter->second == "scp"){
      CameraParameter(dll, params);
      return 0;
   }
   if("fdf" == iter->second){
      FactoryDefault(dll, params["-ip"].c_str());
      return 0;
   }
   if("prm" == iter->second){
      Permanently(dll, params["-ip"].c_str());
      return 0;
   }
   std::cout << "mode: " << '\"' << iter->second << '\"' << " is unknown\n";
	return 0;
}
//----------------------------------------------------------------
void GetImage(AV2000F& dll, PARAMS& params)
{
	float zoom = 1.0;
	int dx = 0, dy = 0;
	IMAGE_RESOLUTION res = imFULL;
	PARAMS::iterator iter = params.find("-out"); // out file with image
	if(iter == params.end()) params["-out"] = "AV_Image.jpg";
	if((iter = params.find("-res")) != params.end())
		if(iter->second == "half") res = imHALF;
	if((iter = params.find("-zoom")) != params.end()){
      zoom = atof(params["-zoom"].c_str());
      res = imZOOM;
   }
	if((iter = params.find("-dx")) != params.end()) dx = atoi(params["-dx"].c_str());
	if((iter = params.find("-dy")) != params.end()) dy = atoi(params["-dy"].c_str());


	// using client
   unsigned long size = 8192, capacity;
	char* data = new char[size]; // image
   int ret;

	if(ret = dll.pCreateClient(1)){
      std::cout << "\nerror on create client, code: " << ret << '\n';
      return;
   }
	dll.pSetClientIp(1, params["-ip"].c_str());

	dll.pSetClientBuffer(1, data, size);

   if(dll.pUpdateVersion(1) && dll.pGetImage2(1, &data, &size, &capacity, res, zoom, dx, dy)){
      std::cout << "Camera idents: " << "model: " << dll.pModel(1) << ", version: " <<  dll.pVersion(1) << '\n';
		std::ofstream fout(params["-out"].c_str(), std::ios_base::binary);
		if(!fout)
			std::cout << "file \"" << params["-out"] << "\" could not be opened\n";
		else{ // save image
			for(unsigned long i = 0; i < size; i++)
				fout << data[i];
			if(!fout != true)
				std::cout << "image saved to file: \"" << params["-out"] << "\"\n";
		}
	}
	else{

      const ClientError* pcler = dll.pGetLastClientError(1);

		std::cout << "error: code: " << pcler->code << ", description: " << pcler->description << '\n';
	}
	dll.pDestroyClient(1);
}
//----------------------------------------------------------------
void GetImageH264(AV2000F& dll, PARAMS& params)
{
	// using client
	unsigned long size = 1024*1024, capacity = 1024*1024;
	char* data = new char[size]; // image
	int ret;
	
	if(ret = dll.pCreateClient(1))
	{
		std::cout << "\nerror on create client, code: " << ret << '\n';
		return;
	}
	dll.pSetClientIp(1, params["-ip"].c_str());
	
	dll.pSetClientBuffer(1, data, size);
	

	if(dll.pUpdateVersion(1) )
	
	{
		
		
		//if(!dll.pGetDefaultImage(1, &data, &size, &capacity)) return;
		PARAMS::iterator iter = params.find("-out"); // out file with image
		if(iter == params.end()) params["-out"] = "test.mpg";


		std::ofstream fout(params["-out"].c_str(), std::ios_base::binary);
		std::cout << "Camera model: " << dll.pModel(1) << " / " <<  dll.pVersion(1) << '\n';
		if(!fout)
			std::cout << "can't open file\n";
		else
		{

			IMAGE_RESOLUTION res = imFULL;
			if((iter = params.find("-res")) != params.end())
				if(iter->second == "half") res = imHALF;
			long quality = 15; // 
			int left = 0, top = 0, width = 1600, height = 1184;
			int codec = H264_CODEC;
			int streamId = 1;
			int kbps = 0;// bit rate control off
			int IntraFramePeriod = 0;// if bit rate control off, then this value is ignored


			for (int frame=0; frame < 25; ++frame)
			{


			        int IFrame = 0; //request P frame if possible. if not available
                                                //SDK will change it to 1, so it must be set every time 
				ret = dll.pGetWindowImageQEx(1, &data, &size , &capacity, imFULL, 0, quality, left, top, width, height, codec, streamId, &IFrame, kbps, IntraFramePeriod);


				std::cout << " return " << size << "bytes" << '\n';
				for(unsigned long i = 0; i < size; i++)
					fout << data[i];
			}

			std::cout << "image saved to file: \"" << params["-out"] << "\"\n";

		}
	}
	else
	{

		const ClientError* pcler = dll.pGetLastClientError(1);

		std::cout << "error: code: " << pcler->code << ", description: " << pcler->description << '\n';
	}
	dll.pDestroyClient(1);

}
//----------------------------------------------------------------
void GetImageDef(AV2000F& dll, PARAMS& params)
{
	PARAMS::iterator iter = params.find("-out"); // out file with image
	if(iter == params.end()) params["-out"] = "AV_Image.jpg";

	IMAGE_RESOLUTION res = imFULL;
        if((iter = params.find("-res")) != params.end())
		if(iter->second == "half") res = imHALF;

	// using client
	unsigned long size = 1024*1024, capacity = 1024*1024;
	char* data = new char[size]; // image
	int ret;

	if(ret = dll.pCreateClient(1)){
		std::cout << "\nerror on create client, code: " << ret << '\n';
		return;
	}
	dll.pSetClientIp(1, params["-ip"].c_str());

	dll.pSetClientBuffer(1, data, size);

	if(dll.pUpdateVersion(1) )
	{
                long num_of_sensor;

		if(!dll.pGetDefaultImage(1, &data, &size, &capacity)) return;

                dll.pGetAV2000Parameter(1,cpMS_NUMBER_OF_SENSOR, &num_of_sensor);

		std::cout << "Camera model: " << dll.pModel(1) << " / " <<  dll.pVersion(1) << '\n';
                std::cout << "chanel " << num_of_sensor << " return " << size << "bytes" << '\n';
                char buf[256];
		sprintf(buf,(char*)"IMG_CH%d.jpg",(int)num_of_sensor);
		std::ofstream fout(buf, std::ios_base::binary);
		if(!fout)
			std::cout << "can't open file\n";
		else{ // save image
			for(unsigned long i = 0; i < size; i++)
				fout << data[i];
			if(!fout != true)
				std::cout << "image saved to file: \"" << buf << "\"\n";
		}
	}
	else{

      		const ClientError* pcler = dll.pGetLastClientError(1);

		std::cout << "error: code: " << pcler->code << ", description: " << pcler->description << '\n';
	}
	dll.pDestroyClient(1);
}
//----------------------------------------------------------------
void FactoryDefault(AV2000F& dll, const char* ip)
{
   const int number = 1;
   const ClientError* perr;
	if(dll.pCreateClient(number)){
      std::cout << "\nerror on create client, code: " << dll.pGetLastClientError(number)->code << '\n';
      return;
   }
	if( !dll.pSetClientIp(number, ip) ){
      std::cout << "\nbad ip, code: " << dll.pGetLastClientError(number)->code  << '\n';
      dll.pDestroyClient(number);
      return;
   }
   if( !dll.pUpdateVersion(number) ){
      std::cout << "\ncamera version not updated, code: " << dll.pGetLastClientError(number)->code << '\n';
      dll.pDestroyClient(number);
      return;
   }
   std::cout << "camera: model: " << dll.pModel(number) << ", version: " << dll.pVersion(number) << '\n';
   if( !dll.pFactoryDefault(number) ){
      std::cout << "\nerror on FactoryDefault, code: " << dll.pGetLastClientError(number)->code << '\n';
      dll.pDestroyClient(number);
      return;
   }
   dll.pDestroyClient(number);
   std::cout << std::endl << "FactoryDefault: success" << std::endl;
}
//----------------------------------------------------------------
void Permanently(AV2000F& dll, const char* ip)
{
   const int number = 1;
   const ClientError* perr;
	if(dll.pCreateClient(number)){
      std::cout << "\nerror on create client, code: " << dll.pGetLastClientError(number)->code << '\n';
      return;
   }
	if( !dll.pSetClientIp(number, ip) ){
      std::cout << "\nbad ip, code: " << dll.pGetLastClientError(number)->code  << '\n';
      dll.pDestroyClient(number);
      return;
   }
   if( !dll.pUpdateVersion(number) ){
      std::cout << "\ncamera version not updated, code: " << dll.pGetLastClientError(number)->code << '\n';
      dll.pDestroyClient(number);
      return;
   }
   std::cout << "camera: model: " << dll.pModel(number) << ", version: " << dll.pVersion(number) << '\n';
   if( !dll.pPermanently(number) ){
      std::cout << "\nerror on Permanently, code: " << dll.pGetLastClientError(number)->code << '\n';
      dll.pDestroyClient(number);
      return;
   }
   dll.pDestroyClient(number);
   std::cout << std::endl << "Permanently: success" << std::endl;
}
//----------------------------------------------------------------
	
struct PAR{
	long value;
	std::string name;
};
//---------------------

void CameraParameter(AV2000F& dll, PARAMS& params)
{

	int success,
       all = (params.find("-all") != params.end()),
       deflt = (params.find("-default") != params.end()),
       ret;

	std::map<CAMERA_PARAMETER, PAR> map;

   const ClientError* perr;

	int get = (params["-md"] == "gcp" ? 1 : 0);
	if((all && get) || params.find("-bright") != params.end() ) { map[cpBRIGHTNESS].value = atoi(params["-bright"].c_str()); map[cpBRIGHTNESS].name = "brightness"; }
	if((all && get) || params.find("-sharp") != params.end()) { map[cpSHARPNESS].value = atoi(params["-sharp"].c_str()); map[cpSHARPNESS].name = "sharpness"; }
	if((all && get) || params.find("-saturation") != params.end() ) { map[cpSATURATION].value = atoi(params["-saturation"].c_str()); map[cpSATURATION].name = "saturation"; }
	if((all && get) || params.find("-blue") != params.end() ) { map[cpBLUE].value = atoi(params["-blue"].c_str()); map[cpBLUE].name = "ajustment blue"; }
	if((all && get) || params.find("-red") != params.end() ) { map[cpRED].value = atoi(params["-red"].c_str()); map[cpRED].name = "ajustment red"; }
	if((all && get) || params.find("-illumination") != params.end() ) { map[cpILLUMINATION].value = atoi(params["-illumination"].c_str()); map[cpILLUMINATION].name = "illumination"; }
	if((all && get) || params.find("-light") != params.end() ) { map[cpLIGHTING].value = atoi(params["-light"].c_str()); map[cpLIGHTING].name = "lighting"; }
   if((all && get) || params.find("-mode") != params.end() ) { map[cpCAMERA_MODE].value = atoi(params["-mode"].c_str()); map[cpCAMERA_MODE].name = "mode"; }
   if((all && get) || params.find("-qfull") != params.end() ) { map[cpQUALITY_FULL].value = atoi(params["-qfull"].c_str()); map[cpQUALITY_FULL].name = "quality full"; }
   if((all && get) || params.find("-qhalf") != params.end() ) { map[cpQUALITY_HALF].value = atoi(params["-qhalf"].c_str()); map[cpQUALITY_HALF].name = "quality half"; }
   if((all && get) || params.find("-qzoom") != params.end() ) { map[cpQUALITY_ZOOM].value = atoi(params["-qzoom"].c_str()); map[cpQUALITY_ZOOM].name = "quality zoom"; }
   if((all && get) || params.find("-scan") != params.end() ) { map[cpDOUBLESCAN].value = atoi(params["-scan"].c_str()); map[cpDOUBLESCAN].name = "double scan"; }
   if((all && get) || params.find("-roll") != params.end() ) { map[cpROLL].value = atoi(params["-roll"].c_str()); map[cpROLL].name = "roll"; }

   if((all && get) || params.find("-iris") != params.end() ) { map[cpIRIS_ENABLED].value = atoi(params["-iris"].c_str()); map[cpIRIS_ENABLED].name = "iris enabled"; }

	//using client
	if(ret = dll.pCreateClient(1)){
      std::cout << "\nerror on create client, code: " << ret << '\n';
      return;
   }
	dll.pSetClientIp(1, params["-ip"].c_str());

   //std::cout << "\nIP: " << dll.pGetClientIp(1) << '\n';

   if(dll.pUpdateVersion(1)){
      std::cout << "camera: model: " << dll.pModel(1) << ", version: " << dll.pVersion(1) << '\n';

      if(deflt){ // reset to default if need
         std::cout << "\ntry to factory default: ";
         dll.pFactoryDefault(1) ? std::cout << "ok" : std::cout << "failed";
         dll.pDestroyClient(1);
         return;
      }

	   for(std::map<CAMERA_PARAMETER, PAR>::iterator iter = map.begin(); iter != map.end(); iter++){
		   std::cout << iter->second.name << " : ";
		   if(get){
			   // 1 - number of client, iter->first - CAMERA_PARAMETER, data - getting value
			   success = dll.pGetAV2000Parameter(1, iter->first, &iter->second.value);

			   if(success) std::cout << iter->second.value;
		   }
		   else{
			   // 1 - number of client, iter->first - CAMERA_PARAMETER, data - setting value

			   success = dll.pSetAV2000Parameter(1, iter->first, &iter->second.value);
			   if(success) std::cout << "ok";
		   }
		   if(!success){
			   perr = dll.pGetLastClientError(1);
			   std::cout << "error: code: " << perr->code << " description: " << perr->description;
		   }
	   	std::cout << '\n';
	   }
   }
   else{
      perr = dll.pGetLastClientError(1);
      std::cout << perr->description << '\n';
   }
	dll.pDestroyClient(1);
}
//----------------------------------------------------------------
void FindCameras(AV2000F& dll)
{

   unsigned number = 32;	
   if(dll.pFindCameras(&number,50 )){
	

	
      std::cout << "detected " << number << " cameras\n";
      AV2000Addr* paddrs = new AV2000Addr[number];
      dll.pGetCameras(paddrs, &number);
      for(unsigned cam = 1; cam <= number; cam++)
      {
         char buff[256];
         sprintf(buff, (char*)"%02X-%02X-%02X-%02X-%02X-%02X",
                 static_cast<unsigned>(paddrs[cam-1].mac[0]),
                 static_cast<unsigned>(paddrs[cam-1].mac[1]),
                 static_cast<unsigned>(paddrs[cam-1].mac[2]),
                 static_cast<unsigned>(paddrs[cam-1].mac[3]),
                 static_cast<unsigned>(paddrs[cam-1].mac[4]),
                 static_cast<unsigned>(paddrs[cam-1].mac[5])
                ); 
         std::cout << "mac: " << buff;
 
         std::cout << "   ip: " << static_cast<unsigned>(paddrs[cam-1].ip[0]);
         for(unsigned i = 1; i < 4; i++)
            std::cout << '.' << static_cast<unsigned>(paddrs[cam-1].ip[i]);

         std::cout << std::endl;
      }
   }
   else{
      const ClientError* perr = dll.pGetLastSetError();
      std::cout << "Error on find cameras: " << perr->description << ", code: " << perr->code << '\n';
   }
}

//----------------------------------------------------------------
void Usage()
{
	std::cout << "\nUSAGE:\n"
			  << "   -switch1 value1 -switch2 value2 ... , where:\n"
			  << "   \"switch\" - name of parameter\n"
			  << "   \"value\" - value of parameter(if need)\n"
			  << "\nallowable parameters:\n"
			  << "  \"md\" - mode of client\n"
			  << "     allowable values for \"md\": \n"
			  << "     \"img\" - get image from camera,\n"
			  << "     \"h264\" - get 25 frames of H.264 stream,\n"
			  << "     \"gcp\" - get current parameters of camera(brightness, saturation etc.),\n"
			  << "     \"scp\" - set current parameters of camera\n"
           << "     \"fdf\" - reset camera settings to factory defaults\n"
           << "     \"prm\" - permanently save camera registers\n"
           << "     \"find\" - find present cameras\n"
			  << "   \"ip\" - IP address of camera\n"
			  << "   \"zoom\" - zoom\n"
			  << "   \"dx\" - axial displacement X (only for get image)\n"
			  << "   \"dy\" - axial displacement Y (only for get image)\n"
			  << "   \"out\" - output image file (only for get image), default value is \"AV2000Image.jpg\"\n"
			  << "   \"res\" - image resolution  (only for get image), value may be \"full\" or \"half\")\n"
			  << "   \"bright\" - \n"
			  << "   \"sharp\" - \n"
			  << "   \"saturation\" - \n"
			  << "   \"blue\" - \n"
			  << "   \"red\" - \n"
			  << "   \"illumination\" - \n"
			  << "   \"light\" - \n"
           << "   \"mode\" - \n"
           << "   \"qfull\" - \n"
           << "   \"qhalf\" - \n"
           << "   \"qzoom\" - \n"
           << "   \"scan\" - this parameters only for get or set current parameters of camera\n"
			  << "   \"all\" - load all known parameters of camera (only for get current parameters of camera).\n"
			  << "\nEXAMPLES: \n"
			  << "   test -md img -ip 192.168.254.10 -res full -zoom 0.33 -dx -10 -dx 30 -out image.jpg\n"
			  << "   test -md img -ip 192.168.254.10 -res half\n"
			  << "   test -md h264 -ip 192.168.254.10 -res full -out test.mpg\n"
			  << "   test -md gcp -ip 192.168.254.10 -bright\n"
			  << "   test -ip 192.168.254.10 -all -md gcp\n"
			  << "   test -md scp -ip 192.168.254.10 -bright 133 -sharp 1\n";
}
//----------------------------------------------------------------
void Allocate(char** pbuffer, unsigned long* psize)
{
	*psize = *psize * 2;
	*pbuffer = new char[*psize];

   //std::cout << *psize << ' ';
}
//----------------------------------------------------------------
void Deallocate(char* buffer)
{
	delete[] buffer;
}
//----------------------------------------------------------------
void Reinited(int number)
{
   std::cout << "Camera" << number << " Reinited\n";
}
//----------------------------------------------------------------


