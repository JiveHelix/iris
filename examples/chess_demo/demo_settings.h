#pragma once


#include <iris/mask_settings.h>
#include <iris/level_settings.h>
#include <tau/color_map_settings.h>
#include <iris/chess_chain_settings.h>
#include <iris/chess_chain_node_settings.h>
#include <iris/views/chess_shape.h>


template<template<typename> typename T>
struct DemoSchema
{
    T<iris::InProcess> maximum;
    T<draw::SizeGroup> imageSize;
    T<iris::ChessChainNodeSettingsGroup> nodeSettings;
    T<iris::ChessChainGroup> chess;
    T<iris::ChessShapeGroup> chessShape;
    T<tau::ColorMapSettingsGroup<int32_t>> color;
};


struct DemoFinisher
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


using DemoGroup = pex::Group<DemoSchema, DemoFinisher>;
using DemoSettings = typename DemoGroup::Plain;
using DemoModel = typename DemoGroup::Model;
using DemoControl = typename DemoGroup::DefaultControl;


DECLARE_OUTPUT_STREAM_OPERATOR(DemoSettings)
