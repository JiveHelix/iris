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


template<typename T>
struct ChessChainFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::enable, "enable"),
        fields::Field(&T::mask, "mask"),
        fields::Field(&T::level, "level"),

        fields::Field(&T::gaussian, "gaussian"),
        fields::Field(&T::gradient, "gradient"),

        fields::Field(&T::canny, "canny"),
        fields::Field(&T::hough, "hough"),
        fields::Field(&T::linesShape, "linesShape"),

        fields::Field(&T::harris, "harris"),
        fields::Field(&T::vertices, "vertices"),
        fields::Field(&T::verticesShape, "verticesShape"),

        fields::Field(&T::chess, "chess"),
        fields::Field(&T::autoDetectSettings, "autoDetectSettings"));
};


template<template<typename> typename T>
struct ChessChainTemplate
{
    T<bool> enable;
    T<MaskGroupMaker> mask;
    T<LevelGroupMaker<int32_t>> level;

    T<GaussianGroupMaker<int32_t>> gaussian;
    T<GradientGroupMaker<int32_t>> gradient;

    T<CannyGroupMaker<double>> canny;
    T<HoughGroupMaker<double>> hough;
    T<draw::LinesShapeGroupMaker> linesShape;

    T<HarrisGroupMaker<double>> harris;
    T<VertexGroupMaker> vertices;
    T<draw::PointsShapeGroupMaker> verticesShape;

    T<ChessGroupMaker> chess;
    T<pex::MakeSignal> autoDetectSettings;

    static constexpr auto fields = ChessChainFields<ChessChainTemplate>::fields;
};


// using ChessChainSettings = typename ChessChainGroup::Plain;
struct ChessChainSettings
    :
    public ChessChainTemplate<pex::Identity>
{
    static ChessChainSettings Default()
    {
        auto defaultGaussian = GaussianSettings<int32_t>::Default();
        defaultGaussian.sigma = 2.0;

        return {{
            true,
            MaskSettings::Default(),
            LevelSettings<int32_t>::Default(),

            defaultGaussian,
            GradientSettings<int32_t>::Default(),

            CannySettings<double>::Default(),
            HoughSettings<double>::Default(),
            draw::LinesShapeSettings::Default(),

            HarrisSettings<double>::Default(),
            VertexSettings::Default(),
            draw::PointsShapeSettings::Default(),

            ChessSettings::Default(),
            {}}};
    }
};


DECLARE_EQUALITY_OPERATORS(ChessChainSettings)


using ChessChainGroup =
    pex::Group<ChessChainFields, ChessChainTemplate, ChessChainSettings>;


struct ChessChainModel: public ChessChainGroup::Model
{
public:
    ChessChainModel();

    void SetImageSizeControl(const draw::SizeControl &sizeControl);

    void SetMaximumControl(const MaximumControl &maximumControl);

private:
    void OnMaximum_(InProcess maximum);

    void OnImageSize_(const draw::Size &size);

private:
    pex::Endpoint<ChessChainModel, draw::SizeControl> imageSizeEndpoint_;
    pex::Endpoint<ChessChainModel, MaximumControl> maximumEndpoint_;
};

using ChessChainControl = typename ChessChainGroup::Control;

using ChessChainGroupMaker = pex::MakeGroup<ChessChainGroup, ChessChainModel>;


DECLARE_OUTPUT_STREAM_OPERATOR(ChessChainSettings)


} // end namespace iris


extern template struct pex::Group
    <
        iris::ChessChainFields,
        iris::ChessChainTemplate,
        iris::ChessChainSettings
    >;
