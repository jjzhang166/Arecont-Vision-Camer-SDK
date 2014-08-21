//---------------------------------------------------------------------------
#pragma hdrstop
//---------------------------------------------------------------------------
#include "GIdTFTPClient.h"
#include "GIdUtils.h"
#include "GUtil.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

//#define INDY

//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
GIdTFTPClient::GIdTFTPClient()
   : blksize(512), transfer(NETASCII), max_cycles(4096), max_repeat_ack(3), max_errors(1024), max_blksize(65532), max_try_connect(3)
{
	ack[0] = 0;
	ack[1] = 4;
}
//---------------------------------------------------------------------------
void GIdTFTPClient::WriteRequest(OPCODE opcode, const GIdString& filename)
{
	const char zero = 0;
	buffer->Clear();
	buffer->Write(&zero, 1);

	char cd = static_cast<char>(opcode); 

	buffer->Write(&cd, 1);
	if(!filename.empty())
		buffer->Write(filename.c_str(), strlen(filename.c_str()));
	buffer->Write(&zero, 1);
	switch(transfer){
		case OCTET : buffer->Write("octet"); break;
		case NETASCII : buffer->Write("netascii"); break;
	}
	buffer->Write(&zero, 1);
	buffer->Write("blksize\0");
	buffer->Write(&zero, 1);
   buffer->Write(ToString(blksize).c_str());
	buffer->Write(&zero, 1);
}
//---------------------------------------------------------------------------
bool GIdTFTPClient::GetConnectTest(const GIdString& name, GBaseMemoryStream& dest)
{
	GIdAddress addr;
   return true;
}

//---------------------------------------------------------------------------
bool GIdTFTPClient::GetConnect(const GIdString& name, GBaseMemoryStream& dest)
{

	GIdAddress addr;

#ifdef INDY
	addr(ANY);
	connect(sock, addr, sizeof(addr));
#endif
	WriteRequest(RRQ, name);
	SendBuffer(buffer->Buffer(), buffer->Size());

   return GetConnectIteration(1, name, dest, addr);

}
//---------------------------------------------------------------------------
bool GIdTFTPClient::GetConnectIteration(unsigned iter, const GIdString& name, GBaseMemoryStream& dest, GIdAddress& addr)
{

   buffer->Clear();

	
   if(iter > max_try_connect)
      throw GIdTFTPException("Get: connection limit is up");

	buffer->Size(ReceiveBuffer(buffer->Buffer(), buffer->Capacity(), addr));

	Opcode();
	switch(opcode){
		case OACK :
			UpdateBlksize();

			GIdUDPBase::Connect(addr);

			SendAknowlegement(0);

			return true;
		case ERR :
			throw GIdTFTPException(("Error From Destination On GetConnect, code: " + ToString(ErrorCode())).c_str());
		case DATA : // for bug AV2000 send two ack on the last data-packet
         return GetConnectIteration(++iter, name, dest, addr);
		default:
			throw GIdTFTPException(("Destination host send unknown command on GetConnect, Command: " + ToString(opcode)).c_str());
	}
}
//---------------------------------------------------------------------------
void GIdTFTPClient::PutConnect(const GIdString& name)
{
	GIdAddress addr;
#ifdef INDY
	addr(ANY);
	connect(sock, addr, sizeof(addr));
#endif
	WriteRequest(WRQ, name);
	SendBuffer(buffer->Buffer(), buffer->Size());
   PutConnectIteration(1, name, addr);
}
//---------------------------------------------------------------------------
void GIdTFTPClient::PutConnectIteration(unsigned iter, const GIdString& name, GIdAddress& addr)
{
   if(iter > max_try_connect)
      throw GIdTFTPException("Put: connection limit is up");

   
	buffer->Size(ReceiveBuffer(buffer->Buffer(), buffer->Capacity(), addr));
	Opcode();
	switch(opcode){
		case ACK :
			GIdUDPBase::Connect(addr);
			break;
		case OACK :
			UpdateBlksize();
			GIdUDPBase::Connect(addr);
			break;
		case DATA : // for bug AV2000 send two ack on the last data-packet
         PutConnectIteration(++iter, name, addr);
			break;
		case ERR :
			throw GIdTFTPException(("Error From Destination On PutConnect, code: " + ToString(ErrorCode())).c_str());
		default:
			throw GIdTFTPException(("Destination host send unknown command on PutConnect, Command: " + ToString(opcode)).c_str());
	}
}
//---------------------------------------------------------------------------
bool GIdTFTPClient::ProcessRRQData(GBaseMemoryStream& dest)
{
	bool result = buffer->Size() < blksize + 4;
	unsigned number = ExtractDataNumber();
	SendAknowlegement(number);
   WriteRRQData(number, dest);
   received_packets.insert(number);
	OnWork(READ, number, buffer->Size() - buffer->Position());
	return result;
}
//---------------------------------------------------------------------------
void GIdTFTPClient::Opcode()
{
	if(buffer->Size() < 3)
		throw GIdTFTPException("Line size is too small");
	buffer->Position(2);
	opcode = static_cast<OPCODE>((*buffer)[0] * 256U + (*buffer)[1]);
}
//---------------------------------------------------------------------------
void GIdTFTPClient::UpdateBlksize()
{
	SkipZeroChar(*buffer);
	const GIdString blkfield = "blksize";
	unsigned i = 0;
	char ch;
	while(buffer->Position() < buffer->Size() && i < blkfield.size()){
		buffer->Read(&ch, 1);
		ch = tolower(ch);
		if(blkfield[i] == ch)
			i++;
		else
			break;
	}
	if(i != blkfield.size())
		throw GIdTFTPException("blksize field is corrupt");
	SkipZeroChar(*buffer);
	GIdString number;
	while(buffer->Read(&ch, 1) && isdigit(ch))
		number += ch;
	if(number.empty())
		throw GIdTFTPException("block size is unknown");
	long bsz = atol(number.c_str());
	if(!bsz || ERANGE == errno || bsz < 1 || bsz > max_blksize )
		throw GIdTFTPException("block size is unknown");
	blksize = bsz;
}
//---------------------------------------------------------------------------
inline void GIdTFTPClient::SendAknowlegement(unsigned num)
{
   last_ack_number = num;
   ack[2] = num / 256;
   ack[3] = num %  256;
   GIdUDPBase::SendBuffer(ack, 4);
}
//---------------------------------------------------------------------------
unsigned GIdTFTPClient::ExtractDataNumber()
{
	if(buffer->Size() - buffer->Position() < 2)
		throw GIdTFTPException("data packet is corrupt");
	char ch;
	buffer->Read(&ch, 1);
	unsigned number = static_cast<unsigned char>(ch) * 256U;
	buffer->Read(&ch, 1);
   number += static_cast<unsigned char>(ch);
	return number;
}
//---------------------------------------------------------------------------
void GIdTFTPClient::WriteRRQData(unsigned number, GBaseMemoryStream& dest)
{
	unsigned long position = (number - 1) * blksize + offset;
	if(dest.Size() < position)
		dest.Size(position);
	dest.Position(position);
	dest.Write(&(*buffer)[buffer->Position()], buffer->Size() - buffer->Position());
}
//---------------------------------------------------------------------------
unsigned long GIdTFTPClient::WriteWRQData(unsigned number, GBaseMemoryStream& src)
{
	unsigned long position = (number - 1)* blksize + offset,
	result;
	src.Position(position);
	result = src.Size() - src.Position();
	if(result > blksize)
		result = blksize;
	const char zero = 0;
	buffer->Clear();
	buffer->Write(&zero, 1);
	char ch = 3;
	buffer->Write(&ch, 1);
	ch = number / 256;
	buffer->Write(&ch, 1);
	ch = number % 256;
	buffer->Write(&ch, 1);
	buffer->Write(&const_cast<GBaseMemoryStream&>(src)[src.Position()], result);
	return result;
}
//---------------------------------------------------------------------------
unsigned long GIdTFTPClient::SendData(unsigned number, const GBaseMemoryStream& src)
{
	unsigned long size = WriteWRQData(number, const_cast<GBaseMemoryStream&>(src));
	buffer->Position(buffer->Position() - size - 4);
	GIdUDPBase::SendBuffer(&(*buffer)[buffer->Position()], size + 4);
	OnWork(WRITE, number, size);
	return size;
}
//---------------------------------------------------------------------------
void GIdTFTPClient::GetReal(const GIdString& name, GBaseMemoryStream& dest)
{
	unsigned cycles = 0, sent_repeat = 0;
   received_packets.clear();


        bool stop = !GetConnect(name, dest);


	OnAfterConnect(GET);
	while(!stop && cycles++ < max_cycles){
		buffer->Clear();
		try {

			buffer->Size(ReceiveBuffer(buffer->Buffer(), buffer->Capacity()));
			sent_repeat = 0;

			Opcode();
			switch(opcode){
				case DATA :

					stop = ProcessRRQData(dest);
					break;
				case OACK :

					UpdateBlksize();
					SendAknowlegement(0);
					break;
				case ERR :
					throw GIdTFTPException(("Error From Destination On GetReal, code: " + ToString(ErrorCode())).c_str());
				default :
					throw GIdTFTPException(("Opcode is bad on GetReal, code: " + ToString(opcode)).c_str());
			}

		}
		catch(GIdETimeout& ex){
			if(++sent_repeat <= max_repeat_ack){
				try { SendAknowlegement(last_ack_number); }
				catch(GIdException&){}
			}
			else
				throw;
		}
	}
   if(cycles >= max_cycles)
      throw GIdTFTPException("cycles out of range");
   if(!CheckAllExistPackets())
      throw GIdTFTPNoAllPacketsException("some packets are absent");
}
//---------------------------------------------------------------------------
void GIdTFTPClient::PutReal(const GBaseMemoryStream& src, const GIdString& name)
{
	PutConnect(name);
	OnAfterConnect(PUT);
	unsigned number = 1, err_ack_tmout = 0, err_ack_bad_num = 0;
	unsigned long send_bytes = blksize;
   bool stop = false;
	while(!stop && send_bytes == blksize && err_ack_tmout < 2 && err_ack_bad_num < 4){
		send_bytes = SendData(number, src);
		buffer->Clear();
		try { buffer->Size(ReceiveBuffer(buffer->Buffer(), buffer->Capacity())); }
		catch(GIdException&) {
			++err_ack_tmout;
			continue;
		}
		Opcode();
		switch(opcode){
			case ACK :
				if(number == ExtractDataNumber()){
					err_ack_tmout = err_ack_bad_num = 0;
					number++;
				}
				else
					err_ack_bad_num++;
				stop = send_bytes < blksize && !err_ack_tmout && !err_ack_bad_num;
				break;
			case OACK :
				number = 1;
				UpdateBlksize();
				break;
			case ERR :
				throw GIdTFTPException(("Error From Destination On PutReal, code: " + ToString(ErrorCode())).c_str());
			default :
				throw GIdTFTPException(("Opcode is bad on PutReal, code: " + ToString(opcode)).c_str());
		}
	}
	if(err_ack_tmout || err_ack_bad_num)
		throw GIdTFTPException("Ack Timeout");

}
//---------------------------------------------------------------------------
void GIdTFTPClient::Get(const GIdString& name, GBaseMemoryStream& dest)
{
	offset = dest.Position();
	try {
		OnBefore(GET);
		GetReal(name, dest);
	}
	catch(GIdException&){
		OnEnd(GET);
      // SendError
		throw;
	}
	OnEnd(GET);
}
//---------------------------------------------------------------------------
void GIdTFTPClient::Put(const GBaseMemoryStream& src, const GIdString& name)
{
	offset = src.Position();
	try {
		OnBefore(PUT);
		PutReal(src, name);
	}
	catch(GIdException&){
		OnEnd(PUT);
      // SendError
		throw;
	}
	OnEnd(PUT);
}
//---------------------------------------------------------------------------
unsigned GIdTFTPClient::ErrorCode()
{
	if(buffer->Size() < 4)
		return 0;
	return (*buffer)[2] * 256 + (*buffer)[3];
}
//---------------------------------------------------------------------------
bool GIdTFTPClient::CheckAllExistPackets()
{
   if(received_packets.empty())
      return false;
   unsigned packet = 1;
   for(std::set<unsigned>::const_iterator iter = received_packets.begin(); iter != received_packets.end(); iter++){
      if(*iter != packet)
         return false;
      packet++;
   }
   return true;
}
//---------------------------------------------------------------------------
void GIdTFTPClient::RepeatAknowlegements(unsigned num)
{
   max_repeat_ack = num;
}
//---------------------------------------------------------------------------
unsigned GIdTFTPClient::RepeatAknowlegements() const
{
   return max_repeat_ack;
}
//---------------------------------------------------------------------------

