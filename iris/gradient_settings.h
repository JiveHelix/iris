#pragma once

#include <fields/fields.h>
#include <pex/group.h>
#include <pex/select.h>
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
        T<pex::MakeSelect<iris::DerivativeSize::Size>> size;
        T<pex::MakeRange<Value, pex::Limit<1>, pex::Limit<10>>> scale;
        T<size_t> threads;
        T<pex::MakeSignal> autoDetectSettings;
        T<double> percentile;

        static constexpr auto fields = GradientFields<Template>::fields;
    };
};


template<typename Value>
struct GradientSettings:
    public GradientTemplate<Value>::template Template<pex::Identity>
{
    static constexpr iris::DerivativeSize::Size defaultSize =
        DerivativeSize::Size::three;

    static constexpr Value defaultScale = 1;
    static constexpr size_t defaultThreads = 4;
    static constexpr double defaultPercentile = 0.995;

    static GradientSettings Default()
    {
        return {{
            true,
            defaultMaximum,
            defaultSize,
            defaultScale,
            defaultThreads,
            {},
            defaultPercentile}};
    }
};


TEMPLATE_OUTPUT_STREAM(GradientSettings)
TEMPLATE_EQUALITY_OPERATORS(GradientSettings)


template<typename Value>
using GradientGroup =
    pex::Group
    <
        GradientFields,
        GradientTemplate<Value>::template Template,
        GradientSettings<Value>
    >;


template<typename Value>
struct GradientModel: public GradientGroup<Value>::Model
{
    GradientModel()
        :
        GradientGroup<Value>::Model()
    {
        this->size.SetChoices(iris::DerivativeSize::GetValidSizes());
    }
};

template<typename Value>
using GradientControl = typename GradientGroup<Value>::Control;


template<typename Value>
using GradientGroupMaker =
    pex::MakeGroup<GradientGroup<Value>, GradientModel<Value>>;


extern template struct GradientSettings<int32_t>;
extern template struct GradientModel<int32_t>;


} // end namespace iris


extern template struct pex::Group
    <
        iris::GradientFields,
        iris::GradientTemplate<int32_t>::template Template,
        iris::GradientSettings<int32_t>
    >;


extern template struct pex::MakeGroup
    <
        iris::GradientGroup<int32_t>,
        iris::GradientModel<int32_t>
    >;

