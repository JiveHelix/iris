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

#include <iris/filter_result.h>
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

    Phasor AddMargin(const tau::Margins &margins) const
    {
        Phasor extended{};
        extended.magnitude = margins.AddMargin(this->magnitude);
        extended.phase = margins.AddMargin(this->phase);

        return extended;
    }

    Phasor RemoveMargin(const tau::Margins &margins) const
    {
        Phasor trimmed{};
        trimmed.magnitude = margins.RemoveMargin(this->magnitude);
        trimmed.phase = margins.RemoveMargin(this->phase);

        return trimmed;
    }

    tau::Size<Eigen::Index> GetSize() const
    {
        assert(
            tau::Size<Eigen::Index>(this->magnitude)
                == tau::Size<Eigen::Index>(this->phase));

        return {this->magnitude};
    }

    void Resize(const tau::Size<Eigen::Index> &size)
    {
        this->magnitude = Matrix(size.height, size.width);
        this->phase = Matrix(size.height, size.width);
    }
};


template<typename Value>
struct GradientResult: public FilterResult
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

    std::shared_ptr<draw::Pixels> Colorize() const
    {
        auto trimmed = this->RemoveMargin();
        auto phasor = trimmed.template GetPhasor<float>();

        tau::HsvPlanes<float> hsv(
            phasor.magnitude.rows(),
            phasor.magnitude.cols());

        GetSaturation(hsv).array() = 1.0;

        GetHue(hsv) = phasor.phase;
        GetValue(hsv) = phasor.magnitude;

        auto asRgb = tau::HsvToRgb<uint8_t>(hsv);

        return draw::Pixels::CreateShared(asRgb);
    }

    GradientResult AddMargin(const tau::Margins &margins) const
    {
        GradientResult extended{};
        extended.maximum = this->maximum;
        extended.SetMargins(margins);
        extended.dx = margins.AddMargin(this->dx);
        extended.dy = margins.AddMargin(this->dy);

        return extended;
    }

    GradientResult RemoveMargin() const
    {
        GradientResult trimmed{};
        trimmed.maximum = this->maximum;
        trimmed.dx = this->margins_.RemoveMargin(this->dx);
        trimmed.dy = this->margins_.RemoveMargin(this->dy);

        return trimmed;
    }

    tau::Size<Eigen::Index> GetSize() const
    {
        assert(this->dx.rows() == this->dy.rows());
        assert(this->dx.cols() == this->dy.cols());

        return {this->dx};
    }

    void Resize(const tau::Size<Eigen::Index> size)
    {
        this->dx.resize(size.height, size.width);
        this->dy.resize(size.height, size.width);
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
    using InputRef = Eigen::Ref<const Matrix>;

    AsyncGradient(
        const Differentiate_ &differentiate,
        const InputRef &input,
        Result &result)
        :
        rowConvolution_(
            differentiate.horizontal,
            input,
            result.dx,
            jive::GetThreadPool()->GetConcurrency()),

        columnConvolution_(
            differentiate.vertical,
            input,
            result.dy,
            jive::GetThreadPool()->GetConcurrency())
    {

    }

    void Wait()
    {
        this->rowConvolution_.Wait();
        this->columnConvolution_.Wait();
    }

private:
    Value maximum_;

    // The gradient kernels sum to zero.
    // Set normalize to false.
    static constexpr bool normalize = false;
    using RowConvolution =
        chunk::RowConvolution<normalize, RowVector, InputRef, Matrix>;

    RowConvolution rowConvolution_;

    using ColumnConvolution =
        chunk::ColumnConvolution<normalize, ColumnVector, InputRef, Matrix>;

    ColumnConvolution columnConvolution_;
};


template<typename Value>
class Gradient
{
public:
    using Matrix = tau::MonoImage<Value>;
    using Result = GradientResult<Value>;

    Gradient() = default;

    Gradient(const Differentiate<Value> &differentiate)
        :
        isEnabled_(true),
        differentiate_(differentiate)
    {

    }

    Gradient(const GradientSettings<Value> &settings)
        :
        isEnabled_(settings.enable),
        differentiate_(settings.maximum, settings.scale, settings.size)
    {

    }

    AsyncGradient<Value> FilterAsync(
        Eigen::Ref<const Matrix> input,
        Result &result) const
    {
        return AsyncGradient<Value>(
            this->differentiate_,
            input,
            result);
    }

    bool Filter(Eigen::Ref<const Matrix> input, Result &result) const
    {
        if (!this->isEnabled_)
        {
            return false;
        }

        result.maximum = this->differentiate_.GetMaximum();
        result.dx.resize(input.rows(), input.cols());
        result.dy.resize(input.rows(), input.cols());

        auto asyncGradient = this->FilterAsync(input, result);
        asyncGradient.Wait();

        return true;
    }

    Eigen::Index GetSize() const
    {
        return this->differentiate_.GetSize();
    }

    tau::Margins ComputeRequiredMargins() const
    {
        return tau::Margins::Create(this->differentiate_.GetSize() / 2);
    }

private:
    bool isEnabled_;
    Differentiate<Value> differentiate_;
};


InProcess DetectGradientScale(
    const GradientResult<InProcess> &result,
    double percentile);


template<typename SourceNode>
class GradientNode
    :
    public Node<SourceNode, Gradient<InProcess>, GradientControl<InProcess>>
{
public:
    using Control = GradientControl<InProcess>;
    using Filter = Gradient<InProcess>;
    using Base = Node<SourceNode, Filter, Control>;

    GradientNode(
        SourceNode &source,
        const Control &control,
        const CancelControl &cancel)
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


extern template struct GradientResult<InProcess>;
extern template class Gradient<InProcess>;
extern template class GradientNode<DefaultGaussianNode>;

using DefaultGradientNode = GradientNode<DefaultGaussianNode>;


} // end namespace iris
