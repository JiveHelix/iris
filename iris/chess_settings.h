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
        fields::Field(&T::enable, "enable"),
        fields::Field(&T::minimumSpacing, "minimumSpacing"),
        fields::Field(&T::groupSeparationDegrees, "groupSeparationDegrees"),
        fields::Field(&T::minimumLinesPerGroup, "minimumLinesPerGroup"),
        fields::Field(&T::maximumSpacing, "maximumSpacing"),
        fields::Field(&T::ratioLimit, "ratioLimit"),
        fields::Field(&T::rowCount, "rowCount"),
        fields::Field(&T::columnCount, "columnCount"),
        fields::Field(&T::maximumVertexDistance, "maximumVertexDistance"));
};


struct ChessTemplate
{
    template<template<typename> typename T>
    struct Template
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

        static constexpr auto fields = ChessFields<Template>::fields;
        static constexpr auto fieldsTypeName = "Chess";
    };
};


struct ChessCustom
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
        ChessFields,
        ChessTemplate::template Template,
        ChessCustom
    >;

using ChessSettings = typename ChessGroup::Plain;
using ChessModel = typename ChessGroup::Model;
using ChessControl = typename ChessGroup::Control;

DECLARE_OUTPUT_STREAM_OPERATOR(ChessSettings)
DECLARE_EQUALITY_OPERATORS(ChessSettings)


} // end namespace iris


extern template struct pex::Group
    <
        iris::ChessFields,
        iris::ChessTemplate::template Template,
        iris::ChessCustom
    >;
