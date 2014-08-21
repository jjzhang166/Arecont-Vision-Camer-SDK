//---------------------------------------------------------------------------

#ifndef GExceptionH
#define GExceptionH
//---------------------------------------------------------------------------
#include <exception>
//---------------------------------------------------------------------------
class GException : public std::exception{
protected:
   char message[256];
   int code;
public:
   explicit GException(int, unsigned char = 0);
   explicit GException(const char*);
   GException(int cd, const char* msg, unsigned char = 0);
   virtual const char* what() const throw();
   int get_code() const;
   unsigned char get_info() const;
};
//---------------------------------------------------------------------------
typedef GException EGException;
//---------------------------------------------------------------------------
#endif

