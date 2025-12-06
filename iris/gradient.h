#pragma once

#include <future>
#include <cmath>
#include <pex/endpoint.h>
#include <tau/eigen.h>
#include <tau/planar.h>
#include <tau/color.h>
#include <tau/vector2d.h>
#include <tau/percentile.h>
#include <tau/color_maps/rgb.h>
#include <tau/mono_image.h>
#include <draw/pixels.h>

#include "iris/error.h"
#include "iris/derivative.h"
#include "iris/gaussian_node.h"
#include "iris/gradient_settings.h"
#include "iris/threadsafe_filter.h"
#include "iris/chunks.h"
#include "iris/node.h"


namespace iris
{


template<typename Float>
struct Phasor
{
    using Matrix = Eigen::MatrixX<Float>;

    Matrix magnitude;
    Matrix phase;

    Float & GetMagnitude(const tau::Point2d<Eigen::Index> &point)
    {
        return this->magnitude(point.y, point.x);
    }

    Float & GetPhase(const tau::Point2d<Eigen::Index> &point)
    {
        return this->phase(point.y, point.x);
    }
};


template<typename Value>
struct GradientResult
{
    Value maximum;
    tau::MonoImage<Value> dx;
    tau::MonoImage<Value> dy;

    GradientResult()
        :
        maximum{},
        dx{},
        dy{}
    {

    }

    GradientResult(Value maximum_, Eigen::Index rows, Eigen::Index cols)
        :
        maximum(maximum_),
        dx(rows, cols),
        dy(rows, cols)
    {

    }

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

    std::shared_ptr<draw::Pixels> Colorize(const tau::Margins &margins) const
    {
        auto trimmed = this->RemoveMargin(margins);

        auto phasor = trimmed.GetPhasor<float>();

        tau::HsvPlanes<float> hsv(
            phasor.magnitude.rows(),
            phasor.magnitude.cols());

        GetSaturation(hsv).array() = 1.0;

        GetHue(hsv) = phasor.phase;
        GetValue(hsv) = phasor.magnitude;

        auto asRgb = tau::HsvToRgb<uint8_t>(hsv);

        return draw::Pixels::CreateShared(asRgb);
    }

    GradientResult RemoveMargin(const tau::Margins &margins) const
    {
        GradientResult trimmed{};
        trimmed.maximum = this->maximum;
        trimmed.dx = margins.RemoveMargin(this->dx);
        trimmed.dy = margins.RemoveMargin(this->dy);

        return trimmed;
    }
};


template<typename Value>
class AsyncGradient
{
public:
    using Differentiate_ = Differentiate<Value>;
    using RowVector = typename Differentiate_::RowVector;
    using ColumnVector = typename Differentiate_::ColumnVector;
    using Result = GradientResult<Value>;
    using Matrix = tau::MonoImage<Value>;

    AsyncGradient(
        const Differentiate_ &differentiate,
        const Matrix &input,
        Result &result,
        size_t threadCount)
        :
        rowConvolution_(
            differentiate.horizontal,
            input,
            result.dx,
            threadCount),

        columnConvolution_(
            differentiate.vertical,
            input,
            result.dy,
            threadCount)
    {

    }

    void Await()
    {
        this->rowConvolution_.Await();
        this->columnConvolution_.Await();
    }

private:
    Value maximum_;

    // The gradient kernels sum to zero.
    // Set normalize to false.
    static constexpr bool normalize = false;
    using RowConvolution =
        chunk::RowConvolution<normalize, RowVector, Matrix, Matrix>;

    RowConvolution rowConvolution_;

    using ColumnConvolution =
        chunk::ColumnConvolution<normalize, ColumnVector, Matrix, Matrix>;

    ColumnConvolution columnConvolution_;
};


template<typename Value>
class Gradient
{
public:
    static constexpr size_t defaultThreads =
        GradientSettings<Value>::defaultThreads;

    using Matrix = tau::MonoImage<Value>;
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

    AsyncGradient<Value> FilterAsync(
        const Matrix &input,
        Result &result) const
    {
        return AsyncGradient<Value>(
            this->differentiate_,
            input,
            result,
            this->threads_);
    }

    bool Filter(const Matrix &input, Result &result) const
    {
        if (!this->isEnabled_)
        {
            return false;
        }

        result.maximum = this->differentiate_.GetMaximum();
        result.dx.resize(input.rows(), input.cols());
        result.dy.resize(input.rows(), input.cols());

        auto asyncGradient = this->FilterAsync(input, result);
        asyncGradient.Await();

        return true;
    }

    Eigen::Index GetSize() const
    {
        return this->differentiate_.GetSize();
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
            PEX_THIS("GradientNode"),
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
        pex::Endpoint<GradientNode, pex::control::DefaultSignal>;

    DetectEndpoint detectEndpoint_;
};


extern template struct GradientResult<int32_t>;
extern template class Gradient<int32_t>;
extern template class GradientNode<DefaultGaussianNode>;

using DefaultGradientNode = GradientNode<DefaultGaussianNode>;


} // end namespace iris
