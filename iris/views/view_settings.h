#pragma once

#include <fields/fields.h>
#include <pex/group.h>

#include "iris/size.h"
#include "iris/views/scale.h"
#include "iris/views/point.h"


namespace iris
{


template<typename T>
struct ViewFields
{
    static constexpr auto fields = std::make_tuple(
        fields::Field(&T::imageSize, "imageSize"),
        fields::Field(&T::viewSize, "viewSize"),
        fields::Field(&T::viewPosition, "viewPosition"),
        fields::Field(&T::scale, "scale"),
        fields::Field(&T::linkZoom, "linkZoom"),
        fields::Field(&T::resetZoom, "resetZoom"),
        fields::Field(&T::fitZoom, "fitZoom"));
};


template<template<typename> typename T>
struct ViewTemplate
{
    T<pex::MakeGroup<SizeGroup>> imageSize;
    T<pex::MakeGroup<SizeGroup>> viewSize;
    T<pex::MakeGroup<PointGroup>> viewPosition;
    T<pex::MakeGroup<ScaleGroup>> scale;
    T<bool> linkZoom;
    T<pex::MakeSignal> resetZoom;
    T<pex::MakeSignal> fitZoom;

    static constexpr auto fields = ViewFields<ViewTemplate>::fields;
};


struct ViewSettings: public ViewTemplate<pex::Identity>
{
    static constexpr int defaultWidth = 1920;
    static constexpr int defaultHeight = 1080;

    static ViewSettings Default()
    {
        return ViewSettings{{
            Size{{defaultWidth, defaultHeight}},
            Size{{defaultWidth, defaultHeight}},
            Point{{0, 0}},
            Scale(1.0, 1.0),
            true,
            {},
            {}}};
    }

    // Compute the coordinates of an unscaled point using current zoom.
    Point GetLogicalPosition(const Point &point) const;
};


using ViewSettingsGroup = pex::Group<ViewFields, ViewTemplate, ViewSettings>;


DECLARE_OUTPUT_STREAM_OPERATOR(ViewSettings)


struct ViewSettingsModel: public ViewSettingsGroup::Model
{
public:
    static constexpr auto observerName = "ViewSettingsModel";

private:
    pex::Terminus<ViewSettingsModel, pex::model::Value<bool>> linkZoomTerminus_;
    PointGroup::Terminus<ViewSettingsModel> viewPositionTerminus_;
    ScaleGroup::Terminus<ViewSettingsModel> scaleTerminus_;
    SizeGroup::Terminus<ViewSettingsModel> imageSizeTerminus_;
    pex::Terminus<ViewSettingsModel, pex::model::Signal> resetZoomTerminus_;
    pex::Terminus<ViewSettingsModel, pex::model::Signal> fitZoomTerminus_;
    tau::Point2d<double> imageCenterPixel_;
    bool ignoreZoom_;
    bool ignoreViewPosition_;

public:
    ViewSettingsModel();

    void SetImageCenterPixel_(const tau::Point2d<double> &point);

    tau::Point2d<double> ComputeImageCenterPixel() const;

    Point GetViewPositionFromCenterImagePixel() const;

    void ResetZoom();

    void FitZoom();

    void RecenterView();

private:
    void SetViewPosition_(const Point &);

    void OnHorizontalZoom_(double horizontalZoom);

    void OnVerticalZoom_(double verticalZoom);

    void OnLinkZoom_(bool isLinked);

    void OnViewPosition_(const Point &);

    void OnImageSize_(const Size &);

    void ResetView_(const Size &imageSize);
};


template<typename Observer>
using ViewSettingsTerminus = typename ViewSettingsGroup::Terminus<Observer>;

using ViewSettingsGroupMaker =
    pex::MakeGroup<ViewSettingsGroup, ViewSettingsModel>;

using ViewSettingsControl = typename ViewSettingsGroup::Control<void>;


} // end namespace iris
