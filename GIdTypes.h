//---------------------------------------------------------------------------
#ifndef GIdTypesH
#define GIdTypesH
//---------------------------------------------------------------------------
#include <string>
//---------------------------------------------------------------------------
typedef std::string GIdString;
//---------------------------------------------------------------------------
struct GIdTimeout : public TIMEVAL{
	GIdTimeout(unsigned long millisecond)
	{
		tv_sec = millisecond / 1000;
		tv_usec = (millisecond % 1000) * 1000;
	}
   //-----------------------------
	void operator()(unsigned long millisecond)
	{
		tv_sec = millisecond / 1000;
		tv_usec = (millisecond % 1000) * 1000;
	}
   //-----------------------------
	unsigned long operator()() const
	{
		return tv_sec * 1000 + tv_usec / 1000;
	}
   //-----------------------------
	operator unsigned long()
	{
		return tv_sec * 1000 + tv_usec / 1000;
	}
};
//---------------------------------------------------------------------------
#endif
