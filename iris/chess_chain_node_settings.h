#pragma once


#include <pex/group.h>
#include <pex/endpoint.h>
#include <draw/node_settings.h>


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
    T<draw::NodeSettingsGroup> mask;
    T<draw::NodeSettingsGroup> level;
    T<draw::NodeSettingsGroup> gaussian;
    T<draw::NodeSettingsGroup> gradient;
    T<draw::NodeSettingsGroup> canny;
    T<draw::NodeSettingsGroup> hough;
    T<draw::NodeSettingsGroup> harris;
    T<draw::NodeSettingsGroup> vertices;
    T<draw::NodeSettingsGroup> chess;

    static constexpr auto fields =
        ChessChainNodeSettingsFields<ChessChainNodeSettingsTemplate>::fields;
};


struct ChessChainNodeSettingsCustom
{
    template<typename Base>
    class Model: public Base
    {
    public:
        Model()
            :
            Base(),
            maskEndpoint_(
                PEX_THIS("ChessChainNodeSettings"),
                this->mask.toggleSelect,
                &Model<Base>::OnMask_),

            levelEndpoint_(
                this,
                this->level.toggleSelect,
                &Model<Base>::OnLevel_),

            gaussianEndpoint_(
                this,
                this->gaussian.toggleSelect,
                &Model<Base>::OnGaussian_),

            gradientEndpoint_(
                this,
                this->gradient.toggleSelect,
                &Model<Base>::OnGradient_),

            cannyEndpoint_(
                this,
                this->canny.toggleSelect,
                &Model<Base>::OnCanny_),

            houghEndpoint_(
                this,
                this->hough.toggleSelect,
                &Model<Base>::OnHough_),

            harrisEndpoint_(
                this,
                this->harris.toggleSelect,
                &Model<Base>::OnHarris_),

            verticesEndpoint_(
                this,
                this->vertices.toggleSelect,
                &Model<Base>::OnVertices_),

            chessEndpoint_(
                this,
                this->chess.toggleSelect,
                &Model<Base>::OnChess_),

            selected_(nullptr)
        {

        }

    private:
        void Toggle_(draw::NodeSettingsModel *toSelect)
        {
            if (this->selected_ == toSelect)
            {
                // Toggle off.
                // None are selected.
                this->selected_->isSelected.Set(false);
                this->selected_ = nullptr;

                return;
            }

            if (this->selected_)
            {
                this->selected_->isSelected.Set(false);
            }

            toSelect->isSelected.Set(true);
            this->selected_ = toSelect;
        }

        void OnMask_()
        {
            this->Toggle_(&this->mask);
        }

        void OnLevel_()
        {
            this->Toggle_(&this->level);
        }

        void OnGaussian_()
        {
            this->Toggle_(&this->gaussian);
        }

        void OnGradient_()
        {
            this->Toggle_(&this->gradient);
        }

        void OnCanny_()
        {
            this->Toggle_(&this->canny);
        }

        void OnHough_()
        {
            this->Toggle_(&this->hough);
        }

        void OnHarris_()
        {
            this->Toggle_(&this->harris);
        }

        void OnVertices_()
        {
            this->Toggle_(&this->vertices);
        }

        void OnChess_()
        {
            this->Toggle_(&this->chess);
        }

    private:
        using Endpoint = draw::NodeToggleSelectEndpoint<Model>;

        Endpoint maskEndpoint_;
        Endpoint levelEndpoint_;
        Endpoint gaussianEndpoint_;
        Endpoint gradientEndpoint_;
        Endpoint cannyEndpoint_;
        Endpoint houghEndpoint_;
        Endpoint harrisEndpoint_;
        Endpoint verticesEndpoint_;
        Endpoint chessEndpoint_;

        draw::NodeSettingsModel *selected_;
    };
};


using ChessChainNodeSettingsGroup =
    pex::Group
    <
        ChessChainNodeSettingsFields,
        ChessChainNodeSettingsTemplate,
        ChessChainNodeSettingsCustom
    >;


using ChessChainNodeSettings = typename ChessChainNodeSettingsGroup::Plain;

using ChessChainNodeSettingsModel =
    typename ChessChainNodeSettingsGroup::Model;

using ChessChainNodeSettingsControl =
    typename ChessChainNodeSettingsGroup::DefaultControl;


bool HasSelectedNode(const ChessChainNodeSettings &);


} // end namespace iris
