#pragma once

#include <fields/fields.h>
#include <pex/interface.h>
#include <tau/eigen.h>
#include <tau/vector2d.h>

#include "iris/gradient.h"
#include "iris/gaussian.h"
#include "iris/harris_settings.h"
#include "iris/suppression.h"
#include "draw/pixels.h"
#include "iris/image.h"


namespace iris
{


template<typename Float>
class Harris
{
public:
    using Result = ImageMatrix<Float>;

    Harris() = default;

    Harris(const HarrisSettings<Float> &settings)
        :
        settings_(settings),
        gaussianKernel_(
            GaussianKernel<Float, Float, 0>(
                settings.sigma,
                static_cast<Float>(0.01),
                settings.threads).Normalize())
    {

    }

    template<typename Value>
    std::optional<Result> Filter(const GradientResult<Value> &gradient)
    {
        if (!this->settings_.enable)
        {
            return {};
        }

        Result dx = gradient.dx.template cast<Float>();
        Result dy = gradient.dy.template cast<Float>();

        Result dxSquared = dx.array().square();
        Result dySquared = dy.array().square();
        Result dxdy = dx.array() * dy.array();

        // Window the gradient data using the gaussian kernel.
        auto threadedDxSquared = ThreadedRowGaussian(
            this->gaussianKernel_,
            dxSquared,
            this->settings_.threads);

        auto threadedDySquared = ThreadedColumnGaussian(
            this->gaussianKernel_,
            dySquared,
            this->settings_.threads);

        auto threadedDxDy = std::async(
            std::launch::async,
            [this, &dxdy]()
            {
                return this->gaussianKernel_.Filter(dxdy);
            });

        Result windowedDxSquared = threadedDxSquared.Get();
        Result windowedDySquared = threadedDySquared.Get();
        Result windowedDxDy = threadedDxDy.get();

        Result response =
            windowedDxSquared.array() * windowedDySquared.array()
            - windowedDxDy.array().square()
            - this->settings_.alpha
                * (windowedDxSquared.array() + windowedDySquared.array())
                    .square();

        Result threshold = this->Threshold(response);

        if (this->settings_.suppress)
        {
            Result result = Suppression(
                this->settings_.threads,
                this->settings_.window,
                threshold);

            return result;
        }

        return threshold;
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
draw::Pixels ColorizeHarris(const Eigen::MatrixBase<Derived> &input)
{
    using Float = typename Derived::Scalar;

    static_assert(
        std::is_floating_point_v<Float>,
        "Expected the output of the harris operator to be floating-point");

    Derived response = input.derived();

    // Scale the maximum value to 1.
    Float maximum = response.maxCoeff();
    response.array() /= maximum;
    response = (response.array() > 0).select(1, response);

    tau::HsvPlanes<Float> hsv(response.rows(), response.cols());

    tau::GetSaturation(hsv).array() = Float(1.0);
    tau::GetHue(hsv).array() = Float(120.0);
    tau::GetValue(hsv) = response;

    auto asRgb = tau::HsvToRgb<uint8_t>(hsv);

    return {
        asRgb.template GetInterleaved<Eigen::RowMajor>(),
        {response.cols(), response.rows()}};
}


} // end namespace iris
