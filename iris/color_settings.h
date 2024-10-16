#pragma once


#include <vector>
#include <string>
#include <pex/linked_ranges.h>
#include <fields/fields.h>
#include <fields/compare.h>

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
        fields::Field(&T::range, "range"),
        fields::Field(&T::maximum, "maximum"));
};


template<typename Value>
struct ColorTemplate
{
    template<template<typename> typename T>
    struct Template
    {
        T<bool> turbo;
        T<typename LevelRanges<Value>::Group> range;
        T<Value> maximum;

        static constexpr auto fields = ColorFields<Template>::fields;
        static constexpr auto fieldsTypeName = "Color";
    };
};


template<typename Value>
struct ColorSettings:
    public ColorTemplate<Value>::template Template<pex::Identity>
{
    ColorSettings()
        :
        ColorTemplate<Value>::template Template<pex::Identity>{
            true,
            typename LevelRanges<Value>::Settings{},
            255}
    {

    }
};


template<typename Value>
struct ColorCustom
{
    using Plain = ColorSettings<Value>;

    template<typename ModelBase>
    struct Model: public ModelBase
    {
    public:
        using MaximumTerminus =
            pex::Terminus<Model, pex::model::Value<Value>>;

        Model()
            :
            ModelBase(),
            maximumTerminus_(this, this->maximum)
        {
            this->maximumTerminus_.Connect(&Model::OnMaximum_);
        }

    private:
        void OnMaximum_(Value maximum_)
        {
            this->range.SetMaximumValue(maximum_);
        }

    private:
        MaximumTerminus maximumTerminus_;
    };
};


TEMPLATE_EQUALITY_OPERATORS(ColorSettings)
TEMPLATE_OUTPUT_STREAM(ColorSettings)


template<typename Value>
using ColorGroup =
    pex::Group
    <
        ColorFields,
        ColorTemplate<Value>::template Template,
        ColorCustom<Value>
    >;

template<typename Value>
using ColorModel = typename ColorGroup<Value>::Model;

template<typename Value>
using ColorControl = typename ColorGroup<Value>::Control;


} // end namespace iris


extern template struct pex::Group
    <
        iris::ColorFields,
        iris::ColorTemplate<int32_t>::template Template,
        iris::ColorCustom<int32_t>
    >;
