#pragma once


#include "iris/vertex.h"
#include <optional>


namespace iris
{


// Return the first vertex that is within the maximum distance.
std::optional<Vertex> FindVertex(
    const tau::Point2d<double> &candidate,
    const Vertices &vertices,
    double maximumDistance);


} // end namespace iris
