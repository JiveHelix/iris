#include "iris/color.h"


namespace iris
{


template class Color<int32_t>;
template class ThreadsafeFilter<ColorGroup<int32_t>, Color<int32_t>>;


} // end namespace iris
