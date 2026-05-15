#pragma once

#include <iostream>

// uncomment to enable debug output
// #define DEBUG_ON

#ifdef NDEBUG
#undef DEBUG_ON
#endif

#ifdef DEBUG_ON
// active macros when DEBUG_ON is defined
#define DPRINTF printf
#define DEBUG_MSG(msg) std::cout << __FILE__ << ":" << __FUNCTION__ << ":" << __LINE__ << ": " << msg << std::endl
#define DEBUG_VAR(var) std::cout << __FILE__ << ":" << __FUNCTION__ << ":" << __LINE__ << ": " << #var << " = " << var << std::endl
#else
// no-op macros when DEBUG_ON is not defined
#define DPRINTF(...)
#define DEBUG_MSG(msg)
#define DEBUG_VAR(var)
#endif
