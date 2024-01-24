#pragma once


#include <draw/lines_shape.h>
#include <iris/mask_settings.h>
#include <iris/level_settings.h>
#include <iris/lines_chain_settings.h>
#include <iris/color_settings.h>

#include "../common/png_settings.h"


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
    T<draw::SizeGroup> imageSize;
    T<iris::MaskGroup> mask;
    T<iris::LevelGroup<int32_t>> level;
    T<iris::LinesChainGroup> lines;
    T<iris::ColorGroup<int32_t>> color;

    static constexpr auto fields = DemoFields<DemoTemplate>::fields;
};


struct DemoCustom
{
    template<typename ModelBase>
    struct Model: public ModelBase
    {
        Model()
            :
            ModelBase(),
            maximumEndpoint_(
                this,
                iris::MaximumControl(this->maximum),
                &Model::OnMaximum_),
            imageSizeEndpoint_(
                this,
                draw::SizeControl(this->imageSize),
                &Model::OnImageSize_)
        {
            iris::InProcess maximumValue = pngMaximum;
            this->level.range.high.SetMaximum(maximumValue);
            this->level.range.high.Set(maximumValue);
            this->level.maximum.Set(maximumValue);
            this->color.range.high.SetMaximum(maximumValue);
            this->color.range.high.Set(maximumValue);
            this->color.maximum.Set(maximumValue);

            this->lines.SetMaximumControl(iris::MaximumControl(this->maximum));
            this->lines.SetImageSizeControl(draw::SizeControl(this->imageSize));

            this->maximum.Set(maximumValue);
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
        pex::Endpoint<Model, iris::MaximumControl> maximumEndpoint_;
        pex::Endpoint<Model, draw::SizeControl> imageSizeEndpoint_;
    };
};


using DemoGroup = pex::Group<DemoFields, DemoTemplate, DemoCustom>;
using DemoSettings = typename DemoGroup::Plain;
using DemoModel = typename DemoGroup::Model;
using DemoControl = typename DemoGroup::Control;


DECLARE_OUTPUT_STREAM_OPERATOR(DemoSettings)
