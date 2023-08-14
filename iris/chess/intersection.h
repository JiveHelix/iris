#pragma once


#include <fields/fields.h>
#include <tau/vector2d.h>


namespace iris
{


struct Intersection
{
    tau::Point2d<size_t> logical;
    tau::Point2d<double> pixel;

    static constexpr auto fields = std::make_tuple(
        fields::Field(&Intersection::logical, "logical"),
        fields::Field(&Intersection::pixel, "pixel"));

    static constexpr auto fieldsTypeName = "Intersection";
};


DECLARE_OUTPUT_STREAM_OPERATOR(Intersection)


using Intersections = std::vector<Intersection>;


std::vector<tau::Point2d<double>> IntersectionsToPixels(
    const Intersections &intersections);


std::vector<tau::Point2d<size_t>> IntersectionsToLogicals(
    const Intersections &intersections);


} // end namespace iris
