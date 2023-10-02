#include "iris/vertex.h"
#include <algorithm>


namespace iris
{


namespace detail
{


std::optional<Vertex> GetCentroid(size_t count, const Points &points)
{
    auto pointsSize = points.size();

    if (pointsSize == 0)
    {
        return {};
    }

    if (pointsSize % 2 != 0)
    {
        // Odd number of harris detections are unlikely to be centered
        // around a vertex.
        return {};
    }

    if (count == 4 && pointsSize < 4)
    {
        return {};
    }

    double centroidX = 0;
    double centroidY = 0;
    auto pointCount = static_cast<double>(pointsSize);

    for (auto &point: points)
    {
        centroidX += point.x;
        centroidY += point.y;
    }

    return Vertex(
        centroidX / pointCount,
        centroidY / pointCount,
        pointCount);
}


Vertices PointGroups::GetVertices() const
{
    Vertices firstPass;

    for (const auto &entry: this->pointGroupByPoint_)
    {
        auto centroid = GetCentroid(this->count_, entry.second);

        if (centroid)
        {
            firstPass.push_back(*centroid);
        }
    }

    return firstPass;
}


void PointGroups::AddPoint_(const tau::Point2d<double> &point)
{
    for (auto &entry: this->pointGroupByPoint_)
    {
        // Use SquaredSum to avoid sqrt.
        if ((point - entry.first).SquaredSum() < this->radiusSquared_)
        {
            entry.second.push_back(point);
            return;
        }
    }

    this->pointGroupByPoint_[point].push_back(point);
}


} // end namespace detail


std::vector<tau::Point2d<double>> VerticesToPoints(
    const Vertices &vertices)
{
    std::vector<tau::Point2d<double>> result;
    result.reserve(vertices.size());

    std::transform(
        std::begin(vertices),
        std::end(vertices),
        std::back_inserter(result),
        [](const Vertex &vertex)
        {
            return vertex.point;
        });

    return result;
}


Vertex::Vertex(double x, double y, double count_)
    :
    point(x, y),
    count(count_)
{

}

bool Vertex::operator>(const Vertex &other) const
{
    if (this->point.template Convert<int>()
        == other.point.template Convert<int>())
    {
        return this->count > other.count;
    }

    return this->point.template Convert<int>()
        > other.point.template Convert<int>();
}

bool Vertex::operator<(const Vertex &other) const
{
    if (this->point.template Convert<int>()
        == other.point.template Convert<int>())
    {
        return this->count < other.count;
    }

    return (this->point.template Convert<int>()
        < other.point.template Convert<int>());
}

// For the purpose of determining unique vertices, vertices with the same
// point compare equal, even if their counts differ.
bool Vertex::operator==(const Vertex &other) const
{
    return (this->point.template Convert<int>()
        == other.point.template Convert<int>());
}


std::ostream & operator<<(std::ostream &output, const Vertex &point)
{
    return output << "Vertex(point: " << point.point
        << ", count: " << point.count << ")";
}


} // end namespace iris
