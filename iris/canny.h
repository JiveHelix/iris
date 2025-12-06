#pragma once


#include <future>
#include <vector>
#include <deque>
#include <tau/eigen.h>
#include <tau/planar.h>
#include <tau/color.h>
#include <tau/color_maps/rgb.h>

#include "iris/canny_settings.h"
#include "iris/gradient.h"
#include "iris/chunks.h"
#include "draw/pixels.h"


// #define LOG_REPEAT_SUPPRESSION


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

    std::shared_ptr<draw::Pixels> Colorize(const tau::Margins &margins) const
    {
        auto trimmed = margins.RemoveMargin(this->matrix);
        tau::HsvPlanes<Float> hsv(trimmed.rows(), trimmed.cols());

        GetSaturation(hsv).array() = Float(1);

        GetHue(hsv).array() = Float(120);

        GetHue(hsv).array() = (trimmed.array() < this->rangeHigh)
            .select(300, GetHue(hsv).array());

        GetValue(hsv) =
            (trimmed.array() >= this->rangeHigh).select(Float(1), trimmed);

        GetValue(hsv) =
            (trimmed.array() >= this->rangeLow
                && trimmed.array() < this->rangeHigh)
            .select(Float(0.7), trimmed);

        auto asRgb = tau::HsvToRgb<uint8_t>(hsv);

        return draw::Pixels::CreateShared(asRgb);
    }
};


template<typename Float>
class Canny
{
public:
    using Result = CannyResult<Float>;
    using Matrix = typename Result::Matrix;

    Canny() = default;

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

        Point GetNext(int direction) const
        {
            switch (direction)
            {
                case 0:
                    // horizontal
                    return Point(this->x + 1, this->y);

                case 1:
                case 3:
                    // diagonal
                    return Point(this->x + 1, this->y + 1);

                case 2:
                    // vertical
                    return Point(this->x, this->y + 1);

                default:
                    throw std::logic_error("Unexpected direction");
            }
        }

        Point GetPrevious(int direction) const
        {
            switch (direction)
            {
                case 0:
                    // horizontal
                    return Point(this->x - 1, this->y);

                case 1:
                case 3:
                    // diagonal
                    return Point(this->x - 1, this->y - 1);

                case 2:
                    // vertical
                    return Point(this->x, this->y - 1);

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

        Solver()
            :
            low_(),
            high_(),
            depth_(),
            rows_(),
            columns_(),
            suppressed_(),
            directions_(),
            result()
        {

        }

        Solver(
            const CannySettings<Float> &settings,
            const Matrix &suppressed,
            const Directions &directions)
        {
            this->Initialize(settings, suppressed, directions);
        }

        void Initialize(
            const CannySettings<Float> &settings,
            const Matrix &suppressed,
            const Directions &directions)
        {
            this->low_ = settings.range.low;
            this->high_ = settings.range.high;
            this->depth_ = settings.depth;
            this->rows_ = suppressed.rows();
            this->columns_ = suppressed.cols();
            this->suppressed_ = suppressed;
            this->directions_ = directions;
            this->result = Matrix::Zero(this->rows_, this->columns_);
        }

        // trigger is the point that caused us to check for an extension of the
        // edge.
        void Extend(const Point &check, const Point &trigger, size_t depth = 0)
        {
            Float value = this->suppressed_(check.y, check.x);

            if (value <= this->low_)
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
        }

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

    static void Solve(
        Solver &solver,
        const chunk::Chunk &chunk,
        const CannySettings<Float> &settings,
        const Eigen::MatrixX<Float> &suppressed,
        const Eigen::MatrixX<int> &directions)
    {
        using Eigen::Index;
        solver.Initialize(settings, suppressed, directions);

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
    }

    static bool LocateCenterPoint(
        int direction,
        Phasor<Float> &phasor,
        Matrix &suppressed,
        Point previous,
        Point point,
        Point next)
    {
        std::deque<Point> similarPoints;
        similarPoints.push_back(point);

        auto previousValue = phasor.GetMagnitude(previous);
        auto value = phasor.GetMagnitude(point);
        auto nextValue = phasor.GetMagnitude(next);

        auto rowCount = phasor.magnitude.rows();
        auto columnCount = phasor.magnitude.cols();

        // Collect all previous values that match
        while (value == previousValue)
        {
            similarPoints.push_front(previous);
            previous = previous.GetPrevious(direction);

            if (!previous.InBounds(rowCount, columnCount))
            {
                break;
            }

            previousValue = phasor.GetMagnitude(previous);
        }

        // Collect all next values that match.
        while (value == nextValue)
        {
            similarPoints.push_back(next);
            next = next.GetNext(direction);

            if (!next.InBounds(rowCount, columnCount))
            {
                break;
            }

            nextValue = phasor.GetMagnitude(next);
        }

        // Choose the middle point to pass through the suppression filter.
        auto middleIndex = similarPoints.size() / 2;

        auto keeper = similarPoints[middleIndex];

        bool repeated = false;

        if (suppressed(keeper.y, keeper.x) > 0.0)
        {
            repeated = true;
        }
        else
        {
            suppressed(keeper.y, keeper.x) = phasor.GetMagnitude(keeper);
        }

        // TODO: Reduce or eliminate repeated calls to this function without
        // degrading the result.
        // Note: Attempted setting all similarPoints to zero in the phasor
        // magnitude, but that degraded the result and made very little
        // difference to the repeat count.

        return repeated;
    }


    template<typename Value>
    bool Filter(const GradientResult<Value> &gradient, Result &result) const
    {
        if (!this->settings_.enable)
        {
            return false;
        }

        using Eigen::Index;

        result.rangeHigh = this->settings_.range.high;
        result.rangeLow = this->settings_.range.low;

        result.phasor = gradient.template GetPhasor<Float>();

        // Copy the phasor for local modification.
        auto phasor = result.phasor;

        // Reduce the phase to 8 sectors
        typename Solver::Directions reducedPhase =
            (phasor.phase.array() / 45).round().template cast<int>();

        // 0 and 4 are the same direction,
        // as are 1 and 5, 2 and 6, and 3 and 7
        typename Solver::Directions directions = tau::Modulo(reducedPhase, 4);

        Index rows = phasor.magnitude.rows();
        Index columns = phasor.magnitude.cols();

        Matrix suppressed = Matrix::Zero(rows, columns);

#ifdef LOG_REPEAT_SUPPRESSION
        size_t repeatCount = 0;
#endif

        // Non-maximum Suppression
        // Ignore the 1 pixel border to reduce branching inside the loop.
        for (Index column = 1; column < columns - 1; ++column)
        {
            for (Index row = 1; row < rows - 1; ++row)
            {
                auto point = Point(column, row);

                Float value = phasor.GetMagnitude(point);

                if (value <= result.rangeLow)
                {
                    continue;
                }

                int direction = directions(point.y, point.x);
                auto neighbors = point.GetNeighbors(direction);
                auto previous = neighbors.first;
                auto next = neighbors.second;

                Float previousValue = phasor.GetMagnitude(previous);
                Float nextValue = phasor.GetMagnitude(next);

                if ((value > previousValue) && (value > nextValue))
                {
                    suppressed(point.y, point.x) = value;
                }
                else if ((value == previousValue) || value == nextValue)
                {
#ifdef LOG_REPEAT_SUPPRESSION
                    if (
                        LocateCenterPoint(
                            direction,
                            phasor,
                            suppressed,
                            previous,
                            point,
                            next)
                        )
                    {
                        ++repeatCount;
                    }
#else
                    LocateCenterPoint(
                        direction,
                        phasor,
                        suppressed,
                        previous,
                        point,
                        next);
#endif
                }
            }
        }

#ifdef LOG_REPEAT_SUPPRESSION
        auto suppressedCount = (suppressed.array() > 0).count();
        auto totalCount = (columns - 1) * (rows - 1);

        std::cout << "suppressed count: " << suppressedCount << std::endl;
        std::cout << "total count: " << totalCount << std::endl;
        std::cout << "repeated count: " << repeatCount << std::endl;

        std::cout << "repeated rate: "
            << 100.0 * static_cast<double>(repeatCount)
                / static_cast<double>(suppressedCount)
            << " %"
            << std::endl;
#endif

        auto chunks = chunk::MakeChunks(this->settings_.threads, rows);

        std::vector<jive::Sentry> threadSentries;
        threadSentries.reserve(chunks.size());
        std::vector<Solver> solvers(chunks.size());
        auto threadPool = jive::GetThreadPool();

        for (auto index: jive::Range<size_t>(0, chunks.size()))
        {
            threadSentries.emplace_back(
                threadPool->AddJob(
                    [&, index]()
                    {
                        Canny<Float>::Solve(
                            solvers[index],
                            chunks[index],
                            this->settings_,
                            suppressed,
                            directions);
                    }));
        }

        result.matrix = Matrix::Zero(rows, columns);

        for (auto index: jive::Range<size_t>(0, chunks.size()))
        {
            threadSentries[index].Wait();

            const auto &chunkResult = solvers[index].result;

            // Each thread may have followed an edge into another thread's
            // boundaries.
            // Only copy non-zero values.
            result.matrix =
                (chunkResult.array() > 0).select(chunkResult, result.matrix);
        }

        return true;
    }

private:
    CannySettings<Float> settings_;
};


extern template struct CannyResult<double>;
extern template class Canny<double>;


} // end namespace iris
