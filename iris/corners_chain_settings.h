#pragma once

#include <fields/fields.h>
#include <pex/group.h>
#include <draw/points_shape.h>
#include "iris/gaussian_settings.h"
#include "iris/gradient_settings.h"
#include "iris/harris_settings.h"
#include "iris/corner_settings.h"


namespace iris
{


template<typename T>
struct CornersChainFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::enable, "enable"),
        fields::Field(&T::gaussian, "gaussian"),
        fields::Field(&T::gradient, "gradient"),
        fields::Field(&T::harris, "harris"),
        fields::Field(&T::corner, "corner"),
        fields::Field(&T::shape, "shape"));
};


template<template<typename> typename T>
struct CornersChainTemplate
{
    T<bool> enable;
    T<GaussianGroupMaker<int32_t>> gaussian;
    T<GradientGroupMaker<int32_t>> gradient;
    T<HarrisGroupMaker<double>> harris;
    T<CornerGroupMaker> corner;
    T<draw::PointsShapeGroupMaker> shape;

    static constexpr auto fields =
        CornersChainFields<CornersChainTemplate>::fields;

    static constexpr auto fieldsTypeName = "CornersChain";
};


struct CornersChainSettings
    :
    public CornersChainTemplate<pex::Identity>
{
    static CornersChainSettings Default()
    {
        auto defaultGaussian = GaussianSettings<int32_t>::Default();
        defaultGaussian.sigma = 2.0;

        return {{
            true,
            defaultGaussian,
            GradientSettings<int32_t>::Default(),
            HarrisSettings<double>::Default(),
            CornerSettings::Default(),
            draw::PointsShapeSettings::Default()}};
    }
};


DECLARE_EQUALITY_OPERATORS(CornersChainSettings)


using CornersChainGroup = pex::Group
    <
        CornersChainFields,
        CornersChainTemplate,
        CornersChainSettings
    >;


struct CornersChainModel: public CornersChainGroup::Model
{
public:
    CornersChainModel()
        :
        CornersChainGroup::Model(),
        maximumEndpoint_(this)
    {

    }

    void SetMaximumControl(const MaximumControl &maximumControl)
    {
        this->maximumEndpoint_.ConnectUpstream(
            maximumControl,
            &CornersChainModel::OnMaximum_);
    }

private:
    void OnMaximum_(int32_t maximum)
    {
        auto deferGaussian = pex::MakeDefer(this->gaussian.maximum);
        auto deferGradient = pex::MakeDefer(this->gradient.maximum);
        deferGaussian.Set(maximum);
        deferGradient.Set(maximum);
    }

    pex::Endpoint<CornersChainModel, MaximumControl> maximumEndpoint_;
};

using CornersChainControl = typename CornersChainGroup::Control;

using CornersChainGroupMaker =
    pex::MakeGroup<CornersChainGroup, CornersChainModel>;


} // end namespace iris



extern template struct pex::Group
    <
        iris::CornersChainFields,
        iris::CornersChainTemplate,
        iris::CornersChainSettings
    >;


extern template struct pex::MakeGroup
    <
        iris::CornersChainGroup,
        iris::CornersChainModel
    >;
