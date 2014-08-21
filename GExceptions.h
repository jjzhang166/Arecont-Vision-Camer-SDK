//---------------------------------------------------------------------------

#ifndef GExceptionsH
#define GExceptionsH
//---------------------------------------------------------------------------
#include "GException.h"
//---------------------------------------------------------------------------
class EGConvert : public EGException{
public:
   explicit EGConvert(const char* msg, int code = 0)
      : EGException(code, msg)
   {
   }
};
//---------------------------------------------------------------------------
class EGOutOfRange : public EGException{
public:
   explicit EGOutOfRange(const char* msg, int code = 0)
      : EGException(code, msg)
   {
   }
};
//---------------------------------------------------------------------------
class EGMemoryAllocation : public EGException{
public:
   explicit EGMemoryAllocation(const char* msg, int code = 0)
      : EGException(code, msg)
   {
   }
};
//---------------------------------------------------------------------------
#endif
