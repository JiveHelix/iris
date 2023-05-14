#pragma once


#include <string>
#include <pex/pex.h>
#include <pex/linked_ranges.h>
#include <fields/fields.h>


namespace iris
{


template<typename T>
struct LevelFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::range, "range"),
        fields::Field(&T::maximum, "maximum"),
        fields::Field(&T::detect, "detect"));
};


using LowLevel = pex::Limit<0>;
using HighLevel = pex::Limit<1023>;

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
struct LevelTemplate
{
    template<template<typename> typename T>
    struct Template
    {
        T<typename LevelRanges<Value>::GroupMaker> range;
        T<Value> maximum;
        T<pex::MakeSignal> detect;

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
            LevelRanges<Value>::Settings::Default(),
            std::numeric_limits<Value>::max(),
            {}}};
    }
};


TEMPLATE_OUTPUT_STREAM(LevelSettings)


template<typename Value>
using LevelGroup =
    pex::Group
    <
        LevelFields,
        LevelTemplate<Value>::template Template,
        LevelSettings<Value>
    >;


template<typename Value>
struct LevelModel: public LevelGroup<Value>::Model
{
public:
    using MaximumTerminus =
        pex::Terminus<LevelModel, pex::model::Value<Value>>;

    LevelModel()
        :
        LevelGroup<Value>::Model(),
        maximumTerminus_(this, this->maximum)
    {
        this->maximumTerminus_.Connect(&LevelModel::OnMaximum_);
    }

    void OnMaximum_(Value maximumValue)
    {
        this->range.SetMaximumValue(maximumValue);
    }

private:
    MaximumTerminus maximumTerminus_;
};


template<typename Value>
using LevelGroupMaker = pex::MakeGroup<LevelGroup<Value>, LevelModel<Value>>;


template<typename Value>
using LevelControl = typename LevelGroup<Value>::template Control<void>;


template<typename Value, typename Observer>
using LevelTerminus = typename LevelGroup<Value>::template Terminus<Observer>;


} // end namespace iris
