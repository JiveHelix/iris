#pragma once

#include <fields/fields.h>
#include <pex/group.h>
#include <draw/points_shape.h>
#include <draw/node_settings.h>
#include "iris/gaussian_settings.h"
#include "iris/gradient_settings.h"
#include "iris/harris_settings.h"
#include "iris/vertex_settings.h"


namespace iris
{


template<template<typename> typename T>
struct VertexChainNodeSettingsSchema
{
    T<draw::NodeSettingsGroup> gaussian;
    T<draw::NodeSettingsGroup> gradient;
    T<draw::NodeSettingsGroup> harris;
    T<draw::NodeSettingsGroup> vertex;
};


using VertexChainNodeSettingsGroup =
    pex::Group<VertexChainNodeSettingsSchema>;


using VertexChainNodeSettingsModel =
    typename VertexChainNodeSettingsGroup::Model;

using VertexChainNodeSettingsControl =
    typename VertexChainNodeSettingsGroup::DefaultControl;


template<template<typename> typename T>
struct VertexChainSchema
{
    T<bool> enable;
    T<GaussianGroup<int32_t>> gaussian;
    T<GradientGroup<int32_t>> gradient;
    T<HarrisGroup<double>> harris;
    T<VertexGroup> vertex;
    T<draw::PointsShapeGroup> shape;

    static constexpr auto fieldsTypeName = "VertexChain";
};


struct VertexChainFinisher
{
    template<typename Base>
    struct Plain: public Base
    {
        Plain()
            :
            Base{
                true,
                GaussianSettings<int32_t>{},
                GradientSettings<int32_t>{},
                HarrisSettings<double>{},
                VertexSettings{},
                draw::PointsShapeSettings{}}
        {
            this->gaussian.sigma = 2.0;
        }
    };

    template<typename Base>
    struct Model: public Base
    {
    public:
        Model()
            :
            Base(),
            maximumEndpoint_(this)
        {
            PEX_NAME("VertexChainModel");
        }

        void SetMaximumControl(const MaximumControl &maximumControl)
        {
            this->maximumEndpoint_.ConnectUpstream(
                maximumControl,
                &Model::OnMaximum_);
        }

    private:
        void OnMaximum_(int32_t maximum)
        {
            auto deferGaussian = pex::MakeDefer(this->gaussian.maximum);
            auto deferGradient = pex::MakeDefer(this->gradient.maximum);
            deferGaussian.Set(maximum);
            deferGradient.Set(maximum);
        }

        pex::Endpoint<Model, MaximumControl> maximumEndpoint_;
    };
};


using VertexChainGroup = pex::Group
    <
        VertexChainSchema,
        VertexChainFinisher
    >;


using VertexChainSettings = typename VertexChainGroup::Plain;
using VertexChainModel = typename VertexChainGroup::Model;
using VertexChainControl = typename VertexChainGroup::DefaultControl;

DECLARE_EQUALITY_OPERATORS(VertexChainSettings)
DECLARE_OUTPUT_STREAM_OPERATOR(VertexChainSettings)


} // end namespace iris


extern template struct pex::Group
    <
        iris::VertexChainSchema,
        iris::VertexChainFinisher
    >;
