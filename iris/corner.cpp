#include "iris/corner.h"
#include <algorithm>


namespace iris
{


namespace detail
{


std::optional<CornerPoint> GetCentroid(size_t count, const Points &points)
{
    auto pointsSize = points.size();

    if (pointsSize == 0)
    {
        return {};
    }

    if (pointsSize % 2 != 0)
    {
        // Odd number of harris detections are unlikely to be centered
        // around an intersection.
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

    return CornerPoint(
        centroidX / pointCount,
        centroidY / pointCount,
        pointCount);
}


CornerPoints PointGroups::GetCornerPoints() const
{
    CornerPoints firstPass;

    for (const auto &entry: this->pointGroupByPoint_)
    {
        auto centroid = GetCentroid(this->count_, entry.second);

        if (centroid)
        {
            firstPass.push_back(*centroid);
        }
    }

    return firstPass;
#if 0
    std::map<tau::Point2d<double>, Points> cornerPointsByPoint;

    CornerPoints result;

    auto AddCornerPoint = [&cornerPointsByPoint](const auto &point)
    {
        for (auto &entry: cornerPointsByPoint)
        {
            if ((point.point - entry.first.point).SquaredSum()
                    < this->radiusSquared_)
            {
                entry.second.push_back(point)
                return;
            }
        }

        cornerPointsByPoint[point].push_back(point);
    };

    // Second pass
    for (auto &cornerPoint: firstPass)
    {
        AddCornerPoint(cornerPoint);
    }
#endif

}


void PointGroups::AddPoint_(const tau::Point2d<double> &point)
{
    for (auto &entry: this->pointGroupByPoint_)
    {
        // Use SquaredSum to avoid many sqrt operations.
        if ((point - entry.first).SquaredSum() < this->radiusSquared_)
        {
            entry.second.push_back(point);
            return;
        }
    }

    this->pointGroupByPoint_[point].push_back(point);
}

#if 0
CornerCollector::CornerCollector(size_t windowSize, size_t count)
    :
    windowSize_(windowSize),
    count_(count),
    points_(),
    corners_()
{
    this->points_.reserve(windowSize * windowSize);
}


const CornerPoints & CornerCollector::GetCorners()
{
    return this->corners_;
}

#endif

} // end namespace detail


std::vector<tau::Point2d<double>> CornerPointsToPoints(
    const CornerPoints &cornerPoints)
{
    std::vector<tau::Point2d<double>> result;
    result.reserve(cornerPoints.size());

    std::transform(
        std::begin(cornerPoints),
        std::end(cornerPoints),
        std::back_inserter(result),
        [](const CornerPoint &cornerPoint)
        {
            return cornerPoint.point;
        });

    return result;
}


CornerPoint::CornerPoint(double x, double y, double count_)
    :
    point(x, y),
    count(count_)
{

}

bool CornerPoint::operator>(const CornerPoint &other) const
{
    if (this->point.template Convert<int>()
        == other.point.template Convert<int>())
    {
        return this->count > other.count;
    }

    return this->point.template Convert<int>()
        > other.point.template Convert<int>();
}

bool CornerPoint::operator<(const CornerPoint &other) const
{
    if (this->point.template Convert<int>()
        == other.point.template Convert<int>())
    {
        return this->count < other.count;
    }

    return (this->point.template Convert<int>()
        < other.point.template Convert<int>());
}

// For the purpose of determining unique corners, corners with the same
// point compare equal, even if their counts differ.
bool CornerPoint::operator==(const CornerPoint &other) const
{
    return (this->point.template Convert<int>()
        == other.point.template Convert<int>());
}


std::ostream & operator<<(std::ostream &output, const CornerPoint &point)
{
    return output << "CornerPoint(point: " << point.point
        << ", count: " << point.count << ")";
}


} // end namespace iris
