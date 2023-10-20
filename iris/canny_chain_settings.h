#pragma once

#include <fields/fields.h>
#include <pex/group.h>
#include <pex/endpoint.h>
#include "iris/default.h"
#include "iris/gaussian_settings.h"
#include "iris/gradient_settings.h"
#include "iris/canny_settings.h"
#include "iris/node_settings.h"


namespace iris
{


template<typename T>
struct CannyChainNodeSettingsFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::gaussian, "gaussian"),
        fields::Field(&T::gradient, "gradient"),
        fields::Field(&T::canny, "canny"));
};


template<template<typename> typename T>
struct CannyChainNodeSettingsTemplate
{
    T<NodeSettingsGroupMaker> gaussian;
    T<NodeSettingsGroupMaker> gradient;
    T<NodeSettingsGroupMaker> canny;
};


using CannyChainNodeSettingsGroup =
    pex::Group
    <
        CannyChainNodeSettingsFields,
        CannyChainNodeSettingsTemplate
    >;


using CannyChainNodeSettingsModel =
    typename CannyChainNodeSettingsGroup::Model;

using CannyChainNodeSettingsControl =
    typename CannyChainNodeSettingsGroup::Control;



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
struct CannyChainTemplate
{
    T<bool> enable;
    T<GaussianGroupMaker<int32_t>> gaussian;
    T<GradientGroupMaker<int32_t>> gradient;
    T<CannyGroupMaker<double>> canny;

    static constexpr auto fields = CannyChainFields<CannyChainTemplate>::fields;
    static constexpr auto fieldsTypeName = "CannyChain";
};


struct CannyChainSettings
    :
    public CannyChainTemplate<pex::Identity>
{
    static CannyChainSettings Default()
    {
        auto defaultGaussian = GaussianSettings<int32_t>::Default();
        defaultGaussian.sigma = 2.0;

        return {{
            true,
            defaultGaussian,
            GradientSettings<int32_t>::Default(),
            CannySettings<double>::Default()}};
    }
};


DECLARE_EQUALITY_OPERATORS(CannyChainSettings)


using CannyChainGroup = pex::Group
    <
        CannyChainFields,
        CannyChainTemplate,
        CannyChainSettings
    >;


struct CannyChainModel: public CannyChainGroup::Model
{
public:
    CannyChainModel()
        :
        CannyChainGroup::Model(),
        maximumEndpoint_(this)
    {

    }

    void SetMaximumControl(const MaximumControl &maximumControl)
    {
        this->maximumEndpoint_.ConnectUpstream(
            maximumControl,
            &CannyChainModel::OnMaximum_);
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
    pex::Endpoint<CannyChainModel, MaximumControl> maximumEndpoint_;
};


using CannyChainControl = typename CannyChainGroup::Control;


using CannyChainGroupMaker = pex::MakeGroup<CannyChainGroup, CannyChainModel>;


} // end namespace iris



extern template struct pex::Group
    <
        iris::CannyChainFields,
        iris::CannyChainTemplate,
        iris::CannyChainSettings
    >;
