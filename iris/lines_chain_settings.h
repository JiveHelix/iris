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


template<template<typename> typename T>
struct LinesChainNodeSettingsSchema
{
    T<CannyChainNodeSettingsGroup> cannyChain;
    T<draw::NodeSettingsGroup> hough;
};


using LinesChainNodeSettingsGroup =
    pex::Group<LinesChainNodeSettingsSchema>;


using LinesChainNodeSettingsModel =
    typename LinesChainNodeSettingsGroup::Model;

using LinesChainNodeSettingsControl =
    typename LinesChainNodeSettingsGroup::DefaultControl;


template<template<typename> typename T>
struct LinesChainSchema
{
    T<bool> enable;
    T<CannyChainGroup> cannyChain;
    T<HoughGroup<double>> hough;
    T<draw::LinesShapeGroup> shape;

    static constexpr auto fieldsTypeName = "LineChain";
};


struct LinesChainFinisher
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
            PEX_NAME("LinesChainModel");
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
        LinesChainSchema,
        LinesChainFinisher
    >;


using LinesChainSettings = typename LinesChainGroup::Plain;
using LinesChainModel = typename LinesChainGroup::Model;
using LinesChainControl = typename LinesChainGroup::DefaultControl;

DECLARE_EQUALITY_OPERATORS(LinesChainSettings)
DECLARE_OUTPUT_STREAM_OPERATOR(LinesChainSettings)


} // end namespace iris


extern template struct pex::Group
    <
        iris::LinesChainSchema,
        iris::LinesChainFinisher
    >;
