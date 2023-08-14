#include "iris/chess/intersection.h"
#include <algorithm>


namespace iris
{


std::vector<tau::Point2d<double>> IntersectionsToPixels(
    const Intersections &intersections)
{
    std::vector<tau::Point2d<double>> pixels;
    pixels.reserve(intersections.size());

    std::transform(
        std::begin(intersections),
        std::end(intersections),
        std::back_inserter(pixels),
        [](const Intersection &intersection)
        {
            return intersection.pixel;
        });

    return pixels;
}


std::vector<tau::Point2d<size_t>> IntersectionsToLogicals(
    const Intersections &intersections)
{
    std::vector<tau::Point2d<size_t>> logicals;
    logicals.reserve(intersections.size());

    std::transform(
        std::begin(intersections),
        std::end(intersections),
        std::back_inserter(logicals),
        [](const Intersection &intersection)
        {
            return intersection.logical;
        });

    return logicals;
}


} // end namespace iris
