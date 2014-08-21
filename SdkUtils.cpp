//---------------------------------------------------------------------------
#pragma hdrstop
//---------------------------------------------------------------------------
#include "SdkUtils.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
GRect CalculateZoomHeaderRect3130(const int, const int, float zoom, int dx, int dy, int)
{
   //CalculateZoomHeaderRect(zoom, dx, dy, coef, header_width, header_height, request_rect);

   if(dx < -50)
      dx = -50;
   else
      if(dx > 50)
         dx = 50;
   if(dy < -50)
      dy = -50;
   else
      if(dy > 50)
         dy = 50;

   const unsigned divisible_width = 1,
                  divisible_height = 1,
                  coef = 1,
                  max_request_width = 100,
                  max_request_height = 100;
   unsigned kx = divisible_width * coef, ky = divisible_height * coef;
   int request_width = (int)( static_cast<float>(max_request_width) / zoom + 0.5 ),
       request_height = (int)( static_cast<float>(max_request_height) / zoom + 0.5 );

   //request_width = RoundUp(request_width, kx);
   if(request_width > max_request_width)
      request_width -= kx;
   //request_height = RoundUp(request_height, ky);
   if(request_height > max_request_height)
      request_height -= ky;

   GRect request_rect;
   request_rect.left = (int)((max_request_width - request_width) * ((50 - dx) / 100.0));
   request_rect.right = request_rect.left + request_width;
   request_rect.top = (int)((max_request_height - request_height) * ((50 - dy) / 100.0));
   request_rect.bottom = request_rect.top + request_height;

   return request_rect;
}
//---------------------------------------------------------------------------
