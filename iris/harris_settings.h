#pragma once


#include <fields/fields.h>
#include <pex/group.h>


namespace iris
{


template<typename T>
struct HarrisFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::enable, "enable"),
        fields::Field(&T::alpha, "alpha"),
        fields::Field(&T::sigma, "sigma"),
        fields::Field(&T::threshold, "threshold"),
        fields::Field(&T::suppress, "suppress"),
        fields::Field(&T::window, "window"),
        fields::Field(&T::threads, "threads"));
};


struct HarrisRanges
{
    using AlphaLow = pex::Limit<0>;
    using AlphaHigh = pex::Limit<0, 25, 100>;

    using SigmaLow = pex::Limit<0, 25, 100>;
    using SigmaHigh = pex::Limit<4>;

    using ThresholdLow = pex::Limit<0>;
    using ThresholdHigh = pex::Limit<0, 25, 100>;

    using WindowLow = pex::Limit<2>;
    using WindowHigh = pex::Limit<10>;
};


template<typename Float, typename Ranges = HarrisRanges>
struct HarrisTemplate
{
    static_assert(std::is_floating_point_v<Float>);

    template<template<typename> typename T>
    struct Template
    {
        T<bool> enable;

        T
        <
            pex::MakeRange
            <
                Float,
                typename Ranges::AlphaLow,
                typename Ranges::AlphaHigh
            >
        > alpha;

        T
        <
            pex::MakeRange
            <
                Float,
                typename Ranges::SigmaLow,
                typename Ranges::SigmaHigh
            >
        > sigma;

        T
        <
            pex::MakeRange
            <
                Float,
                typename Ranges::ThresholdLow,
                typename Ranges::ThresholdHigh
            >
        > threshold;

        T<bool> suppress;

        T
        <
            pex::MakeRange
            <
                Eigen::Index,
                typename Ranges::WindowLow,
                typename Ranges::WindowHigh
            >
        > window;

        T<size_t> threads;

        static constexpr auto fields = HarrisFields<Template>::fields;
        static constexpr auto fieldsTypeName = "Harris";
    };
};


template<typename Float>
struct HarrisSettings
    :
    public HarrisTemplate<Float>::template Template<pex::Identity>
{
    static HarrisSettings Default()
    {
        static constexpr Float defaultAlpha = static_cast<Float>(0.14);
        static constexpr Float defaultSigma = static_cast<Float>(1.8);
        static constexpr Float defaultThreshold = static_cast<Float>(0.04);
        static constexpr Eigen::Index defaultWindow = 3;
        static constexpr size_t defaultThreads = 4;

        return {{
            true,
            defaultAlpha,
            defaultSigma,
            defaultThreshold,
            true,
            defaultWindow,
            defaultThreads}};
    }
};


DECLARE_COMPARISON_OPERATORS(HarrisSettings<float>)
DECLARE_COMPARISON_OPERATORS(HarrisSettings<double>)


template<typename Float, typename Ranges = HarrisRanges>
using HarrisGroup = pex::Group
    <
        HarrisFields,
        HarrisTemplate<Float, Ranges>::template Template,
        HarrisSettings<Float>
    >;

template<typename Float, typename Ranges = HarrisRanges>
using HarrisModel = typename HarrisGroup<Float, Ranges>::Model;

template<typename Float, typename Ranges = HarrisRanges>
using HarrisControl =
    typename HarrisGroup<Float, Ranges>::template Control<void>;

template<typename Observer, typename Float, typename Ranges = HarrisRanges>
using HarrisTerminus =
    typename HarrisGroup<Float, Ranges>::template Terminus<Observer>;


} // end namespace iris
