#pragma once


#include <string>
#include <fields/fields.h>
#include <pex/linked_ranges.h>
#include "iris/size.h"


namespace iris
{


template<typename T>
struct WaveformFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::enable, "enable"),
        fields::Field(&T::levelCount, "levelCount"),
        fields::Field(&T::columnCount, "columnCount"),
        fields::Field(&T::verticalScale, "verticalScale"),
        fields::Field(&T::brightness, "brightness"),
        fields::Field(&T::brightnessCount, "brightnessCount"),
        fields::Field(&T::displayedSize, "displayedSize"));
};

using LowBrightness = pex::Limit<0, 4, 10>;
using HighBrightness = pex::Limit<0, 7, 10>;

using BrightnessRanges =
    pex::LinkedRanges
    <
        double,
        pex::Limit<0>,
        pex::Limit<0, 4, 10>,
        pex::Limit<1>,
        pex::Limit<0, 7, 10>
    >;

template<template<typename> typename T>
struct WaveformTemplate
{
    T<bool> enable;
    T<pex::MakeRange<size_t, pex::Limit<1>, pex::Limit<1024>>> levelCount;
    T<pex::MakeRange<size_t, pex::Limit<1>, pex::Limit<1920>>> columnCount;
    T<pex::MakeRange<double, pex::Limit<1>, pex::Limit<10>>> verticalScale;
    T<BrightnessRanges::GroupMaker> brightness;
    T<pex::MakeRange<size_t, pex::Limit<1>, pex::Limit<256>>> brightnessCount;
    T<Size> displayedSize;

    static constexpr auto fields = WaveformFields<WaveformTemplate>::fields;
    static constexpr auto fieldsTypeName = "Waveform";
};


struct WaveformSettings: public WaveformTemplate<pex::Identity>
{
    static constexpr size_t defaultLevelCount = 200;
    static constexpr size_t defaultColumnCount = 400;
    static constexpr double defaultVerticalZoom = 1.0;
    static constexpr size_t defaultBrightnessCount = 20;
    static constexpr Size defaultDisplayedSize{200, 300};

    static WaveformSettings Default()
    {
        return {{
            true,
            defaultLevelCount,
            defaultColumnCount,
            defaultVerticalZoom,
            BrightnessRanges::Settings::Default(),
            defaultBrightnessCount,
            defaultDisplayedSize}};
    }
};


using WaveformGroup =
    pex::Group<WaveformFields, WaveformTemplate, WaveformSettings>;

using WaveformControl = typename WaveformGroup::Control<void>;

template<typename Observer>
using WaveformTerminus = typename WaveformGroup::Terminus<Observer>;

using WaveformGroupMaker = pex::MakeGroup<WaveformGroup>;


} // end namespace iris
