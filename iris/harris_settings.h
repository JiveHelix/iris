#pragma once


#include <fields/fields.h>
#include <pex/group.h>
#include <tau/eigen_shim.h>


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
    using SigmaHigh = pex::Limit<10>;

    using ThresholdLow = pex::Limit<0>;
    using ThresholdHigh = pex::Limit<0, 25, 100>;

    using WindowLow = pex::Limit<2>;
    using WindowHigh = pex::Limit<20>;
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
    static constexpr Float defaultAlpha = static_cast<Float>(0.21);
    static constexpr Float defaultSigma = static_cast<Float>(3.85);
    static constexpr Float defaultThreshold = static_cast<Float>(0.01);
    static constexpr Eigen::Index defaultWindow = 6;
    static constexpr size_t defaultThreads = 4;

    HarrisSettings()
        :
        HarrisTemplate<Float>::template Template<pex::Identity>{
            true,
            defaultAlpha,
            defaultSigma,
            defaultThreshold,
            true,
            defaultWindow,
            defaultThreads}
    {

    }
};


TEMPLATE_EQUALITY_OPERATORS(HarrisSettings)


template<typename Float, typename Ranges = HarrisRanges>
using HarrisGroup = pex::Group
    <
        HarrisFields,
        HarrisTemplate<Float, Ranges>::template Template,
        pex::PlainT<HarrisSettings<Float>>
    >;

template<typename Float, typename Ranges = HarrisRanges>
using HarrisModel = typename HarrisGroup<Float, Ranges>::Model;

template<typename Float, typename Ranges = HarrisRanges>
using HarrisControl =
    typename HarrisGroup<Float, Ranges>::Control;


} // end namespace iris


extern template struct pex::Group
    <
        iris::HarrisFields,
        iris::HarrisTemplate<float, iris::HarrisRanges>::template Template,
        pex::PlainT<iris::HarrisSettings<float>>
    >;

extern template struct pex::Group
    <
        iris::HarrisFields,
        iris::HarrisTemplate<double, iris::HarrisRanges>::template Template,
        pex::PlainT<iris::HarrisSettings<double>>
    >;
