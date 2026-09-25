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
struct LevelSchema
{
    template<template<typename> typename T>
    struct Schema
    {
        T<bool> enable;
        T<typename LevelRanges<Value>::Group> range;
        T<pex::Filtered<Value, MaximumFilter<Value>>> maximum;
        T<pex::MakeSignal> autoDetectSettings;

        using DetectRange =
            pex::MakeRange<double, pex::Limit<0>, pex::Limit<0, 49, 100>>;

        T<DetectRange> detectMargin;

        static constexpr auto fieldsTypeName = "Level";
    };
};


template<typename Value>
struct LevelSettings:
    public LevelSchema<Value>::template Schema<pex::Identity>
{
    LevelSettings()
        :
        LevelSchema<Value>::template Schema<pex::Identity>{
            true,
            typename LevelRanges<Value>::Settings{},
            defaultMaximum,
            {},
            0.05}
    {

    }
};


TEMPLATE_OUTPUT_STREAM(LevelSettings)
TEMPLATE_EQUALITY_OPERATORS(LevelSettings)


template<typename Value>
struct LevelFinisher
{
    using Plain = LevelSettings<Value>;

    template<typename Base>
    struct Model: public Base
    {
    public:
        Model()
            :
            Base(),

            maximumEndpoint_(
                PEX_THIS("LevelModel"),
                this->maximum,
                &Model::OnMaximum_)
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
                Model,
                decltype(Model::maximum)
            >;

        MaximumEndpoint maximumEndpoint_;
    };
};


template<typename Value>
using LevelGroup =
    pex::Group
    <
        LevelSchema<Value>::template Schema,
        LevelFinisher<Value>
    >;


template<typename Value>
using LevelModel = typename LevelGroup<Value>::Model;

template<typename Value>
using LevelControl = typename LevelGroup<Value>::DefaultControl;


extern template struct LevelSettings<int32_t>;


} // end namespace iris


extern template struct pex::Group
    <
        iris::LevelSchema<int32_t>::template Schema,
        iris::LevelFinisher<int32_t>
    >;
