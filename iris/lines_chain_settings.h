#pragma once

#include <fields/fields.h>
#include <pex/group.h>
#include <draw/lines_shape.h>
#include "iris/default.h"
#include "iris/canny_chain_settings.h"
#include "iris/hough_settings.h"


namespace iris
{


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
    T<CannyChainGroupMaker> cannyChain;
    T<HoughGroupMaker<double>> hough;
    T<draw::LinesShapeGroupMaker> shape;

    static constexpr auto fields = LinesChainFields<LinesChainTemplate>::fields;
    static constexpr auto fieldsTypeName = "LineChain";
};


struct LinesChainSettings
    :
    public LinesChainTemplate<pex::Identity>
{
    static LinesChainSettings Default()
    {
        return {{
            true,
            CannyChainSettings::Default(),
            HoughSettings<double>::Default(),
            draw::LinesShapeSettings::Default()}};
    }
};


DECLARE_EQUALITY_OPERATORS(LinesChainSettings)


using LinesChainGroup = pex::Group
    <
        LinesChainFields,
        LinesChainTemplate,
        LinesChainSettings
    >;


struct LinesChainModel: public LinesChainGroup::Model
{
public:
    LinesChainModel()
        :
        LinesChainGroup::Model(),
        imageSizeEndpoint_(this)
    {

    }

    void SetMaximumControl(const MaximumControl &maximumControl)
    {
        this->cannyChain.SetMaximumControl(maximumControl);
    }

    void SetImageSizeControl(const draw::SizeControl &sizeControl)
    {
        this->imageSizeEndpoint_.ConnectUpstream(
            sizeControl,
            &LinesChainModel::SetImageSize);
    }

    void SetImageSize(const draw::Size &size)
    {
        this->hough.imageSize.Set(size);
    }

    pex::Endpoint<LinesChainModel, draw::SizeControl> imageSizeEndpoint_;
};


using LinesChainControl = typename LinesChainGroup::Control;


using LinesChainGroupMaker = pex::MakeGroup<LinesChainGroup, LinesChainModel>;


} // end namespace iris



extern template struct pex::Group
    <
        iris::LinesChainFields,
        iris::LinesChainTemplate,
        iris::LinesChainSettings
    >;


extern template struct pex::MakeGroup
    <
        iris::LinesChainGroup,
        iris::LinesChainModel
    >;
