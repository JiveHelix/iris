#include "iris/chess/find_vertex.h"


namespace iris
{


// Return the first vertex that is within the maximum distance.
std::optional<Vertex> FindVertex(
    const tau::Point2d<double> &candidate,
    const Vertices &vertices,
    double maximumDistance)
{
    if (vertices.empty())
    {
        throw std::logic_error("must have vertices");
    }

    for (const auto &vertex: vertices)
    {
        auto distance = candidate.Distance(vertex.point);

        if (distance < maximumDistance)
        {
            return {vertex};
        }
    }

    return {};
}


} // end namespace iris
