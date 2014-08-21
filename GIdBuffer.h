//---------------------------------------------------------------------------
#ifndef GIdBufferH
#define GIdBufferH
//---------------------------------------------------------------------------
class GIdBuffer{
   const unsigned long align;
   unsigned long size;
   char* buff;
private:
   void Resize(unsigned long);
protected:
   GIdBuffer();
   ~GIdBuffer();
   unsigned long Capacity() const { return size; }
   void Size(unsigned long);
   unsigned long Size() const { return size; }
   char* operator[](unsigned long idx) { return buff + idx; }
};
//---------------------------------------------------------------------------
#endif
