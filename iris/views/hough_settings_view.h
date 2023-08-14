#pragma once


#include <wxpex/collapsible.h>
#include <wxpex/labeled_widget.h>
#include <wxpex/slider.h>
#include <wxpex/radio_box.h>
#include <wxpex/view.h>

#include "iris/hough_settings.h"
#include "iris/views/defaults.h"


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

        auto pane = this->GetBorderPane(borderStyle);

        auto enable = wxpex::LabeledWidget(
            pane,
            "Enable",
            new wxpex::CheckBox(pane, "", controls.enable));

        auto rhoCount = wxpex::LabeledWidget(
            pane,
            "rho count",
            new wxpex::Field(pane, controls.rhoCount));

        auto thetaCount = wxpex::LabeledWidget(
            pane,
            "theta count",
            new wxpex::Field(pane, controls.thetaCount));

        auto angleRange = LabeledWidget(
            pane,
            "Angle range",
            new ValueSlider(
                pane,
                controls.angleRange,
                controls.angleRange.value));

        auto weighted = wxpex::LabeledWidget(
            pane,
            "Weighted",
            new wxpex::CheckBox(pane, "", controls.weighted));

        auto suppress = wxpex::LabeledWidget(
            pane,
            "Suppress",
            new wxpex::CheckBox(pane, "", controls.suppress));

        auto window = LabeledWidget(
            pane,
            "Window",
            new ValueSlider(
                pane,
                controls.window,
                controls.window.value));

        auto threshold = LabeledWidget(
            pane,
            "Threshold",
            new ValueSlider(
                pane,
                controls.threshold,
                controls.threshold.value));

        auto threads = wxpex::LabeledWidget(
            pane,
            "Threads",
            new wxpex::Field(pane, controls.threads));

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
            threads);

        this->ConfigureBorderPane(borderPixels, std::move(sizer));
    }
};


} // end namespace iris
