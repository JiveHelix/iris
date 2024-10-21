#pragma once


#include <draw/color_map.h>
#include "iris/threadsafe_filter.h"


namespace iris
{


template<typename Value>
using ThreadsafeColorMap =
    ThreadsafeFilter
    <
        draw::ColorMapSettingsGroup<Value>,
        draw::ColorMap<Value>
    >;


extern template class ThreadsafeFilter
<
    draw::ColorMapSettingsGroup<int32_t>,
    draw::ColorMap<int32_t>
>;


} // end namespace iris
