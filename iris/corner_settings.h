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
        fields::Field(&T::enable, "enable"),
        fields::Field(&T::window, "window"),
        fields::Field(&T::count, "count"),
        fields::Field(&T::threads, "threads"));
};


template<template<typename> typename T>
struct CornerTemplate
{
    using WindowLow = pex::Limit<3>;
    using WindowHigh = pex::Limit<64>;

    using CountLow = pex::Limit<1>;
    using CountHigh = pex::Limit<4>;

    T<bool> enable;
    T<pex::MakeRange<double, WindowLow, WindowHigh>> window;
    T<pex::MakeSelect<Eigen::Index>> count;
    T<size_t> threads;

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
        static constexpr Eigen::Index defaultWindow = 50;
        static constexpr Eigen::Index defaultCount = 2;
        static constexpr size_t defaultThreads = 4;

        return {{true, defaultWindow, defaultCount, defaultThreads}};
    }
};


DECLARE_EQUALITY_OPERATORS(CornerSettings)


using CornerGroup = pex::Group
    <
        CornerFields,
        CornerTemplate,
        CornerSettings
    >;


struct CornerModel: public CornerGroup::Model
{
    CornerModel()
        :
        CornerGroup::Model()
    {
        this->count.SetChoices({2, 4});
    }
};


using CornerControl = typename CornerGroup::Control;

using CornerGroupMaker = pex::MakeGroup<CornerGroup, CornerModel>;


} // end namespace iris


extern template struct pex::Group
    <
        iris::CornerFields,
        iris::CornerTemplate,
        iris::CornerSettings
    >;
