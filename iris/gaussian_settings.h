#pragma once


#include <limits>
#include <fields/fields.h>
#include <pex/range.h>
#include <pex/group.h>


namespace iris
{


template<typename T>
struct GaussianFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::enable, "enable"),
        fields::Field(&T::sigma, "sigma"),
        fields::Field(&T::threshold, "threshold"),
        fields::Field(&T::threads, "threads"),
        fields::Field(&T::maximumInput, "maximumInput"));

    static constexpr auto fieldsTypeName = "Gaussian";
};


template
<
    typename Value,
    typename SigmaLow = pex::Limit<1>,
    typename SigmaHigh = pex::Limit<10>
>
struct GaussianTemplate
{
    template<template<typename> typename T>
    struct Template
    {
        T<bool> enable;
        T<pex::MakeRange<double, SigmaLow, SigmaHigh>> sigma;
        T<double> threshold;
        T<size_t> threads;
        T<Value> maximumInput;

        static constexpr auto fields = GaussianFields<Template>::fields;
    };

};


template<typename Value>
struct GaussianSettings:
    public GaussianTemplate<Value>::template Template<pex::Identity>
{
    static GaussianSettings Default()
    {
        static constexpr double defaultSigma = 1.0;
        static constexpr double defaultThreshold = 0.01;
        static constexpr size_t defaultThreads = 4;
        static constexpr Value defaultMaximumInput = 255;

        return {{
            true,
            defaultSigma,
            defaultThreshold,
            defaultThreads,
            defaultMaximumInput}};
    }
};


TEMPLATE_OUTPUT_STREAM(GaussianSettings)


template<typename Value>
using GaussianGroup = pex::Group
<
    GaussianFields,
    GaussianTemplate<Value>::template Template,
    GaussianSettings<Value>
>;


template<typename Value>
using GaussianGroupMaker = pex::MakeGroup<GaussianGroup<Value>>;


template<typename Value>
using GaussianModel = typename GaussianGroup<Value>::Model;

template<typename Value>
using GaussianControl = typename GaussianGroup<Value>::template Control<void>;

template<typename Value, typename Observer>
using GaussianTerminus =
    typename GaussianGroup<Value>::template Terminus<Observer>;


} // end namespace iris
