#pragma once


#include <Eigen/Dense>
#include <pex/control_value.h>
#include <draw/size.h>


namespace iris
{


static constexpr auto defaultMaximum = 255;
inline const draw::Size defaultImageSize{1920, 1080};

using InProcess = int32_t;

using ProcessMatrix =
    Eigen::Matrix
    <
        InProcess,
        Eigen::Dynamic,
        Eigen::Dynamic,
        Eigen::RowMajor
    >;


using MaximumControl = pex::control::Value<pex::model::Value<InProcess>>;


} // end namespace iris
