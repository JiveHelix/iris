#pragma once

#include <fields/fields.h>
#include <pex/group.h>
#include <pex/endpoint.h>
#include <draw/node_settings.h>
#include "iris/default.h"
#include "iris/gaussian_settings.h"
#include "iris/gradient_settings.h"
#include "iris/canny_settings.h"


namespace iris
{


template<template<typename> typename T>
struct CannyChainNodeSettingsSchema
{
    T<draw::NodeSettingsGroup> gaussian;
    T<draw::NodeSettingsGroup> gradient;
    T<draw::NodeSettingsGroup> canny;
};


using CannyChainNodeSettingsGroup =
    pex::Group<CannyChainNodeSettingsSchema>;


using CannyChainNodeSettingsModel =
    typename CannyChainNodeSettingsGroup::Model;

using CannyChainNodeSettingsControl =
    typename CannyChainNodeSettingsGroup::DefaultControl;



template<typename T>
struct CannyChainFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::enable, "enable"),
        fields::Field(&T::gaussian, "gaussian"),
        fields::Field(&T::gradient, "gradient"),
        fields::Field(&T::canny, "canny"));
};


template<template<typename> typename T>
struct CannyChainSchema
{
    T<bool> enable;
    T<GaussianGroup<int32_t>> gaussian;
    T<GradientGroup<int32_t>> gradient;
    T<CannyGroup<double>> canny;

    static constexpr auto fields = CannyChainFields<CannyChainSchema>::fields;
    static constexpr auto fieldsTypeName = "CannyChain";
};


struct CannyChainFinisher
{
    template<typename PlainBase>
    struct Plain: public PlainBase
    {
        Plain()
            :
            PlainBase{
                true,
                GaussianSettings<int32_t>{},
                GradientSettings<int32_t>{},
                CannySettings<double>{}}
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
            PEX_NAME("CannyChainModel");
        }

        void SetMaximumControl(const MaximumControl &maximumControl)
        {
            this->maximumEndpoint_.ConnectUpstream(
                maximumControl,
                &Model::OnMaximum_);
        }

    private:
        void OnMaximum_(InProcess maximum)
        {
            auto deferGaussian = pex::MakeDefer(this->gaussian.maximum);
            auto deferGradient = pex::MakeDefer(this->gradient.maximum);
            deferGaussian.Set(maximum);
            deferGradient.Set(maximum);
        }

    private:
        pex::Endpoint<Model, MaximumControl> maximumEndpoint_;
    };
};


using CannyChainGroup = pex::Group
    <
        CannyChainSchema,
        CannyChainFinisher
    >;

using CannyChainSettings = typename CannyChainGroup::Plain;
using CannyChainControl = typename CannyChainGroup::DefaultControl;
using CannyChainModel = typename CannyChainGroup::Model;


DECLARE_OUTPUT_STREAM_OPERATOR(CannyChainSettings)
DECLARE_EQUALITY_OPERATORS(CannyChainSettings)


} // end namespace iris


extern template struct pex::Group
    <
        iris::CannyChainSchema,
        iris::CannyChainFinisher
    >;
