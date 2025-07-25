#pragma once

#include <fields/fields.h>
#include <pex/group.h>
#include <draw/lines_shape.h>
#include <draw/node_settings.h>
#include "iris/default.h"
#include "iris/canny_chain_settings.h"
#include "iris/hough_settings.h"


namespace iris
{


template<typename T>
struct LinesChainNodeSettingsFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::cannyChain, "cannyChain"),
        fields::Field(&T::hough, "hough"));
};


template<template<typename> typename T>
struct LinesChainNodeSettingsTemplate
{
    T<CannyChainNodeSettingsGroup> cannyChain;
    T<draw::NodeSettingsGroup> hough;
};


using LinesChainNodeSettingsGroup =
    pex::Group
    <
        LinesChainNodeSettingsFields,
        LinesChainNodeSettingsTemplate
    >;


using LinesChainNodeSettingsModel =
    typename LinesChainNodeSettingsGroup::Model;

using LinesChainNodeSettingsControl =
    typename LinesChainNodeSettingsGroup::Control;


template<typename T>
struct LinesChainFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::enable, "enable"),
        fields::Field(&T::cannyChain, "cannyChain"),
        fields::Field(&T::hough, "hough"),
        fields::Field(&T::shape, "shape"));
};


template<template<typename> typename T>
struct LinesChainTemplate
{
    T<bool> enable;
    T<CannyChainGroup> cannyChain;
    T<HoughGroup<double>> hough;
    T<draw::LinesShapeGroup> shape;

    static constexpr auto fields = LinesChainFields<LinesChainTemplate>::fields;
    static constexpr auto fieldsTypeName = "LineChain";
};


struct LinesChainCustom
{
    template<typename Base>
    struct Plain: public Base
    {
        Plain()
            :
            Base{
                true,
                CannyChainSettings{},
                HoughSettings<double>{},
                draw::LinesShapeSettings{}}
        {

        }
    };

    template<typename Base>
    struct Model: public Base
    {
    public:
        Model()
            :
            Base(),
            imageSizeEndpoint_(this)
        {
            REGISTER_PEX_NAME(this, "LinesChainModel");
        }

        void SetMaximumControl(const MaximumControl &maximumControl)
        {
            this->cannyChain.SetMaximumControl(maximumControl);
        }

        void SetImageSizeControl(const draw::SizeControl &sizeControl)
        {
            this->imageSizeEndpoint_.ConnectUpstream(
                sizeControl,
                &Model::SetImageSize);
        }

        void SetImageSize(const draw::Size &size)
        {
            this->hough.imageSize.Set(size);
        }

        pex::Endpoint<Model, draw::SizeControl> imageSizeEndpoint_;
    };
};


using LinesChainGroup = pex::Group
    <
        LinesChainFields,
        LinesChainTemplate,
        LinesChainCustom
    >;


using LinesChainSettings = typename LinesChainGroup::Plain;
using LinesChainModel = typename LinesChainGroup::Model;
using LinesChainControl = typename LinesChainGroup::Control;

DECLARE_EQUALITY_OPERATORS(LinesChainSettings)
DECLARE_OUTPUT_STREAM_OPERATOR(LinesChainSettings)


} // end namespace iris


extern template struct pex::Group
    <
        iris::LinesChainFields,
        iris::LinesChainTemplate,
        iris::LinesChainCustom
    >;
