#pragma once


#include <Eigen/Dense>
#include <fields/fields.h>
#include <pex/group.h>


namespace iris
{


template<typename T>
struct CornerFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::window, "window"),
        fields::Field(&T::count, "count"));
};


template<template<typename> typename T>
struct CornerTemplate
{
    using WindowLow = pex::Limit<3>;
    using WindowHigh = pex::Limit<32>;

    using CountLow = pex::Limit<1>;
    using CountHigh = pex::Limit<4>;

    T<pex::MakeRange<Eigen::Index, WindowLow, WindowHigh>> window;
    T<pex::MakeRange<Eigen::Index, CountLow, CountHigh>> count;

    static constexpr auto fields =
        CornerFields<CornerTemplate>::fields;

    static constexpr auto fieldsTypeName = "Corner";
};


struct CornerSettings
    :
    public CornerTemplate<pex::Identity>
{
    static CornerSettings Default()
    {
        static constexpr Eigen::Index defaultWindow = 6;
        static constexpr Eigen::Index defaultCount = 4;

        return {{defaultWindow, defaultCount}};
    }
};


DECLARE_COMPARISON_OPERATORS(CornerSettings)


using CornerGroup = pex::Group
    <
        CornerFields,
        CornerTemplate,
        CornerSettings
    >;

using CornerModel = typename CornerGroup::Model;

using CornerControl = typename CornerGroup::Control<void>;

template<typename Observer>
using CornerTerminus = typename CornerGroup::Terminus<Observer>;


} // end namespace iris
