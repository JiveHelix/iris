#include "iris/level_adjust.h"


namespace iris
{


template class LevelAdjust<int32_t, float>;
template class LevelAdjustNode<DefaultMaskNode, int32_t, float>;


} // end namespace iris
