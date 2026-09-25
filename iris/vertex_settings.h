#pragma once


#include <fields/fields.h>
#include <pex/group.h>
#include <tau/eigen_shim.h>


namespace iris
{


struct VertexChoices
{
    using Type = Eigen::Index;

    static std::vector<Eigen::Index> GetChoices()
    {
        return {2, 4};
    }
};


template<template<typename> typename T>
struct VertexSchema
{
    using WindowLow = pex::Limit<3>;
    using WindowHigh = pex::Limit<64>;

    using CountLow = pex::Limit<1>;
    using CountHigh = pex::Limit<4>;

    T<bool> enable;
    T<pex::MakeRange<double, WindowLow, WindowHigh>> window;
    T<pex::MakeSelect<VertexChoices>> count;

    static constexpr auto fieldsTypeName = "Vertex";
};


struct VertexFinisher
{
    template<typename Base>
    struct Plain: public Base
    {
        static constexpr Eigen::Index defaultWindow = 40;
        static constexpr Eigen::Index defaultCount = 4;

        Plain()
            :
            Base{
                true,
                defaultWindow,
                defaultCount}
        {

        }
    };
};



using VertexGroup = pex::Group
    <
        VertexSchema,
        VertexFinisher
    >;


using VertexSettings = typename VertexGroup::Plain;
using VertexModel = typename VertexGroup::Model;
using VertexControl = typename VertexGroup::DefaultControl;


DECLARE_OUTPUT_STREAM_OPERATOR(VertexSettings)
DECLARE_EQUALITY_OPERATORS(VertexSettings)


} // end namespace iris


extern template struct pex::Group
    <
        iris::VertexSchema,
        iris::VertexFinisher
    >;
