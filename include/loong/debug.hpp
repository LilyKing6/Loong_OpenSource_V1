// Copyright (c) 2023-2026 Lily King.
#pragma once

#include <iostream>

// 取消注释以启用调试输出
// #define DEBUG_ON

#ifdef NDEBUG
#undef DEBUG_ON
#endif

#ifdef DEBUG_ON
// DEBUG_ON 定义时激活的宏
#define DPRINTF printf
#define DEBUG_MSG(msg) std::cout << __FILE__ << ":" << __FUNCTION__ << ":" << __LINE__ << ": " << msg << std::endl
#define DEBUG_VAR(var) std::cout << __FILE__ << ":" << __FUNCTION__ << ":" << __LINE__ << ": " << #var << " = " << var << std::endl
#else
// DEBUG_ON 未定义时的空操作宏
#define DPRINTF(...)
#define DEBUG_MSG(msg)
#define DEBUG_VAR(var)
#endif
