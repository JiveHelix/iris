#pragma once


#include <vector>
#include <string>
#include <pex/linked_ranges.h>
#include <fields/fields.h>

#include "iris/level_settings.h"


namespace iris
{


namespace colors
{


struct SignedGradient
{
    static constexpr auto name = "Signed Gradient";
};


struct Turbo
{
    static constexpr auto name = "Turbo";
};


struct Grayscale
{
    static constexpr auto name = "Gray";
};


} // end namespace colors


template<typename T>
struct ColorFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::turbo, "turbo"),
        fields::Field(&T::level, "level"),
        fields::Field(&T::maximum, "maximum"));
};


template<typename Value>
struct ColorTemplate
{
    template<template<typename> typename T>
    struct Template
    {
        T<bool> turbo;
        T<typename LevelRanges<Value>::GroupMaker> level;
        T<Value> maximum;

        static constexpr auto fields = ColorFields<Template>::fields;
    };
};


template<typename Value>
struct ColorSettings:
    public ColorTemplate<Value>::template Template<pex::Identity>
{
    static ColorSettings Default()
    {
        return {{
            true,
            LevelRanges<Value>::Settings::Default(),
            std::numeric_limits<Value>::max()}};
    }
};


template<typename Value>
using ColorGroup =
    pex::Group
    <
        ColorFields,
        ColorTemplate<Value>::template Template,
        ColorSettings<Value>
    >;


template<typename Value>
struct ColorModel: public ColorGroup<Value>::Model
{
public:
    using MaximumTerminus =
        pex::Terminus<ColorModel, pex::model::Value<Value>>;

    ColorModel()
        :
        ColorGroup<Value>::Model(),
        maximumTerminus_(this, this->maximum)
    {
        this->maximumTerminus_.Connect(&ColorModel::OnMaximum_);
    }

    void OnMaximum_(Value maximumValue)
    {
        this->level.SetMaximumValue(maximumValue);
    }

private:
    MaximumTerminus maximumTerminus_;
};


template<typename Value>
using ColorControl = typename ColorGroup<Value>::template Control<void>;


template<typename Value, typename Observer>
using ColorTerminus = typename ColorGroup<Value>::template Terminus<Observer>;


template<typename Value>
using ColorGroupMaker = pex::MakeGroup<ColorGroup<Value>, ColorModel<Value>>;


} // end namespace iris
