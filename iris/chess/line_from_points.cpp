#include "line_from_points.h"


namespace iris
{


LineFromPoints::LineFromPoints(
        double angleToleranceDegrees,
        double minimumSpacing,
        const Point &first,
        const Point &second)
    :
    Base(first, second),
    angleToleranceDegrees_(angleToleranceDegrees),
    minimumSpacing_(minimumSpacing),
    points_()
{
    assert(first != second);
    this->points_.push_back(first);
    this->points_.push_back(second);

    // points_ must remain sorted for later unique insertion to work.
    std::sort(begin(this->points_), end(this->points_));
}


bool LineFromPoints::operator<(const LineFromPoints &other) const
{
    return this->LessThan(other, this->angleToleranceDegrees_);
}


bool LineFromPoints::operator==(const LineFromPoints &other) const
{
    return this->IsColinear(
        other,
        this->angleToleranceDegrees_,
        this->minimumSpacing_);
}


double LineFromPoints::GetError(const Point &point_) const
{
    return this->DistanceToPoint(point_);
}


std::optional<LineFromPoints::Iterator> LineFromPoints::GetInsertion_(const Point &point_)
{
    return tau::GetUniqueInsertion(this->points_, point_);
}


void LineFromPoints::Combine(const LineFromPoints &other)
{
    for (const auto &point_: other.points_)
    {
        auto insertion = this->GetInsertion_(point_);

        if (insertion)
        {
            this->points_.insert(*insertion, point_);
        }
    }

    // Recreate the line to account for the new points.
    auto updated = Base(this->points_);
    this->point = updated.point;
    this->vector = updated.vector;
}


bool LineFromPoints::AddPoint(const Point &point_)
{
    auto insertion = this->GetInsertion_(point_);

    if (!insertion)
    {
        // This point is already a member of this line.
        return false;
    }

    auto sanity = std::find(begin(this->points_), end(this->points_), point_);

    if (sanity != end(this->points_))
    {
        // Insanity!
        throw std::runtime_error("point already exists!");
    }

    this->points_.insert(*insertion, point_);

    // Recreate the line to account for the new point.
    auto updated = Base(this->points_);
    this->point = updated.point;
    this->vector = updated.vector;

    return true;
}


void LineFromPoints::RemoveOutliers(double maximumVertexDistance)
{
    double pointError = 0;

    for (const auto &point_: this->points_)
    {
        pointError = std::max(pointError, this->GetError(point_));
    }

    auto pointsEnd = std::remove_if(
        begin(this->points_),
        end(this->points_),
        [this, maximumVertexDistance] (const auto &point_) -> bool
        {
            return this->GetError(point_) > maximumVertexDistance;
        });

    if (pointsEnd != end(this->points_))
    {
        this->points_.erase(pointsEnd, end(this->points_));

        if (this->points_.size() >=2)
        {
            // Recreate the line to account for the point removal.
            auto updated = Base(this->points_);
            this->point = updated.point;
            this->vector = updated.vector;
        }
        // else, there are not enough points to define a line.
    }
}


size_t LineFromPoints::GetPointCount() const
{
    return this->points_.size();
}


LineFromPoints::Points LineFromPoints::GetPoints() const
{
    return this->points_;
}


} // end namespace iris
