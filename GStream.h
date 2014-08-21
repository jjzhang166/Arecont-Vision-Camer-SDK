//---------------------------------------------------------------------------
#ifndef GStreamH
#define GStreamH
//---------------------------------------------------------------------------
#include "GException.h"
//---------------------------------------------------------------------------
class GStream{
public:
   typedef char Byte;
public:
   virtual ~GStream(){}
   virtual void Buffer(Byte*, unsigned long) = 0;
   virtual char* Buffer() const = 0;
   virtual unsigned long Position(unsigned long Offset) = 0;
   virtual unsigned long Position() const = 0;
   virtual void Size(unsigned long NewSize) = 0;
   virtual unsigned long Size() const = 0;
   virtual unsigned long Read(Byte *Buffer, unsigned long Count) = 0;
   virtual unsigned long Write(const Byte *Buffer, unsigned long Count) = 0;
   virtual unsigned long Write(const char* Buffer) = 0;
   virtual unsigned long CopyFrom(GStream& Source, unsigned long Count) = 0;
   virtual unsigned long Capacity() const = 0;
   virtual void Clear() = 0;
};
//---------------------------------------------------------------------------
class EStreamException : public GException{
public:
   EStreamException(const char* msg)
      : GException(0, msg, 1)
   {
   }
};
//---------------------------------------------------------------------------
class EStreamNoMemory : public EStreamException{
public:
   explicit EStreamNoMemory()
      : EStreamException("memory not allocated")
   {
   }
};
//---------------------------------------------------------------------------
class EStreamOutOfRange : public EStreamException{
public:
   explicit EStreamOutOfRange(const char* msg)
      : EStreamException(msg)
   {
   }
};
//---------------------------------------------------------------------------
class EStreamSome : public EStreamException{
public:
   explicit EStreamSome(const char* msg)
      : EStreamException(msg)
   {
   }
};
//---------------------------------------------------------------------------
#endif
