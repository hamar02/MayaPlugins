#pragma once

#include <windows.h>
#include <sstream>      // std::ostringstream

#define VSOutputPrint( s )            \
{                             \
   std::ostringstream os_;    \
   os_ << s;                   \
   OutputDebugString( os_.str().c_str() );  \
}