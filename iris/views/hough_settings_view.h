#pragma once


#include <wxpex/collapsible.h>
#include <wxpex/labeled_widget.h>
#include <wxpex/slider.h>
#include <wxpex/radio_box.h>
#include <wxpex/view.h>

#include "iris/hough_settings.h"


namespace iris
{


template<typename Value>
class HoughSettingsView: public wxpex::Collapsible
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    HoughSettingsView(
        wxWindow *parent,
        HoughControl<Value> controls,
        const LayoutOptions &layoutOptions = LayoutOptions{})
        :
        wxpex::Collapsible(parent, "Hough")
    {
        using namespace wxpex;

        auto enable = wxpex::LabeledWidget(
            this->GetPane(),
            "Enable",
            new wxpex::CheckBox(this->GetPane(), "", controls.enable));

        auto rhoCount = wxpex::LabeledWidget(
            this->GetPane(),
            "rho count",
            new wxpex::Field(this->GetPane(), controls.rhoCount));

        auto thetaCount = wxpex::LabeledWidget(
            this->GetPane(),
            "theta count",
            new wxpex::Field(this->GetPane(), controls.thetaCount));

        auto angleRange = LabeledWidget(
            this->GetPane(),
            "Angle range",
            new ValueSlider(
                this->GetPane(),
                controls.angleRange,
                controls.angleRange.value));

        auto weighted = wxpex::LabeledWidget(
            this->GetPane(),
            "Weighted",
            new wxpex::CheckBox(this->GetPane(), "", controls.weighted));

        auto suppress = wxpex::LabeledWidget(
            this->GetPane(),
            "Suppress",
            new wxpex::CheckBox(this->GetPane(), "", controls.suppress));

        auto window = LabeledWidget(
            this->GetPane(),
            "Window",
            new ValueSlider(
                this->GetPane(),
                controls.window,
                controls.window.value));

        auto threshold = LabeledWidget(
            this->GetPane(),
            "Threshold",
            new ValueSlider(
                this->GetPane(),
                controls.threshold,
                controls.threshold.value));

        auto fake = wxpex::LabeledWidget(
            this->GetPane(),
            "fake",
            new wxpex::CheckBox(this->GetPane(), "", controls.fakeLines));

        auto fakeCount = LabeledWidget(
            this->GetPane(),
            "fake count",
            new ValueSlider(
                this->GetPane(),
                controls.fakeCount,
                controls.fakeCount.value));

        auto threads = wxpex::LabeledWidget(
            this->GetPane(),
            "Threads",
            new wxpex::Field(this->GetPane(), controls.threads));

        auto sizer = LayoutLabeled(
            layoutOptions,
            enable,
            rhoCount,
            thetaCount,
            angleRange,
            weighted,
            suppress,
            window,
            threshold,
            fake,
            fakeCount,
            threads);

        this->ConfigureTopSizer(sizer.release());
    }
};


} // end namespace iris
