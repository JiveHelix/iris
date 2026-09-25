#pragma once


#include <iris/mask_settings.h>
#include <iris/level_adjust.h>
#include <iris/vertex_chain_settings.h>
#include <tau/color_map_settings.h>


template<template<typename> typename T>
struct DemoSchema
{
    T<iris::InProcess> maximum;
    T<iris::MaskGroup> mask;
    T<iris::LevelGroup<int32_t>> level;
    T<iris::VertexChainGroup> vertexChain;
    T<tau::ColorMapSettingsGroup<int32_t>> color;
};


struct DemoFinisher
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
            this->color.maximum.Set(maximumValue);
        }

    private:
        pex::Endpoint<Model, iris::MaximumControl> maximumEndpoint_;
    };
};


using DemoGroup = pex::Group<DemoSchema, DemoFinisher>;

using DemoSettings = typename DemoGroup::Plain;
using DemoModel = typename DemoGroup::Model;
using DemoControl = typename DemoGroup::DefaultControl;


DECLARE_OUTPUT_STREAM_OPERATOR(DemoSettings)


