#pragma once


#include <future>
#include <vector>
#include <optional>
#include <tau/eigen.h>
#include <tau/stack.h>
#include <tau/planar.h>
#include <tau/color.h>
#include <tau/color_maps/rgb.h>
#include <tau/line2d.h>

#include "iris/hough_settings.h"
#include "iris/canny.h"
#include "iris/chunks.h"
#include "iris/suppression.h"


namespace iris
{


template<typename Float>
struct HoughResult
{
    using Lines = std::vector<tau::Line2d<Float>>;

    using Matrix =
        Eigen::Matrix<Float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

    Matrix space;
    Lines lines;

    template<typename Value>
    tau::MatrixLike<Value, Matrix> GetScaledSpace(Float targetMaximum) const
    {
        auto maximum = this->space.maxCoeff();

        if (maximum < Float(1))
        {
            return tau::MatrixLike<Value, Matrix>::Zero(
                this->space.rows(),
                this->space.cols());
        }

        auto scaled = (this->space.array() * targetMaximum / maximum)
                .floor().template cast<Value>();

        return scaled;
    }
};


template<typename Float>
struct EdgePoint
{
    using Index = typename Eigen::Index;
    Index row;
    Index column;
    Float weight;
    Float phase;

    tau::Point2d<Index> GetPoint2d() const
    {
        return {this->column, this->row};
    }
};


template<typename Float>
using EdgePoints = std::vector<EdgePoint<Float>>;


template<typename Data, typename Float>
class WeightedEdgeMaker
{
public:
    using Phase = typename Phasor<Float>::Matrix;

    WeightedEdgeMaker(
        Eigen::MatrixBase<Data> &&edges,
        Phase &&phase) = delete;

    WeightedEdgeMaker(
        const Eigen::MatrixBase<Data> &edges,
        const Phase &phase)
        :
        edges_(edges),
        phase_(phase)
    {

    }

    EdgePoint<Float> operator()(Eigen::Index row, Eigen::Index column)
    {
        return {
            row,
            column,
            this->edges_(row, column),
            this->phase_(row, column)};
    }

protected:
    const Eigen::MatrixBase<Data> & edges_;
    const Phase & phase_;
};


template<typename Data, typename Float>
class EdgeMaker: public WeightedEdgeMaker<Data, Float>
{
public:
    using Base = WeightedEdgeMaker<Data, Float>;
    using Base::Base;

    EdgePoint<Float> operator()(Eigen::Index row, Eigen::Index column)
    {
        return {
            row,
            column,
            1,
            this->phase_(row, column)};
    }
};


template
<
    template<typename, typename> typename EdgeFunctor,
    typename Data,
    typename Float
>
EdgePoints<Float> MakeEdgePoints(
    const Eigen::MatrixBase<Data> &edges,
    const Phasor<Float> &phasor)
{
    using Eigen::Index;
    EdgePoints<Float> result;

    result.reserve(
        static_cast<size_t>((edges.array() > 0).count()));

    EdgeFunctor<Data, Float> edgeFunctor(edges, phasor.phase);

    if constexpr (tau::MatrixTraits<Data>::isColumnMajor)
    {
        for (Index column = 0; column < edges.cols(); ++column)
        {
            for (Index row = 0; row < edges.rows(); ++row)
            {
                if (edges(row, column) > 0)
                {
                    result.push_back(edgeFunctor(row, column));
                }
            }
        }
    }
    else
    {
        // row major
        for (Index row = 0; row < edges.rows(); ++row)
        {
            for (Index column = 0; column < edges.cols(); ++column)
            {
                if (edges(row, column) > 0)
                {
                    result.push_back(edgeFunctor(row, column));
                }
            }
        }
    }

    return result;
}


template<typename Float>
class Hough
{
public:
    using Result = HoughResult<Float>;
    using Matrix = typename Result::Matrix;
    using Index = Eigen::Index;

    class Accumulator
    {
    public:
        using RowVector = Eigen::RowVectorX<Float>;

        Accumulator() = default;

        static Float GetToIndexFactor(Index rhoCount, Float maximumRho)
        {
            auto maximumRhoIndex = static_cast<Float>(rhoCount - 1);

            // rho can be negative, so there are 2x as many indices as
            // maximumRho_.
            return maximumRhoIndex / (2 * maximumRho);
        }

        Accumulator(
                const draw::Size &imageSize,
                size_t rhoCount,
                size_t thetaCount,
                Float angleRange)
            :
            center_(imageSize.ToPoint2d().template Convert<Float>() / 2),
            maximumRho_(this->center_.Magnitude()),
            rhoCount_(static_cast<Index>(rhoCount)),

            toIndexFactor_(
                GetToIndexFactor(this->rhoCount_, this->maximumRho_)),

            thetaCount_(static_cast<Index>(thetaCount)),
            angleRange_(angleRange),
            thetas_(
                RowVector::LinSpaced(
                    this->thetaCount_,
                    static_cast<Float>(0),
                    static_cast<Float>(tau::Angles<Float>::pi))),
            sinesAndCosines_(
                tau::VerticalStack(
                    this->thetas_.array().sin().eval(),
                    this->thetas_.array().cos().eval())),
            space_(
                Matrix::Zero(
                    static_cast<Index>(rhoCount),
                    static_cast<Index>(thetaCount)))
        {

        }

        void AddPoint_(
            tau::Point2d<Float> &centeredPoint,
            Float weight,
            Float lowAngle,
            Float highAngle)
        {
            // Lookup indices.
            Index lowIndex = this->ToThetaIndex(lowAngle);
            Index highIndex = this->ToThetaIndex(highAngle);
            Index thetaCount = highIndex - lowIndex;

            if (thetaCount == 0)
            {
                return;
            }

            // rho = x * cosine(theta) + y * sine(theta)
            Eigen::RowVector<Float, 2> pointVector(
                centeredPoint.y,
                centeredPoint.x);

            // Compute rho only for the selected angle range.
            RowVector rhos = pointVector
                * this->sinesAndCosines_.block(0, lowIndex, 2, thetaCount);

            assert(rhos.cols() == thetaCount);
            assert(rhos.maxCoeff() <= this->maximumRho_);

            auto indices = this->ToRhoIndex(rhos);

            assert(indices.cols() == thetaCount);
            assert(lowIndex + thetaCount <= this->space_.cols());
            assert(indices.maxCoeff() < this->space_.rows());

            for (Index i = 0; i < thetaCount; ++i)
            {
                this->space_(indices(i), lowIndex + i) += weight;
            }
        }

        void AddPoint(
            tau::Point2d<Index> pointIndex,
            Float weight,
            Float angle)
        {
            auto point = pointIndex.template Convert<Float>();
            point -= this->center_;

            angle = std::fmod(angle, static_cast<Float>(180));
            Float lowAngle = angle - this->angleRange_;
            Float highAngle = angle + this->angleRange_;

            // Handle wrap at 0 and 180.
            if (lowAngle < 0)
            {
                this->AddPoint_(point, weight, lowAngle + 180, 180);
                this->AddPoint_(point, weight, 0, angle);
            }
            else
            {
                this->AddPoint_(point, weight, lowAngle, angle);
            }

            if (highAngle > 180)
            {
                this->AddPoint_(point, weight, 0, highAngle - 180);
                this->AddPoint_(point, weight, angle, 180);
            }
            else
            {
                this->AddPoint_(point, weight, angle, highAngle);
            }

        }

        template<typename Derived>
        auto ToRhoIndex(const Eigen::MatrixBase<Derived> &rho) const
            -> tau::MatrixLike<Index, Derived>
        {
            // Shift rho values to the positive.

            return ((rho.array() + this->maximumRho_) * this->toIndexFactor_)
                .round()
                .template cast<Index>();
        }

        Float ToRho(Index index) const
        {
            return (static_cast<Float>(index) / this->toIndexFactor_)
                - this->maximumRho_;
        }

        Index ToThetaIndex(Float theta) const
        {
            return static_cast<Index>(
                std::round(
                    theta * static_cast<Float>(this->thetaCount_ - 1) / 180));
        }

        Float ToTheta(Index index) const
        {
            return this->thetas_(index);
        }

        const Matrix & GetSpace() const
        {
            return this->space_;
        }

        void SetSpace(const Matrix &space)
        {
            this->space_ = space;
        }

        std::vector<tau::Line2d<Float>> GetLines(Float threshold)
        {
            std::vector<tau::Line2d<Float>> result;

            for (
                Index rowIndex = 0;
                rowIndex < this->space_.rows();
                ++rowIndex)
            {
                for (
                    Index columnIndex = 0;
                    columnIndex < this->space_.cols();
                    ++columnIndex)
                {
                    if (this->space_(rowIndex, columnIndex) > threshold)
                    {
                        auto rho = this->ToRho(rowIndex);

                        auto degrees =
                            tau::ToDegrees(this->ToTheta(columnIndex));

                        result.emplace_back(rho, degrees);
                    }
                }
            }

            // Shift the lines so they are relative to the image origin.
            for (auto &line: result)
            {
                line.point += this->center_;
            }

            return result;
        }

    private:
        tau::Point2d<Float> center_;
        Float maximumRho_;
        Index rhoCount_;
        Float toIndexFactor_;
        Index thetaCount_;
        Float angleRange_;
        RowVector thetas_;
        Matrix sinesAndCosines_;
        Matrix space_;
    };

    Hough() = default;

    Hough(const HoughSettings<Float> &settings)
        :
        settings_(settings)
    {

    }

    static Accumulator Accumulate(
        const HoughSettings<Float> &settings,
        const EdgePoints<Float> &edgePoints)
    {
        Accumulator accumulator(
            settings.imageSize,
            settings.rhoCount,
            settings.thetaCount,
            settings.angleRange);

        for (auto &edgePoint: edgePoints)
        {
            accumulator.AddPoint(
                edgePoint.GetPoint2d(),
                edgePoint.weight,
                edgePoint.phase);
        }

        return accumulator;
    }

    std::optional<Result> Filter(const CannyResult<Float> &canny) const
    {
        if (!this->settings_.enable)
        {
            return {};
        }

        EdgePoints<Float> edgePoints;

        if (this->settings_.weighted)
        {
            edgePoints =
                MakeEdgePoints<WeightedEdgeMaker>(canny.matrix, canny.phasor);
        }
        else
        {
            edgePoints =
                MakeEdgePoints<EdgeMaker>(canny.matrix, canny.phasor);
        }

        Index maximumColumn = 0;
        Index maximumRow = 0;

        for (auto &edgePoint: edgePoints)
        {
            maximumColumn = std::max(maximumColumn, edgePoint.column);
            maximumRow = std::max(maximumRow, edgePoint.row);
        }

        auto chunks = chunk::MakeChunks(
            this->settings_.threads,
            static_cast<Index>(edgePoints.size()));

        std::vector<std::future<Accumulator>> threadResults;

        for (auto &chunk: chunks)
        {
            auto begin = std::begin(edgePoints) + chunk.index;
            auto end = begin + chunk.count;

            threadResults.push_back(
                std::async(
                    std::launch::async,
                    &Hough<Float>::Accumulate,
                    this->settings_,
                    EdgePoints<Float>(begin, end)));
        }

        auto accumulator = threadResults.at(0).get();
        Matrix combined = accumulator.GetSpace();

        for (size_t i = 1; i < threadResults.size(); ++i)
        {
            combined += threadResults[i].get().GetSpace();
        }

        Result hough{};

        if (this->settings_.suppress)
        {
            auto windowSize = this->settings_.window;

            hough.space = Suppression(
                this->settings_.threads,
                windowSize,
                combined);

            // Run suppression again on the seam between 180 and 0.
            auto seam = Matrix(hough.space.rows(), 2 * windowSize);

            seam.leftCols(windowSize) =
                hough.space.rightCols(windowSize);

            seam.rightCols(windowSize) =
                hough.space.leftCols(windowSize).colwise().reverse();

            auto suppressedSeam = Suppression(
                this->settings_.threads,
                windowSize,
                seam);

            hough.space.rightCols(windowSize) =
                suppressedSeam.leftCols(windowSize);

            hough.space.leftCols(windowSize) =
                suppressedSeam.rightCols(windowSize).colwise().reverse();

            accumulator.SetSpace(hough.space);
            hough.lines = accumulator.GetLines(this->settings_.threshold);
        }
        else
        {
            hough.space = combined;
        }

        return hough;
    }

private:
    HoughSettings<Float> settings_;
};


} // end namespace iris
