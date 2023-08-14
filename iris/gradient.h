#pragma once

#include <future>
#include <cmath>
#include <optional>
#include <pex/endpoint.h>
#include <tau/eigen.h>
#include <tau/planar.h>
#include <tau/color.h>
#include <tau/percentile.h>
#include <tau/color_maps/rgb.h>

#include "iris/error.h"
#include "iris/image.h"
#include "iris/derivative.h"
#include "iris/gaussian_node.h"
#include "iris/gradient_settings.h"
#include "iris/threadsafe_filter.h"
#include "iris/chunks.h"
#include "draw/pixels.h"
#include "iris/node.h"


namespace iris
{


template<typename Float>
struct Phasor
{
    using Matrix = Eigen::MatrixX<Float>;

    Matrix magnitude;
    Matrix phase;
};


template<typename Value>
struct GradientResult
{
    Value maximum;
    ImageMatrix<Value> dx;
    ImageMatrix<Value> dy;

    template<typename Float>
    static Eigen::MatrixX<Float> Magnitude(
        const Eigen::MatrixX<Float> &dx,
        const Eigen::MatrixX<Float> &dy)
    {
        static_assert(std::is_floating_point_v<Float>);

        Eigen::MatrixX<Float> result =
            (dx.array().square() + dy.array().square()).sqrt();

        // Clamp all magnitudes higher than one.
        // There shouldn't be, except for rounding errors.
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

    draw::Pixels Colorize() const
    {
        auto phasor = this->GetPhasor<float>();

        tau::HsvPlanes<float> hsv(
            phasor.magnitude.rows(),
            phasor.magnitude.cols());

        GetSaturation(hsv).array() = 1.0;

        GetHue(hsv) = phasor.phase;
        GetValue(hsv) = phasor.magnitude;

        auto asRgb = tau::HsvToRgb<uint8_t>(hsv);

        return {
            asRgb.template GetInterleaved<Eigen::RowMajor>(),
            {phasor.magnitude.cols(), phasor.magnitude.rows()}};
    }
};


template<typename Value>
class AsyncGradientResult
{
public:
    using Differentiate_ = Differentiate<Value>;
    using RowVector = typename Differentiate_::RowVector;
    using ColumnVector = typename Differentiate_::ColumnVector;
    using Result = GradientResult<Value>;
    using Matrix = ImageMatrix<Value>;

    AsyncGradientResult(
        const Differentiate<Value> &differentiate,
        const Matrix &data,
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
    chunk::RowConvolution<RowVector, Matrix> rowConvolution_;
    chunk::ColumnConvolution<ColumnVector, Matrix> columnConvolution_;
};


template<typename Value>
class Gradient
{
public:
    static constexpr size_t defaultThreads =
        GradientSettings<Value>::defaultThreads;

    using Matrix = ImageMatrix<Value>;
    using Result = GradientResult<Value>;

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
        differentiate_(settings.maximum, settings.scale, settings.size),
        threads_(settings.threads)
    {

    }

    AsyncGradientResult<Value> FilterAsync(
        const Matrix &data) const
    {
        return AsyncGradientResult<Value>(
            this->differentiate_,
            data,
            this->threads_);
    }

    std::optional<Result> Filter(const Matrix &data) const
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


int32_t DetectGradientScale(
    const GradientResult<int32_t> &result,
    double percentile);


template<typename SourceNode>
class GradientNode
    :
    public Node<SourceNode, Gradient<int32_t>, GradientControl<int32_t>>
{
public:
    using Control = GradientControl<int32_t>;
    using Filter = Gradient<int32_t>;
    using Base = Node<SourceNode, Filter, Control>;

    GradientNode(
        SourceNode &source,
        Control control,
        CancelControl cancel)
        :
        Base("Gradient", source, control, cancel),
        control_(control),
        detectEndpoint_(
            this,
            control.autoDetectSettings,
            &GradientNode::AutoDetectSettings)
    {

    }

    void AutoDetectSettings()
    {
        auto settings = this->control_.Get();
        settings.scale = 1;

        // Update the filter with our temporary adjustment to the scale.
        this->OnSettingsChanged(settings);
        auto filtered = this->GetResult();

        if (!filtered)
        {
            std::cerr << "Unable to detect gradient without input."
                << std::endl;

            this->OnSettingsChanged(this->control_.Get());

            return;
        }

        auto detected = DetectGradientScale(
            *filtered,
            this->control_.percentile.Get());

        this->control_.scale.Set(detected);
    }

    Control control_;

    using DetectEndpoint =
        pex::Endpoint<GradientNode, pex::control::Signal<>>;

    DetectEndpoint detectEndpoint_;
};


extern template struct GradientResult<int32_t>;
extern template class Gradient<int32_t>;
extern template class GradientNode<DefaultGaussianNode>;

using DefaultGradientNode = GradientNode<DefaultGaussianNode>;


} // end namespace iris
