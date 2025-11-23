#ifndef DEBUG_H
#define DEBUG_H

#include <iostream>

// #define DEBUG_ON

#ifdef NDEBUG
#undef DEBUG_ON
#endif

#ifdef DEBUG_ON
#define DPRINTF printf
#define DEBUG_MSG(msg) std::cout << __FILE__ << ":" << __FUNCTION__ << ":" <<__LINE__ << ": " << msg << std::endl
#define DEBUG_VAR(var) std::cout << __FILE__ << ":" << __FUNCTION__ << ":" <<__LINE__ << ": " << #var << " = " << var << std::endl
#else
#define DPRINTF
#define DEBUG_MSG(msg)
#define DEBUG_VAR(var)
#endif



#endif
