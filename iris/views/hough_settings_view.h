#pragma once


#include <wxpex/collapsible.h>
#include <wxpex/labeled_widget.h>
#include <wxpex/slider.h>
#include <wxpex/radio_box.h>
#include <wxpex/view.h>

#include <draw/views/node_settings_view.h>
#include "iris/hough_settings.h"
#include "iris/views/defaults.h"


namespace iris
{


template<typename Value>
class HoughSettingsView: public draw::CollapsibleNodeSettingsView
{
public:
    using LayoutOptions = wxpex::LayoutOptions;

    HoughSettingsView(
        wxWindow *parent,
        HoughControl<Value> controls,
        std::optional<draw::NodeSettingsControl> nodeSettingsControl,
        const LayoutOptions &layoutOptions = LayoutOptions{})
        :
        draw::CollapsibleNodeSettingsView(
            parent,
            "Hough Lines",
            nodeSettingsControl)
    {
        using namespace wxpex;

        auto panel = this->GetPanel();

        auto enable = wxpex::LabeledWidget(
            panel,
            "Enable",
            new wxpex::CheckBox(panel, "", controls.enable));

        auto rhoCount = wxpex::LabeledWidget(
            panel,
            "rho count",
            new wxpex::Field(panel, controls.rhoCount));

        auto thetaCount = wxpex::LabeledWidget(
            panel,
            "theta count",
            new wxpex::Field(panel, controls.thetaCount));

        auto angleRange = LabeledWidget(
            panel,
            "Angle range",
            new ValueSlider(
                panel,
                controls.angleRange,
                controls.angleRange.value));

        auto weighted = wxpex::LabeledWidget(
            panel,
            "Weighted",
            new wxpex::CheckBox(panel, "", controls.weighted));

        auto suppress = wxpex::LabeledWidget(
            panel,
            "Suppress",
            new wxpex::CheckBox(panel, "", controls.suppress));

        auto window = LabeledWidget(
            panel,
            "Window",
            new ValueSlider(
                panel,
                controls.window,
                controls.window.value));

        auto threshold = LabeledWidget(
            panel,
            "Threshold",
            new ValueSlider(
                panel,
                controls.threshold,
                controls.threshold.value));

        auto includeEdges = wxpex::LabeledWidget(
            panel,
            "Include edges",
            new wxpex::CheckBox(panel, "", controls.includeEdges));

        auto edgeTolerance = wxpex::LabeledWidget(
            panel,
            "Edge tolerance",
            new wxpex::Field(panel, controls.edgeTolerance));

        auto threads = wxpex::LabeledWidget(
            panel,
            "Threads",
            new wxpex::Field(panel, controls.threads));

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
            includeEdges,
            edgeTolerance,
            threads);

        this->ConfigureSizer(std::move(sizer));
    }
};


} // end namespace iris
