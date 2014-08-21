/*
//---------------------------------------------------------------------------
#pragma hdrstop
//---------------------------------------------------------------------------
#include "GString.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
using namespace std;
//---------------------------------------------------------------------------
GString::GString()
{
}
//---------------------------------------------------------------------------
GString::GString(const string& str)
   : string(str)
{
}
//---------------------------------------------------------------------------
GString::GString(const char* str)
   : string(str)
{
}
//---------------------------------------------------------------------------
GString::GString(char ch)
   : string(&ch)
{
}
//---------------------------------------------------------------------------
GString::GString(const char* p, string::size_type sz)
   : string(p, sz)
{
}
//---------------------------------------------------------------------------
GString& GString::operator=(const char* str)
{
   string::operator=(str);
   return *this;
}
//---------------------------------------------------------------------------
char& GString::operator[](size_type idx)
{
   return string::operator[](idx-1);
}
//---------------------------------------------------------------------------
const char& GString::operator[](size_type idx) const
{
   return string::operator[](idx-1);
}
//---------------------------------------------------------------------------
GString GString::SubString(size_type pos, int count)
{
   return substr(pos > size() ? 0 : pos-1, count);
}
//---------------------------------------------------------------------------
int GString::Pos(const GString& str)
{
   string::size_type pos = find(str);
   return string::npos == pos ? 0 : pos+1;
}
//---------------------------------------------------------------------------
int GString::Pos(char ch)
{
   string::size_type pos = find(ch);
   return string::npos == pos ? 0 : pos+1;
}
//---------------------------------------------------------------------------
string::size_type GString::Length() const
{
   return size();
}
//---------------------------------------------------------------------------
*/
