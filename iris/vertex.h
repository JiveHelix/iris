#pragma once


#include <fields/fields.h>
#include <pex/interface.h>
#include <pex/range.h>
#include <tau/eigen.h>
#include <tau/vector2d.h>
#include <tau/percentile.h>
#include <draw/point.h>
#include "iris/image.h"
#include "iris/vertex_settings.h"
#include "iris/threadsafe_filter.h"


namespace iris
{


using ValuePoints = std::vector<draw::ValuePoint<double>>;


struct Vertex
{
    tau::Point2d<double> point;
    double count;
    ValuePoints valuePoints;

    Vertex(double x, double y, const ValuePoints &valuePoints_);

    bool operator>(const Vertex &other) const;

    bool operator<(const Vertex &other) const;

    // For the purpose of determining unique vertices, vertices with the same
    // point compare equal, even if their counts differ.
    bool operator==(const Vertex &other) const;
};


using Vertices = std::vector<Vertex>;


std::vector<tau::Point2d<double>> VerticesToPoints(const Vertices &);
ValuePoints VerticesToValuePoints(const Vertices &);


std::ostream & operator<<(std::ostream &, const Vertex &);


namespace detail
{


std::optional<Vertex> GetCentroid(size_t count, const ValuePoints &points);


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
        ValuePoints points;
        points.reserve(static_cast<size_t>((input.array() > 0).count()));

        using Index = Eigen::Index;

        for (Index row = 0; row < input.rows(); ++row)
        {
            for (Index column = 0; column < input.cols(); ++column)
            {
                T value = input(row, column);

                if (value != 0)
                {
                    points.emplace_back(
                        static_cast<double>(column),
                        static_cast<double>(row),
                        static_cast<double>(value));
                }
            }
        }

        for (auto &point: points)
        {
            this->AddPoint_(point);
        }

        for (auto & [key, group]: this->pointGroupByPoint_)
        {
            if (group.size() > 4)
            {
                // Retain the top 4 values.
                std::sort(
                    group.begin(),
                    group.end(),
                    [](const auto &first, const auto &second)
                    {
                        return first.value > second.value;
                    });

                group.resize(4);
            }

            assert(!group.empty());
            auto it = std::begin(group);
            double maximumValue = it->value;

            while (it != std::end(group))
            {
                maximumValue = std::max(maximumValue, (it++)->value);
            }

            for (auto &point: group)
            {
                point.value /= maximumValue;
            }
        }
    }

    Vertices GetVertices() const;

private:
    void AddPoint_(const draw::ValuePoint<double> &point);

    double radiusSquared_;
    size_t count_;

    std::map<tau::Point2d<double>, ValuePoints> pointGroupByPoint_;
};


} // end namespace detail


class VertexFinder
{
public:
    using Result = Vertices;

    VertexFinder() = default;

    VertexFinder(const VertexSettings &settings)
        :
        isEnabled_(settings.enable),
        count_(static_cast<size_t>(settings.count)),
        windowSize_(settings.window)
    {
        assert(
            settings.count
                < static_cast<Eigen::Index>(settings.window * settings.window));

        assert(settings.count > 0);
    }

    std::optional<Result> Filter(const ImageMatrix<double> &input)
    {
        if (!this->isEnabled_)
        {
            return {};
        }

        detail::PointGroups pointGroups(
            this->windowSize_ / 2,
            this->count_);

        pointGroups.AddMatrix(input);

        return pointGroups.GetVertices();
    }

private:
    bool isEnabled_;
    size_t count_;
    double windowSize_;
};


} // end namespace iris
