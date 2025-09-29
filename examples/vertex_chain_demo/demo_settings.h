#pragma once


#include <iris/mask_settings.h>
#include <iris/level_adjust.h>
#include <iris/vertex_chain_settings.h>
#include <tau/color_map_settings.h>


template<typename T>
struct DemoFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::maximum, "maximum"),
        fields::Field(&T::mask, "mask"),
        fields::Field(&T::level, "level"),
        fields::Field(&T::vertexChain, "vertexChain"),
        fields::Field(&T::color, "color"));
};


template<template<typename> typename T>
struct DemoTemplate
{
    T<iris::InProcess> maximum;
    T<iris::MaskGroup> mask;
    T<iris::LevelGroup<int32_t>> level;
    T<iris::VertexChainGroup> vertexChain;
    T<tau::ColorMapSettingsGroup<int32_t>> color;

    static constexpr auto fields = DemoFields<DemoTemplate>::fields;
};


struct DemoCustom
{
    template<typename Base>
    struct Model: public Base
    {
        Model()
            :
            Base(),
            maximumEndpoint_(
                this,
                iris::MaximumControl(this->maximum),
                &Model::OnMaximum_)
        {
            this->vertexChain.SetMaximumControl(
                iris::MaximumControl(this->maximum));
        }

        void OnMaximum_(iris::InProcess maximumValue)
        {
            this->level.maximum.Set(maximumValue);
        }

    private:
        pex::Endpoint<Model, iris::MaximumControl> maximumEndpoint_;
    };
};


using DemoGroup = pex::Group<DemoFields, DemoTemplate, DemoCustom>;

using DemoSettings = typename DemoGroup::Plain;
using DemoModel = typename DemoGroup::Model;
using DemoControl = typename DemoGroup::DefaultControl;


DECLARE_OUTPUT_STREAM_OPERATOR(DemoSettings)


