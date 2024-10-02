#pragma once


#include <pex/group.h>
#include <draw/shapes.h>
#include <draw/look.h>
#include <draw/font_look.h>
#include <draw/views/points_shape_view.h>
#include <draw/views/lines_shape_view.h>
#include "iris/chess/chess_solution.h"


namespace iris
{


template<typename T>
struct ChessShapeFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::displayVertices, "displayVertices"),
        fields::Field(&T::labelVertices, "labelVertices"),
        fields::Field(&T::displayHorizontals, "displayHorizontals"),
        fields::Field(&T::displayVerticals, "displayVerticals"),
        fields::Field(&T::verticesShape, "verticesShape"),
        fields::Field(&T::labelsLook, "labelsLook"),
        fields::Field(&T::horizontalsShape, "horizontalsShape"),
        fields::Field(&T::verticalsShape, "verticalsShape"));
};


template<template<typename> typename T>
struct ChessShapeTemplate
{
    T<bool> displayVertices;
    T<bool> labelVertices;
    T<bool> displayHorizontals;
    T<bool> displayVerticals;
    T<draw::PointsShapeGroup> verticesShape;
    T<draw::FontLookGroup> labelsLook;
    T<draw::LinesShapeGroup> horizontalsShape;
    T<draw::LinesShapeGroup> verticalsShape;

    static constexpr auto fields =
        ChessShapeFields<ChessShapeTemplate>::fields;

    static constexpr auto fieldsTypeName = "ChessShape";
};


struct ChessShapeSettings: public ChessShapeTemplate<pex::Identity>
{
    static ChessShapeSettings Default();
};


DECLARE_EQUALITY_OPERATORS(ChessShapeSettings)
DECLARE_OUTPUT_STREAM_OPERATOR(ChessShapeSettings)


class ChessShape
    :
    public draw::DrawnShape
{
public:
    ChessShape() = default;

    ChessShape(
        const ChessShapeSettings &settings,
        const ChessSolution &chessSolution);

    void Draw(draw::DrawContext &context) override;

    ChessShapeSettings settings_;
    ChessSolution chessSolution_;
};


using ChessShapeGroup = pex::Group
<
    ChessShapeFields,
    ChessShapeTemplate,
    pex::PlainT<ChessShapeSettings>
>;

using ChessShapeModel = typename ChessShapeGroup::Model;
using ChessShapeControl = typename ChessShapeGroup::Control;


} // end namespace iris



extern template struct pex::Group
<
    iris::ChessShapeFields,
    iris::ChessShapeTemplate,
    pex::PlainT<iris::ChessShapeSettings>
>;
