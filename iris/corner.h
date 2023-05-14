#pragma once


#include <fields/fields.h>
#include <pex/interface.h>
#include <pex/range.h>
#include <tau/eigen.h>
#include <tau/vector2d.h>
#include <tau/image.h>
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

CornerPoints GetPoints(const tau::ImageMatrixFloat &input);


namespace detail
{


class CornerCollector
{
public:
    using Index = typename Eigen::Index;

    CornerCollector(size_t windowSize, size_t count);

    const CornerPoints & GetCorners();

    void CollectFromWindow(
        tau::ImageMatrixFloat &input,
        Eigen::Index windowRow,
        Eigen::Index windowColumn);

private:
    size_t windowSize_;
    size_t count_;
    tau::Point2dCollection<double> points_;
    CornerPoints corners_;
};


} // end namespace detail


class Corner
{
public:
    Corner(const CornerSettings &settings);

    CornerPoints Filter(const tau::ImageMatrixFloat &input);

private:
    size_t count_;
    Eigen::Index windowSize_;
};


using ThreadsafeCorner = ThreadsafeFilter<CornerGroup, Corner>;


} // end namespace iris
