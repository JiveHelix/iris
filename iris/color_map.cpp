#include "iris/color_map.h"


namespace iris
{


template class ThreadsafeFilter
<
    draw::ColorMapSettingsGroup<int32_t>,
    draw::ColorMap<int32_t>
>;


} // end namespace iris
