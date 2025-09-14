#pragma once


#include <limits>
#include <fields/fields.h>
#include <pex/range.h>
#include <pex/group.h>
#include "iris/default.h"


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
        fields::Field(&T::maximum, "maximum"));

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
        T<Value> maximum;

        static constexpr auto fields = GaussianFields<Template>::fields;
    };

};


template<typename Value>
struct GaussianSettings:
    public GaussianTemplate<Value>::template Template<pex::Identity>
{
    using Base =
        typename GaussianTemplate<Value>::template Template<pex::Identity>;

    static constexpr double defaultSigma = 1.0;
    static constexpr double defaultThreshold = 0.01;
    static constexpr size_t defaultThreads = 4;

    GaussianSettings()
        :
        Base{
            true,
            defaultSigma,
            defaultThreshold,
            defaultThreads,
            defaultMaximum}
    {

    }
};


TEMPLATE_OUTPUT_STREAM(GaussianSettings)
TEMPLATE_EQUALITY_OPERATORS(GaussianSettings)


template<typename Value>
using GaussianGroup = pex::Group
<
    GaussianFields,
    GaussianTemplate<Value>::template Template,
    pex::PlainT<GaussianSettings<Value>>
>;


template<typename Value>
using GaussianModel = typename GaussianGroup<Value>::Model;

template<typename Value>
using GaussianControl = typename GaussianGroup<Value>::DefaultControl;


extern template struct GaussianSettings<int32_t>;


} // end namespace iris


extern template struct pex::Group
<
    iris::GaussianFields,
    iris::GaussianTemplate<int32_t>::template Template,
    pex::PlainT<iris::GaussianSettings<int32_t>>
>;
