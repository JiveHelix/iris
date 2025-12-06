#pragma once

#include <fields/fields.h>
#include <pex/interface.h>
#include <tau/eigen.h>
#include <tau/vector2d.h>
#include <draw/pixels.h>
#include <tau/mono_image.h>

#include "iris/gradient.h"
#include "iris/gaussian.h"
#include "iris/harris_settings.h"
#include "iris/suppression.h"


namespace iris
{


template<typename Float>
class Harris
{
public:
    using Result = tau::MonoImage<Float>;

    Harris() = default;

    Harris(const HarrisSettings<Float> &settings)
        :
        settings_(settings),
        gaussianKernel_(
            GaussianKernel<Float, Float, 0>(
                settings.sigma,
                static_cast<Float>(0.01),
                Partials::both,
                settings.threads).Normalize())
    {

    }

    template<typename Value>
    bool Filter(const GradientResult<Value> &gradient, Result &result)
    {
        if (!this->settings_.enable)
        {
            return false;
        }

        Result dx = gradient.dx.template cast<Float>();
        Result dy = gradient.dy.template cast<Float>();

        Result dxSquared = dx.array().square();
        Result dxSquaredResult(dxSquared.rows(), dxSquared.cols());
        Result dySquared = dy.array().square();
        Result dySquaredResult(dySquared.rows(), dySquared.cols());
        Result dxdy = dx.array() * dy.array();
        Result dxdyResult(dxdy.rows(), dxdy.cols());

        // Window the gradient data using the gaussian kernel.
        auto threadedDxSquared = ThreadedRowGaussian(
            this->gaussianKernel_,
            dxSquared,
            dxSquaredResult,
            this->settings_.threads);

        auto threadedDySquared = ThreadedColumnGaussian(
            this->gaussianKernel_,
            dySquared,
            dySquaredResult,
            this->settings_.threads);

        auto threadedDxDy = jive::GetThreadPool()->AddJob(
            [this, &dxdy, &dxdyResult]()
            {
                this->gaussianKernel_.Filter(dxdy, dxdyResult);
            });

        threadedDxSquared.Await();
        threadedDySquared.Await();
        threadedDxDy.Wait();

        Result response =
            dxSquaredResult.array() * dySquaredResult.array()
            - dxdyResult.array().square()
            - this->settings_.alpha
                * (dxSquaredResult.array() + dySquaredResult.array())
                    .square();

        if (this->settings_.suppress)
        {
            Suppression(
                this->settings_.threads,
                this->settings_.window,
                this->Threshold(response),
                result);

            return true;
        }

        result = this->Threshold(response);

        return true;
    }

    Result Threshold(const Result &response)
    {
        Float thresholdValue = this->settings_.threshold * response.maxCoeff();
        return (response.array() < thresholdValue).select(0, response);
    }

private:
    HarrisSettings<Float> settings_;
    GaussianKernel<Float, Float, 0> gaussianKernel_;
};


template<typename Float>
using ThreadsafeHarris =
    ThreadsafeFilter<HarrisGroup<Float>, Harris<Float>>;


template<typename Derived>
std::shared_ptr<draw::Pixels> ColorizeHarris(
    const tau::Margins &margins,
    const Eigen::MatrixBase<Derived> &input)
{
    using Float = typename Derived::Scalar;

    static_assert(
        std::is_floating_point_v<Float>,
        "Expected the output of the harris operator to be floating-point");

    Derived response = input.derived();

    // Scale the maximum value to 1
    // and the minimum value to 0.4.
    Float maximum = response.maxCoeff();
    response.array() *= (0.6 / maximum);
    response.array() += 0.4;
    tau::Select(response) <= 0.4 = 0.0;

    auto validSize = margins.GetValidSize(response);

    tau::HsvPlanes<Float> hsv(validSize.rows, validSize.columns);

    tau::GetSaturation(hsv).array() = Float(1.0);
    tau::GetHue(hsv).array() = Float(120.0);
    tau::GetValue(hsv) = margins.RemoveMargin(response);

    auto asRgb = tau::HsvToRgb<uint8_t>(hsv);

    return draw::Pixels::CreateShared(asRgb);
}


} // end namespace iris
