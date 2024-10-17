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


template<typename T>
struct MaskFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::imageSize, "imageSize"),
        fields::Field(&T::enable, "enable"),
        fields::Field(&T::showOutline, "showOutline"),
        fields::Field(&T::polygons, "polygons"),
        fields::Field(&T::feather, "feather"));
};


template<template<typename> typename T>
class MaskTemplate
{
public:
    T<draw::SizeGroup> imageSize;
    T<bool> enable;
    T<bool> showOutline;
    T<draw::OrderedShapes> polygons;
    T<GaussianGroup<double>> feather;

    static constexpr auto fields =
        MaskFields<MaskTemplate>::fields;

    static constexpr auto fieldsTypeName = "Mask";
};


class MaskSettings: public MaskTemplate<pex::Identity>
{
public:
    MaskSettings()
        :
        MaskTemplate<pex::Identity>{
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
    MaskFields,
    MaskTemplate,
    pex::PlainT<MaskSettings>
>;


using MaskModel = typename MaskGroup::Model;
using MaskControl = typename MaskGroup::Control;


} // end namespace iris


extern template struct pex::Group
<
    iris::MaskFields,
    iris::MaskTemplate,
    pex::PlainT<iris::MaskSettings>
>;
