#pragma once

#include <fields/fields.h>
#include <pex/interface.h>
#include <tau/eigen.h>
#include <tau/vector2d.h>
#include <draw/pixels.h>
#include <tau/mono_image.h>

#include <iris/filter_result.h>
#include "iris/gradient.h"
#include "iris/gaussian.h"
#include "iris/harris_settings.h"
#include "iris/suppression.h"


namespace iris
{


template<typename Float>
struct HarrisResult: public FilterResult
{
    using Data = tau::MonoImage<Float>;
    Data data;

    tau::Size<Eigen::Index> GetSize() const
    {
        return {
            this->data.cols(),
            this->data.rows()};
    }

    void Resize(const tau::Size<Eigen::Index> &size)
    {
        this->data = Data::Zero(size.height, size.width);
    }

    std::shared_ptr<draw::Pixels> Colorize() const
    {
        static_assert(
            std::is_floating_point_v<Float>,
            "Expected the output of the harris operator to be floating-point");

        Data response = this->data;

        // Scale the maximum value to 1
        // and the minimum value to 0.4.
        Float maximum = response.maxCoeff();
        response.array() *= (0.6 / maximum);
        response.array() += 0.4;
        tau::Select(response) <= 0.4 = 0.0;

        auto validSize = this->margins_.GetValidSize(response);

        tau::HsvPlanes<Float> hsv(validSize.rows, validSize.columns);

        tau::GetSaturation(hsv).array() = Float(1.0);
        tau::GetHue(hsv).array() = Float(120.0);
        tau::GetValue(hsv) = this->margins_.RemoveMargin(response);

        auto asRgb = tau::HsvToRgb<uint8_t>(hsv);

        return draw::Pixels::CreateShared(asRgb);
    }

    HarrisResult AddMargin(const tau::Margins &margins) const
    {
        HarrisResult result{};
        result.data = margins.AddMargin(this->data);
        result.SetMargins(margins);

        return result;
    }

    HarrisResult RemoveMargin() const
    {
        HarrisResult result{};
        result.data = this->margins_.RemoveMargin(this->data);

        return result;
    }
};


template<typename Float>
class Harris
{
public:
    static constexpr bool wantsMargins = true;

    using Result = HarrisResult<Float>;
    using Data = typename Result::Data;

    Harris() = default;

    Harris(const HarrisSettings<Float> &settings)
        :
        settings_(settings),
        gaussianKernel_(
            GaussianKernel<Float, Float, 0>(
                settings.sigma,
                static_cast<Float>(0.01),
                Partials::both).Normalize())
    {

    }

    template<typename Value>
    bool Filter(
        const GradientResult<Value> &gradient,
        Result &result,
        const tau::Margins &margins)
    {
        if (!this->settings_.enable)
        {
            return false;
        }

        Data dx = gradient.dx.template cast<Float>();
        Data dy = gradient.dy.template cast<Float>();

        Data dxSquared = dx.array().square();
        Data dxSquaredResult(dxSquared.rows(), dxSquared.cols());
        Data dySquared = dy.array().square();
        Data dySquaredResult(dySquared.rows(), dySquared.cols());
        Data dxdy = dx.array() * dy.array();
        Data dxdyResult(dxdy.rows(), dxdy.cols());

        // Window the gradient data using the gaussian kernel.
        ThreadedKernelConvolve(
            this->gaussianKernel_,
            dxSquared,
            dxSquaredResult,
            Partials::rows);

        ThreadedKernelConvolve(
            this->gaussianKernel_,
            dySquared,
            dySquaredResult,
            Partials::columns);

        ThreadedKernelConvolve(
            this->gaussianKernel_,
            dxdy,
            dxdyResult,
            Partials::both);

        Data response =
            dxSquaredResult.array() * dySquaredResult.array()
            - dxdyResult.array().square()
            - this->settings_.alpha
                * (dxSquaredResult.array() + dySquaredResult.array())
                    .square();

        this->Threshold(response);

        margins.ZeroMargin(response);

        if (this->settings_.suppress)
        {
            Suppression<Float>(
                this->settings_.window,
                response,
                result.data);

            return true;
        }

        result.data = response;

        return true;
    }

    tau::Margins ComputeRequiredMargins() const
    {
        return tau::Margins::Create(this->gaussianKernel_.size / 2);
    }

    void Threshold(Data &response)
    {
        Float thresholdValue = this->settings_.threshold * response.maxCoeff();
        response = (response.array() < thresholdValue).select(0, response);
    }

private:
    HarrisSettings<Float> settings_;
    GaussianKernel<Float, Float, 0> gaussianKernel_;
};


template<typename Float>
using ThreadsafeHarris =
    ThreadsafeFilter<HarrisGroup<Float>, Harris<Float>>;


} // end namespace iris
