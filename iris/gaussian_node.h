#pragma once


#include "iris/gaussian.h"
#include "iris/level_adjust.h"
#include "iris/node.h"


namespace iris
{


extern template class Node
    <
        DefaultLevelAdjustNode,
        DefaultGaussian,
        GaussianControl<int32_t>
    >;


using DefaultGaussianNode = Node
    <
        DefaultLevelAdjustNode,
        DefaultGaussian,
        GaussianControl<int32_t>
    >;


} // end namespace iris
