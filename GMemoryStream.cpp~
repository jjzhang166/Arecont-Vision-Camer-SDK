
//---------------------------------------------------------------------------
#pragma hdrstop
//---------------------------------------------------------------------------
#include "GMemoryStream.h"
#include <fstream>
#include <string.h>
//---------------------------------------------------------------------------
#ifdef BCB6
#pragma package(smart_init)
#endif

//---------------------------------------------------------------------------
inline void GBaseMemoryStream::Resize(unsigned long sz)
{
	Reserve(sz);
	size = sz;
}
//---------------------------------------------------------------------------
GBaseMemoryStream::GBaseMemoryStream()
: memory(0), capacity(0), size(0), position(0)
{

}
//---------------------------------------------------------------------------
GBaseMemoryStream::~GBaseMemoryStream()
{
}
//---------------------------------------------------------------------------
void GBaseMemoryStream::Buffer(Byte* data, unsigned long sz)
{
	memory = data;
	capacity = sz;
	if(size > capacity)
		size = capacity;
	if(position > size)
		position = size;
}
//---------------------------------------------------------------------------
inline GStream::Byte* GBaseMemoryStream::Buffer() const
{
	return memory;
}
//---------------------------------------------------------------------------
inline unsigned long GBaseMemoryStream::Position(unsigned long offset)
{
#ifdef _DEBUG
	if(offset > /*size*/capacity)
		throw EStreamOutOfRange("seek not valid");
#endif
	return (position = offset);
}
//---------------------------------------------------------------------------
unsigned long GBaseMemoryStream::Read(Byte *Buffer, unsigned long Count)
{
	Byte* uac = static_cast<Byte*>(Buffer);
	unsigned long counter = 0;
	while(counter < Count && position < size)
		uac[counter++] = memory[position++];
	return counter;
}
//---------------------------------------------------------------------------
void  GBaseMemoryStream::Save(const char* name)
{
	std::ofstream fout(name, std::ios_base::binary);
	if(!fout)
		throw EStreamSome(/*"output file: \"" + name + "\" could not be opened"*/"save error");
	while(position < size)
		fout.put(memory[position++]);
}
//---------------------------------------------------------------------------
inline GStream::Byte& GBaseMemoryStream::operator[](unsigned long idx)
{
	return memory[idx];
}
//---------------------------------------------------------------------------
inline unsigned long GBaseMemoryStream::Position() const
{
	return position;
}
//---------------------------------------------------------------------------
inline unsigned long GBaseMemoryStream::Capacity() const
{
	return capacity;
}
//---------------------------------------------------------------------------
void GBaseMemoryStream::Clear()
{
	size = position = 0;
}
//---------------------------------------------------------------------------
inline void GBaseMemoryStream::Size(unsigned long NewSize)
{
	Resize(NewSize);
	if(position > NewSize)
		position = NewSize;
}
//---------------------------------------------------------------------------
inline unsigned long GBaseMemoryStream::Size() const
{
	return size;
}
//---------------------------------------------------------------------------
unsigned long  GBaseMemoryStream::Write(const Byte *Buffer, unsigned long Count)
{
	Byte* uac = const_cast<Byte*>(Buffer);
	unsigned long new_position = position + Count;
	if(capacity < new_position)
		Reserve(new_position);
	while(Count-- > 0)
		memory[position + Count] = uac[Count];
	Count = new_position - position;
	position = new_position;
	if(size < position)
		size = position;
	return Count;
}
//---------------------------------------------------------------------------
unsigned long  GBaseMemoryStream::Write(const char* Buffer)
{
	Byte* uac = const_cast<Byte*>(Buffer);
	unsigned long Count = strlen(Buffer);
	unsigned long new_position = position + Count;
	if(capacity < new_position)
		Reserve(new_position);
	while(Count-- > 0)
		memory[position + Count] = uac[Count];
	Count = new_position - position;
	position = new_position;
	if(size < position)
		size = position;
	return Count;
}
//---------------------------------------------------------------------------
unsigned long GBaseMemoryStream::CopyFrom(GStream& source, unsigned long Count)
{
	if(!Count){
		source.Position(0);
		Count = source.Size();
	}
	unsigned long new_position = position + Count;
	if(capacity < new_position)
		Reserve(new_position);
	source.Read(memory, Count);
	position = new_position;
	if(size < position)
		size = position;
	return Count;
}
//---------------------------------------------------------------------------
void  GBaseMemoryStream::Load(const char* name)
{
	std::ifstream fin(name, std::ios_base::binary);
	if(!fin)
		throw EStreamSome(/*"read file: \"" + name + "\" could not be opened"*/"load error");
	char ch;
	bool read = fin.get(ch);
	while(read){
		while(position < capacity && read){
			size += position >= size;
			memory[position++] = ch;
			read = fin.get(ch);
		}
		Reserve(position + 1);
	}
	if(!fin.eof())
		throw EStreamSome("on end of read file");
}

//---------------------------------------------------------------------------
GMemoryStream::GMemoryStream()
   : align(8192)
{
}
//---------------------------------------------------------------------------
GMemoryStream::GMemoryStream(unsigned long sz)
   : align(8192)
{
	Resize(sz);
}
//---------------------------------------------------------------------------
GMemoryStream::~GMemoryStream()
{
   FreeMemory();
}
//---------------------------------------------------------------------------
void GMemoryStream::Reserve(unsigned long sz)
{
	if(sz > capacity){
		unsigned long new_capacity = (sz / align + static_cast<bool>(sz % align)) * align;
		Byte* new_memory = new Byte[new_capacity];
		for(unsigned long i = 0; i < size; i++)
			new_memory[i] = memory[i];
		delete[] memory;
      memory = new_memory;
		capacity = new_capacity;
	}
}
//---------------------------------------------------------------------------
void GMemoryStream::FreeMemory()
{
	delete[] memory;
	memory = 0;
   capacity = size= 0;
}
//---------------------------------------------------------------------------


void GCloneMemoryStream::Size(unsigned long NewSize)
{
	if(NewSize > capacity)
		throw EStreamOutOfRange("GCloneMemoryStream Error On Set Size: new size is larger then capacity");
	size = NewSize;
	if(position > NewSize)
		position = NewSize;
}
//---------------------------------------------------------------------------
unsigned long GCloneMemoryStream::Size() const
{
	return size;
}
//---------------------------------------------------------------------------
unsigned long GCloneMemoryStream::Write(const Byte *Buffer, unsigned long Count)
{
	throw EStreamException("GCloneMemoryStream Error: function Write is forbideen");
}
//---------------------------------------------------------------------------
unsigned long GCloneMemoryStream::Write(const char* Buffer)
{
	throw EStreamException("GCloneMemoryStream Error: function Write is forbideen");
}
//---------------------------------------------------------------------------
unsigned long GCloneMemoryStream::CopyFrom(GStream& Source, unsigned long Count)
{
	throw EStreamException("GCloneMemoryStream Error: function CopyFrom is forbideen");
}
//---------------------------------------------------------------------------
GCloneMemoryStream& GCloneMemoryStream::Assign(const GBaseMemoryStream& src)
{
	size = src.Size();
	position = src.Position();
	capacity = src.Capacity();
	memory = src.Buffer();
   return *this;
}
//---------------------------------------------------------------------------
void GExternalAllocateStream::Reserve(unsigned long sz)
{
	unsigned long requested_size = sz;
	if(sz > capacity){
		Byte* prev = memory;
		if(handler_alloc)
			handler_alloc(&memory, &sz);
		else
			throw std::bad_alloc();
		if(!memory || sz < requested_size){
         memory = prev;
			throw EStreamNoMemory();
      }
		capacity = sz;
		for(unsigned long i = 0; i < size; i++)
			memory[i] = prev[i];
		if(handler_dealloc)
			handler_dealloc(prev);
	}
}
//---------------------------------------------------------------------------
GExternalAllocateStream::HANDLER_ALLOC GExternalAllocateStream::handler_alloc = 0;
//---------------------------------------------------------------------------
GExternalAllocateStream::HANDLER_DEALLOC GExternalAllocateStream::handler_dealloc = 0;
//---------------------------------------------------------------------------
void GExternalAllocateStream::DefineHandlerAlloc(HANDLER_ALLOC handler)
{
	handler_alloc = handler;
}
//---------------------------------------------------------------------------
void GExternalAllocateStream::DefineHandlerDealloc(HANDLER_DEALLOC handler)
{
	handler_dealloc = handler;
}
//---------------------------------------------------------------------------
GExternalAllocateStream::GExternalAllocateStream()
{
}
//---------------------------------------------------------------------------
GExternalAllocateStream::~GExternalAllocateStream()
{
	if(handler_dealloc)
		handler_dealloc(memory);
}
//---------------------------------------------------------------------------

