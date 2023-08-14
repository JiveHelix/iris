#pragma once


#include <draw/lines_shape.h>
#include <iris/mask_settings.h>
#include <iris/level_settings.h>
#include <iris/lines_chain_settings.h>
#include <iris/color_settings.h>


template<typename T>
struct DemoFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::maximum, "maximum"),
        fields::Field(&T::imageSize, "imageSize"),
        fields::Field(&T::mask, "mask"),
        fields::Field(&T::level, "level"),
        fields::Field(&T::lines, "lines"),
        fields::Field(&T::color, "color"));
};


template<template<typename> typename T>
struct DemoTemplate
{
    T<iris::InProcess> maximum;
    T<pex::MakeGroup<draw::SizeGroup>> imageSize;
    T<iris::MaskGroupMaker> mask;
    T<iris::LevelGroupMaker<int32_t>> level;
    T<iris::LinesChainGroupMaker> lines;
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
            &DemoModel::OnMaximum_),
        imageSizeEndpoint_(
            this,
            draw::SizeControl(this->imageSize),
            &DemoModel::OnImageSize_)
    {
        this->lines.SetMaximumControl(iris::MaximumControl(this->maximum));
        this->lines.SetImageSizeControl(draw::SizeControl(this->imageSize));
    }

    void OnMaximum_(iris::InProcess maximumValue)
    {
        this->level.maximum.Set(maximumValue);
    }

    void OnImageSize_(const draw::Size &size)
    {
        this->mask.imageSize.Set(size);
    }

private:
    pex::Endpoint<DemoModel, iris::MaximumControl> maximumEndpoint_;
    pex::Endpoint<DemoModel, draw::SizeControl> imageSizeEndpoint_;
};

using DemoControl = typename DemoGroup::Control;


DECLARE_OUTPUT_STREAM_OPERATOR(DemoSettings)
