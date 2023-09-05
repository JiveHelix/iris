#pragma once


#include <string>
#include <jive/power.h>
#include <pex/pex.h>
#include <pex/linked_ranges.h>
#include <pex/endpoint.h>
#include <wxpex/async.h>
#include <fields/fields.h>
#include "iris/default.h"


namespace iris
{


template<typename T>
struct LevelFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::enable, "enable"),
        fields::Field(&T::range, "range"),
        fields::Field(&T::maximum, "maximum"),
        fields::Field(&T::autoDetectSettings, "autoDetectSettings"),
        fields::Field(&T::detectMargin, "detectMargin"));
};


using LowLevel = pex::Limit<0>;
using HighLevel = pex::Limit<255>;

template<typename Value>
using LevelRanges =
    pex::LinkedRanges
    <
        Value,
        LowLevel,
        LowLevel,
        HighLevel,
        HighLevel
    >;


template<typename Value>
struct MaximumFilter
{
    static Value Set(Value value)
    {
        value = std::max(value, 0);
        value = std::min(std::numeric_limits<Value>::max() - 1, value);
        return value;
    }
};


template<typename Value>
struct LevelTemplate
{
    template<template<typename> typename T>
    struct Template
    {
        T<bool> enable;
        T<typename LevelRanges<Value>::GroupMaker> range;
        T<pex::Filtered<Value, MaximumFilter<Value>>> maximum;
        T<pex::MakeSignal> autoDetectSettings;

        using DetectRange =
            pex::MakeRange<double, pex::Limit<0>, pex::Limit<0, 49, 100>>;

        T<DetectRange> detectMargin;

        static constexpr auto fields = LevelFields<Template>::fields;
        static constexpr auto fieldsTypeName = "Level";
    };
};


template<typename Value>
struct LevelSettings:
    public LevelTemplate<Value>::template Template<pex::Identity>
{
    static LevelSettings Default()
    {
        return {{
            true,
            LevelRanges<Value>::Settings::Default(),
            defaultMaximum,
            {},
            0.05}};
    }
};


TEMPLATE_OUTPUT_STREAM(LevelSettings)
TEMPLATE_EQUALITY_OPERATORS(LevelSettings)


template<typename Value>
using LevelGroup =
    pex::Group
    <
        LevelFields,
        LevelTemplate<Value>::template Template,
        LevelSettings<Value>
    >;


template<typename Value>
using LevelControl = typename LevelGroup<Value>::Control;


template<typename Value>
struct LevelModel: public LevelGroup<Value>::Model
{
public:
    LevelModel()
        :
        LevelGroup<Value>::Model(),
        maximumEndpoint_(this, this->maximum, &LevelModel::OnMaximum_)
    {

    }

private:
    void OnMaximum_(Value maximumValue)
    {
        this->range.SetMaximumValue(maximumValue);
    }

private:
    using MaximumEndpoint =
        pex::Endpoint
        <
            LevelModel,
            decltype(LevelControl<Value>::maximum)
        >;

    MaximumEndpoint maximumEndpoint_;
};


template<typename Value>
using LevelGroupMaker = pex::MakeGroup<LevelGroup<Value>, LevelModel<Value>>;



extern template struct LevelSettings<int32_t>;


} // end namespace iris


extern template struct pex::Group
    <
        iris::LevelFields,
        iris::LevelTemplate<int32_t>::template Template,
        iris::LevelSettings<int32_t>
    >;


extern template struct pex::MakeGroup
    <
        iris::LevelGroup<int32_t>,
        iris::LevelModel<int32_t>
    >;
