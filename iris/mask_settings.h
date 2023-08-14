#pragma once


#include <fields/fields.h>
#include <pex/group.h>
#include <draw/polygon.h>
#include <draw/size.h>
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
        fields::Field(&T::polygon, "polygon"),
        fields::Field(&T::feather, "feather"));
};


template<template<typename> typename T>
class MaskTemplate
{
public:
    T<draw::SizeGroupMaker> imageSize;
    T<bool> enable;
    T<bool> showOutline;
    T<draw::PolygonGroupMaker> polygon;
    T<GaussianGroupMaker<double>> feather;

    static constexpr auto fields =
        MaskFields<MaskTemplate>::fields;

    static constexpr auto fieldsTypeName = "Mask";
};


class MaskSettings: public MaskTemplate<pex::Identity>
{
public:
    static MaskSettings Default()
    {
        auto defaultGaussian = GaussianSettings<double>::Default();
        defaultGaussian.sigma = 10.0;

        return {{
            defaultImageSize,
            true,
            true,
            draw::Polygon::Default(),
            defaultGaussian}};
    }
};


DECLARE_OUTPUT_STREAM_OPERATOR(MaskSettings)
DECLARE_EQUALITY_OPERATORS(MaskSettings)


using MaskGroup = pex::Group
<
    MaskFields,
    MaskTemplate,
    MaskSettings
>;


using MaskModel = typename MaskGroup::Model;
using MaskControl = typename MaskGroup::Control;


using MaskGroupMaker = pex::MakeGroup<MaskGroup>;


} // end namespace iris


extern template struct pex::Group
<
    iris::MaskFields,
    iris::MaskTemplate,
    iris::MaskSettings
>;
