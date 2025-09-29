#include "iris/color_map.h"


namespace iris
{


template class ThreadsafeFilter
<
    tau::ColorMapSettingsGroup<int32_t>,
    tau::ColorMap<int32_t>
>;


} // end namespace iris
