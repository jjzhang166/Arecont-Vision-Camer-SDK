//---------------------------------------------------------------------------
#ifndef GIdTFTPClientH
#define GIdTFTPClientH
//---------------------------------------------------------------------------
#include "GIdUDPClient.h"
#include "GIdTFTPBase.h"
#include <set>
//---------------------------------------------------------------------------
class GIdTFTPClient : public GIdUDPClient, public GIdTFTPBase{
public:
   enum OPERATION { GET, PUT } operation;
   enum WORK_MODE { READ, WRITE };
private:
   const unsigned max_cycles, max_errors, max_blksize, max_try_connect;
   unsigned blksize, max_repeat_ack, last_ack_number;
   unsigned long offset;
   TRANSFER transfer;
   char ack[4];
   OPCODE opcode;
   std::set<unsigned> received_packets;
protected:
   void WriteRequest(OPCODE code, const GIdString& filename);
   bool GetConnect(const GIdString& name, GBaseMemoryStream& dest);
   bool GetConnectTest(const GIdString& name, GBaseMemoryStream& dest);
   bool GetConnectIteration(unsigned iter, const GIdString& name, GBaseMemoryStream& dest, GIdAddress& addr);
   void PutConnect(const GIdString& name);
   void PutConnectIteration(unsigned iter, const GIdString& name, GIdAddress& addr);
   void Opcode();
   void UpdateBlksize();
   void SendAknowlegement(unsigned);
   unsigned ExtractDataNumber();
   void WriteRRQData(unsigned, GBaseMemoryStream&);
   unsigned long WriteWRQData(unsigned, GBaseMemoryStream&);
   unsigned long SendData(unsigned, const GBaseMemoryStream&);
   void GetReal(const GIdString& name, GBaseMemoryStream& mem);
   void PutReal(const GBaseMemoryStream& mem, const GIdString& name);
   bool ProcessRRQData(GBaseMemoryStream& dest);
   unsigned ErrorCode();
   bool CheckAllExistPackets();
private:
   virtual void OnBefore(OPERATION){}
   virtual void OnAfterConnect(OPERATION){}
   virtual void OnWork(WORK_MODE, unsigned, unsigned long size){}
   virtual void OnEnd(OPERATION){}
public:
   GIdTFTPClient();
   void Get(const GIdString& name, GBaseMemoryStream& mem);
   void Put(const GBaseMemoryStream&, const GIdString& name);
   void RequestedBlockSize(unsigned sz) { blksize = sz; }
   unsigned RequestedBlockSize() const { return blksize; }
   void RepeatAknowlegements(unsigned);
   unsigned RepeatAknowlegements() const;
};
//---------------------------------------------------------------------------
template <class T> class GIdTFTPClientUse : public GIdTFTPClient{
   typedef void (T::*Handler_OnBefore)(OPERATION);
   typedef void (T::*Handler_OnAfterConnect)(OPERATION);
   typedef void (T::*Handler_OnWork)(WORK_MODE, unsigned, unsigned long size);
   typedef void (T::*Hanler_OnEnd)(OPERATION);
private:
   T* object;
   Handler_OnBefore handler_on_before;
   Handler_OnAfterConnect handler_on_after_connect;
   Handler_OnWork handler_on_work;
   Hanler_OnEnd handler_on_end;
public:
   explicit GIdTFTPClientUse(T& obj)
      : object(&obj), handler_on_before(0), handler_on_after_connect(0), handler_on_work(0), handler_on_end(0)
   {
   }
   //---------------------------------------------------------------------
   void DefineHadlerOnBefore(Handler_OnBefore val) { handler_on_before = val; }
   //---------------------------------------------------------------------
   void DefineHadlerOnAfterConnect(Handler_OnAfterConnect val) { handler_on_after_connect = val; }
   //---------------------------------------------------------------------
   void DefineHandlerOnWork(Handler_OnWork val) { handler_on_work = val; }
   //---------------------------------------------------------------------
   void DefineHadlerOnEnd(Hanler_OnEnd val) { handler_on_end = val; }
   //---------------------------------------------------------------------
   void OnBefore(OPERATION opr)
   {
      if(handler_on_before)
         (object->*handler_on_before)(opr);
   }
   //---------------------------------------------------------------------
   void OnAfterConnect(OPERATION opr)
   {
      if(handler_on_after_connect)
         (object->*handler_on_after_connect)(opr);
   }
   //---------------------------------------------------------------------
   void OnWork(WORK_MODE mode, unsigned packet, unsigned long size)
   {
      if(handler_on_work)
         (object->*handler_on_work)(mode, packet, size);
   }
   //---------------------------------------------------------------------
   void OnEnd(OPERATION opr)
   {
      if(handler_on_end)
         (object->*handler_on_end)(opr);
   }
   //---------------------------------------------------------------------
};
//---------------------------------------------------------------------------
#endif
