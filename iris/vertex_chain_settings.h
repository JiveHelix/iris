#pragma once

#include <fields/fields.h>
#include <pex/group.h>
#include <draw/points_shape.h>
#include "iris/gaussian_settings.h"
#include "iris/gradient_settings.h"
#include "iris/harris_settings.h"
#include "iris/vertex_settings.h"


namespace iris
{


template<typename T>
struct VertexChainFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::enable, "enable"),
        fields::Field(&T::gaussian, "gaussian"),
        fields::Field(&T::gradient, "gradient"),
        fields::Field(&T::harris, "harris"),
        fields::Field(&T::vertex, "vertex"),
        fields::Field(&T::shape, "shape"));
};


template<template<typename> typename T>
struct VertexChainTemplate
{
    T<bool> enable;
    T<GaussianGroupMaker<int32_t>> gaussian;
    T<GradientGroupMaker<int32_t>> gradient;
    T<HarrisGroupMaker<double>> harris;
    T<VertexGroupMaker> vertex;
    T<draw::PointsShapeGroupMaker> shape;

    static constexpr auto fields =
        VertexChainFields<VertexChainTemplate>::fields;

    static constexpr auto fieldsTypeName = "VertexChain";
};


struct VertexChainSettings
    :
    public VertexChainTemplate<pex::Identity>
{
    static VertexChainSettings Default()
    {
        auto defaultGaussian = GaussianSettings<int32_t>::Default();
        defaultGaussian.sigma = 2.0;

        return {{
            true,
            defaultGaussian,
            GradientSettings<int32_t>::Default(),
            HarrisSettings<double>::Default(),
            VertexSettings::Default(),
            draw::PointsShapeSettings::Default()}};
    }
};


DECLARE_EQUALITY_OPERATORS(VertexChainSettings)


using VertexChainGroup = pex::Group
    <
        VertexChainFields,
        VertexChainTemplate,
        VertexChainSettings
    >;


struct VertexChainModel: public VertexChainGroup::Model
{
public:
    VertexChainModel()
        :
        VertexChainGroup::Model(),
        maximumEndpoint_(this)
    {

    }

    void SetMaximumControl(const MaximumControl &maximumControl)
    {
        this->maximumEndpoint_.ConnectUpstream(
            maximumControl,
            &VertexChainModel::OnMaximum_);
    }

private:
    void OnMaximum_(int32_t maximum)
    {
        auto deferGaussian = pex::MakeDefer(this->gaussian.maximum);
        auto deferGradient = pex::MakeDefer(this->gradient.maximum);
        deferGaussian.Set(maximum);
        deferGradient.Set(maximum);
    }

    pex::Endpoint<VertexChainModel, MaximumControl> maximumEndpoint_;
};

using VertexChainControl = typename VertexChainGroup::Control;

using VertexChainGroupMaker =
    pex::MakeGroup<VertexChainGroup, VertexChainModel>;


} // end namespace iris



extern template struct pex::Group
    <
        iris::VertexChainFields,
        iris::VertexChainTemplate,
        iris::VertexChainSettings
    >;


extern template struct pex::MakeGroup
    <
        iris::VertexChainGroup,
        iris::VertexChainModel
    >;
