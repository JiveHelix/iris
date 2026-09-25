#pragma once


#include <pex/group.h>
#include <wxpex/async.h>
#include "iris/mask_settings.h"
#include "iris/level_settings.h"
#include "iris/lines_chain_settings.h"
#include "iris/vertex_chain_settings.h"
#include "iris/chess_settings.h"


namespace iris
{


template<template<typename> typename T>
struct ChessChainSchema
{
    T<bool> enable;
    T<MaskGroup> mask;
    T<LevelGroup<int32_t>> level;

    T<GaussianGroup<int32_t>> gaussian;
    T<GradientGroup<int32_t>> gradient;

    T<CannyGroup<double>> canny;
    T<HoughGroup<double>> hough;
    T<draw::LinesShapeGroup> linesShape;

    T<HarrisGroup<double>> harris;
    T<VertexGroup> vertices;
    T<draw::PointsShapeGroup> verticesShape;

    T<ChessGroup> chess;
    T<pex::MakeSignal> autoDetectSettings;

    static constexpr auto fieldsTypeName = "ChessChainSettings";
};


struct ChessChainFinisher
{
    template<typename Base>
    struct Plain: public Base
    {
        Plain()
            :
            Base{
                true,
                MaskSettings{},
                LevelSettings<int32_t>{},

                GaussianSettings<int32_t>{},
                GradientSettings<int32_t>{},

                CannySettings<double>{},
                HoughSettings<double>{},
                draw::LinesShapeSettings{},

                HarrisSettings<double>{},
                VertexSettings{},
                draw::PointsShapeSettings{},

                ChessSettings{},
                {}}
        {
            this->gaussian.sigma = 2.0;
        }
    };

    template<typename Base>
    struct Model
        :
        public Base
    {
    public:
        Model()
            :
            Base(),
            imageSizeEndpoint_(this),
            maximumEndpoint_(this)
        {
            PEX_NAME("ChessChainModel");
        }

        using Base::operator=;

        void SetImageSizeControl(const draw::SizeControl &sizeControl)
        {
            this->imageSizeEndpoint_.ConnectUpstream(
                sizeControl,
                &Model::OnImageSize_);
        }

        void SetMaximumControl(const MaximumControl &maximumControl)
        {
            this->maximumEndpoint_.ConnectUpstream(
                maximumControl,
                &Model::OnMaximum_);
        }

    private:
        void OnMaximum_(InProcess maximum)
        {
            auto deferLevel = pex::MakeDefer(this->level.maximum);
            auto deferGaussian = pex::MakeDefer(this->gaussian.maximum);
            auto deferGradient = pex::MakeDefer(this->gradient.maximum);

            deferLevel.Set(maximum);
            deferGaussian.Set(maximum);
            deferGradient.Set(maximum);
        }

        void OnImageSize_(const draw::Size &size)
        {
            auto deferMask = pex::MakeDefer(this->mask.imageSize);
            auto deferHough = pex::MakeDefer(this->hough.imageSize);
            deferMask.Set(size);
            deferHough.Set(size);
        }

    private:
        pex::Endpoint<Model, draw::SizeControl> imageSizeEndpoint_;
        pex::Endpoint<Model, MaximumControl> maximumEndpoint_;
    };
};


using ChessChainGroup =
    pex::Group<ChessChainSchema, ChessChainFinisher>;

using ChessChainSettings = typename ChessChainGroup::Plain;
using ChessChainControl = typename ChessChainGroup::DefaultControl;
using ChessChainModel = typename ChessChainGroup::Model;


DECLARE_EQUALITY_OPERATORS(ChessChainSettings)
DECLARE_OUTPUT_STREAM_OPERATOR(ChessChainSettings)



} // end namespace iris


extern template struct pex::Group
    <
        iris::ChessChainSchema,
        iris::ChessChainFinisher
    >;
