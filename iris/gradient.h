#pragma once

#include <future>
#include <cmath>
#include <optional>
#include <tau/eigen.h>
#include <tau/planar.h>
#include <tau/color.h>
#include <tau/color_maps/rgb.h>

#include "iris/error.h"
#include "iris/derivative.h"
#include "iris/gradient_settings.h"
#include "iris/threadsafe_filter.h"
#include "iris/chunks.h"


namespace iris
{


template<typename Float>
struct Phasor
{
    using Matrix = Eigen::MatrixX<Float>;

    Matrix magnitude;
    Matrix phase;
};


template<typename Value, typename Data>
struct GradientResult
{
    Value maximum;
    Data dx;
    Data dy;

    template<typename Float>
    static Eigen::MatrixX<Float> Magnitude(
        const Eigen::MatrixX<Float> &dx,
        const Eigen::MatrixX<Float> &dy)
    {
        static_assert(std::is_floating_point_v<Float>);

        Eigen::MatrixX<Float> result =
            (dx.array().square() + dy.array().square()).sqrt();

        // Clamp all magnitudes higher than one.
        return (result.array() > 1).select(1, result);
    }

    template<typename Float>
    static Eigen::MatrixX<Float> Phase_deg(
        const Eigen::MatrixX<Float> &dx,
        const Eigen::MatrixX<Float> &dy)
    {
        static_assert(std::is_floating_point_v<Float>);

        Eigen::MatrixX<Float> asRadians = dy.array().binaryExpr(
            dx.array(),
            [](auto y, auto x) { return std::atan2(y, x); });

        return tau::ToDegrees(asRadians);
    }

    template<typename Float>
    Phasor<Float> GetPhasor() const
    {
        using Result = Phasor<Float>;
        using Matrix = typename Result::Matrix;

        Matrix dxFloat = this->dx.template cast<Float>();
        Matrix dyFloat = this->dy.template cast<Float>();

        // Scale the derivatives to -1 to 1.
        dxFloat.array() /= static_cast<Float>(this->maximum);
        dyFloat.array() /= static_cast<Float>(this->maximum);

        Matrix phase = Phase_deg(dxFloat, dyFloat);
        phase.array() += 360;
        phase = tau::Modulo(phase, 360);

        return {Magnitude(dxFloat, dyFloat), phase};
    }

    template<typename Pixel>
    tau::RgbPixels<Pixel> Colorize() const
    {
        auto phasor = this->GetPhasor<float>();

        tau::HsvPlanes<float> hsv(
            phasor.magnitude.rows(),
            phasor.magnitude.cols());

        GetSaturation(hsv).array() = 1.0;

        GetHue(hsv) = phasor.phase;
        GetValue(hsv) = phasor.magnitude;

        auto asRgb = tau::HsvToRgb<Pixel>(hsv);

        return {
            asRgb.template GetInterleaved<Eigen::RowMajor>(),
            {phasor.magnitude.cols(), phasor.magnitude.rows()}};
    }
};


template<typename Value, typename Data>
class AsyncGradientResult
{
public:
    using Differentiate_ = Differentiate<Value>;
    using RowVector = typename Differentiate_::RowVector;
    using ColumnVector = typename Differentiate_::ColumnVector;
    using Result = GradientResult<Value, Data>;

    AsyncGradientResult(
        const Differentiate<Value> &differentiate,
        const Eigen::MatrixBase<Data> &data,
        size_t threadCount)
        :
        maximum_(differentiate.GetMaximum()),
        rowConvolution_(differentiate.horizontal, data, threadCount),
        columnConvolution_(differentiate.vertical, data, threadCount)
    {

    }

    Result Get()
    {
        return
        {
            this->maximum_,
            this->rowConvolution_.Get(),
            this->columnConvolution_.Get(),
        };
    }

private:
    Value maximum_;
    chunk::RowConvolution<RowVector, Data> rowConvolution_;
    chunk::ColumnConvolution<ColumnVector, Data> columnConvolution_;
};


template<typename Data>
class Gradient
{
public:
    using Value = typename Data::Scalar;

    static constexpr size_t defaultThreads =
        GradientSettings<Value>::defaultThreads;

    using Result = GradientResult<Value, Data>;

    Gradient() = default;

    Gradient(const Differentiate<Value> &differentiate)
        :
        isEnabled_(true),
        differentiate_(differentiate),
        threads_(defaultThreads)
    {

    }

    Gradient(const GradientSettings<Value> &settings)
        :
        isEnabled_(settings.enable),
        differentiate_(settings.maximumInput, settings.scale, settings.size),
        threads_(settings.threads)
    {

    }

    AsyncGradientResult<Value, Data> FilterAsync(
        const Eigen::MatrixBase<Data> &data) const
    {
        static_assert(
            std::is_same_v<Value, typename Data::Scalar>,
            "Expected Scalar type to match configuration type.");

        return AsyncGradientResult<Value, Data>(
            this->differentiate_,
            data,
            this->threads_);
    }

    std::optional<Result> Filter(
        const Eigen::MatrixBase<Data> &data) const
    {
        if (!this->isEnabled_)
        {
            return {};
        }

        auto asyncResult = this->FilterAsync(data);
        return asyncResult.Get();
    }

private:
    bool isEnabled_;
    Differentiate<Value> differentiate_;
    size_t threads_;
};


template<typename Data>
using ThreadsafeGradient =
    ThreadsafeFilter<GradientGroup<typename Data::Scalar>, Gradient<Data>>;


} // end namespace iris
