#pragma once


#include <jive/range.h>
#include <fields/fields.h>
#include <pex/interface.h>
#include <pex/linked_ranges.h>


namespace iris
{


template<typename T>
struct VertexChessFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::minimumPointsPerLine, "minimumPointsPerLine"),
        fields::Field(&T::maximumPointError, "maximumPointError"),
        fields::Field(&T::angleToleranceDegrees, "angleToleranceDegrees"));
};


template<template<typename> typename T>
struct VertexChessTemplate
{
    using PointsLow = pex::Limit<3>;
    using PointsHigh = pex::Limit<32>;

    T<pex::MakeRange<size_t, PointsLow, PointsHigh>> minimumPointsPerLine;
    T<double> maximumPointError;
    T<double> angleToleranceDegrees;

    static constexpr auto fields =
        VertexChessFields<VertexChessTemplate>::fields;

    static constexpr auto fieldsTypeName = "VertexChess";
};


struct VertexChessSettings
    :
    public VertexChessTemplate<pex::Identity>
{
    static VertexChessSettings Default()
    {
        static constexpr size_t defaultMinimumPointsPerLine = 4;
        static constexpr double defaultMaximumPointError = 4.0;
        static constexpr double defaultAngleTolerance = 4.0;

        return {{
            defaultMinimumPointsPerLine,
            defaultMaximumPointError,
            defaultAngleTolerance}};
    }
};


DECLARE_EQUALITY_OPERATORS(VertexChessSettings)
DECLARE_OUTPUT_STREAM_OPERATOR(VertexChessSettings)


using VertexChessGroup =
    pex::Group
    <
        VertexChessFields,
        VertexChessTemplate,
        VertexChessSettings
    >;

using VertexChessControl = typename VertexChessGroup::Control;

using VertexChessGroupMaker = pex::MakeGroup<VertexChessGroup>;


template<typename T>
struct ChessFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::enable, "enable"),
        fields::Field(&T::useVertices, "useVertices"),
        fields::Field(&T::vertexChess, "vertexChess"),
        fields::Field(&T::minimumSpacing, "minimumSpacing"),
        fields::Field(&T::enableGroup, "enableGroup"),
        fields::Field(&T::groupSeparationDegrees, "groupSeparationDegrees"),
        fields::Field(&T::minimumLinesPerGroup, "minimumLinesPerGroup"),
        fields::Field(&T::maximumSpacing, "maximumSpacing"),
        fields::Field(&T::ratioLimit, "ratioLimit"),
        fields::Field(&T::rowCount, "rowCount"),
        fields::Field(&T::columnCount, "columnCount"),
        fields::Field(&T::angleFilter, "angleFilter"));
};


struct ChessTemplate
{
    using AngleFilterLow = pex::Limit<0>;
    using AngleFilterHigh = pex::Limit<180>;

    using AngleFilterRanges =
        pex::LinkedRanges
        <
            double,
            AngleFilterLow,
            AngleFilterLow,
            AngleFilterHigh,
            AngleFilterHigh
        >;

    template<template<typename> typename T>
    struct Template
    {
        T<bool> enable;
        T<bool> useVertices;
        T<VertexChessGroupMaker> vertexChess;
        T<double> minimumSpacing;
        T<bool> enableGroup;
        T<double> groupSeparationDegrees;
        T<size_t> minimumLinesPerGroup;
        T<double> maximumSpacing;
        T<double> ratioLimit;
        T<size_t> rowCount;
        T<size_t> columnCount;
        T<AngleFilterRanges::GroupMaker> angleFilter;

        static constexpr auto fields = ChessFields<Template>::fields;
        static constexpr auto fieldsTypeName = "Chess";
    };
};


struct ChessSettings
    :
    public ChessTemplate::template Template<pex::Identity>
{
    static ChessSettings Default()
    {
        static constexpr double defaultLineSeparation = 4.0;
        static constexpr size_t defaultMinimumLinesPerGroup = 3;
        static constexpr double defaultSpacingLimit = 150.0;
        static constexpr double defaultSpacingRatioThreshold = .2;
        static constexpr size_t defaultRowCount = 8;
        static constexpr size_t defaultColumnCount = 12;
        static constexpr size_t defaultGroupSeparation_degrees = 20;

        return {{
            true,
            true,
            VertexChessSettings::Default(),
            defaultLineSeparation,
            true,
            defaultGroupSeparation_degrees,
            defaultMinimumLinesPerGroup,
            defaultSpacingLimit,
            defaultSpacingRatioThreshold,
            defaultRowCount,
            defaultColumnCount,
            {{0, 180}}}};
    }
};


DECLARE_OUTPUT_STREAM_OPERATOR(ChessSettings)
DECLARE_EQUALITY_OPERATORS(ChessSettings)


using ChessGroup =
    pex::Group
    <
        ChessFields,
        ChessTemplate::template Template,
        ChessSettings
    >;

using ChessModel = typename ChessGroup::Model;

using ChessControl = typename ChessGroup::Control;

using ChessGroupMaker = pex::MakeGroup<ChessGroup>;


} // end namespace iris


extern template struct pex::Group
    <
        iris::VertexChessFields,
        iris::VertexChessTemplate,
        iris::VertexChessSettings
    >;


extern template struct pex::Group
    <
        iris::ChessFields,
        iris::ChessTemplate::template Template,
        iris::ChessSettings
    >;
