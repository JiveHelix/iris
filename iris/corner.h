#pragma once


#include <fields/fields.h>
#include <pex/interface.h>
#include <pex/range.h>
#include <tau/eigen.h>
#include <tau/vector2d.h>
#include "iris/image.h"
#include "iris/corner_settings.h"
#include "iris/threadsafe_filter.h"


namespace iris
{


struct CornerPoint
{
    tau::Point2d<double> point;
    double count;

    CornerPoint(double x, double y, double count_);

    bool operator>(const CornerPoint &other) const;

    bool operator<(const CornerPoint &other) const;

    // For the purpose of determining unique corners, corners with the same
    // point compare equal, even if their counts differ.
    bool operator==(const CornerPoint &other) const;
};


using CornerPoints = std::vector<CornerPoint>;


std::vector<tau::Point2d<double>> CornerPointsToPoints(const CornerPoints &);


std::ostream & operator<<(std::ostream &, const CornerPoint &);


template<typename T>
CornerPoints GetPoints(const ImageMatrix<T> &input)
{
    using Eigen::Index;

    CornerPoints points;
    static_assert(tau::MatrixTraits<ImageMatrix<T>>::isRowMajor);

    for (Index row = 0; row < input.rows(); ++row)
    {
        for (Index column = 0; column < input.cols(); ++column)
        {
            if (input(row, column) != 0)
            {
                points.emplace_back(
                    static_cast<double>(column),
                    static_cast<double>(row),
                    1);
            }
        }
    }

    return points;
}


namespace detail
{


using Points = tau::Point2dCollection<double>;


std::optional<CornerPoint> GetCentroid(size_t count, const Points &points);


class PointGroups
{
public:
    PointGroups(double radius, size_t count)
        :
        radiusSquared_(radius * radius),
        count_(count),
        pointGroupByPoint_()
    {

    }

    template<typename T>
    void AddMatrix(const ImageMatrix<T> &input)
    {
        // Create a vector of all of the non-zero values.
        Points points;
        points.reserve(static_cast<size_t>((input.array() > 0).count()));

        using Index = Eigen::Index;

        for (Index row = 0; row < input.rows(); ++row)
        {
            for (Index column = 0; column < input.cols(); ++column)
            {
                if (input(row, column) != 0)
                {
                    points.emplace_back(
                        static_cast<double>(column),
                        static_cast<double>(row));
                }
            }
        }

        for (auto &point: points)
        {
            this->AddPoint_(point);
        }
    }

    CornerPoints GetCornerPoints() const;

private:
    void AddPoint_(const tau::Point2d<double> &point);

    double radiusSquared_;
    size_t count_;

    std::map<tau::Point2d<double>, Points> pointGroupByPoint_;
};


#if 0

class CornerCollector
{
public:
    using Index = typename Eigen::Index;

    CornerCollector(size_t windowSize, size_t count);

    const CornerPoints & GetCorners();

    template<typename T>
    void CollectFromWindow(
        ImageMatrix<T> &input,
        Eigen::Index windowRow,
        Eigen::Index windowColumn)
    {
        auto MakePoint = [this, &input] (Index row, Index column) -> void
        {
            if (input(row, column) != 0)
            {
                this->points_.emplace_back(
                    static_cast<double>(column),
                    static_cast<double>(row));
            }
        };

        // Iterate over the window at this position.
        this->points_.clear();
        auto windowSize = static_cast<Index>(this->windowSize_);
        static_assert(tau::MatrixTraits<ImageMatrix<T>>::isRowMajor);

        for (Index row = 0; row < windowSize; ++row)
        {
            Index detectionRow = windowRow + row;

            for (Index column = 0; column < windowSize; ++column)
            {
                MakePoint(detectionRow, windowColumn + column);
            }
        }

        auto pointsSize = this->points_.size();

        if (pointsSize == 0)
        {
            return;
        }

        if (pointsSize % 2 != 0)
        {
            // Odd number of harris detections are unlikely to be centered
            // around an intersection.
            return;
        }

        if (this->count_ == 4 && pointsSize < 4)
        {
            return;
        }

        double centroidX = 0;
        double centroidY = 0;
        auto pointCount = static_cast<double>(this->points_.size());

        for (auto &point: this->points_)
        {
            centroidX += point.x;
            centroidY += point.y;

            // Remove the used points so they do not contribute to other corner
            // detections.
            input(static_cast<Index>(point.y), static_cast<Index>(point.x)) = 0;
        }

        this->corners_.emplace_back(
            centroidX / pointCount,
            centroidY / pointCount,
            pointCount);
    }


private:
    size_t windowSize_;
    size_t count_;
    tau::Point2dCollection<double> points_;
    CornerPoints corners_;
};
#endif


} // end namespace detail


template<typename T>
class Corner
{
public:
    using Result = CornerPoints;

    Corner() = default;

    Corner(const CornerSettings &settings)
        :
        isEnabled_(settings.enable),
        count_(static_cast<size_t>(settings.count)),
        windowSize_(settings.window)
    {
        assert(settings.count < settings.window * settings.window);
        assert(settings.count > 0);
    }

    std::optional<Result> Filter(const ImageMatrix<T> &input)
    {
        if (!this->isEnabled_)
        {
            return {};
        }

        detail::PointGroups pointGroups(
            this->windowSize_ / 2,
            this->count_);

        pointGroups.AddMatrix(input);

        return pointGroups.GetCornerPoints();

#if 0
        using Eigen::Index;

        // Make a modifiable copy of the input points.
        ImageMatrix<T> points = input.derived();

        Index rowCount = input.rows();
        Index columnCount = input.cols();

        Index limitRow = rowCount - this->windowSize_ + 1;
        Index limitColumn = columnCount - this->windowSize_ + 1;

        detail::CornerCollector cornerCollector(
            static_cast<size_t>(this->windowSize_),
            this->count_);

        // Move the window.
        for (Index row = 0; row < limitRow; ++row)
        {
            for (Index column = 0; column < limitColumn; ++column)
            {
                cornerCollector.CollectFromWindow(points, row, column);
            }
        }

        return cornerCollector.GetCorners();
#endif
    }

private:
    bool isEnabled_;
    size_t count_;
    double windowSize_;
};


} // end namespace iris
