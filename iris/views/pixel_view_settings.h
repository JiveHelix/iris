#pragma once

#include <pex/group.h>
#include <wxpex/async.h>

#include "iris/views/data_view_settings.h"
#include "iris/pixels.h"


namespace iris
{


using PixelViewGroup = DataViewGroup<Pixels>;
using PixelViewModel = DataViewModel<Pixels>;
using PixelViewControl = DataViewControl<Pixels>;

template<typename Observer>
using PixelViewTerminus = DataViewTerminus<Pixels, Observer>;

using PixelViewGroupMaker = DataViewGroupMaker<Pixels>;


} // end namespace iris
