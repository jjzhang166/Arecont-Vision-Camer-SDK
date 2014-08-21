#ifndef AV2000TypesH
#define AV2000TypesH
//---------------------------------------------------------------------------
enum EXIF_STRING_ID {CAMERA_MAKE_STRING = 0 , CAMERA_MODEL_STRING, SOFTWARE_STRING, COPYRIGHT_STRING};
enum IMAGE_RESOLUTION {imFULL, imHALF, imQUARTER, imZOOM, imNONE};
enum IRIS_STATUS { irUNKNOWN = 0, irMANUAL, irDISABLED, irIDLE, irEVALUATING, irEVALUATING_TOO_DARK, irCLOSING, irCLOSED, irOPENING, irSTABLE, irCALIBRATING, irEND };

#ifdef BUILD_LOCALMACHINE
#else
enum CAMERA_MODEL{
        AV1300 = 1300, AV1305 = 1305,
        AV2100 = 2100, AV2105 = 2105,
        AV3100 = 3100, AV3105 = 3105,
        AV3130 = 3130, AV3135 = 3135,
        AV5100 = 5100, AV5105 = 5105,
        AV8180 = 8180, AV8185 = 8185,
        AV8360 = 8360, AV8365 = 8365,
        AV2000 = 2000 
};
#endif

enum CodecID {JPEG_CODEC = 0 , H264_CODEC = 1};

//---------------------------------------------------------------------------
enum CAMERA_PARAMETER{
   cpBRIGHTNESS, cpAUTO_EXPOSITION, cpSATURATION, cpSHARPNESS, cpBLUE, cpRED,
   cpILLUMINATION, cpLIGHTING, cpCAMERA_MODE,
   cpQUALITY_FULL, cpQUALITY_HALF, cpQUALITY_ZOOM, cpDOUBLESCAN, cpROLL,
   cpIRIS_ENABLED, cpIRIS_SPEED, cpIRIS_GAIN, /*cpIRIS_PEROSITION_ENABLED,*/  //14
   cpIRIS_REPOSITION_F_STOPS, cpIRIS_REPOSITION_F_STOPS_MIN, cpIRIS_REPOSITION_PERIOD,
   cpIRIS_REPOSITION_STABLE_PERIOD,
   cpDAY_NIGHT_MODE, cpDAY_NIGHT_TRIGGER_NIGHT, cpDAY_NIGHT_TRIGGER_DAY,   //21
   cpEXPOSURE_MODE, cpEXPOSURE_WINDOW_LEFT, cpEXPOSURE_WINDOW_TOP, cpEXPOSURE_WINDOW_WIDTH,
   cpEXPOSURE_WINDOW_HEIGHT,
   cpSENSOR_LEFT, cpSENSOR_TOP, cpSENSOR_WIDTH, cpSENSOR_HEIGHT,
   cpSENSOR_BLACK_WHITE_LEFT, cpSENSOR_BLACK_WHITE_TOP, cpSENSOR_BLACK_WHITE_WIDTH,
   cpSENSOR_BLACK_WHITE_HEIGHT,
   cpREQUEST_LEFT, cpREQUEST_TOP, cpREQUEST_WIDTH, cpREQUEST_HEIGHT,
   cpQUALITY, cpRESOLUTION, cpPER_CENT_IMAGE_RECTANGLE, cpREQUESTED_BLOCK_SIZE, //41
   cpMD_ENABLED, cpMD_MODE, cpMD_LEVEL_THRESH, cpMD_TOTAL_ZONES, cpMD_ZONE_SIZE,
   cpMD_EXPLOSURE_SENSITIVITY, cpMD_MATRIX, cpMD_DETAIL,
   cpMS_NUMBER_OF_SENSOR, cpMS_CHANNEL_ENABLE, cpMS_FULL_RES_ENABLE, cpMS_ZOOM_WIN_ENABLE,
   cpMS_ONE_SHOT_ENABLE, cpMS_IS_ZOOMED, cpMS_QUAD_MODE,
   cpWIDTH, cpHEIGHT, cpSHORT_EXPOSURES,
   cpTIMEOUT_ON_CAMERA, cpREPEAT_FROM_CAMERA, // experimental do not use
   cpSC_ENABLED, cpSC_STROBE, cpSC_ALWAYS_SEND, cpSC_OUTPUT_HIGH, cpSC_OUTPUT_LOW, cpSC_OPTO_INPUT,
   cpH264, cpBIT_RATE, cpGAMMA, cpLIGHTINGFIX,
   cpEND
};
//---------------------------------------------------------------------------
struct ClientError{
   int code;
   char description[256];
};
//---------------------------------------------------------------------------
struct ZoomInfo{
   float zoom;
   int dx, dy;
};
//---------------------------------------------------------------------------
struct AV2000Addr{
   unsigned char ip[4],
                 mac[6];
};
//---------------------------------------------------------------------------

// registers control
typedef void (*PTR_InfoSetRegister)(int, unsigned, unsigned);
typedef void (*PTR_InfoGetRegister)(int, unsigned, unsigned);
typedef void (*PTR_ReceivePacket)(int client, unsigned packet, unsigned long size);
typedef void (*PTR_SendAck)(int client, unsigned number);
//---------------------------------------------------------------------------
#endif
