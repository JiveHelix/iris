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
    T<draw::SizeGroup> imageSize;
    T<draw::SizeGroup> viewSize;
    T<draw::PointGroup> viewPosition;
    T<draw::ScaleGroup> scale;
    T<bool> linkZoom;
    T<pex::MakeSignal> resetZoom;
    T<pex::MakeSignal> fitZoom;

    static constexpr auto fields = ViewFields<ViewTemplate>::fields;
};


struct ViewSettingsCustom
{
    template<typename Base>
    struct ViewSettings: public Base
    {
        static constexpr int defaultWidth = 1920;
        static constexpr int defaultHeight = 1080;

        ViewSettings()
            :
            Base{
                draw::Size{{defaultWidth, defaultHeight}},
                draw::Size{{defaultWidth, defaultHeight}},
                draw::Point{{0, 0}},
                draw::Scale(1.0, 1.0),
                true,
                {},
                {}}
        {

        }

        static ViewSettings Default()
        {
            return ViewSettings();
        }

        // Compute the coordinates of the unscaled pixel using current zoom.
        draw::Point GetLogicalPosition(const draw::Point &point) const
        {
            return (point + this->viewPosition) / this->scale;
        }
    };

    template<typename ModelBase>
    struct Model: public ModelBase
    {
    public:
        static constexpr auto observerName = "ViewSettingsModel";

    private:
        using ScaleEndpoint =
            pex::EndpointGroup<Model, draw::ScaleControl>;

        pex::Endpoint<Model, draw::PointControl> viewPositionEndpoint_;
        ScaleEndpoint scaleEndpoint_;
        pex::Endpoint<Model, draw::SizeControl> imageSizeEndpoint_;
        pex::Terminus<Model, pex::model::Value<bool>> linkZoomTerminus_;
        pex::Terminus<Model, pex::model::Signal> resetZoomTerminus_;
        pex::Terminus<Model, pex::model::Signal> fitZoomTerminus_;
        tau::Point2d<double> imageCenterPixel_;
        bool ignoreZoom_;
        bool ignoreViewPosition_;

    public:
        Model()
            :
            ViewSettingsGroup::Model(),
            viewPositionEndpoint_(
                this,
                draw::PointControl(this->viewPosition),
                &Model::OnViewPosition_),
            scaleEndpoint_(
                this,
                draw::ScaleControl(this->scale)),
            imageSizeEndpoint_(this, this->imageSize, &Model::OnImageSize_),
            linkZoomTerminus_(this, this->linkZoom),
            resetZoomTerminus_(this, this->resetZoom),
            fitZoomTerminus_(this, this->fitZoom),
            ignoreZoom_(false),
            ignoreViewPosition_(false)
        {
            this->scaleEndpoint_.horizontal.Connect(
                &Model::OnHorizontalZoom_);

            this->scaleEndpoint_.vertical.Connect(
                &Model::OnVerticalZoom_);

            this->linkZoomTerminus_.Connect(&Model::OnLinkZoom_);
            this->resetZoomTerminus_.Connect(&Model::ResetZoom);
            this->fitZoomTerminus_.Connect(&Model::FitZoom);

            this->ResetView_(this->imageSize.Get());
        }

        void SetImageCenterPixel_(const tau::Point2d<double> &point)
        {
            this->imageCenterPixel_ = point;
        }

        tau::Point2d<double> ComputeImageCenterPixel() const
        {
            auto viewPosition_ = this->viewPosition.Get().template Convert<double>();

            auto halfView =
                this->viewSize.Get().ToPoint2d().template Convert<double>() / 2.0;

            auto viewCenterPixel = viewPosition_ + halfView;

            auto imageCenterPixel = viewCenterPixel / this->scale.Get();

            auto asIntegers = imageCenterPixel.template Convert<int, tau::Floor>();
            auto size = this->imageSize.Get();

            if (asIntegers.x >= size.width)
            {
                imageCenterPixel.x = static_cast<double>(size.width - 1);
            }
            else if (asIntegers.x < 0)
            {
                imageCenterPixel.x = 0.0;
            }

            if (asIntegers.y >= size.height)
            {
                imageCenterPixel.y = static_cast<double>(size.height - 1);
            }
            else if (asIntegers.y < 0)
            {
                imageCenterPixel.y = 0.0;
            }

            return imageCenterPixel;
        }

        draw::Point GetViewPositionFromCenterImagePixel() const
        {
            auto scaledCenterPixel = this->imageCenterPixel_ * this->scale.Get();

            auto halfView =
                this->viewSize.Get().template Convert<double>().ToPoint2d() / 2.0;

            auto result = (scaledCenterPixel - halfView)
                .template Convert<int, tau::Floor>();

            return result;
        }

        void ResetZoom()
        {
            this->scale.Set(draw::Scale{});
            this->ResetView_(this->imageSize.Get());
        }

        void FitZoom()
        {
            auto imageSize_ = tau::Size<double>(this->imageSize.Get());
            auto viewSize_ = this->viewSize.Get().template Convert<double>();

            // imageSize_ * fit = viewSize_
            viewSize_ /= imageSize_;
            auto fit = draw::Scale{viewSize_.height, viewSize_.width};

            auto scaleDeferred =
                pex::Defer<decltype(this->scale)>(this->scale);

            if (this->linkZoom.Get())
            {
                double smaller = std::min(fit.horizontal, fit.vertical);
                fit.horizontal = smaller;
                fit.vertical = smaller;
                scaleDeferred.Set(fit);
            }
            else
            {
                // Fit horizontal and vertical zoom independently
                scaleDeferred.Set(fit);
            }

            // Reset the imageCenterPixel_ before the zoom scale is notified.
            this->ResetView_(this->imageSize.Get());
        }

        void RecenterView()
        {
            this->SetViewPosition_(this->GetViewPositionFromCenterImagePixel());
        }

    private:
        void OnHorizontalZoom_(double horizontalZoom)
        {
            if (this->ignoreZoom_)
            {
                return;
            }

            if (this->linkZoom.Get())
            {
                this->ignoreZoom_ = true;
                this->scale.vertical.Set(horizontalZoom);
                this->ignoreZoom_ = false;
            }

            this->RecenterView();
        }

        void OnVerticalZoom_(double verticalZoom)
        {
            if (this->ignoreZoom_)
            {
                return;
            }

            if (this->linkZoom.Get())
            {
                this->ignoreZoom_ = true;
                this->scale.horizontal.Set(verticalZoom);
                this->ignoreZoom_ = false;
            }

            this->RecenterView();
        }

        void OnLinkZoom_(bool isLinked)
        {
            if (isLinked)
            {
                this->ignoreZoom_ = true;
                this->scale.vertical.Set(this->scale.horizontal.Get());
                this->ignoreZoom_ = false;
            }
        }

        void OnViewPosition_(const draw::Point &)
        {
            if (this->ignoreViewPosition_)
            {
                return;
            }

            this->SetImageCenterPixel_(this->ComputeImageCenterPixel());
        }

        void OnImageSize_(const Size &imageSize_)
        {
            // Reset the view when the image size changes.
            this->ResetView_(imageSize_);
        }

        void ResetView_(const Size &imageSize_)
        {
            this->SetImageCenterPixel_(
                imageSize_.template Convert<double>().ToPoint2d() / 2.0);

            this->RecenterView();
        }

        void SetViewPosition_(const draw::Point &point)
        {
            // We need to notify observers of the change to view position without
            // calling our own handler `OnViewPosition_`
            this->ignoreViewPosition_ = true;
            this->viewPosition.Set(point);
            this->ignoreViewPosition_ = false;
        }
    };
};


using ViewSettingsGroup =
    pex::Group<ViewFields, ViewTemplate, ViewSettingsCustom>;


using ViewSettings = typename ViewSettingsGroup::Plain;

DECLARE_OUTPUT_STREAM_OPERATOR(ViewSettings)
DECLARE_EQUALITY_OPERATORS(ViewSettings)

using ViewSettingsModel = typename ViewSettingsGroup::Model;
using ViewSettingsControl = typename ViewSettingsGroup::Control;


} // end namespace iris


extern template struct pex::Group
    <
        iris::ViewFields,
        iris::ViewTemplate,
        iris::ViewSettingsCustom
    >;
