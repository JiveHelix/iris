#include "iris/gaussian_node.h"


namespace iris
{


template class Gaussian<int32_t, 0, double>;

template class Node
    <
        DefaultLevelAdjustNode,
        DefaultGaussian,
        GaussianControl<int32_t>
    >;


} // end namespace iris
