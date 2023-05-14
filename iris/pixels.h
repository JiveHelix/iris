#pragma once


#include <tau/color_maps/rgb.h>
#include <wxpex/async.h>


namespace iris
{


using Pixels = tau::RgbPixels<uint8_t>;
using PixelMatrix = typename Pixels::Data;


using AsyncPixels = wxpex::MakeAsync<Pixels>;
using PixelsControl = typename AsyncPixels::Control<void>;

template<typename Observer>
using PixelsTerminus = typename AsyncPixels::Terminus<Observer>;


} // end namespace iris
