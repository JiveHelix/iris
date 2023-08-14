#include "iris/chess/find_point.h"


namespace iris
{


std::optional<tau::Point2d<double>> FindPoint(
    const tau::Point2d<double> &candidate,
    const std::vector<tau::Point2d<double>> &horizontalPoints,
    const std::vector<tau::Point2d<double>> &verticalPoints,
    double maximumPointError)
{
    if (horizontalPoints.empty())
    {
        throw std::logic_error("must have points");
    }

    for (const auto &point: horizontalPoints)
    {
        auto distance = candidate.Distance(point);

        if (distance < maximumPointError)
        {
            return {point};
        }
    }

    for (const auto &point: verticalPoints)
    {
        auto distance = candidate.Distance(point);

        if (distance < maximumPointError)
        {
            return {point};
        }
    }

    return {};
}


} // end namespace iris
