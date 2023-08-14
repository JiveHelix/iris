#pragma once


#include <vector>
#include <tau/line2d.h>
#include <tau/vector2d.h>


namespace iris
{


class LineFromPoints: public tau::Line2d<double>
{
public:
    using Base = tau::Line2d<double>;
    using Point = tau::Point2d<double>;
    using Points = std::vector<Point>;

    using Base::Base;

    LineFromPoints(
        double angleToleranceDegrees,
        double minimumSpacing,
        const Point &first,
        const Point &second);

    double GetError(const Point &point_) const;

    void Combine(const LineFromPoints &other);

    bool AddPoint(const Point &point);

    void RemoveOutliers(double maximumPointError);

    size_t GetPointCount() const;

    Points GetPoints() const;

    bool operator<(const LineFromPoints &other) const;

    bool operator==(const LineFromPoints &other) const;

private:
    using Iterator = typename Points::iterator;
    std::optional<Iterator> GetInsertion_(const Point &point);

private:
    double angleToleranceDegrees_;
    double minimumSpacing_;
    Points points_;
};


} // end namespace iris
