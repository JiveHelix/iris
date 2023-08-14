#pragma once


#include <iris/mask_settings.h>
#include <iris/level_adjust.h>
#include <iris/corners_chain_settings.h>
#include <iris/color_settings.h>


template<typename T>
struct DemoFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::maximum, "maximum"),
        fields::Field(&T::mask, "mask"),
        fields::Field(&T::level, "level"),
        fields::Field(&T::cornersChain, "cornersChain"),
        fields::Field(&T::color, "color"));
};


template<template<typename> typename T>
struct DemoTemplate
{
    T<iris::InProcess> maximum;
    T<iris::MaskGroupMaker> mask;
    T<iris::LevelGroupMaker<int32_t>> level;
    T<iris::CornersChainGroupMaker> cornersChain;
    T<iris::ColorGroupMaker<int32_t>> color;

    static constexpr auto fields = DemoFields<DemoTemplate>::fields;
};


using DemoGroup = pex::Group<DemoFields, DemoTemplate>;
using DemoSettings = typename DemoGroup::Plain;

struct DemoModel: public DemoGroup::Model
{
    DemoModel()
        :
        DemoGroup::Model(),
        maximumEndpoint_(
            this,
            iris::MaximumControl(this->maximum),
            &DemoModel::OnMaximum_)
    {
        this->cornersChain.SetMaximumControl(
            iris::MaximumControl(this->maximum));
    }

    void OnMaximum_(iris::InProcess maximumValue)
    {
        this->level.maximum.Set(maximumValue);
    }

private:
    pex::Endpoint<DemoModel, iris::MaximumControl> maximumEndpoint_;
};

using DemoControl = typename DemoGroup::Control;


DECLARE_OUTPUT_STREAM_OPERATOR(DemoSettings)


