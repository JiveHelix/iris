#pragma once


#include <optional>
#include <fields/fields.h>
#include <pex/group.h>
#include <tau/color.h>


template<typename T>
struct NodeSettingsFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::select, "select"),
        fields::Field(&T::highlight, "highlight"),
        fields::Field(&T::highlightColor, "highlightColor"));
};



template<template<typename> typename T>
struct NodeSettingsTemplate
{
    T<pex::MakeSignal> select;
    T<bool> highlight;
    T<pex::MakeGroup<tau::HsvGroup<double>>> highlightColor;

    static constexpr auto fields =
        NodeSettingsFields<NodeSettingsTemplate>::fields;
};


struct NodeSettings: public NodeSettingsTemplate<pex::Identity>
{
    static NodeSettings Default()
    {
        static const auto darkGreen = tau::Hsv<double>{{136.0, 0.57, 0.36}};

        return {{
            {},
            false,
            darkGreen}};

    }
};


DECLARE_EQUALITY_OPERATORS(NodeSettings)
DECLARE_OUTPUT_STREAM_OPERATOR(NodeSettings)


using NodeSettingsGroup =
    pex::Group
    <
        NodeSettingsFields,
        NodeSettingsTemplate,
        NodeSettings
    >;

using NodeSettingsModel = typename NodeSettingsGroup::Model;
using NodeSettingsControl = typename NodeSettingsGroup::Control;
using NodeSettingsGroupMaker = pex::MakeGroup<NodeSettingsGroup>;


using OptionalNodeSettings = std::optional<NodeSettingsControl>;
