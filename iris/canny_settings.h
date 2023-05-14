#pragma once

#include <fields/fields.h>
#include <pex/group.h>
#include <pex/select.h>
#include <pex/linked_ranges.h>
#include "iris/derivative.h"


namespace iris
{


template<typename T>
struct CannyFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::enable, "enable"),
        fields::Field(&T::range, "range"),
        fields::Field(&T::depth, "depth"),
        fields::Field(&T::threads, "threads"));

    static constexpr auto fieldsTypeName = "Canny";
};


// Canny hysteresis ranges from 0 to 1.
using CannyLowerBound = pex::Limit<0>;
using CannyUpperBound = pex::Limit<1>;
using CannyLow = pex::Limit<0, 1, 10>;
using CannyHigh = pex::Limit<0, 25, 100>;

template<typename Float>
using CannyRanges =
    pex::LinkedRanges
    <
        Float,
        CannyLowerBound,
        CannyLow,
        CannyUpperBound,
        CannyHigh
    >;


template<typename Float>
struct CannyTemplate
{
    template<template<typename> typename T>
    struct Template
    {
        T<bool> enable;
        T<typename CannyRanges<Float>::GroupMaker> range;
        T<size_t> depth;
        T<size_t> threads;

        static constexpr auto fields = CannyFields<Template>::fields;
    };
};


template<typename Float>
struct CannySettings:
    public CannyTemplate<Float>::template Template<pex::Identity>
{
    static constexpr size_t defaultDepth = 32;
    static constexpr size_t defaultThreads = 4;

    static CannySettings Default()
    {
        CannySettings settings{{
            true,
            CannyRanges<Float>::Settings::Default(),
            defaultDepth,
            defaultThreads}};

        return settings;
    }
};


TEMPLATE_OUTPUT_STREAM(CannySettings)


template<typename Float>
using CannyGroup =
    pex::Group
    <
        CannyFields,
        CannyTemplate<Float>::template Template,
        CannySettings<Float>
    >;


template<typename Float>
using CannyModel = typename CannyGroup<Float>::Model;

template<typename Float>
using CannyControl = typename CannyGroup<Float>::template Control<void>;

template<typename Float, typename Observer>
using CannyTerminus =
    typename CannyGroup<Float>::template Terminus<Observer>;

template<typename Float>
using CannyGroupMaker = pex::MakeGroup<CannyGroup<Float>>;


} // end namespace iris
