#pragma once

#include <fields/fields.h>
#include <pex/group.h>
#include <pex/select.h>
#include <wxpex/async.h>
#include "iris/derivative.h"
#include "iris/default.h"


namespace iris
{


template<typename Value>
struct GradientSchema
{
    template<template<typename> typename T>
    struct Schema
    {
        T<bool> enable;
        T<Value> maximum;
        T<DerivativeSize::MakeSelect> size;
        T<pex::MakeRange<Value, pex::Limit<1>, pex::Limit<10>>> scale;
        T<pex::MakeSignal> autoDetectSettings;
        T<double> percentile;
    };
};


template<typename Value>
struct GradientFinisher
{
    template<typename Base>
    struct Plain: public Base
    {
        static constexpr iris::DerivativeSize::Size defaultSize =
            DerivativeSize::Size::three;

        static constexpr Value defaultScale = 1;
        static constexpr double defaultPercentile = 0.995;

        Plain()
            :
            Base{
                true,
                defaultMaximum,
                defaultSize,
                defaultScale,
                {},
                defaultPercentile}
        {

        }
    };
};


template<typename Value>
using GradientGroup =
    pex::Group
    <
        GradientSchema<Value>::template Schema,
        GradientFinisher<Value>
    >;

template<typename Value>
using GradientModel = typename GradientGroup<Value>::Model;

template<typename Value>
using GradientControl = typename GradientGroup<Value>::DefaultControl;

template<typename Value>
using GradientSettings = typename GradientGroup<Value>::Plain;


DECLARE_OUTPUT_STREAM_OPERATOR(GradientSettings<int32_t>)
DECLARE_EQUALITY_OPERATORS(GradientSettings<int32_t>)


} // end namespace iris


extern template struct pex::Group
    <
        iris::GradientSchema<int32_t>::template Schema,
        iris::GradientFinisher<int32_t>
    >;
