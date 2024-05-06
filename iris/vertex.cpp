#include "iris/vertex.h"
#include <algorithm>


namespace iris
{


namespace detail
{


std::optional<Vertex> GetCentroid(size_t count, const ValuePoints &points)
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
        points);
}


Vertices PointGroups::GetVertices() const
{
    Vertices result;

    for (const auto &entry: this->pointGroupByPoint_)
    {
        auto centroid = GetCentroid(this->count_, entry.second);

        if (centroid)
        {
            result.push_back(*centroid);
        }
    }

    return result;
}


void PointGroups::AddPoint_(const draw::ValuePoint<double> &point)
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


ValuePoints VerticesToValuePoints(const Vertices &vertices)
{
    ValuePoints valuePoints;

    for (const auto &vertex: vertices)
    {
        valuePoints.insert(
            std::end(valuePoints),
            std::begin(vertex.valuePoints),
            std::end(vertex.valuePoints));
    }

    return valuePoints;
}


Vertex::Vertex(
    double x,
    double y,
    const ValuePoints &valuePoints_)
    :
    point(x, y),
    count(valuePoints_.size()),
    valuePoints(valuePoints_)
{


}


bool Vertex::operator>(const Vertex &other) const
{
    if (this->point.template Cast<int>()
        == other.point.template Cast<int>())
    {
        return this->count > other.count;
    }

    return this->point.template Cast<int>()
        > other.point.template Cast<int>();
}

bool Vertex::operator<(const Vertex &other) const
{
    if (this->point.template Cast<int>()
        == other.point.template Cast<int>())
    {
        return this->count < other.count;
    }

    return (this->point.template Cast<int>()
        < other.point.template Cast<int>());
}

// For the purpose of determining unique vertices, vertices with the same
// point compare equal, even if their counts differ.
bool Vertex::operator==(const Vertex &other) const
{
    return (this->point.template Cast<int>()
        == other.point.template Cast<int>());
}


std::ostream & operator<<(std::ostream &output, const Vertex &point)
{
    return output << "Vertex(point: " << point.point
        << ", count: " << point.count << ")";
}


} // end namespace iris
