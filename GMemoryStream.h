//---------------------------------------------------------------------------
#ifndef GMemoryStreamH
#define GMemoryStreamH
//---------------------------------------------------------------------------
#include "GStream.h"
//#include "GTestMemoryInfo.h"
//---------------------------------------------------------------------------
class GBaseMemoryStream : public GStream{
protected:
   Byte* memory;
   unsigned long size, position, capacity;
protected: // protect
   GBaseMemoryStream(const GBaseMemoryStream&);
   GBaseMemoryStream& operator=(const GBaseMemoryStream&);
protected:
   virtual void Reserve(unsigned long) = 0;
public:
   GBaseMemoryStream();
   ~GBaseMemoryStream();
   virtual void Resize(unsigned long);
   virtual void Buffer(Byte*, unsigned long);
   virtual Byte* Buffer() const;
   virtual unsigned long Position(unsigned long Offset);
   virtual unsigned long Capacity() const;
   virtual unsigned long Read(Byte *Buffer, unsigned long Count);
   void Save(const char*);
   virtual Byte& operator[](unsigned long idx);
   virtual unsigned long Position() const;
   virtual void Clear();
   virtual void Size(unsigned long NewSize);
   virtual unsigned long Size() const;
   virtual unsigned long Write(const Byte *Buffer, unsigned long Count);
   virtual unsigned long Write(const char* Buffer);
   virtual unsigned long CopyFrom(GStream& Source, unsigned long Count);
   void Load(const char*);
};
//---------------------------------------------------------------------------

class GMemoryStream : public GBaseMemoryStream{
   //static GTestMemoryInfo test_memory_info;
protected:
   const unsigned long align;
protected: // protect
   GMemoryStream(const GMemoryStream&);
   GMemoryStream& operator=(const GMemoryStream&);
public:
   GMemoryStream();
   explicit GMemoryStream(unsigned long);
   virtual ~GMemoryStream();
   virtual void Reserve(unsigned long);
   void FreeMemory();
   //static GTestMemoryInfo::Info MemoryInfo();
};
//---------------------------------------------------------------------------

class GCloneMemoryStream : public GBaseMemoryStream{
public:
   virtual void Size(unsigned long NewSize);
   virtual unsigned long Size() const;
   virtual unsigned long Write(const Byte *Buffer, unsigned long Count);
   virtual unsigned long Write(const char* Buffer);
   virtual unsigned long CopyFrom(GStream& Source, unsigned long Count);
   virtual GCloneMemoryStream& Assign(const GBaseMemoryStream&);
   virtual void Reserve(unsigned long){}
};
//---------------------------------------------------------------------------

class GExternalAllocateStream : public GBaseMemoryStream{
   typedef void (*HANDLER_ALLOC)(char**, unsigned long*);
   typedef void (*HANDLER_DEALLOC)(char*);
private:
   static HANDLER_ALLOC handler_alloc;
   static HANDLER_DEALLOC handler_dealloc;
private:
   virtual void Reserve(unsigned long);
public:
   static void DefineHandlerAlloc(HANDLER_ALLOC);
   static void DefineHandlerDealloc(HANDLER_DEALLOC);
   GExternalAllocateStream();
   virtual ~GExternalAllocateStream();
};
//---------------------------------------------------------------------------
#endif
