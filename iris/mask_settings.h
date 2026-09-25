#pragma once


#include <fields/fields.h>
#include <pex/group.h>
#include <draw/polygon.h>
#include <draw/size.h>
#include <draw/shapes.h>
#include <draw/polygon_shape.h>
#include "iris/gaussian.h"
#include "iris/default.h"


namespace iris
{


template<template<typename> typename T>
class MaskSchema
{
public:
    T<draw::SizeGroup> imageSize;
    T<bool> enable;
    T<bool> showOutline;
    T<draw::OrderedShapes> polygons;
    T<GaussianGroup<double>> feather;

    static constexpr auto fieldsTypeName = "Mask";
};


class MaskSettings: public MaskSchema<pex::Identity>
{
public:
    MaskSettings()
        :
        MaskSchema<pex::Identity>{
            defaultImageSize,
            true,
            true,
            {},
            {}}
    {
        this->feather.sigma = 10.0;
    }
};


DECLARE_OUTPUT_STREAM_OPERATOR(MaskSettings)
DECLARE_EQUALITY_OPERATORS(MaskSettings)


using MaskGroup = pex::Group
<
    MaskSchema,
    pex::PlainT<MaskSettings>
>;


using MaskModel = typename MaskGroup::Model;
using MaskControl = typename MaskGroup::DefaultControl;


} // end namespace iris


extern template struct pex::Group
<
    iris::MaskSchema,
    pex::PlainT<iris::MaskSettings>
>;
