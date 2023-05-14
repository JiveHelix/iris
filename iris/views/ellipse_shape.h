#pragma once


#include <pex/group.h>
#include "iris/ellipse.h"
#include "iris/views/look.h"
#include "iris/views/shapes.h"


namespace iris
{


template<typename T>
struct EllipseShapeFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::ellipse, "ellipse"),
        fields::Field(&T::look, "look"));
};


template<template<typename> typename T>
class EllipseShapeTemplate
{
public:
    T<pex::MakeGroup<iris::EllipseGroup<double>>> ellipse;
    T<pex::MakeGroup<iris::LookGroup>> look;

    static constexpr auto fields =
        EllipseShapeFields<EllipseShapeTemplate>::fields;

    static constexpr auto fieldsTypeName = "EllipseShape";
};


class EllipseShape:
    public Shape,
    public EllipseShapeTemplate<pex::Identity>
{
public:
    EllipseShape() = default;

    EllipseShape(const Ellipse<double> &ellipse_, const Look &look_)
        :
        EllipseShapeTemplate<pex::Identity>({ellipse_, look_})
    {

    }

    void Draw(wxpex::GraphicsContext &context) override
    {
        wxpex::MaintainTransform transform(context);

        if (this->look.antialias)
        {
            context->SetAntialiasMode(wxANTIALIAS_DEFAULT);
        }
        else
        {
            context->SetAntialiasMode(wxANTIALIAS_NONE);
        }

        if (this->look.strokeEnable)
        {
            context->SetPen(
                context->CreatePen(
                    wxPen(
                        wxpex::ToWxColour(this->look.strokeColor),
                        this->look.strokeWeight)));
        }
        else
        {
            context->SetPen(context->CreatePen(*wxTRANSPARENT_PEN));
        }

        if (this->look.fillEnable)
        {
            context->SetBrush(
                context->CreateBrush(
                    wxBrush(
                        wxpex::ToWxColour(this->look.fillColor))));
        }
        else
        {
            context->SetBrush(context->CreateBrush(*wxTRANSPARENT_BRUSH));
        }

        context->Translate(this->ellipse.center.x, this->ellipse.center.y);
        context->Rotate(tau::ToRadians(this->ellipse.angle));

        double ellipseMajor = this->ellipse.scale * this->ellipse.major;
        double ellipseMinor = this->ellipse.scale * this->ellipse.minor;

        context->DrawEllipse(
            -ellipseMajor / 2.0,
            -ellipseMinor / 2.0,
            ellipseMajor,
            ellipseMinor);
    }
};


using EllipseShapeGroup = pex::Group
<
    EllipseShapeFields,
    EllipseShapeTemplate,
    EllipseShape
>;

using EllipseShapeModel = typename EllipseShapeGroup::Model;
using EllipseShapeControl = typename EllipseShapeGroup::template Control<void>;

template<typename Observer>
using EllipseShapeTerminus =
    typename EllipseShapeGroup::template Terminus<Observer>;


} // end namespace iris
