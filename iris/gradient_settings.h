#pragma once

#include <fields/fields.h>
#include <pex/group.h>
#include <pex/select.h>
#include <wxpex/async.h>
#include "iris/derivative.h"
#include "iris/default.h"


namespace iris
{


template<typename T>
struct GradientFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::enable, "enable"),
        fields::Field(&T::maximum, "maximum"),
        fields::Field(&T::size, "size"),
        fields::Field(&T::scale, "scale"),
        fields::Field(&T::threads, "threads"),
        fields::Field(&T::autoDetectSettings, "autoDetectSettings"),
        fields::Field(&T::percentile, "percentile"));

    static constexpr auto fieldsTypeName = "Gradient";
};


template<typename Value>
struct GradientTemplate
{
    template<template<typename> typename T>
    struct Template
    {
        T<bool> enable;
        T<Value> maximum;
        T<DerivativeSize::MakeSelect> size;
        T<pex::MakeRange<Value, pex::Limit<1>, pex::Limit<10>>> scale;
        T<size_t> threads;
        T<pex::MakeSignal> autoDetectSettings;
        T<double> percentile;

        static constexpr auto fields = GradientFields<Template>::fields;
    };
};


template<typename Value>
struct GradientCustom
{
    template<typename Base>
    struct Plain: public Base
    {
        static constexpr iris::DerivativeSize::Size defaultSize =
            DerivativeSize::Size::three;

        static constexpr Value defaultScale = 1;
        static constexpr size_t defaultThreads = 4;
        static constexpr double defaultPercentile = 0.995;

        Plain()
            :
            Base{
                true,
                defaultMaximum,
                defaultSize,
                defaultScale,
                defaultThreads,
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
        GradientFields,
        GradientTemplate<Value>::template Template,
        GradientCustom<Value>
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
        iris::GradientFields,
        iris::GradientTemplate<int32_t>::template Template,
        iris::GradientCustom<int32_t>
    >;
