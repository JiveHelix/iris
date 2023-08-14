#pragma once


#include <tau/vector2d.h>
#include <optional>


namespace iris
{


std::optional<tau::Point2d<double>> FindPoint(
    const tau::Point2d<double> &candidate,
    const std::vector<tau::Point2d<double>> &horizontalPoints,
    const std::vector<tau::Point2d<double>> &verticalPoints,
    double maximumPointError);


} // end namespace iris
