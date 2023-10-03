#pragma once

#ifdef ENABLE_CHESS_LOG

#include <pex/log.h>

#define CHESS_LOG(...) \
\
    pex::ToStream( \
        std::cout, \
        "[chess:", \
        jive::path::Base(__FILE__), \
        ":", \
        __FUNCTION__, \
        ":", \
        __LINE__, \
        "] ", \
        __VA_ARGS__); assert(std::cout.good())

#else

#define CHESS_LOG(...)

#endif // ENABLE_CHESS_LOG
