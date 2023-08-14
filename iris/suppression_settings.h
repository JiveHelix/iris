#pragma once


#include <Eigen/Dense>
#include <fields/fields.h>
#include <pex/group.h>


namespace iris
{


template<typename T>
struct SuppressionFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::window, "window"),
        fields::Field(&T::count, "count"));
};


struct SuppressionRanges
{
    using WindowLow = pex::Limit<2>;
    using WindowHigh = pex::Limit<10>;

    using CountLow = pex::Limit<1>;
    using CountHigh = pex::Limit<10>;
};


template<typename Ranges = SuppressionRanges>
struct SuppressionTemplate
{
    using WindowLow = typename Ranges::WindowLow;
    using WindowHigh = typename Ranges::WindowHigh;

    using CountLow = typename Ranges::CountLow;
    using CountHigh = typename Ranges::CountHigh;

    template<template<typename> typename T>
    struct Template
    {
        T<pex::MakeRange<Eigen::Index, WindowLow, WindowHigh>> window;
        T<pex::MakeRange<Eigen::Index, CountLow, CountHigh>> count;

        static constexpr auto fields = SuppressionFields<Template>::fields;
        static constexpr auto fieldsTypeName = "Suppression";
    };
};


struct SuppressionSettings
    :
    public SuppressionTemplate<SuppressionRanges>
        ::template Template<pex::Identity>
{
    static SuppressionSettings Default()
    {
        static constexpr Eigen::Index defaultWindow = 3;
        static constexpr Eigen::Index defaultCount = 1;

        return {{defaultWindow, defaultCount}};
    }
};


DECLARE_EQUALITY_OPERATORS(SuppressionSettings)


using SuppressionGroup =
    pex::Group
    <
        SuppressionFields,
        SuppressionTemplate<>::template Template,
        SuppressionSettings
    >;

using SuppressionModel = typename SuppressionGroup::Model;

using SuppressionControl = typename SuppressionGroup::Control;


} // end namespace iris
