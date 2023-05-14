#pragma once


#include <jive/range.h>
#include <fields/fields.h>
#include <pex/interface.h>
#include <pex/linked_ranges.h>


namespace iris
{


template<typename T>
struct ChessFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::minimumPointsPerLine, "minimumPointsPerLine"),
        fields::Field(&T::maximumPointError, "maximumPointError"),
        fields::Field(&T::angleToleranceDegrees, "angleToleranceDegrees"),
        fields::Field(&T::lineSeparation, "lineSeparation"),
        fields::Field(&T::enableGroup, "enableGroup"),
        fields::Field(&T::groupSeparationDegrees, "groupSeparationDegrees"),
        fields::Field(&T::minimumLinesPerGroup, "minimumLinesPerGroup"),
        fields::Field(&T::spacingLimit, "spacingLimit"),
        fields::Field(&T::rowCount, "rowCount"),
        fields::Field(&T::columnCount, "columnCount"),
        fields::Field(&T::angleFilter, "angleFilter"));
};


struct ChessTemplate
{
    using PointsLow = pex::Limit<3>;
    using PointsHigh = pex::Limit<32>;

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
        T<pex::MakeRange<size_t, PointsLow, PointsHigh>> minimumPointsPerLine;
        T<double> maximumPointError;
        T<double> angleToleranceDegrees;
        T<double> lineSeparation;
        T<bool> enableGroup;
        T<double> groupSeparationDegrees;
        T<size_t> minimumLinesPerGroup;
        T<double> spacingLimit;
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
        static constexpr size_t defaultMinimumPointsPerLine = 5;

        static constexpr double defaultMaximumPointError = 4.0;

        static constexpr double defaultAngleTolerance = 4.0;
        static constexpr double defaultLineSeparation = 4.0;
        static constexpr size_t defaultMinimumLinesPerGroup = 3;
        static constexpr double defaultSpacingLimit = 180.0;
        static constexpr size_t defaultRowCount = 7;
        static constexpr size_t defaultColumnCount = 10;
        static constexpr size_t defaultGroupSeparation_degrees = 20;

        return {{
            defaultMinimumPointsPerLine,
            defaultMaximumPointError,
            defaultAngleTolerance,
            defaultLineSeparation,
            true,
            defaultGroupSeparation_degrees,
            defaultMinimumLinesPerGroup,
            defaultSpacingLimit,
            defaultRowCount,
            defaultColumnCount,
            {{0, 180}}}};
    }
};


DECLARE_OUTPUT_STREAM_OPERATOR(ChessSettings)


using ChessGroup =
    pex::Group
    <
        iris::ChessFields,
        ChessTemplate::template Template,
        ChessSettings
    >;

using ChessModel = typename ChessGroup::Model;

using ChessControl = typename ChessGroup::Control<void>;

template<typename Observer>
using ChessTerminus = typename ChessGroup::Terminus<Observer>;


} // end namespace iris
