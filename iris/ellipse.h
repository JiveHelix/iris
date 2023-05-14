#pragma once

#include <fields/fields.h>
#include <pex/group.h>
#include <pex/range.h>
#include <tau/vector2d.h>
#include "iris/views/scale.h"


namespace iris
{


template<typename T>
struct EllipseFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::center, "center"),
        fields::Field(&T::major, "major"),
        fields::Field(&T::minor, "minor"),
        fields::Field(&T::angle, "angle"),
        fields::Field(&T::scale, "scale"));
};


template<typename Value>
struct EllipseTemplate
{
    using AxisRange = pex::MakeRange<Value, pex::Limit<0>, pex::Limit<1000>>;
    using AngleRange = pex::MakeRange<Value, pex::Limit<0>, pex::Limit<180>>;

    template<template<typename> typename T>
    struct Template
    {
        T<pex::MakeGroup<tau::Point2dGroup<Value>>> center;
        T<AxisRange> major;
        T<AxisRange> minor;
        T<AngleRange> angle;
        T<ScaleRange> scale;

        static constexpr auto fields = EllipseFields<Template>::fields;
    };
};


template<typename Value>
struct Ellipse: public EllipseTemplate<Value>::template Template<pex::Identity>
{
    static Ellipse Default()
    {
        return {{
            {{static_cast<Value>(400), static_cast<Value>(400)}},
            static_cast<Value>(200),
            static_cast<Value>(300),
            static_cast<Value>(45),
            1.0}};
    }
};


template<typename Value>
using EllipseGroup = pex::Group
<
    EllipseFields,
    EllipseTemplate<Value>::template Template,
    Ellipse<Value>
>;

template<typename Value>
using EllipseModel = typename EllipseGroup<Value>::Model;

template<typename Value>
using EllipseControl = typename EllipseGroup<Value>::template Control<void>;

template<typename Value, typename Observer>
using EllipseTerminus =
    typename EllipseGroup<Value>::template Terminus<Observer>;


TEMPLATE_OUTPUT_STREAM(Ellipse)


} // end namespace iris
