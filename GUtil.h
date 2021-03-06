//---------------------------------------------------------------------------
#ifndef GUtilH
#define GUtilH
//---------------------------------------------------------------------------
#include "GSysConfig.h"
#include "GString.h"
#include <errno.h>
#include <string.h>
//---------------------------------------------------------------------------
struct GRect{
	int left, top, right, bottom;
   //-------
	GRect()
	: left(0), top(0), right(0), bottom(0)
	{
	}
   //-------
	GRect(int l, int t, int r, int b)
	: left(l), top(t), right(r), bottom(b)
	{
	}
   //-------
	int Width() const
	{
		return right - left;
	}
   //-------
	int Height() const
	{
		return bottom - top;
	}
};
//---------------------------------------------------------------------------
class NumberToChar{
	struct Conformity{
		char array[10];
		Conformity();
	};
	static Conformity conformity;
public:
	char operator()(unsigned);
};
//---------------------------------------------------------------------------
struct GErr{
   bool err;
   enum { max = 256 };
   char description[max], name[max], msg[max];
   int code;
private:
   GErr(const GErr&);
   void operator=(const GErr&);
public:
   GErr();
   void operator()(const char* ds, const char* n, const char* msg, int c = 0);
   void Reset();
};
//---------------------------------------------------------------------------

std::string ReplaceSymbol(const std::string&, char old_symbol, char new_symbol);
int RoundAuto(int source, int divisible);
int RoundUp(int source, int divisible);
int RoundDown(int source, int divisible);
bool CheckInto(const int min, const int max, int val);

GString ExcludeTrailingBackslash(const GString&);
bool CopyFile(const char* dest, const char* source);
bool CopyFile(const GString& dest, const GString& source);

void Invert(char* array, unsigned long size);
long ToLong(const char*, int radix = 10);
unsigned long ToUnsignedLong(const char*, int radix = 10);
double ToDouble(const char*);
long ToLong(const std::string&, int radix = 10);
unsigned long ToUnsignedLong(const std::string&, int radix = 10);
double ToDouble(const std::string&);


std::string ToString(unsigned char number, int radix = 10);
std::string ToString(int number);
std::string ToString(unsigned number, int radix = 10);
std::string ToString(long number);
std::string ToString(unsigned long number, int radix = 10);
std::string ToString(double number);

#ifdef _windows_
   std::wstring ToWString(unsigned char number, int radix = 10);
   std::wstring ToWString(int number, int radix = 10);
   std::wstring ToWString(unsigned number, int radix = 10);
   std::wstring ToWString(long number, int radix = 10);
   std::wstring ToWString(unsigned long number, int radix = 10);
#endif


long ToLong(const wchar_t*, int radix = 10);
unsigned long ToUnsignedLong(const wchar_t*, int radix = 10);
double ToDouble(const wchar_t*);
long ToLong(const std::wstring&, int radix = 10);
unsigned long ToUnsignedLong(const std::wstring&, int radix = 10);
double ToDouble(const std::wstring&);



//---------------------------------------------------------------------------
template <class T> void Copy(const T* src, T* dest, unsigned long size)
{
	for(unsigned long i = 0; i < size; i++)
		dest[i] = src[i];
}
//---------------------------------------------------------------------------
template <class T> bool Compare(const T* left, const T* right, unsigned long size)
{
   while(size--)
      if(left[size] != right[size])
         return false;
   return true;
}
//---------------------------------------------------------------------------
template <class T> class CompareInvert{
public:
   bool operator()(const T& left, const T& right) const
   {
      return right < left;
   }
};
//---------------------------------------------------------------------------
template<class T> T CycleShift(T t)
{
   T ret = 0;
   for(int i = 1; i <= sizeof(T) * 8 - 1; i++){
      ret |= t & 1;
      t >>= 1;
      ret <<= 1;
   }
   ret |= t & 1;
   return ret;
}
//---------------------------------------------------------------------------
#endif
