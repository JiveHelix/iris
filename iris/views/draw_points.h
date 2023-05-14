#pragma once


#include <wxpex/wxshim.h>
#include "iris/corner.h"



namespace iris
{


void DrawPoints(const CornerPoints &points, wxDC &dc, int radius);


} // end namespace iris
