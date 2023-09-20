#pragma once


#include <wxpex/wxshim.h>


namespace iris
{


#ifdef __WXMSW__
inline constexpr auto borderStyle = wxBORDER_NONE;
#else
inline constexpr auto borderStyle = wxBORDER_SIMPLE;
#endif


} // end namespace iris
