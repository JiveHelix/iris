#pragma once


#include <fields/fields.h>
#include <pex/group.h>
#include <tau/eigen_shim.h>


namespace iris
{


struct SuppressionRanges
{
    using WindowLow = pex::Limit<2>;
    using WindowHigh = pex::Limit<10>;

    using CountLow = pex::Limit<1>;
    using CountHigh = pex::Limit<10>;
};


template<typename Ranges = SuppressionRanges>
struct SuppressionSchema
{
    using WindowLow = typename Ranges::WindowLow;
    using WindowHigh = typename Ranges::WindowHigh;

    using CountLow = typename Ranges::CountLow;
    using CountHigh = typename Ranges::CountHigh;

    template<template<typename> typename T>
    struct Schema
    {
        T<pex::MakeRange<Eigen::Index, WindowLow, WindowHigh>> window;
        T<pex::MakeRange<Eigen::Index, CountLow, CountHigh>> count;

        static constexpr auto fieldsTypeName = "Suppression";
    };
};


struct SuppressionSettings
    :
    public SuppressionSchema<SuppressionRanges>
        ::template Schema<pex::Identity>
{
    using Base =
        SuppressionSchema<SuppressionRanges>
            ::template Schema<pex::Identity>;

    static constexpr Eigen::Index defaultWindow = 3;
    static constexpr Eigen::Index defaultCount = 1;

    SuppressionSettings()
        :
        Base{defaultWindow, defaultCount}
    {

    }
};


DECLARE_EQUALITY_OPERATORS(SuppressionSettings)


using SuppressionGroup =
    pex::Group
    <
        SuppressionSchema<>::template Schema,
        pex::PlainT<SuppressionSettings>
    >;

using SuppressionModel = typename SuppressionGroup::Model;
using SuppressionControl = typename SuppressionGroup::DefaultControl;


} // end namespace iris
