#pragma once

#include <fields/fields.h>
#include <pex/group.h>
#include <pex/select.h>
#include <pex/range.h>
#include <pex/endpoint.h>
#include <draw/size.h>
#include "iris/default.h"
#include "iris/derivative.h"


namespace iris
{


template<typename T>
struct HoughFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::enable, "enable"),
        fields::Field(&T::imageSize, "imageSize"),
        fields::Field(&T::rhoCount, "rhoCount"),
        fields::Field(&T::thetaCount, "thetaCount"),
        fields::Field(&T::angleRange, "angleRange"),
        fields::Field(&T::weighted, "weighted"),
        fields::Field(&T::suppress, "suppress"),
        fields::Field(&T::window, "window"),
        fields::Field(&T::threshold, "threshold"),
        fields::Field(&T::includeEdges, "includeEdges"),
        fields::Field(&T::edgeTolerance, "edgeTolerance"),
        fields::Field(&T::threads, "threads"));

    static constexpr auto fieldsTypeName = "Hough";
};


using AngleRangeLow = pex::Limit<0>;
using AngleRangeHigh = pex::Limit<180>;

using WindowRange =
    pex::MakeRange<Eigen::Index, pex::Limit<3>, pex::Limit<128>>;

template<typename T>
using ThresholdRange = pex::MakeRange<T, pex::Limit<0>, pex::Limit<256>>;

using FakeRange = pex::MakeRange<size_t, pex::Limit<2>, pex::Limit<32>>;

template<typename Float>
struct HoughTemplate
{
    template<template<typename> typename T>
    struct Template
    {
        T<bool> enable;
        T<draw::SizeGroup> imageSize;
        T<size_t> rhoCount;
        T<size_t> thetaCount;
        T<pex::MakeRange<Float, AngleRangeLow, AngleRangeHigh>> angleRange;
        T<bool> weighted;
        T<bool> suppress;
        T<WindowRange> window;
        T<ThresholdRange<Float>> threshold;
        T<bool> includeEdges;
        T<Float> edgeTolerance;
        T<size_t> threads;

        static constexpr auto fields = HoughFields<Template>::fields;
    };
};


template<typename Float>
struct HoughCustom
{
    template<typename Base>
    struct Plain: public Base
    {
        // About half-pixel resolution in 1920x1080 image.
        static constexpr size_t defaultRhoCount = 2200;

        // 10ths of a degree
        static constexpr size_t defaultThetaCount = 1024;

        // How far to search beyond the detected edge angle.
        static constexpr Float defaultAngleRange = 15;

        // The size of the non-maximum suppression window.
        static constexpr Eigen::Index defaultWindow = 24;

        // Minimum value to be considered a line.
        static constexpr size_t defaultThreshold = 110;

        static constexpr size_t defaultEdgeTolerance = 4;

        static constexpr size_t defaultThreads = 4;

        Plain()
            :
            Base{
                true,
                defaultImageSize,
                defaultRhoCount,
                defaultThetaCount,
                defaultAngleRange,
                true,
                true,
                defaultWindow,
                defaultThreshold,
                false,
                defaultEdgeTolerance,
                defaultThreads}
        {

        }
    };

    template<typename Base>
    struct Model: public Base
    {
        using This = Model<Base>;

        using WindowEndpoint =
            typename pex::Endpoint<This, decltype(This::window)>;

        using RhoCountEndpoint =
            typename pex::Endpoint<This, decltype(This::rhoCount)>;

        using ThetaCountEndpoint =
            typename pex::Endpoint<This, decltype(This::thetaCount)>;

    public:
        Model()
            :
            Base(),
            windowEndpoint_(
                this,
                this->window,
                &Model::OnWindow_),
            rhoCountEndpoint_(
                this,
                this->rhoCount,
                &Model::OnRhoCount_),
            thetaCountEndpoint_(
                this,
                this->thetaCount,
                &Model::OnThetaCount_)
        {

        }

    private:
        using Index = typename Eigen::Index;

        void OnWindow_(Index windowSize)
        {
            auto maximumWindowSize = Index(
                std::min(this->thetaCount.Get(), this->rhoCount.Get()));

            if (windowSize > maximumWindowSize)
            {
                this->window.Set(maximumWindowSize);
            }
        }

        void OnRhoCount_(size_t rhoCount_)
        {
            auto maximumWindowSize =
                Index(std::min(this->thetaCount.Get(), rhoCount_));

            if (this->window.Get() > maximumWindowSize)
            {
                this->window.Set(maximumWindowSize);
            }
        }

        void OnThetaCount_(size_t thetaCount_)
        {
            auto maximumWindowSize =
                Index(std::min(thetaCount_, this->rhoCount.Get()));

            if (this->window.Get() > maximumWindowSize)
            {
                this->window.Set(maximumWindowSize);
            }
        }

        WindowEndpoint windowEndpoint_;
        RhoCountEndpoint rhoCountEndpoint_;
        ThetaCountEndpoint thetaCountEndpoint_;
    };
};


template<typename Float>
using HoughGroup =
    pex::Group
    <
        HoughFields,
        HoughTemplate<Float>::template Template,
        HoughCustom<Float>
    >;


template<typename Float>
using HoughModel = typename HoughGroup<Float>::Model;

template<typename Float>
using HoughControl = typename HoughGroup<Float>::Control;

template<typename Float>
using HoughSettings = typename HoughGroup<Float>::Plain;


DECLARE_EQUALITY_OPERATORS(HoughSettings<float>)
DECLARE_OUTPUT_STREAM_OPERATOR(HoughSettings<float>)

DECLARE_EQUALITY_OPERATORS(HoughSettings<double>)
DECLARE_OUTPUT_STREAM_OPERATOR(HoughSettings<double>)


} // end namespace iris


extern template struct pex::Group
    <
        iris::HoughFields,
        iris::HoughTemplate<float>::template Template,
        iris::HoughCustom<float>
    >;


extern template struct pex::Group
    <
        iris::HoughFields,
        iris::HoughTemplate<double>::template Template,
        iris::HoughCustom<double>
    >;
