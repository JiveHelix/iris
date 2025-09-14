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


struct VertexChoices
{
    using Type = Eigen::Index;

    static std::vector<Eigen::Index> GetChoices()
    {
        return {2, 4};
    }
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
    T<pex::MakeSelect<VertexChoices>> count;
    T<size_t> threads;

    static constexpr auto fields =
        VertexFields<VertexTemplate>::fields;

    static constexpr auto fieldsTypeName = "Vertex";
};


struct VertexCustom
{
    template<typename Base>
    struct Plain: public Base
    {
        static constexpr Eigen::Index defaultWindow = 40;
        static constexpr Eigen::Index defaultCount = 4;
        static constexpr size_t defaultThreads = 4;

        Plain()
            :
            Base{
                true,
                defaultWindow,
                defaultCount,
                defaultThreads}
        {

        }
    };
};



using VertexGroup = pex::Group
    <
        VertexFields,
        VertexTemplate,
        VertexCustom
    >;


using VertexSettings = typename VertexGroup::Plain;
using VertexModel = typename VertexGroup::Model;
using VertexControl = typename VertexGroup::DefaultControl;


DECLARE_OUTPUT_STREAM_OPERATOR(VertexSettings)
DECLARE_EQUALITY_OPERATORS(VertexSettings)


} // end namespace iris


extern template struct pex::Group
    <
        iris::VertexFields,
        iris::VertexTemplate,
        iris::VertexCustom
    >;
