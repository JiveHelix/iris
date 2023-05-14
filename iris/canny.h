#pragma once


#include <future>
#include <vector>
#include <optional>
#include <tau/eigen.h>
#include <tau/planar.h>
#include <tau/color.h>
#include <tau/color_maps/rgb.h>

#include "iris/canny_settings.h"
#include "iris/gradient.h"
#include "iris/chunks.h"


namespace iris
{


template<typename Float>
struct CannyResult
{
    using Matrix =
        Eigen::Matrix<Float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

    Phasor<Float> phasor;
    Matrix matrix;
    Float rangeHigh;
    Float rangeLow;

    template<typename Pixel>
    tau::RgbPixels<Pixel> Colorize() const
    {
        tau::HsvPlanes<float> hsv(this->matrix.rows(), this->matrix.cols());

        GetSaturation(hsv).array() = 1.0;

        GetHue(hsv).array() = 120.0;

        GetHue(hsv).array() = (matrix.array() < this->rangeHigh)
            .select(300.0, GetHue(hsv).array());

        GetValue(hsv) = (matrix.array() >= this->rangeHigh).select(1.0, matrix);
        GetValue(hsv) =
            (matrix.array() >= this->rangeLow
                && matrix.array() < this->rangeHigh)
            .select(static_cast<Float>(0.7), matrix);

        auto asRgb = tau::HsvToRgb<Pixel>(hsv);

        return {
            asRgb.template GetInterleaved<Eigen::RowMajor>(),
            {this->matrix.cols(), this->matrix.rows()}};
    }
};


template<typename Float>
class Canny
{
public:
    using Result = CannyResult<Float>;
    using Matrix = typename Result::Matrix;

    Canny(const CannySettings<Float> &settings)
        :
        settings_(settings)
    {

    }

    struct Point: public tau::Point2d<Eigen::Index>
    {
        using Base = tau::Point2d<Eigen::Index>;
        using Neighbors = std::pair<Point, Point>;

        using Base::Base;

        Neighbors GetNeighbors(int direction) const
        {
            switch (direction)
            {
                case 0:
                    // horizontal
                    return std::make_pair(
                        Point(this->x - 1, this->y),
                        Point(this->x + 1, this->y));

                case 1:
                case 3:
                    // diagonal
                    return std::make_pair(
                        Point(this->x - 1, this->y - 1),
                        Point(this->x + 1, this->y + 1));

                case 2:
                    // vertical
                    return std::make_pair(
                        Point(this->x, this->y - 1),
                        Point(this->x, this->y + 1));

                default:
                    throw std::logic_error("Unexpected direction");
            }
        }

        bool InBounds(Eigen::Index rows, Eigen::Index columns) const
        {
            return this->AndGreaterEqual(Point(0, 0))
                && this->AndLess(Point(columns, rows));
        }
    };

    struct Solver
    {
        using Directions =
            Eigen::Matrix<int, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

        Solver(
            const CannySettings<Float> &settings,
            const Matrix &suppressed,
            const Directions &directions)
            :
            low_(settings.range.low),
            high_(settings.range.high),
            depth_(settings.depth),
            rows_(suppressed.rows()),
            columns_(suppressed.cols()),
            suppressed_(suppressed),
            directions_(directions),
            result(Matrix::Zero(this->rows_, this->columns_))
        {

        }

        // Do not allow temporaries to bind to const references.
        Solver(Matrix &&, Directions &&) = delete;

        // trigger is the point that caused us to check for an extension of the
        // edge.
        void Extend(const Point &check, const Point &trigger, size_t depth = 0)
        {
            Float value = this->suppressed_(check.y, check.x);

            if (value < this->low_)
            {
                return;
            }

            // This point can be considered on the edge.
            if (this->result(check.y, check.x) > 0)
            {
                // We have been here before.
                // End the recursion.
                return;
            }

            this->result(check.y, check.x) = value;

            if (depth >= this->depth_)
            {
                return;
            }

            // Check the neighbors along the perpendicular edge.
            int direction = (this->directions_(check.y, check.x) + 2) % 4;
            auto neighbors = check.GetNeighbors(direction);

            if (neighbors.first != trigger
                    && neighbors.first.InBounds(this->rows_, this->columns_))
            {
                // Check the first neighbor for inclusion.
                this->Extend(neighbors.first, check, depth + 1);
            }

            if (neighbors.second != trigger
                    && neighbors.second.InBounds(this->rows_, this->columns_))
            {
                // Check the second neighbor for inclusion.
                this->Extend(neighbors.second, check, depth + 1);
            }
        };

    private:
        Float low_;
        Float high_;
        size_t depth_;
        Eigen::Index rows_;
        Eigen::Index columns_;
        Matrix suppressed_;
        Directions directions_;

    public:
        Matrix result;
    };

    static Matrix Solve(
        const chunk::Chunk &chunk,
        const CannySettings<Float> &settings,
        const Eigen::MatrixX<Float> &suppressed,
        const Eigen::MatrixX<int> &directions)
    {
        using Eigen::Index;

        Solver solver(settings, suppressed, directions);

        Index columns = suppressed.cols();

        // Apply strong/weak hysteresis to suppressed result.
        for (Index row = chunk.index; row < chunk.index + chunk.count; ++row)
        {
            for (Index column = 0; column < columns; ++column)
            {
                if (suppressed(row, column) < settings.range.high)
                {
                    // Not high enough to trigger edge detection.
                    continue;
                }

                // This pixel is high enough to be considered an edge.
                // Follow the neighboring pixels as long as they are above the
                // lower threshold.
                auto candidate = Point(column, row);
                solver.Extend(candidate, candidate);
            }
        }

        return solver.result;
    }

    template<typename Value, typename Data>
    std::optional<Result> Filter(
        const GradientResult<Value, Data> &gradient) const
    {
        if (!this->settings_.enable)
        {
            return {};
        }

        using Eigen::Index;

        Result canny{};
        canny.rangeHigh = this->settings_.range.high;
        canny.rangeLow = this->settings_.range.low;

        canny.phasor = gradient.template GetPhasor<Float>();

        // Reduce the phase to 8 sectors
        typename Solver::Directions reducedPhase =
            (canny.phasor.phase.array() / 45).round().template cast<int>();

        // 0 and 4 are the same direction,
        // as are 1 and 5, 2 and 6, and 3 and 7
        typename Solver::Directions directions = tau::Modulo(reducedPhase, 4);

        Index rows = canny.phasor.magnitude.rows();
        Index columns = canny.phasor.magnitude.cols();

        Matrix suppressed = Matrix::Zero(rows, columns);

        // Non-maximum Suppression
        // Ignore the 1 pixel border to reduce branching inside the loop.
        for (Index column = 1; column < columns - 1; ++column)
        {
            for (Index row = 1; row < rows - 1; ++row)
            {
                auto point = Point(column, row);

                Float value = canny.phasor.magnitude(point.y, point.x);
                int direction = directions(point.y, point.x);
                auto neighbors = point.GetNeighbors(direction);

                Float compare0 = canny.phasor.magnitude(
                    neighbors.first.y,
                    neighbors.first.x);

                Float compare1 = canny.phasor.magnitude(
                    neighbors.second.y,
                    neighbors.second.x);

                if ((value >= compare0) && (value >= compare1))
                {
                    suppressed(point.y, point.x) = value;
                }
            }
        }

        std::vector<std::future<Matrix>> threadResults;

        auto chunks = chunk::MakeChunks(this->settings_.threads, rows);

        for (auto &chunk: chunks)
        {
            threadResults.push_back(
                std::async(
                    std::launch::async,
                    &Canny<Float>::Solve,
                    chunk,
                    this->settings_,
                    suppressed,
                    directions));
        }

        canny.matrix = Matrix::Zero(rows, columns);

        for (auto &threadResult: threadResults)
        {
            Matrix result = threadResult.get();

            // Each thread may have followed an edge into another thread's
            // boundaries.
            // Only copy non-zero values.
            canny.matrix = (result.array() > 0).select(result, canny.matrix);
        }

        return canny;
    }

private:
    CannySettings<Float> settings_;
};


} // end namespace iris
