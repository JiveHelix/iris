#pragma once

#include <fields/fields.h>
#include <pex/group.h>
#include <pex/select.h>
#include <pex/linked_ranges.h>
#include "iris/derivative.h"


namespace iris
{


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
struct CannySchema
{
    template<template<typename> typename T>
    struct Schema
    {
        T<bool> enable;
        T<typename CannyRanges<Float>::Group> range;
        T<size_t> depth;
    };
};


template<typename Float>
struct CannySettings:
    public CannySchema<Float>::template Schema<pex::Identity>
{
    static constexpr size_t defaultDepth = 32;

    CannySettings()
        :
        CannySchema<Float>::template Schema<pex::Identity>{
            true,
            typename CannyRanges<Float>::Settings{},
            defaultDepth}
    {

    }
};


TEMPLATE_OUTPUT_STREAM(CannySettings)
TEMPLATE_EQUALITY_OPERATORS(CannySettings)


template<typename Float>
using CannyGroup =
    pex::Group
    <
        CannySchema<Float>::template Schema,
        pex::PlainT<CannySettings<Float>>
    >;


template<typename Float>
using CannyModel = typename CannyGroup<Float>::Model;

template<typename Float>
using CannyControl = typename CannyGroup<Float>::DefaultControl;


extern template struct CannySettings<float>;
extern template struct CannySettings<double>;


} // end namespace iris


extern template struct pex::LinkedRanges
    <
        float,
        iris::CannyLowerBound,
        iris::CannyLow,
        iris::CannyUpperBound,
        iris::CannyHigh
    >;


extern template struct pex::LinkedRanges
    <
        double,
        iris::CannyLowerBound,
        iris::CannyLow,
        iris::CannyUpperBound,
        iris::CannyHigh
    >;


extern template struct pex::Group
    <
        iris::CannySchema<float>::template Schema,
        pex::PlainT<iris::CannySettings<float>>
    >;


extern template struct pex::Group
    <
        iris::CannySchema<double>::template Schema,
        pex::PlainT<iris::CannySettings<double>>
    >;
