#pragma once


#include <tau/eigen.h>


namespace iris
{


template<typename T>
using ImageMatrix = Eigen::Matrix
<
    T,
    Eigen::Dynamic,
    Eigen::Dynamic,
    Eigen::RowMajor
>;


} // end namespace iris
