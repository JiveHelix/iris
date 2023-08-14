#pragma once

#include <fields/fields.h>
#include <pex/group.h>
#include <pex/endpoint.h>

#include <draw/size.h>
#include <draw/scale.h>
#include <draw/point.h>


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
    T<pex::MakeGroup<draw::SizeGroup>> imageSize;
    T<pex::MakeGroup<draw::SizeGroup>> viewSize;
    T<pex::MakeGroup<draw::PointGroup>> viewPosition;
    T<pex::MakeGroup<draw::ScaleGroup>> scale;
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
            draw::Size{{defaultWidth, defaultHeight}},
            draw::Size{{defaultWidth, defaultHeight}},
            draw::Point{{0, 0}},
            draw::Scale(1.0, 1.0),
            true,
            {},
            {}}};
    }

    // Compute the coordinates of an unscaled point using current zoom.
    draw::Point GetLogicalPosition(const draw::Point &point) const;
};


using ViewSettingsGroup = pex::Group<ViewFields, ViewTemplate, ViewSettings>;


DECLARE_OUTPUT_STREAM_OPERATOR(ViewSettings)


struct ViewSettingsModel: public ViewSettingsGroup::Model
{
public:
    static constexpr auto observerName = "ViewSettingsModel";

private:
    using ScaleEndpoint =
        pex::EndpointGroup<ViewSettingsModel, draw::ScaleControl>;

    pex::Endpoint<ViewSettingsModel, draw::PointControl> viewPositionEndpoint_;
    ScaleEndpoint scaleEndpoint_;
    pex::Endpoint<ViewSettingsModel, draw::SizeControl> imageSizeEndpoint_;
    pex::Terminus<ViewSettingsModel, pex::model::Value<bool>> linkZoomTerminus_;
    pex::Terminus<ViewSettingsModel, pex::model::Signal> resetZoomTerminus_;
    pex::Terminus<ViewSettingsModel, pex::model::Signal> fitZoomTerminus_;
    tau::Point2d<double> imageCenterPixel_;
    bool ignoreZoom_;
    bool ignoreViewPosition_;

public:
    ViewSettingsModel();

    void SetImageCenterPixel_(const tau::Point2d<double> &point);

    tau::Point2d<double> ComputeImageCenterPixel() const;

    draw::Point GetViewPositionFromCenterImagePixel() const;

    void ResetZoom();

    void FitZoom();

    void RecenterView();

private:
    void SetViewPosition_(const draw::Point &);

    void OnHorizontalZoom_(double horizontalZoom);

    void OnVerticalZoom_(double verticalZoom);

    void OnLinkZoom_(bool isLinked);

    void OnViewPosition_(const draw::Point &);

    void OnImageSize_(const Size &);

    void ResetView_(const Size &imageSize);
};


using ViewSettingsGroupMaker =
    pex::MakeGroup<ViewSettingsGroup, ViewSettingsModel>;

using ViewSettingsControl = typename ViewSettingsGroup::Control;


} // end namespace iris



extern template struct pex::Group
    <
        iris::ViewFields,
        iris::ViewTemplate,
        iris::ViewSettings
    >;


extern template struct pex::MakeGroup
    <
        iris::ViewSettingsGroup,
        iris::ViewSettingsModel
    >;
