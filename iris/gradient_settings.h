#pragma once

#include <fields/fields.h>
#include <pex/group.h>
#include <pex/select.h>
#include "iris/derivative.h"


namespace iris
{


template<typename T>
struct GradientFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::enable, "enable"),
        fields::Field(&T::maximumInput, "maximumInput"),
        fields::Field(&T::size, "size"),
        fields::Field(&T::scale, "scale"),
        fields::Field(&T::threads, "threads"));

    static constexpr auto fieldsTypeName = "Gradient";
};


template<typename Value>
struct GradientTemplate
{
    template<template<typename> typename T>
    struct Template
    {
        T<bool> enable;
        T<Value> maximumInput;
        T<pex::MakeSelect<iris::DerivativeSize::Size>> size;
        T<pex::MakeRange<Value, pex::Limit<1>, pex::Limit<10>>> scale;
        T<size_t> threads;

        static constexpr auto fields = GradientFields<Template>::fields;
    };
};


template<typename Value>
struct GradientSettings:
    public GradientTemplate<Value>::template Template<pex::Identity>
{
    static constexpr Value defaultMaximumInput = 255;

    static constexpr iris::DerivativeSize::Size defaultSize =
        DerivativeSize::Size::three;

    static constexpr Value defaultScale = 1;
    static constexpr size_t defaultThreads = 4;

    static GradientSettings Default()
    {
        return {{
            true,
            defaultMaximumInput,
            defaultSize,
            defaultScale,
            defaultThreads}};
    }
};


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
using GradientControl = typename GradientGroup<Value>::template Control<void>;

template<typename Value, typename Observer>
using GradientTerminus =
    typename GradientGroup<Value>::template Terminus<Observer>;

template<typename Value>
using GradientGroupMaker =
    pex::MakeGroup<GradientGroup<Value>, GradientModel<Value>>;


} // end namespace iris
