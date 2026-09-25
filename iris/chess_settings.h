#pragma once


#include <jive/range.h>
#include <fields/fields.h>
#include <pex/interface.h>
#include <pex/linked_ranges.h>


namespace iris
{


struct ChessSchema
{
    template<template<typename> typename T>
    struct Schema
    {
        T<bool> enable;
        T<double> minimumSpacing;
        T<double> groupSeparationDegrees;
        T<size_t> minimumLinesPerGroup;
        T<double> maximumSpacing;
        T<double> ratioLimit;
        T<size_t> rowCount;
        T<size_t> columnCount;
        T<double> maximumVertexDistance;

        static constexpr auto fieldsTypeName = "Chess";
    };
};


struct ChessFinisher
{
    template<typename Base>
    struct Plain
        :
        public Base
    {
        static constexpr double defaultLineSeparation = 4.0;
        static constexpr size_t defaultMinimumLinesPerGroup = 3;
        static constexpr double defaultSpacingLimit = 200.0;
        static constexpr double defaultSpacingRatioThreshold = 0.1;
        static constexpr size_t defaultRowCount = 16;
        static constexpr size_t defaultColumnCount = 16;
        static constexpr size_t defaultGroupSeparation_degrees = 20;
        static constexpr double defaultMaximumVertexDistance = 4.0;

        Plain()
            :
            Base{
                true,
                defaultLineSeparation,
                defaultGroupSeparation_degrees,
                defaultMinimumLinesPerGroup,
                defaultSpacingLimit,
                defaultSpacingRatioThreshold,
                defaultRowCount,
                defaultColumnCount,
                defaultMaximumVertexDistance}
        {

        }
    };
};


using ChessGroup =
    pex::Group
    <
        ChessSchema::template Schema,
        ChessFinisher
    >;

using ChessSettings = typename ChessGroup::Plain;
using ChessModel = typename ChessGroup::Model;
using ChessControl = typename ChessGroup::DefaultControl;

DECLARE_OUTPUT_STREAM_OPERATOR(ChessSettings)
DECLARE_EQUALITY_OPERATORS(ChessSettings)


} // end namespace iris


extern template struct pex::Group
    <
        iris::ChessSchema::template Schema,
        iris::ChessFinisher
    >;
