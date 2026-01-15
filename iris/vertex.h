#pragma once


#include <fields/fields.h>
#include <pex/interface.h>
#include <pex/range.h>
#include <tau/eigen.h>
#include <tau/vector2d.h>
#include <tau/percentile.h>
#include <draw/point.h>
#include <tau/mono_image.h>
#include <tau/margins.h>
#include <iris/filter_result.h>
#include <iris/vertex_settings.h>
#include <iris/threadsafe_filter.h>
#include <iris/harris.h>


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

    static ValuePoints AddMargin(
        const tau::Margins &margins,
        const ValuePoints &valuePoints)
    {
        ValuePoints result;
        result.reserve(valuePoints.size());

        for (auto &point: valuePoints)
        {
            result.emplace_back(
                point.x + margins.horizontalMargin,
                point.y + margins.verticalMargin,
                point.value);
        }

        return result;
    }

    static ValuePoints RemoveMargin(
        const tau::Margins &margins,
        const ValuePoints &valuePoints)
    {
        ValuePoints result;
        result.reserve(valuePoints.size());

        for (auto &point: valuePoints)
        {
            result.emplace_back(
                point.x - margins.horizontalMargin,
                point.y - margins.verticalMargin,
                point.value);
        }

        return result;
    }

    Vertex AddMargin(const tau::Margins &margins) const
    {
        return Vertex(
            this->point.x + margins.horizontalMargin,
            this->point.y + margins.verticalMargin,
            AddMargin(margins, this->valuePoints));
    }

    Vertex RemoveMargin(const tau::Margins &margins) const
    {
        return Vertex(
            this->point.x - margins.horizontalMargin,
            this->point.y - margins.verticalMargin,
            RemoveMargin(margins, this->valuePoints));
    }
};


struct Vertices: public FilterResult
{
    std::vector<Vertex> vertices;

    Vertices AddMargin(const tau::Margins &margins) const
    {
        Vertices result{};
        result.SetMargins(margins);
        result.vertices.reserve(this->vertices.size());

        for (auto &vertex: this->vertices)
        {
            result.vertices.push_back(vertex.AddMargin(margins));
        }

        return result;
    }

    Vertices RemoveMargin(const tau::Margins &margins) const
    {
        Vertices result{};
        result.vertices.reserve(this->vertices.size());

        for (auto &vertex: this->vertices)
        {
            result.vertices.push_back(vertex.RemoveMargin(margins));
        }

        return result;
    }

    tau::Size<Eigen::Index> GetSize() const
    {
        return {0, 0};
    }

    void Resize(const tau::Size<Eigen::Index> &)
    {

    }
};


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
    void AddMatrix(const Eigen::Ref<const tau::MonoImage<T>> &input)
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
    static constexpr bool wantsMargins = true;

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

    tau::Margins ComputeRequiredMargins() const
    {
        return {0, 0};
    }

    bool Filter(
        const HarrisResult<double> &input,
        Result &result,
        const tau::Margins &inputMargins)
    {
        if (!this->isEnabled_)
        {
            return false;
        }

        detail::PointGroups pointGroups(
            this->windowSize_ / 2,
            this->count_);

        pointGroups.AddMatrix<double>(inputMargins.GetValidView(input.data));
        result = Result(pointGroups.GetVertices());

        return true;
    }

private:
    bool isEnabled_;
    size_t count_;
    double windowSize_;
};


} // end namespace iris
