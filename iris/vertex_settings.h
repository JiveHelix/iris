#pragma once


#include <fields/fields.h>
#include <pex/group.h>
#include <tau/eigen_shim.h>


namespace iris
{


template<typename T>
struct VertexFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::enable, "enable"),
        fields::Field(&T::window, "window"),
        fields::Field(&T::count, "count"),
        fields::Field(&T::threads, "threads"));
};


template<template<typename> typename T>
struct VertexTemplate
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
        VertexFields<VertexTemplate>::fields;

    static constexpr auto fieldsTypeName = "Vertex";
};


struct VertexSettings
    :
    public VertexTemplate<pex::Identity>
{
    static VertexSettings Default()
    {
        static constexpr Eigen::Index defaultWindow = 40;
        static constexpr Eigen::Index defaultCount = 4;
        static constexpr size_t defaultThreads = 4;

        return {{true, defaultWindow, defaultCount, defaultThreads}};
    }
};


DECLARE_EQUALITY_OPERATORS(VertexSettings)


using VertexGroup = pex::Group
    <
        VertexFields,
        VertexTemplate,
        VertexSettings
    >;


struct VertexModel: public VertexGroup::Model
{
    VertexModel()
        :
        VertexGroup::Model()
    {
        this->count.SetChoices({2, 4});
    }
};


using VertexControl = typename VertexGroup::Control;

using VertexGroupMaker = pex::MakeGroup<VertexGroup, VertexModel>;


} // end namespace iris


extern template struct pex::Group
    <
        iris::VertexFields,
        iris::VertexTemplate,
        iris::VertexSettings
    >;
