#pragma once


#include <iris/mask_settings.h>
#include <iris/level_settings.h>
#include <draw/color_map_settings.h>
#include <iris/chess_chain_settings.h>
#include <iris/chess_chain_node_settings.h>
#include <iris/views/chess_shape.h>


template<typename T>
struct DemoFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::maximum, "maximum"),
        fields::Field(&T::imageSize, "imageSize"),
        fields::Field(&T::nodeSettings, "nodeSettings"),
        fields::Field(&T::chess, "chess"),
        fields::Field(&T::chessShape, "chessShape"),
        fields::Field(&T::color, "color"));
};


using InProcess = int32_t;


template<template<typename> typename T>
struct DemoTemplate
{
    T<iris::InProcess> maximum;
    T<draw::SizeGroup> imageSize;
    T<iris::ChessChainNodeSettingsGroup> nodeSettings;
    T<iris::ChessChainGroup> chess;
    T<iris::ChessShapeGroup> chessShape;
    T<draw::ColorMapSettingsGroup<int32_t>> color;

    static constexpr auto fields = DemoFields<DemoTemplate>::fields;
};


struct DemoCustom
{
    template<typename Base>
    struct Model: public Base
    {
    public:
        Model()
            :
            Base(),
            maximumEndpoint_(
                this,
                iris::MaximumControl(this->maximum),
                &Model::OnMaximum_)
        {
            this->chess.SetImageSizeControl(draw::SizeControl(this->imageSize));
            this->chess.SetMaximumControl(iris::MaximumControl(this->maximum));
        }

    private:
        void OnMaximum_(iris::InProcess maximumValue)
        {
            this->color.maximum.Set(maximumValue);
        }

        pex::Endpoint<Model, iris::MaximumControl> maximumEndpoint_;
    };
};


using DemoGroup = pex::Group<DemoFields, DemoTemplate, DemoCustom>;
using DemoSettings = typename DemoGroup::Plain;
using DemoModel = typename DemoGroup::Model;
using DemoControl = typename DemoGroup::Control;


DECLARE_OUTPUT_STREAM_OPERATOR(DemoSettings)
