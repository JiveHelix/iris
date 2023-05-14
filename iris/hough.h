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
    using Matrix =
        Eigen::Matrix<Float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

    Matrix space;
    std::vector<tau::Line2d<Float>> lines;

    template<typename Value>
    tau::MatrixLike<Value, Matrix> GetScaledSpace(float maximumValue)
    {
        auto maximum = this->space.maxCoeff();

        return (this->space.array() * maximumValue / maximum)
                .floor().template cast<Value>();
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

        Accumulator(
                const Size &imageSize,
                size_t rhoCount,
                size_t thetaCount,
                Float angleRange)
            :
            center_(imageSize.ToPoint2d().template Convert<Float>() / 2),
            maximumRho_(this->center_.Magnitude()),
            rhoCount_(static_cast<Index>(rhoCount)),

            toIndexFactor_(
                static_cast<Float>(this->rhoCount_ - 1)
                    / (2 * this->maximumRho_)),

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

            assert(indices.cols() == rhos.cols());
            assert(lowIndex + rhos.cols() <= this->space_.cols());

            for (Index i = 0; i < rhos.cols(); ++i)
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
            // Shift row values to the positive.

            return ((rho.array() + this->maximumRho_) * this->toIndexFactor_)
                .floor()
                .template cast<Index>();
        }

        Float ToRho(Index index) const
        {
            return (static_cast<Float>(index) / this->toIndexFactor_) - this->maximumRho_;
        }

        Index ToThetaIndex(Float theta) const
        {
            return static_cast<Index>(
                std::floor(
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
            using IndexPoint = tau::Point2d<Index>;
            std::vector<IndexPoint> rhoThetas;

            std::cout << "\nGetLines(" << threshold << ")" << std::endl;

            for (Index row = 0; row < this->space_.rows(); ++row)
            {
                for (Index column = 0; column < this->space_.cols(); ++column)
                {
                    if (this->space_(row, column) > threshold)
                    {
                        rhoThetas.emplace_back(
                            row,
                            column);

                        result.emplace_back(
                            this->ToRho(row),
                            tau::ToDegrees(this->ToTheta(column)));
                    }
                }
            }

            // Shift the lines so they are relative to the image origin.
            for (auto &line: result)
            {
                line.point += this->center_;
            }

            if (rhoThetas.size() < 2)
            {
                return result;
            }

            std::sort(
                rhoThetas.begin(),
                rhoThetas.end());

            auto rhoThetaIndex = std::begin(rhoThetas);
            auto next = rhoThetaIndex + 1;
            Index count = 0;

            while (next != std::end(rhoThetas))
            {
                std::cout << '\n' << ++count << ": " << *rhoThetaIndex << ": "
                    << rhoThetaIndex->Distance(*next) << std::endl;

                std::cout
                    << "rho: " << this->ToRho(rhoThetaIndex->x)
                    << ", theta: "
                    << tau::ToDegrees(this->ToTheta(rhoThetaIndex->y))
                    << ", value: "
                    << this->space_(rhoThetaIndex->x, rhoThetaIndex->y)
                    << std::endl;

                ++rhoThetaIndex;
                ++next;
            }

            std::cout << '\n' << ++count << ": " << *rhoThetaIndex << std::endl;

            std::cout
                << "rho: " << this->ToRho(rhoThetaIndex->x)
                << ", theta: "
                << tau::ToDegrees(this->ToTheta(rhoThetaIndex->y))
                << ", value: "
                << this->space_(rhoThetaIndex->x, rhoThetaIndex->y)
                << std::endl;

            return result;
        }

        std::vector<tau::Line2d<Float>>
        FakeLines(size_t fakeCount)
        {
            std::vector<tau::Line2d<Float>> result;

            size_t angleCount = fakeCount / 2;

            Float angleStep = Float(180.0) / static_cast<Float>(angleCount);

            size_t count = 0;
            std::cout << "fakeCount: " << fakeCount << std::endl;
            std::cout << "angleCount: " << angleCount << std::endl;

            for (Float rho = -300; rho < 301; rho += 600)
            {
                for (size_t i = 0; i < angleCount; ++i)
                {
                    Float angle = static_cast<Float>(i) * angleStep;
                    result.emplace_back(rho, angle);

                    std::cout << ++count << " rho: " << rho
                        << ", angle: " << angle << std::endl;
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

        std::cout << edgePoints.size() << " edge points" << std::endl;
        std::cout << "maximumColumn: " << maximumColumn << std::endl;
        std::cout << "maximumRow: " << maximumRow << std::endl;

        auto chunks = chunk::MakeChunks(
            this->settings_.threads,
            static_cast<Index>(edgePoints.size()));

        std::vector<std::future<Accumulator>> threadResults;

        size_t count = 0;

        for (auto &chunk: chunks)
        {
            std::cout << "thread " << ++count << ": index = "
                << chunk.index
                << ", count = " << chunk.count
                << std::endl;

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

        if (this->settings_.fakeLines)
        {
            hough.lines = accumulator.FakeLines(this->settings_.fakeCount);
            hough.space = combined;
            return hough;
        }

        if (this->settings_.suppress)
        {
            hough.space = Suppression(
                this->settings_.threads,
                this->settings_.window,
                combined);

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
