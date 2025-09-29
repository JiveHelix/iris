#pragma once


#include <tau/color_map.h>
#include "iris/threadsafe_filter.h"


namespace iris
{


template<typename Value>
using ThreadsafeColorMap =
    ThreadsafeFilter
    <
        tau::ColorMapSettingsGroup<Value>,
        tau::ColorMap<Value>
    >;


extern template class ThreadsafeFilter
<
    tau::ColorMapSettingsGroup<int32_t>,
    tau::ColorMap<int32_t>
>;


} // end namespace iris
