#ifndef AVErrorCodesH
#define AVErrorCodesH
//---------------------------------------------------------------------------
namespace EAV{
   const int ZERO_POINTER                    = 1,
             NO_ALLOCATE_MEMORY              = 2,
             BAD_SOCK_ADDRESS                = 3,
             TIMEOUT_ON_SOCK                 = 4,
             READ_STRING_FROM_SOCK           = 5,
             TFTP_PROTOCOL                   = 6,
             CAMERA_PARAMATER_OUT_OF_RANGE    = 7,
             UNKNOWN_CAMERA                  = 8,
             PARAMETER_NOT_SUPPORTED         = 9,
             VALUE_OF_PARAMETER_UNKNOWN      = 10,
             UNKNOWN_ERROR                   = 11,
             CLIENT_ALREADY_EXISTS           = 12,
             CLIENT_OUT_OF_RANGE             = 13,
             CLIENT_NOT_EXISTS               = 14,
             TFTP_MISSING_PACKETS             = 15;
};
//---------------------------------------------------------------------------
#endif

 
