#pragma once

#include <fields/fields.h>
#include <pex/interface.h>
#include <tau/eigen.h>
#include <tau/vector2d.h>

#include "iris/gradient.h"
#include "iris/gaussian.h"
#include "iris/harris_settings.h"
#include "iris/suppression.h"
#include "iris/pixels.h"


namespace iris
{


template<typename Float>
class Harris
{
public:
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

    template<typename Value, typename Data>
    std::optional<tau::MatrixLike<Float, Data>> Filter(
        const GradientResult<Value, Data> &gradient)
    {
        if (!this->settings_.enable)
        {
            return {};
        }

        using FloatData = tau::MatrixLike<Float, Data>;
        FloatData dx = gradient.dx.template cast<Float>();
        FloatData dy = gradient.dy.template cast<Float>();

        FloatData dxSquared = dx.array().square();
        FloatData dySquared = dy.array().square();
        FloatData dxdy = dx.array() * dy.array();

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

        FloatData windowedDxSquared = threadedDxSquared.Get();
        FloatData windowedDySquared = threadedDySquared.Get();
        FloatData windowedDxDy = threadedDxDy.get();

        FloatData response =
            windowedDxSquared.array() * windowedDySquared.array()
            - windowedDxDy.array().square()
            - this->settings_.alpha
                * (windowedDxSquared.array() + windowedDySquared.array())
                    .square();

        FloatData threshold = this->Threshold(response);

        if (this->settings_.suppress)
        {
            return Suppression(
                this->settings_.threads,
                this->settings_.window,
                threshold);
        }

        return threshold;
    }

    template<typename Data>
    Data Threshold(Data response)
    {
        Float thresholdValue = this->settings_.threshold * response.maxCoeff();
        return (response.array() < thresholdValue).select(0, response);
    }

private:
    HarrisSettings<Float> settings_;
    GaussianKernel<Float, Float, 0> gaussianKernel_;
};


template<typename Float>
using ThreadsafeHarris = ThreadsafeFilter<HarrisGroup<Float>, Harris<Float>>;


template<typename Derived>
Pixels ColorizeHarris(const Eigen::MatrixBase<Derived> &input)
{
    static_assert(
        std::is_floating_point_v<typename Derived::Scalar>,
        "Expected the output of the harris operator to be floating-point");

    Derived response = input.derived();

    // Scale the maximum value to 1.
    float maximum = response.maxCoeff();
    response.array() /= maximum;
    response = (response.array() > 0).select(1, response);

    tau::HsvPlanes<float> hsv(response.rows(), response.cols());

    tau::GetSaturation(hsv).array() = 1.0f;
    tau::GetHue(hsv).array() = 120.0f;
    tau::GetValue(hsv) = response;

    auto asRgb = tau::HsvToRgb<uint8_t>(hsv);

    return {
        asRgb.template GetInterleaved<Eigen::RowMajor>(),
        response.rows(),
        response.cols()};
}


} // end namespace iris
