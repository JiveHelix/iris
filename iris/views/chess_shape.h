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


template<template<typename> typename T>
struct ChessShapeSchema
{
    T<bool> displayVertices;
    T<bool> labelVertices;
    T<bool> displayHorizontals;
    T<bool> displayVerticals;
    T<draw::PointsShapeGroup> verticesShape;
    T<draw::FontLookGroup> labelsLook;
    T<draw::LinesShapeGroup> horizontalsShape;
    T<draw::LinesShapeGroup> verticalsShape;

    static constexpr auto fieldsTypeName = "ChessShape";
};


struct ChessShapeSettings: public ChessShapeSchema<pex::Identity>
{
    ChessShapeSettings();
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
    ChessShapeSchema,
    pex::PlainT<ChessShapeSettings>
>;

using ChessShapeModel = typename ChessShapeGroup::Model;
using ChessShapeControl = typename ChessShapeGroup::DefaultControl;


} // end namespace iris



extern template struct pex::Group
<
    iris::ChessShapeSchema,
    pex::PlainT<iris::ChessShapeSettings>
>;
