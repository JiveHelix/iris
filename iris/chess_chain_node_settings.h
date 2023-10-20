#pragma once


#include <pex/group.h>
#include <pex/endpoint.h>
#include "iris/node_settings.h"


namespace iris
{


template<typename T>
struct ChessChainNodeSettingsFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::mask, "mask"),
        fields::Field(&T::level, "level"),
        fields::Field(&T::gaussian, "gaussian"),
        fields::Field(&T::gradient, "gradient"),
        fields::Field(&T::canny, "canny"),
        fields::Field(&T::hough, "hough"),
        fields::Field(&T::harris, "harris"),
        fields::Field(&T::vertices, "vertices"),
        fields::Field(&T::chess, "chess"));
};


template<template<typename> typename T>
struct ChessChainNodeSettingsTemplate
{
    T<NodeSettingsGroupMaker> mask;
    T<NodeSettingsGroupMaker> level;
    T<NodeSettingsGroupMaker> gaussian;
    T<NodeSettingsGroupMaker> gradient;
    T<NodeSettingsGroupMaker> canny;
    T<NodeSettingsGroupMaker> hough;
    T<NodeSettingsGroupMaker> harris;
    T<NodeSettingsGroupMaker> vertices;
    T<NodeSettingsGroupMaker> chess;

    static constexpr auto fields =
        ChessChainNodeSettingsFields<ChessChainNodeSettingsTemplate>::fields;
};


using ChessChainNodeSettingsGroup =
    pex::Group
    <
        ChessChainNodeSettingsFields,
        ChessChainNodeSettingsTemplate
    >;


using ChessChainNodeSettingsControl =
    typename ChessChainNodeSettingsGroup::Control;


using ChessChainNodeSettings = typename ChessChainNodeSettingsGroup::Plain;


bool HasHighlight(const ChessChainNodeSettings &);


struct ChessChainNodeSettingsModel: public ChessChainNodeSettingsGroup::Model
{
    using Endpoint = pex::Endpoint
        <
            ChessChainNodeSettingsModel,
            decltype(NodeSettingsControl::select)
        >;

    ChessChainNodeSettingsModel();

private:
    void OnMask_();
    void OnLevel_();
    void OnGaussian_();
    void OnGradient_();
    void OnCanny_();
    void OnHough_();
    void OnHarris_();
    void OnVertices_();
    void OnChess_();

    void Toggle(NodeSettingsModel *toSelect);

private:
    Endpoint maskEndpoint_;
    Endpoint levelEndpoint_;
    Endpoint gaussianEndpoint_;
    Endpoint gradientEndpoint_;
    Endpoint cannyEndpoint_;
    Endpoint houghEndpoint_;
    Endpoint harrisEndpoint_;
    Endpoint verticesEndpoint_;
    Endpoint chessEndpoint_;

    NodeSettingsModel *selected_;
};


using ChessChainNodeSettingsGroupMaker =
    pex::MakeGroup
    <
        ChessChainNodeSettingsGroup,
        ChessChainNodeSettingsModel
    >;


} // end namespace iris
