#pragma once

#include <fields/fields.h>
#include <pex/group.h>
#include <pex/select.h>
#include <pex/range.h>
#include "iris/derivative.h"
#include "iris/size.h"


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
        fields::Field(&T::fakeLines, "fakeLines"),
        fields::Field(&T::fakeCount, "fakeCount"),
        fields::Field(&T::threads, "threads"));

    static constexpr auto fieldsTypeName = "Hough";
};


using AngleRangeLow = pex::Limit<0>;
using AngleRangeHigh = pex::Limit<180>;

using WindowRange =
    pex::MakeRange<Eigen::Index, pex::Limit<3>, pex::Limit<256>>;

template<typename T>
using ThresholdRange = pex::MakeRange<T, pex::Limit<0>, pex::Limit<768>>;

using FakeRange = pex::MakeRange<size_t, pex::Limit<2>, pex::Limit<32>>;

template<typename Float>
struct HoughTemplate
{
    template<template<typename> typename T>
    struct Template
    {
        T<bool> enable;
        T<pex::MakeGroup<SizeGroup>> imageSize;
        T<size_t> rhoCount;
        T<size_t> thetaCount;
        T<pex::MakeRange<Float, AngleRangeLow, AngleRangeHigh>> angleRange;
        T<bool> weighted;
        T<bool> suppress;
        T<WindowRange> window;
        T<ThresholdRange<Float>> threshold;
        T<bool> fakeLines;
        T<FakeRange> fakeCount;
        T<size_t> threads;

        static constexpr auto fields = HoughFields<Template>::fields;
    };
};


template<typename Float>
struct HoughSettings:
    public HoughTemplate<Float>::template Template<pex::Identity>
{
    static constexpr int defaultImageWidth = 1920;
    static constexpr int defaultImageHeight = 1080;

    // About half-pixel resolution in 1920x1080 image.
    // static constexpr size_t defaultRhoCount = 2200;
    static constexpr size_t defaultRhoCount = 1024;

    // 10ths of a degree
    static constexpr size_t defaultThetaCount = 1024;

    // How far to search beyond the detected edge angle.
    static constexpr Float defaultAngleRange = 15;

    // The size of the non-maximum suppression window.
    static constexpr Eigen::Index defaultWindow = 24;

    // Minimum value to be considered a line.
    static constexpr size_t defaultThreshold = 96;

    static constexpr size_t defaultThreads = 4;

    static HoughSettings Default()
    {
        HoughSettings settings{{
            true,
            {defaultImageWidth, defaultImageHeight},
            defaultRhoCount,
            defaultThetaCount,
            defaultAngleRange,
            true,
            true,
            defaultWindow,
            defaultThreshold,
            false,
            16,
            defaultThreads}};

        return settings;
    }
};


TEMPLATE_OUTPUT_STREAM(HoughSettings)


template<typename Float>
using HoughGroup =
    pex::Group
    <
        HoughFields,
        HoughTemplate<Float>::template Template,
        HoughSettings<Float>
    >;


template<typename Float>
using HoughModel = typename HoughGroup<Float>::Model;

template<typename Float>
using HoughControl = typename HoughGroup<Float>::template Control<void>;

template<typename Float, typename Observer>
using HoughTerminus =
    typename HoughGroup<Float>::template Terminus<Observer>;

template<typename Float>
using HoughGroupMaker = pex::MakeGroup<HoughGroup<Float>>;


} // end namespace iris
