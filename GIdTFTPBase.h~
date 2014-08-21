//---------------------------------------------------------------------------
#ifndef GIdTFTPBaseH
#define GIdTFTPBaseH
//---------------------------------------------------------------------------
#include "GIdBase.h"
#include "GIdTypes.h"
#include "GStream.h"
#include "GUtil.h"
#include <string.h>

//---------------------------------------------------------------------------



//---------------------------------------------------------------------------
class GIdTFTPException : public GIdException{
public:
   explicit GIdTFTPException(const char* msg)
      : GIdException(msg, 0, 1)
   {
   }
};
//---------------------------------------------------------------------------
class GIdTFTPNoAllPacketsException : public GIdTFTPException{
public:
   explicit GIdTFTPNoAllPacketsException(const char* msg)
      : GIdTFTPException(msg)
   {
   }
};
//---------------------------------------------------------------------------
class GIdTFTPBase{
public:
   enum OPCODE { RRQ = 1, WRQ = 2, DATA = 3, ACK = 4, ERR = 5, OACK = 6};
   enum TRANSFER { OCTET, NETASCII };
   struct Packet;
};
//---------------------------------------------------------------------------
struct GIdTFTPBase::Packet{
   // definitions
   struct HeaderRRQ;
   struct Data;
   struct Ack;
   //struct Error;
   // members
   const OPCODE opcode;
   const char zero;
   //functions
   explicit Packet(const OPCODE& code)
      : opcode(code), zero(0)
   {
   }
   //---------------------------------
   void Write(GStream& dest)
   {
      dest.Write(&zero, 1);
	  char cd = static_cast<char>(opcode);
      dest.Write(&cd, 1);
   }
};
//---------------------------------------------------------------------------
struct GIdTFTPBase::Packet::HeaderRRQ : public Packet{
   GIdString filename;
   TRANSFER transfer;
   unsigned blksize;
   // functions
   //---------------------------------
   HeaderRRQ(const GIdString& fnm, TRANSFER trf, unsigned bsz)
      : Packet(RRQ), filename(fnm), transfer(trf), blksize(bsz)
   {
   }
   //---------------------------------
   void Write(GStream& dest)
   {
      Packet::Write(dest);
      if(!filename.empty())
         dest.Write(filename.c_str(), strlen(filename.c_str()));
      dest.Write(&zero, 1);
      switch(transfer){
      case OCTET : dest.Write("octet"); break;
      case NETASCII : dest.Write("netascii"); break;
      }
      dest.Write(&zero, 1);
      dest.Write("blksize\0");
      dest.Write(&zero, 1);
      dest.Write(ToString(blksize).c_str());
      dest.Write(&zero, 1);
   }
};
//---------------------------------------------------------------------------
struct GIdTFTPBase::Packet::Data : public Packet{
   unsigned number;
   // functions
   //---------------------------------
   explicit Data(unsigned num)
      : Packet(DATA), number(num)
   {
   }
   //---------------------------------
   void Write(GStream& dest)
   {
      Packet::Write(dest);
   }
   //---------------------------------
   static unsigned ReadNumber(GStream& src)
   {
      if(src.Size() - src.Position() < 2)
         throw GIdTFTPException("data packet is corrupt");
      char ch;
      src.Read(&ch, 1);
      unsigned number = static_cast<unsigned>(ch) * 256;
      src.Read(&ch, 1);
      return (number += ch);
   }
   //---------------------------------
   void Read(GStream& src)
   {
      number = ReadNumber(src);
   }
   //---------------------------------
   static Data Create(GStream& src)
   {
      return Data(ReadNumber(src));
   }
};
//---------------------------------------------------------------------------
struct GIdTFTPBase::Packet::Ack : public Packet{
   unsigned number;
   // functions
   //---------------------------------
   Ack(unsigned num)
      : Packet(ACK), number(num)
   {
   }
   //---------------------------------
   void Write(GStream& dest)
   {
      Packet::Write(dest);
      char n = number / 256;
      dest.Write(&n, 1);
      n = number % 256;
      dest.Write(&n, 1);
   }
};
//---------------------------------------------------------------------------
#endif
