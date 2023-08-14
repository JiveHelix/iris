#pragma once


#include <pex/group.h>
#include "iris/mask_settings.h"
#include "iris/level_settings.h"
#include "iris/lines_chain_settings.h"
#include "iris/corners_chain_settings.h"
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
        fields::Field(&T::corners, "corners"),
        fields::Field(&T::lines, "lines"),
        fields::Field(&T::chess, "chess"),
        fields::Field(&T::autoDetectSettings, "autoDetectSettings"));
};


template<template<typename> typename T>
struct ChessChainTemplate
{
    T<bool> enable;
    T<MaskGroupMaker> mask;
    T<LevelGroupMaker<int32_t>> level;
    T<CornersChainGroupMaker> corners;
    T<LinesChainGroupMaker> lines;
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
        return {{
            true,
            MaskSettings::Default(),
            LevelSettings<int32_t>::Default(),
            CornersChainSettings::Default(),
            LinesChainSettings::Default(),
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
